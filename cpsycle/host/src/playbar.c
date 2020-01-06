// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playbar.h"
#include <exclusivelock.h>
#include <portable.h>

#define TIMERID_PLAYBAR 400

enum {
	PLAY_SONG,
	PLAY_SEL,
	PLAY_BEATS
};

static void playbar_initalign(PlayBar*);
static void onloopclicked(PlayBar*, psy_ui_Component* sender);
static void onrecordnotesclicked(PlayBar*, psy_ui_Component* sender);
static void onplaymodeselchanged(PlayBar*,
	psy_ui_ComboBox* sender, int sel);
static void onnumplaybeatsless(PlayBar*, psy_ui_Button* sender);
static void onnumplaybeatsmore(PlayBar*, psy_ui_Button* sender);
static void onplayclicked(PlayBar*, psy_ui_Component* sender);
static void startplay(PlayBar*);
static void onstopclicked(PlayBar*, psy_ui_Component* sender);
static void ontimer(PlayBar*, psy_ui_Component* sender, int timerid);

void playbar_init(PlayBar* self, psy_ui_Component* parent, Workspace* workspace)
{			
	self->workspace = workspace;
	self->player = &workspace->player;
	ui_component_init(&self->component, parent);
	// ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	// loop
	ui_button_init(&self->loop, &self->component);
	ui_button_settext(&self->loop, "Loop");	
	psy_signal_connect(&self->loop.signal_clicked, self, onloopclicked);	
	// record
	ui_button_init(&self->recordnotes, &self->component);
	ui_button_settext(&self->recordnotes, "Record Notes");	
	psy_signal_connect(&self->recordnotes.signal_clicked, self, onrecordnotesclicked);
	ui_button_init(&self->play, &self->component);
	ui_button_settext(&self->play, workspace_translate(workspace, "play"));
	psy_signal_connect(&self->play.signal_clicked, self, onplayclicked);	
	// playmode
	ui_combobox_init(&self->playmode, &self->component);	
	ui_combobox_addstring(&self->playmode, "Song");
	ui_combobox_addstring(&self->playmode, "Sel");
	ui_combobox_addstring(&self->playmode, "Beats");	
	ui_combobox_setcharnumber(&self->playmode, 6);
	ui_combobox_setcursel(&self->playmode, 0);
	psy_signal_connect(&self->playmode.signal_selchanged, self,
		onplaymodeselchanged);
	// play beat num
	ui_edit_init(&self->loopbeatsedit, &self->component, 0);
	ui_edit_settext(&self->loopbeatsedit, "4.00");
	ui_button_init(&self->loopbeatsless, &self->component);
	ui_button_seticon(&self->loopbeatsless, UI_ICON_LESS);
	psy_signal_connect(&self->loopbeatsless.signal_clicked, self,
		onnumplaybeatsless);
	ui_button_init(&self->loopbeatsmore, &self->component);
	ui_button_seticon(&self->loopbeatsmore, UI_ICON_MORE);
	psy_signal_connect(&self->loopbeatsmore.signal_clicked, self,
		onnumplaybeatsmore);
	// stop
	ui_button_init(&self->stop, &self->component);
	ui_button_settext(&self->stop, workspace_translate(workspace, "stop"));
	psy_signal_connect(&self->stop.signal_clicked, self, onstopclicked);
	psy_signal_connect(&self->component.signal_timer, self, ontimer);
	playbar_initalign(self);
	ui_component_starttimer(&self->component, TIMERID_PLAYBAR, 100);
}

void playbar_initalign(PlayBar* self)
{
	ui_margin margin;

	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(0.5),
		ui_value_makepx(0), ui_value_makepx(0));
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT, &margin));
}

void onplaymodeselchanged(PlayBar* self, psy_ui_ComboBox* sender, int sel)
{
	lock_enter();
	switch (ui_combobox_cursel(&self->playmode)) {
		case PLAY_SONG:
			player_stop(self->player);
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYALL);			
		break;
		case PLAY_SEL:
			player_stop(self->player);
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYSEL);
			startplay(self);
		break;
		case PLAY_BEATS:
			player_stop(self->player);
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYNUMBEATS);
			startplay(self);
		break;
		default:
			player_stop(self->player);
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYALL);
			startplay(self);
		break;
	}
	lock_leave();
}

