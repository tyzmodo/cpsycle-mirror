// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songtrackbar.h"
// audio
#include <songio.h>
// platform
#include "../../detail/portable.h"

#define MIN_TRACKS 4

static void songtrackbar_build(SongTrackBar*);
static void songtrackbar_onselchange(SongTrackBar*, psy_ui_Component* sender,
	int index);
static void songtrackbar_onsongtracknumchanged(SongTrackBar*, Workspace*,
	uintptr_t numsongtracks);
static void songtrackbar_onsongchanged(SongTrackBar*, Workspace*,
	int flag, psy_audio_SongFile* songfile);

void songtrackbar_init(SongTrackBar* self, psy_ui_Component* parent, Workspace*
	workspace)
{	
	psy_ui_Margin margin;
		
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->headerlabel, &self->component);
	psy_ui_label_settext(&self->headerlabel, "trackbar.tracks");
	psy_ui_combobox_init(&self->trackbox, &self->component);	
	psy_ui_combobox_setcharnumber(&self->trackbox, 4);
	songtrackbar_build(self);	
	psy_signal_connect(&self->trackbox.signal_selchanged, self,
		songtrackbar_onselchange);	
	psy_signal_connect(&workspace->player.signal_numsongtrackschanged, self,
		songtrackbar_onsongtracknumchanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		songtrackbar_onsongchanged);	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
}

void songtrackbar_build(SongTrackBar* self)
{
	int track;
	char text[20];

	for (track = MIN_TRACKS; track < 65; ++track) {
		psy_snprintf(text, 20, "%d", track);
		psy_ui_combobox_addtext(&self->trackbox, text);
	}	
	psy_ui_combobox_setcursel(&self->trackbox,
		psy_audio_player_numsongtracks(workspace_player(self->workspace)) - MIN_TRACKS);
}

void songtrackbar_onselchange(SongTrackBar* self, psy_ui_Component* sender,
	int index)
{		
	psy_audio_player_setnumsongtracks(workspace_player(self->workspace), index + MIN_TRACKS);
	if (workspace_song(self->workspace)) {
		psy_audio_patterns_setsongtracks(&workspace_song(self->workspace)->patterns, index + MIN_TRACKS);
	}
}

void songtrackbar_onsongtracknumchanged(SongTrackBar* self,
	Workspace* workspace, uintptr_t numsongtracks)
{
	psy_ui_combobox_setcursel(&self->trackbox, numsongtracks - MIN_TRACKS);
}

void songtrackbar_onsongchanged(SongTrackBar* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{	
	psy_ui_combobox_setcursel(&self->trackbox,
		psy_audio_player_numsongtracks(&workspace->player) - MIN_TRACKS);
}