void onnumplaybeatsless(PlayBar* self, psy_ui_Button* sender)
{
	psy_dsp_beat_t numplaybeats;
	char text[40];
	
	numplaybeats = (psy_dsp_beat_t) atof(ui_edit_text(&self->loopbeatsedit));
	if (numplaybeats > 1) {
		numplaybeats -= 1;
	}
	sequencer_setnumplaybeats(&self->player->sequencer, numplaybeats);
	psy_snprintf(text, 40, "%f", (double) numplaybeats);
	ui_edit_settext(&self->loopbeatsedit, text);
}

void onnumplaybeatsmore(PlayBar* self, psy_ui_Button* sender)
{
	psy_dsp_beat_t numplaybeats;
	char text[40];
	
	numplaybeats = (psy_dsp_beat_t) atof(ui_edit_text(&self->loopbeatsedit));	
	numplaybeats += 1;		
	sequencer_setnumplaybeats(&self->player->sequencer, numplaybeats);
	psy_snprintf(text, 40, "%f", (double) numplaybeats);
	ui_edit_settext(&self->loopbeatsedit, text);
}

void onplayclicked(PlayBar* self, psy_ui_Component* sender)
{
	switch (ui_combobox_cursel(&self->playmode)) {
		case PLAY_SONG:
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYALL);
		break;
		case PLAY_SEL:
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYSEL);
		break;
		case PLAY_BEATS:
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYNUMBEATS);
		break;
		default:
			sequencer_setplaymode(&self->player->sequencer,
				SEQUENCERPLAYMODE_PLAYALL);
		break;
	};	
	if (!player_playing(self->player)) {
		startplay(self);
	}
}

void startplay(PlayBar* self)
{
	psy_audio_Sequence* sequence;
	SequencePosition editposition;
	SequenceEntry* entry;
	psy_dsp_beat_t startposition;
	psy_dsp_beat_t numplaybeats;
	
	ui_button_highlight(&self->play);
	sequence = self->player->sequencer.sequence;
	editposition = self->workspace->sequenceselection.editposition;
	entry = sequenceposition_entry(&editposition);
	if (entry) {
		lock_enter();		
		player_stop(self->player);
		startposition = entry->offset;
		if (sequencer_playmode(&self->player->sequencer)
			== SEQUENCERPLAYMODE_PLAYNUMBEATS) {
			PatternEditPosition editposition;

			editposition = workspace_patterneditposition(self->workspace);			
			startposition += (psy_dsp_beat_t) editposition.offset;
			numplaybeats = (psy_dsp_beat_t) atof(ui_edit_text(&self->loopbeatsedit));
			self->player->sequencer.numplaybeats = numplaybeats;
		}
		player_setposition(self->player, startposition);
		player_start(self->player);
		lock_leave();
	}
}

void onstopclicked(PlayBar* self, psy_ui_Component* sender)
{
	player_stop(self->player);
}

void onloopclicked(PlayBar* self, psy_ui_Component* sender)
{
	if (sequencer_looping(&self->player->sequencer)) {
		sequencer_stoploop(&self->player->sequencer);
		ui_button_disablehighlight(&self->loop);
	} else {
		sequencer_loop(&self->player->sequencer);
		ui_button_highlight(&self->loop);
	}
}

void onrecordnotesclicked(PlayBar* self, psy_ui_Component* sender)
{	
	if (player_recordingnotes(self->player)) {
		player_stoprecordingnotes(self->player);
		ui_button_disablehighlight(&self->recordnotes);
	} else {
		player_startrecordingnotes(self->player);
		ui_button_highlight(&self->recordnotes);
	}
}

void ontimer(PlayBar* self, psy_ui_Component* sender, int timerid)
{
	if (player_playing(self->player)) {
		ui_button_highlight(&self->play);
	} else {
		ui_button_disablehighlight(&self->play);	
	}
	if (sequencer_looping(&self->player->sequencer)) {
		ui_button_highlight(&self->loop);
	} else {
		ui_button_disablehighlight(&self->loop);
	}
}
