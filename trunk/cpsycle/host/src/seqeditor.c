// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "seqeditor.h"
// host
#include "cmdsgeneral.h"
#include "sequencetrackbox.h"
#include "pianoroll.h"
// audio
#include <exclusivelock.h>
#include <patterns.h>
#include <songio.h>
// ui
#include <uiapp.h>
#include "resources/resource.h"
// std
#include <math.h>
#include <string.h>
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

#define DEFAULT_PXPERBEAT 5

void seqeditortrackstate_init(SeqEditorTrackState* self)
{
	self->pxperbeat = DEFAULT_PXPERBEAT;
	self->defaultlineheight = psy_ui_value_makeeh(1.5);
	self->lineheight = self->defaultlineheight;
	self->linemargin = psy_ui_value_makeeh(0.2);
}

// SeqEditorRuler
static void seqeditorruler_ondraw(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_drawruler(SeqEditorRuler*, psy_ui_Graphics*);
static void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler*, psy_audio_SequenceSelection* sender);
static void seqeditorruler_onpreferredsize(SeqEditorRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
// vtable
static psy_ui_ComponentVtable seqeditorruler_vtable;
static bool seqeditorruler_vtable_initialized = FALSE;

static void seqeditorruler_vtable_init(SeqEditorRuler* self)
{
	if (!seqeditorruler_vtable_initialized) {
		seqeditorruler_vtable = *(self->component.vtable);
		seqeditorruler_vtable.ondraw = (psy_ui_fp_component_ondraw)
			seqeditorruler_ondraw;
		seqeditorruler_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			seqeditorruler_onpreferredsize;
		seqeditorruler_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditorruler_init(SeqEditorRuler* self, psy_ui_Component* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	seqeditorruler_vtable_init(self);
	self->component.vtable = &seqeditorruler_vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->trackstate = trackstate;
	self->workspace = workspace;
	self->rulerbaselinecolour = psy_ui_colour_make(0x00555555);
	self->rulermarkcolour = psy_ui_colour_make(0x00666666);
	psy_signal_connect(&workspace->newsequenceselection.signal_changed, self,
		seqeditorruler_onsequenceselectionchanged);
}

void seqeditorruler_ondraw(SeqEditorRuler* self, psy_ui_Graphics* g)
{	
	seqeditorruler_drawruler(self, g);
}

void seqeditorruler_drawruler(SeqEditorRuler* self, psy_ui_Graphics* g)
{
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	intptr_t baseline;
	intptr_t linewidth;
	psy_dsp_big_beat_t duration;
	psy_dsp_big_beat_t clipstart;
	psy_dsp_big_beat_t clipend;
	psy_dsp_big_beat_t currbeat;
	
	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);
	baseline = size.height - 1;	
	duration = (size.width + psy_ui_component_scrollleftpx(&self->component)) /
		(psy_dsp_big_beat_t)self->trackstate->pxperbeat;
	//psy_audio_sequence_duration(&workspace_song(self->workspace)->sequence);
	linewidth = (intptr_t)(duration * self->trackstate->pxperbeat);
	psy_ui_setcolour(g, self->rulerbaselinecolour);
	psy_ui_drawline(g, 0, baseline, linewidth, baseline);
	clipstart = 0;
	clipend = duration;
	for (currbeat = clipstart; currbeat <= clipend; currbeat += 16.0) {
		intptr_t cpx;
		char txt[40];

		cpx = (intptr_t)(currbeat * self->trackstate->pxperbeat);
		psy_ui_drawline(g, cpx, baseline, cpx, baseline - tm.tmHeight / 3);
		psy_snprintf(txt, 40, "%d", (int)(currbeat));
		psy_ui_textout(g, (int)cpx + 3, baseline - tm.tmHeight, txt, strlen(txt));
	}
}

void seqeditorruler_onsequenceselectionchanged(SeqEditorRuler* self, psy_audio_SequenceSelection* sender)
{
	psy_ui_component_invalidate(&self->component);
}

void seqeditorruler_onpreferredsize(SeqEditorRuler* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (workspace_song(self->workspace)) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		intptr_t linewidth;
		psy_dsp_big_beat_t duration;

		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		duration = psy_audio_sequence_duration(&workspace_song(self->workspace)->sequence);
		linewidth = (intptr_t)(duration * self->trackstate->pxperbeat);
		rv->width = psy_ui_value_makepx((double)linewidth);
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
	rv->height = psy_ui_value_makeeh(1.0);
}

// SeqEditorTrack
// prototypes
static void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g, intptr_t x, intptr_t y);
static void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static bool seqeditortrack_onmousedown_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static bool seqeditortrack_onmousemove_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static bool seqeditortrack_onmouseup_virtual(SeqEditorTrack*,
	psy_ui_MouseEvent*);
static void seqeditortracks_invalidatebitmap(SeqEditorTracks*);
static void seqeditortrack_outputstatusposition(SeqEditorTrack*, intptr_t x);
// vtable
static SeqEditorTrackVTable seqeditortrack_vtable;
static bool seqeditortrack_vtable_initialized = FALSE;

static void seqeditortrack_vtable_init(void)
{
	if (!seqeditortrack_vtable_initialized) {
		seqeditortrack_vtable.ondraw = (seqeditortrack_fp_ondraw)
			seqeditortrack_ondraw_virtual;
		seqeditortrack_vtable.onpreferredsize = (seqeditortrack_fp_onpreferredsize)
			seqeditortrack_onpreferredsize_virtual;
		seqeditortrack_vtable.onmousedown = (seqeditortrack_fp_onmousedown)
			seqeditortrack_onmousedown_virtual;
		seqeditortrack_vtable.onmousemove = (seqeditortrack_fp_onmousemove)
			seqeditortrack_onmousemove_virtual;
		seqeditortrack_vtable.onmouseup = (seqeditortrack_fp_onmouseup)
			seqeditortrack_onmouseup_virtual;
		seqeditortrack_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortrack_init(SeqEditorTrack* self, SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	seqeditortrack_vtable_init();
	self->vtable = &seqeditortrack_vtable;
	self->trackstate = trackstate;
	self->workspace = workspace;
	self->parent = parent;
	self->currtrack = NULL;
	self->trackindex = 0;
	self->drag_sequenceitem_node = NULL;
	self->dragstarting = FALSE;
	self->bitmapvalid = FALSE;	
	psy_ui_bitmap_init(&self->bitmap);
}

void seqeditortrack_dispose(SeqEditorTrack* self)
{
	psy_ui_bitmap_dispose(&self->bitmap);
}

SeqEditorTrack* seqeditortrack_alloc(void)
{
	return (SeqEditorTrack*)malloc(sizeof(SeqEditorTrack));
}

SeqEditorTrack* seqeditortrack_allocinit(SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	SeqEditorTrack* rv;

	rv = seqeditortrack_alloc();
	if (rv) {
		seqeditortrack_init(rv, parent, trackstate, workspace);
	}
	return rv;
}

void seqeditortrack_updatetrack(SeqEditorTrack* self, 
	psy_audio_SequenceTrackNode* tracknode,
	psy_audio_SequenceTrack* track,
	uintptr_t trackindex)
{
	self->currtrack = track;
	self->currtracknode = tracknode;
	self->trackindex = trackindex;
}

void seqeditortrack_ondraw_virtual(SeqEditorTrack* self, psy_ui_Graphics* g,
	intptr_t x, intptr_t y)
{
	psy_List* p;
	intptr_t c;	
	psy_ui_TextMetric tm;
	intptr_t lineheight;
	psy_ui_Rectangle bg;

	if (!workspace_song(self->workspace)) {
		return;
	}
	if (!self->currtrack) {		
		return;
	}	
	tm = psy_ui_component_textmetric(&self->parent->component);
	lineheight = (intptr_t)psy_ui_value_px(&self->trackstate->lineheight, &tm);
	bg = psy_ui_rectangle_make(0, y,		
		psy_ui_component_intsize(&self->parent->component).width, lineheight);
	psy_ui_drawsolidrectangle(g, bg,
		psy_ui_component_backgroundcolour(&self->parent->component));	
	if (!self->bitmapvalid && self->currtrack->entries) {
		psy_ui_Graphics gr;			
		psy_ui_IntSize size;		
		psy_ui_Size preferredsize;		

		psy_ui_bitmap_dispose(&self->bitmap);
		seqeditortrack_onpreferredsize_virtual(self, NULL, &preferredsize);
		bg = psy_ui_rectangle_make(0, 0,
			(intptr_t)psy_ui_value_px(&preferredsize.width, &tm),
			lineheight);
		size = psy_ui_intsize_make(bg.right, lineheight);
		psy_ui_bitmap_init_size(&self->bitmap, size);
		psy_ui_graphics_init_bitmap(&gr, &self->bitmap);
		psy_ui_setfont(&gr, psy_ui_component_font(&self->parent->component));
		psy_ui_setbackgroundmode(&gr, psy_ui_TRANSPARENT);
		psy_ui_settextcolour(&gr, psy_ui_component_colour(&self->parent->component));				
		psy_ui_drawsolidrectangle(&gr, bg,
			psy_ui_component_backgroundcolour(&self->parent->component));
		for (p = self->currtrack->entries, c = 0; p != NULL;
			psy_list_next(&p), ++c) {
			psy_audio_SequenceEntry* sequenceentry;
			psy_audio_Pattern* pattern;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
				psy_audio_sequenceentry_patternslot(sequenceentry));
			if (pattern) {				
				intptr_t patternwidth;
				bool selected;
				char text[256];				
				psy_ui_Rectangle r;
				intptr_t centery;
				
				centery = (size.height - tm.tmHeight) / 2;
				patternwidth = (intptr_t)(psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat);				
				r = psy_ui_rectangle_make((intptr_t)(sequenceentry->offset * self->trackstate->pxperbeat),
					0, patternwidth, lineheight);
				selected =
					self->workspace->newsequenceselection.editposition.track == self->trackindex &&
					self->workspace->newsequenceselection.editposition.order == c;
				if (selected) {
					psy_ui_drawsolidrectangle(&gr, r, psy_ui_colour_make(0x00514536));
					psy_ui_setcolour(&gr, psy_ui_colour_make(0x00555555));
					psy_ui_drawrectangle(&gr, r);
				} else {
					psy_ui_drawsolidrectangle(&gr, r, psy_ui_colour_make(0x00333333));
					psy_ui_setcolour(&gr, psy_ui_colour_make(0x00444444));
					psy_ui_drawrectangle(&gr, r);
				}
				r = psy_ui_rectangle_make((intptr_t)(sequenceentry->offset * self->trackstate->pxperbeat),
					centery, patternwidth, tm.tmHeight);
				if (generalconfig_showingpatternnames(psycleconfig_general(
					workspace_conf(self->workspace)))) {
					psy_audio_Pattern* pattern;

					pattern = psy_audio_sequenceentry_pattern(sequenceentry,
						&workspace_song(self->workspace)->patterns);
					if (pattern) {
						psy_snprintf(text, 20, "%02X: %s", c,
							psy_audio_pattern_name(pattern));
					} else {
						psy_snprintf(text, 20, "%02X:%02X(ERR)", c,
							(int)psy_audio_sequenceentry_patternslot(sequenceentry));
					}
					psy_ui_textoutrectangle(&gr, r.left + 2, r.top, psy_ui_ETO_CLIPPED, r,
						text, strlen(text));
				} else {
					psy_snprintf(text, 256, "%02X",
						(int)psy_audio_sequenceentry_patternslot(sequenceentry));
					psy_ui_textoutrectangle(&gr, r.left + 2, r.top, psy_ui_ETO_CLIPPED, r,
						text, strlen(text));
				}
				if (self->parent->drawpatternevents) {
					PianoGridDraw griddraw;
					PianoGridState gridstate;
					KeyboardState keyboardstate;
					psy_audio_PatternSelection selection;
					double lh;							
					
					keyboardstate_init(&keyboardstate, self->parent->skin);
					lh = psy_ui_value_px(&self->trackstate->lineheight, &tm);
					lh = lh / tm.tmHeight;
					keyboardstate.defaultkeyheight = psy_ui_value_makeeh(
						 lh / keyboardstate_numkeys(&keyboardstate));
					keyboardstate.keyheight = keyboardstate.defaultkeyheight;
					pianogridstate_init(&gridstate, self->parent->skin);
					pianogridstate_setpattern(&gridstate, pattern);					
					pianogridstate_setlpb(&gridstate, psy_audio_player_lpb(
						workspace_player(self->workspace)));
					gridstate.pxperbeat = self->trackstate->pxperbeat;
					psy_audio_patternselection_init(&selection);					
					pianogriddraw_init(&griddraw,						
						&keyboardstate, &gridstate,
						psy_ui_value_zero(), psy_ui_value_zero(),
						0.0,
						NULL,
						PIANOROLL_TRACK_DISPLAY_ALL,
						FALSE, FALSE,
						selection,							
						size, tm, self->workspace);
					pianogriddraw_preventclip(&griddraw);
					pianogriddraw_preventgrid(&griddraw);
					pianogriddraw_preventcursor(&griddraw);
					pianogriddraw_preventplaybar(&griddraw);					
					psy_ui_setorigin(&gr, -r.left, 0);
					pianogriddraw_ondraw(&griddraw, &gr);
					psy_ui_setorigin(&gr, 0, 0);					
				}
			}			
		}	
		psy_ui_graphics_dispose(&gr);
		self->bitmapvalid = TRUE;
	}
	if (self->bitmapvalid) {
		psy_ui_Size bitmapsize;

		bitmapsize = psy_ui_bitmap_size(&self->bitmap);
		psy_ui_drawbitmap(g, &self->bitmap, 0, y,
			(intptr_t)psy_ui_value_px(&bitmapsize.width, &tm),
			(intptr_t)psy_ui_value_px(&bitmapsize.height, &tm),
			0, 0);
	}
	if (self->drag_sequenceitem_node && !self->dragstarting) {
		psy_ui_Rectangle r;
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;
		psy_audio_SequenceEntry* sequenceentry;
		intptr_t cpx;

		sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(
			self->drag_sequenceitem_node);
		tm = psy_ui_component_textmetric(&self->parent->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->parent->component), &tm);
		cpx = (intptr_t)(self->itemdragposition * self->trackstate->pxperbeat);
		r = psy_ui_rectangle_make(cpx, y, 2, lineheight);
		psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00CACACA));		
	}
}

void seqeditortrack_onpreferredsize_virtual(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_dsp_big_beat_t trackduration;

	trackduration = 0.0;
	if (self->currtrack) {
		trackduration = psy_audio_sequencetrack_duration(self->currtrack,
			&workspace_song(self->workspace)->patterns);
	}
	rv->width = psy_ui_value_makepx(self->trackstate->pxperbeat *
		trackduration);
	rv->height = self->trackstate->lineheight;
}

bool seqeditortrack_onmousedown_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	psy_List* sequenceitem_node;
	uintptr_t selected;
	
	psy_ui_component_capture(&self->parent->component);
	self->drag_sequenceitem_node = NULL;
	self->dragstartpx = ev->x;
	for (sequenceitem_node = self->currtrack->entries, selected = 0;
		sequenceitem_node != NULL;
		psy_list_next(&sequenceitem_node), ++selected) {
		psy_audio_SequenceEntry* entry;
		psy_audio_Pattern* pattern;

		entry = (psy_audio_SequenceEntry*)psy_list_entry(sequenceitem_node);
		pattern = psy_audio_patterns_at(&workspace_song(self->workspace)->patterns,
			entry->patternslot);
		if (pattern) {
			psy_ui_Rectangle r;
			intptr_t patternwidth;
			psy_ui_TextMetric tm;
			intptr_t lineheight;

			tm = psy_ui_component_textmetric(&self->parent->component);
			lineheight = (intptr_t)psy_ui_value_px(&self->trackstate->lineheight, &tm);
			patternwidth = (intptr_t)(psy_audio_pattern_length(pattern) * self->trackstate->pxperbeat);
			r = psy_ui_rectangle_make((intptr_t)(entry->offset * self->trackstate->pxperbeat), 0,
				patternwidth, lineheight);
			if (psy_ui_rectangle_intersect(&r, ev->x, ev->y)) {
				if (self->currtrack) {
					uintptr_t trackindex;

					trackindex = psy_list_entry_index(workspace_song(self->workspace)->sequence.tracks,
						self->currtrack);
					if (trackindex != UINTPTR_MAX) {
						workspace_setsequenceeditposition(
							self->workspace,
							psy_audio_orderindex_make(trackindex, selected));						
					}
				}
				self->drag_sequenceitem_node = sequenceitem_node;
				self->parent->capture = self;
				self->itemdragposition = entry->offset;
				self->dragstarting = TRUE;
				seqeditortrack_outputstatusposition(self, ev->x);				
			}
		}		
	}
	if (ev->button == 2) {
		self->drag_sequenceitem_node = NULL;
	}
	return TRUE;
}

bool seqeditortrack_onmousemove_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{		
	if (self->drag_sequenceitem_node) {
		psy_audio_SequenceEntry* sequenceentry;	
		psy_dsp_big_beat_t dragposition;

		sequenceentry = (psy_audio_SequenceEntry*)self->drag_sequenceitem_node->entry;
		dragposition = seqeditortrackstate_pxtobeat(self->trackstate, ev->x);
		if (self->dragstarting && ((abs(self->dragstartpx - ev->x) < 2) ||
			dragposition - (sequenceentry->offset - sequenceentry->repositionoffset) == 0.0)) {
			// just select the pattern
			return TRUE;
		}
		if (self->parent->dragmode == SEQEDITORDRAG_MOVE) {
			if (dragposition - (sequenceentry->offset - sequenceentry->repositionoffset) >= 0) {
				self->itemdragposition = dragposition;
				self->itemdragposition =
					(intptr_t)(dragposition * psy_audio_player_lpb(workspace_player(self->workspace))) /
					(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace));
			} else {
				self->itemdragposition = sequenceentry->offset - sequenceentry->repositionoffset;
			}
		} else {
			self->itemdragposition = dragposition;
			if (self->itemdragposition < 0.0) {
				self->itemdragposition = 0.0;
			}
			self->itemdragposition =
				(intptr_t)(dragposition * psy_audio_player_lpb(workspace_player(self->workspace))) /
				(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace));
		}
		self->dragstarting = FALSE;
		psy_ui_component_invalidate(&self->parent->component);
	}
	seqeditortrack_outputstatusposition(self, ev->x);	
	return TRUE;
}

bool seqeditortrack_onmouseup_virtual(SeqEditorTrack* self,
	psy_ui_MouseEvent* ev)
{
	bool rv;

	rv = TRUE;
	psy_ui_component_releasecapture(&self->parent->component);
	self->parent->capture = NULL;
	if (ev->button == 1) {
		self->parent->capture = NULL;
		if (self->drag_sequenceitem_node && !self->dragstarting) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)
				self->drag_sequenceitem_node->entry;
			if (self->parent->dragmode == SEQEDITORDRAG_MOVE) {
				sequenceentry->repositionoffset = self->itemdragposition -
					(sequenceentry->offset - sequenceentry->repositionoffset);
				sequenceentry->offset = self->itemdragposition;
				psy_audio_sequence_reposition_track(
					&workspace_song(self->workspace)->sequence, self->currtrack);
				psy_signal_emit(
					&workspace_song(self->workspace)->sequence.sequencechanged,
					self, 0);
			} else {				
				self->drag_sequenceitem_node = NULL;
				self->dragstarting = FALSE;				
				psy_audio_sequence_reorder(
					&self->parent->workspace->song->sequence,
					psy_audio_orderindex_make(
						self->trackindex,
						sequenceentry->row),
					self->itemdragposition);				
				return FALSE;
			}			
		}
		self->drag_sequenceitem_node = NULL;
		self->dragstarting = FALSE;
	} else if (ev->button == 2) {		
		psy_audio_player_sendcmd(workspace_player(self->workspace),
			"general", psy_eventdrivercmd_makeid(CMD_IMM_INFOPATTERN));	
	}	
	return rv;
}

void seqeditortrack_outputstatusposition(SeqEditorTrack* self, intptr_t x)
{
	psy_dsp_big_beat_t position;
	char text[256];

	position = seqeditortrackstate_pxtobeat(self->trackstate, x);
	// quantize to lpb raster
	position =
		(intptr_t)(position * psy_audio_player_lpb(workspace_player(self->workspace))) /
		(psy_dsp_big_beat_t)psy_audio_player_lpb(workspace_player(self->workspace));
	psy_snprintf(text, 256, "Sequence Position %.3fb", (float)position);
	workspace_outputstatus(self->workspace, text);
}


// SeqEditorTrackHeader
// prototypes
static void seqeditortrackheader_ondraw(SeqEditorTrackHeader* self, psy_ui_Graphics* g, intptr_t x, intptr_t y);
static void seqeditortrackheader_onpreferredsize(SeqEditorTrackHeader*,
	const psy_ui_Size* limit, psy_ui_Size* rv);
static bool seqeditortrackheader_onmousedown(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
static bool seqeditortrackheader_onmousemove(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
static bool seqeditortrackheader_onmouseup(SeqEditorTrackHeader*,
	psy_ui_MouseEvent*);
// vtable
static SeqEditorTrackVTable seqeditortrackheader_vtable;
static bool seqeditortrackheader_vtable_initialized = FALSE;

static void seqeditortrackheader_vtable_init(SeqEditorTrackHeader* self)
{
	if (!seqeditortrackheader_vtable_initialized) {
		seqeditortrackheader_vtable = *(self->base.vtable);
		seqeditortrackheader_vtable.ondraw = (seqeditortrack_fp_ondraw)
			seqeditortrackheader_ondraw;
		seqeditortrackheader_vtable.onpreferredsize = (seqeditortrack_fp_onpreferredsize)
			seqeditortrackheader_onpreferredsize;
		seqeditortrackheader_vtable.onmousedown = (seqeditortrack_fp_onmousedown)
			seqeditortrackheader_onmousedown;
		seqeditortrackheader_vtable.onmousemove = (seqeditortrack_fp_onmousemove)
			seqeditortrackheader_onmousemove;
		seqeditortrackheader_vtable.onmouseup = (seqeditortrack_fp_onmouseup)
			seqeditortrackheader_onmouseup;
		seqeditortrackheader_vtable_initialized = TRUE;
	}
}
void seqeditortrackheader_init(SeqEditorTrackHeader* self,
	struct SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	seqeditortrack_init(&self->base, parent, trackstate, workspace);
	seqeditortrackheader_vtable_init(self);
	self->base.vtable = &seqeditortrackheader_vtable;
}

SeqEditorTrackHeader* seqeditortrackheader_alloc(void)
{
	return (SeqEditorTrackHeader*)malloc(sizeof(SeqEditorTrackHeader));
}

SeqEditorTrackHeader* seqeditortrackheader_allocinit(SeqEditorTracks* parent,
	SeqEditorTrackState* trackstate, Workspace* workspace)
{
	SeqEditorTrackHeader* rv;

	rv = seqeditortrackheader_alloc();
	if (rv) {
		seqeditortrackheader_init(rv, parent, trackstate, workspace);
	}
	return rv;
}

void seqeditortrackheader_ondraw(SeqEditorTrackHeader* self,
	psy_ui_Graphics* g, intptr_t x, intptr_t y)
{	
	SequenceTrackBox trackbox;
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;	
		
	tm = psy_ui_component_textmetric(&self->base.parent->component);
	size = psy_ui_component_intsize(&self->base.parent->component);	
	sequencetrackbox_init(&trackbox,
		psy_ui_rectangle_make(
			x, y, size.width,
			(intptr_t)psy_ui_value_px(&self->base.trackstate->lineheight, &tm)),
		tm, self->base.currtrack,
		(self->base.workspace->song) ? &self->base.workspace->song->sequence : NULL,
		self->base.trackindex,
		self->base.workspace->newsequenceselection.editposition.track == self->base.trackindex);
	trackbox.showname = TRUE;
	sequencetrackbox_draw(&trackbox, g);
}

void seqeditortrackheader_onpreferredsize(SeqEditorTrackHeader* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	assert(rv);

	*rv = psy_ui_size_make(psy_ui_value_makeew(40.0),
		self->base.trackstate->lineheight);
}

bool seqeditortrackheader_onmousedown(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	if (self->base.currtrack) {		
		SequenceTrackBox trackbox;
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;
		psy_audio_Sequence* sequence;

		sequence = &self->base.workspace->song->sequence;
		tm = psy_ui_component_textmetric(&self->base.parent->component);
		size = psy_ui_component_intsize(&self->base.parent->component);		
		sequencetrackbox_init(&trackbox,
			psy_ui_rectangle_make(
				0, 0, size.width,
				(intptr_t)psy_ui_value_px(&self->base.trackstate->lineheight, &tm)),
			tm,
			self->base.currtrack,
			(self->base.workspace->song) ? &self->base.workspace->song->sequence : NULL,
			self->base.trackindex,
			self->base.workspace->newsequenceselection.editposition.track == self->base.trackindex);
		switch (sequencetrackbox_hittest(&trackbox, ev->x, 0)) {
		case SEQUENCETRACKBOXEVENT_MUTE:
			if (self->base.currtrack) {
				if (psy_audio_sequence_istrackmuted(sequence, self->base.trackindex)) {
					psy_audio_sequence_unmutetrack(sequence, self->base.trackindex);
				} else {
					psy_audio_sequence_mutetrack(sequence, self->base.trackindex);
				}				
			}
			break;
		case SEQUENCETRACKBOXEVENT_SOLO:
			if (self->base.currtrack) {
				if (psy_audio_sequence_istracksoloed(sequence, self->base.trackindex)) {
					psy_audio_sequence_deactivatesolotrack(sequence);
				} else {
					psy_audio_sequence_activatesolotrack(sequence, self->base.trackindex);
				}
			}
			break;
		case SEQUENCETRACKBOXEVENT_DEL: {
			psy_audio_SequencePosition position;
			position = psy_audio_sequence_at(&self->base.workspace->song->sequence,
				self->base.trackindex, 0);
			psy_audio_exclusivelock_enter();
			self->base.currtrack = NULL;
			psy_audio_sequence_removetrack(&self->base.workspace->song->sequence,
				position.tracknode);
			psy_audio_exclusivelock_leave();
			return FALSE;
			break; }
		// fallthrough
		case SEQUENCETRACKBOXEVENT_SELECT:
		default: {
			uintptr_t track;

			track = self->base.trackindex;
			if (track >= psy_audio_sequence_sizetracks(
					&self->base.workspace->song->sequence)) {
				if (psy_audio_sequence_sizetracks(&self->base.workspace->song->sequence) > 0) {
					track = psy_audio_sequence_sizetracks(&self->base.workspace->song->sequence) - 1;
				} else {
					track = 0;
				}
			}
			workspace_setsequenceeditposition(self->base.workspace,
				psy_audio_orderindex_make(track, 0));
			break; }
		}
		psy_ui_component_invalidate(&self->base.parent->component);
	} else {
		psy_audio_exclusivelock_enter();
		psy_audio_sequence_appendtrack(&self->base.workspace->song->sequence,
			psy_audio_sequencetrack_allocinit());
		psy_audio_exclusivelock_leave();		
		return FALSE;
	}
	return TRUE;
}

bool seqeditortrackheader_onmousemove(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	return TRUE;
}

bool seqeditortrackheader_onmouseup(SeqEditorTrackHeader* self,
	psy_ui_MouseEvent* ev)
{
	return TRUE;
}

// SeqEditorTracks
// prototypes
static void seqeditortracks_ondestroy(SeqEditorTracks*, psy_ui_Component*
	sender);
static void seqeditortracks_ondraw(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_drawplayline(SeqEditorTracks*, psy_ui_Graphics*);
static void seqeditortracks_onpreferredsize(SeqEditorTracks*, psy_ui_Size*
	limit, psy_ui_Size* rv);
static void seqeditortracks_onmousedown(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmousemove(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static void seqeditortracks_onmouseup(SeqEditorTracks*,
	psy_ui_MouseEvent*);
static bool seqeditortracks_notifymouse(SeqEditorTracks*, psy_ui_MouseEvent*,
	bool (*fp_notify)(SeqEditorTrack*, psy_ui_MouseEvent*));
static void seqeditortracks_build(SeqEditorTracks*);
static psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks*);
static void seqeditortracks_onsequenceselectionchanged(SeqEditorTracks*, psy_audio_SequenceSelection*);
static void seqeditortracks_ontimer(SeqEditorTracks*, uintptr_t timerid);
// vtable
static psy_ui_ComponentVtable seqeditortracks_vtable;
static bool seqeditortracks_vtable_initialized = FALSE;

static void seqeditortracks_vtable_init(SeqEditorTracks* self)
{
	if (!seqeditortracks_vtable_initialized) {
		seqeditortracks_vtable = *(self->component.vtable);
		seqeditortracks_vtable.ondraw = (psy_ui_fp_component_ondraw)
			seqeditortracks_ondraw;
		seqeditortracks_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			seqeditortracks_onpreferredsize;
		seqeditortracks_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			seqeditortracks_onmousedown;
		seqeditortracks_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			seqeditortracks_onmousemove;
		seqeditortracks_vtable.onmouseup = (psy_ui_fp_component_onmouseup)
			seqeditortracks_onmouseup;
		seqeditortracks_vtable.ontimer = (psy_ui_fp_component_ontimer)
			seqeditortracks_ontimer;
		seqeditortracks_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditortracks_init(SeqEditorTracks* self, psy_ui_Component* parent,
	SeqEditorTrackState* trackstate, int mode, Workspace* workspace)
{
	self->workspace = workspace;
	self->trackstate = trackstate;
	self->mode = mode;
	self->drawpatternevents = TRUE;	
	self->dragmode = SEQEDITORDRAG_MOVE;
	psy_ui_component_init(&self->component, parent);	
	seqeditortracks_vtable_init(self);
	self->component.vtable = &seqeditortracks_vtable;
	self->tracks = NULL;
	self->lastplaylinepx = -1;
	self->capture = NULL;
	psy_ui_component_doublebuffer(&self->component);	
	psy_ui_component_setwheelscroll(&self->component, 1);	
	psy_ui_component_setoverflow(&self->component, psy_ui_OVERFLOW_SCROLL);	
	psy_signal_connect(&self->component.signal_destroy, self,
		seqeditortracks_ondestroy);
	seqeditortracks_build(self);
	psy_signal_connect(&workspace->newsequenceselection.signal_changed, self,
		seqeditortracks_onsequenceselectionchanged);
	if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
		psy_ui_component_starttimer(&self->component, 0, 50);
	}
}

void seqeditortracks_ondestroy(SeqEditorTracks* self, psy_ui_Component* sender)
{
	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
}

void seqeditortracks_build(SeqEditorTracks* self)
{
	psy_audio_Sequence* sequence;

	psy_list_deallocate(&self->tracks, (psy_fp_disposefunc)
		seqeditortrack_dispose);
	sequence = seqeditortracks_sequence(self);
	if (sequence) {
		psy_audio_SequenceTrackNode* t;
		uintptr_t c;
		
		for (t = sequence->tracks, c = 0; t != NULL;
				psy_list_next(&t), ++c) {
			SeqEditorTrack* seqedittrack;

			if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
				seqedittrack = seqeditortrack_allocinit(self, self->trackstate,
					self->workspace);
			} else if (self->mode == SEQEDITOR_TRACKMODE_HEADER) {
				seqedittrack = seqeditortrackheader_base(
					seqeditortrackheader_allocinit(self, self->trackstate,
						self->workspace));
			} else {
				seqedittrack = NULL;
			}
			if (seqedittrack) {
				psy_list_append(&self->tracks, seqedittrack);
				seqeditortrack_updatetrack(seqedittrack,
					t, (psy_audio_SequenceTrack*)t->entry, c);
			}
		}
		// add an empty track for the add track button			
		SeqEditorTrack* seqedittrack;

		seqedittrack = seqeditortrackheader_base(
			seqeditortrackheader_allocinit(self, self->trackstate,
				self->workspace));
		if (seqedittrack) {
			psy_list_append(&self->tracks, seqedittrack);
			seqeditortrack_updatetrack(seqedittrack,
				NULL, NULL, c + 1);
		}		
	}
	self->capture = NULL;
}

void seqeditortracks_ondraw(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	psy_audio_Sequence* sequence = seqeditortracks_sequence(self);	
	if (sequence) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		double cpx, cpy;
		uintptr_t c;
		psy_ui_TextMetric tm;
		double linemargin;
		double lineheight;

		tm = psy_ui_component_textmetric(&self->component);
		psy_ui_settextcolour(g, psy_ui_colour_make(0x00FFFFFF));
		cpx = 0;		
		cpy = 0;
		linemargin = psy_ui_value_px(&self->trackstate->linemargin, &tm);		
		lineheight = psy_ui_value_px(&self->trackstate->lineheight, &tm);
		for (seqeditnode = self->tracks, seqnode = sequence->tracks, c = 0;
				seqeditnode != NULL;
				seqnode = (seqnode) ? seqnode->next : seqnode,
				psy_list_next(&seqeditnode),
				++c) {
			if (cpy > g->clip.top - lineheight - linemargin) {
				SeqEditorTrack* seqedittrack;
				psy_audio_SequenceTrack* seqtrack;

				seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
				if (seqnode) {
					seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
				} else {
					seqtrack = NULL;
				}
				seqeditortrack_updatetrack(seqedittrack, seqnode, seqtrack, c);
				seqeditortrack_ondraw(seqedittrack, g, (intptr_t)cpx, (intptr_t)cpy);				
			}
			cpy += lineheight + linemargin;
			if (cpy > g->clip.bottom) {
				break;
			}
		}		
		if (self->mode == SEQEDITOR_TRACKMODE_ENTRY) {
			seqeditortracks_drawplayline(self, g);
		}
	}
}

void seqeditortracks_drawplayline(SeqEditorTracks* self, psy_ui_Graphics* g)
{
	if (psy_audio_player_playing(workspace_player(self->workspace))) {		
		psy_ui_Rectangle position;		
		
		position = psy_ui_component_scrolledposition(&self->component);		
		psy_ui_setcolour(g, self->skin->playbar);
		psy_ui_drawline(g, self->lastplaylinepx, position.top, self->lastplaylinepx,
			position.bottom);		
	}
}

bool seqeditortracks_playlinechanged(SeqEditorTracks* self)
{	
	intptr_t cpx;
		
	cpx = (intptr_t)(psy_audio_player_position(workspace_player(self->workspace)) *
		self->trackstate->pxperbeat);
	return cpx != self->lastplaylinepx;
}

psy_audio_Sequence* seqeditortracks_sequence(SeqEditorTracks* self)
{
	if (workspace_song(self->workspace)) {
		return &workspace_song(self->workspace)->sequence;
	}
	return NULL;
}

void seqeditortracks_onpreferredsize(SeqEditorTracks* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_Point maxsize;

	maxsize.x = psy_ui_value_makeew(0.0);
	maxsize.y = psy_ui_value_makeeh(0.0);
	if (workspace_song(self->workspace)) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		psy_ui_TextMetric tm;
		uintptr_t c;		
	
		tm = psy_ui_component_textmetric(&self->component);		
		for (seqeditnode = self->tracks, c = 0,
				seqnode = workspace_song(self->workspace)->sequence.tracks;
				seqeditnode != NULL;
				psy_list_next(&seqeditnode),
				seqnode = (seqnode)
					? seqnode->next
					: NULL,
				++c) {
			SeqEditorTrack* seqedittrack;
			psy_audio_SequenceTrack* seqtrack;
			psy_ui_Size limit;
			psy_ui_Size preferredtracksize;

			limit = psy_ui_component_size(&self->component);
			seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
			if (seqnode) {
				seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
			} else {
				seqtrack = NULL;
			}
			seqeditortrack_updatetrack(seqedittrack, seqnode, seqtrack, c);
			seqeditortrack_onpreferredsize(seqedittrack, &limit, &preferredtracksize);			
			maxsize.x = psy_ui_max_values(maxsize.x, preferredtracksize.width, &tm);
			psy_ui_value_add(&maxsize.y, &preferredtracksize.height, &tm);
			psy_ui_value_add(&maxsize.y, &self->trackstate->linemargin, &tm);
		}		
	}
	rv->width = maxsize.x;
	rv->height = maxsize.y;
}

void seqeditortracks_onmousedown(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	if (!seqeditortracks_notifymouse(self, ev, seqeditortrack_onmousedown)) {
		seqeditortracks_build(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void seqeditortracks_onmousemove(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	seqeditortracks_notifymouse(self, ev, seqeditortrack_onmousemove);
}

void seqeditortracks_onmouseup(SeqEditorTracks* self,
	psy_ui_MouseEvent* ev)
{
	seqeditortracks_notifymouse(self, ev, seqeditortrack_onmouseup);
}

bool seqeditortracks_notifymouse(SeqEditorTracks* self, psy_ui_MouseEvent* ev,
	bool (*fp_notify)(SeqEditorTrack*, psy_ui_MouseEvent*))
{
	bool rv;
	psy_audio_Sequence* sequence;
	
	rv = TRUE;
	sequence = seqeditortracks_sequence(self);
	if (sequence) {
		psy_audio_SequenceTrackNode* seqnode;
		psy_List* seqeditnode;
		double cpy;
		intptr_t c;
		psy_ui_TextMetric tm;
		double linemargin;
		double lineheight;

		cpy = 0;
		tm = psy_ui_component_textmetric(&self->component);
		linemargin = psy_ui_value_px(&self->trackstate->linemargin, &tm);		
		lineheight = psy_ui_value_px(&self->trackstate->lineheight, &tm);
		if (self->capture) {
			psy_ui_MouseEvent trackev;

			trackev = *ev;
			trackev.y = ev->y - (intptr_t)(self->capture->trackindex *
				(lineheight + linemargin));
			fp_notify(self->capture, &trackev);			
		} else {
			for (seqeditnode = self->tracks, seqnode = sequence->tracks, c = 0;
					seqeditnode != NULL;
					seqnode = (seqnode) ? seqnode->next : NULL,
					psy_list_next(&seqeditnode),
					++c) {
				SeqEditorTrack* seqedittrack;
				psy_audio_SequenceTrack* seqtrack;

				seqedittrack = (SeqEditorTrack*)psy_list_entry(seqeditnode);
				if (seqnode) {
					seqtrack = (psy_audio_SequenceTrack*)psy_list_entry(seqnode);
				} else {
					seqtrack = NULL;
				}
				seqeditortrack_updatetrack(seqedittrack, seqnode, seqtrack, c);
				cpy += lineheight + linemargin;
				if (ev->y >= c * (lineheight + linemargin) &&
						ev->y < (c + 1) * (lineheight + linemargin)) {
					psy_ui_MouseEvent trackev;

					trackev = *ev;
					trackev.y = ev->y - (intptr_t)(cpy - (lineheight + linemargin));
					rv = fp_notify(seqedittrack, &trackev);
					break;
				}
			}
		}
	}
	return rv;
}

void seqeditortracks_onsequenceselectionchanged(SeqEditorTracks* self,
	psy_audio_SequenceSelection* sender)
{			
	seqeditortracks_invalidatebitmap(self);
}

void seqeditortracks_invalidatebitmap(SeqEditorTracks* self)
{
	psy_List* seqeditnode;

	for (seqeditnode = self->tracks; seqeditnode != NULL; psy_list_next(&seqeditnode)) {
		SeqEditorTrack* track;		

		track = (SeqEditorTrack*)psy_list_entry(seqeditnode);
		track->bitmapvalid = FALSE;
	}
	psy_ui_component_invalidate(&self->component);
}

void seqeditortracks_ontimer(SeqEditorTracks* self, uintptr_t timerid)
{		
	if (psy_audio_player_playing(workspace_player(self->workspace))) {
		if (seqeditortracks_playlinechanged(self)) {
			intptr_t playlinepx;			
			psy_ui_Rectangle redrawrect;
						
			playlinepx = seqeditortrackstate_beattopx(self->trackstate,
				psy_audio_player_position(workspace_player(self->workspace)));
			redrawrect = psy_ui_component_scrolledposition(&self->component);
			if (self->lastplaylinepx == -1) {
				psy_ui_rectangle_setleft(&redrawrect, playlinepx);
				psy_ui_rectangle_setwidth(&redrawrect, 2);
			} else if (playlinepx > self->lastplaylinepx) {
				psy_ui_rectangle_setleft(&redrawrect, self->lastplaylinepx);
				psy_ui_rectangle_setright(&redrawrect, playlinepx + 1);
			} else {
				psy_ui_rectangle_setleft(&redrawrect, playlinepx);
				psy_ui_rectangle_setright(&redrawrect, self->lastplaylinepx + 1);				
			}
			self->lastplaylinepx = playlinepx;
			psy_ui_component_invalidaterect(&self->component, redrawrect);
		}
	} else if (self->lastplaylinepx != -1) {
		psy_ui_Rectangle redrawrect;

		redrawrect = psy_ui_component_scrolledposition(&self->component);		
		psy_ui_rectangle_setleft(&redrawrect, self->lastplaylinepx);
		psy_ui_rectangle_setwidth(&redrawrect, 2);
		self->lastplaylinepx = -1;
		psy_ui_component_invalidate(&self->component);
	}	
}

// SeqEditor
void seqeditorbar_init(SeqEditorBar* self, psy_ui_Component* parent)
{
	psy_ui_Margin topmargin;

	psy_ui_margin_init_all(&topmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	zoombox_init(&self->zoombox_beat, &self->component);	
	psy_ui_component_setmargin(&self->component, &topmargin);
	psy_ui_button_init_text(&self->move, &self->component,
		"Move");
	psy_ui_button_init_text(&self->reorder, &self->component,
		"Reorder");
}

void seqeditorbar_setdragmode(SeqEditorBar* self, SeqEditorDragMode mode)
{
	switch (mode) {
	case SEQEDITORDRAG_MOVE:
		psy_ui_button_highlight(&self->move);
		psy_ui_button_disablehighlight(&self->reorder);
		break;
	case SEQEDITORDRAG_REORDER:
		psy_ui_button_highlight(&self->reorder);
		psy_ui_button_disablehighlight(&self->move);
		break;
	default:
		break;
	}
}	

// SeqEditor
// prototypes
static void seqeditor_onsongchanged(SeqEditor*, Workspace*, int flag,
	psy_audio_SongFile*);
static void seqeditor_updatesong(SeqEditor*, psy_audio_Song*);
static void seqeditor_onsequencechanged(SeqEditor*, psy_audio_Sequence*
	sender);
static void seqeditor_onentryscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onheaderscroll(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_onconfigure(SeqEditor*, GeneralConfig* sender,
	psy_Property*);
static void seqeditor_onzoomboxbeatchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_onzoomboxheightchanged(SeqEditor*, ZoomBox* sender);
static void seqeditor_updatescrollstep(SeqEditor*);
static void seqeditor_updateoverflow(SeqEditor*);
static void seqeditor_ondragmodemove(SeqEditor*, psy_ui_Component* sender);
static void seqeditor_ondragmodereorder(SeqEditor*, psy_ui_Component* sender);
// vtable
static psy_ui_ComponentVtable seqeditor_vtable;
static bool seqeditor_vtable_initialized = FALSE;

static void seqeditor_vtable_init(SeqEditor* self)
{
	if (!seqeditor_vtable_initialized) {
		seqeditor_vtable = *(self->component.vtable);		
		seqeditor_vtable_initialized = TRUE;
	}
}
// implementation
void seqeditor_init(SeqEditor* self, psy_ui_Component* parent,
	PatternViewSkin* skin,
	Workspace* workspace)
{		
	psy_ui_Margin topmargin;
	
	psy_ui_margin_init_all(&topmargin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	seqeditor_vtable_init(self);
	self->component.vtable = &seqeditor_vtable;
	psy_ui_component_doublebuffer(&self->component);
	seqeditortrackstate_init(&self->trackstate);
	self->workspace = workspace;
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	seqeditorbar_init(&self->bar, &self->left);
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->bar.zoombox_beat.signal_changed, self,
		seqeditor_onzoomboxbeatchanged);
	// track header
	seqeditortracks_init(&self->trackheaders, &self->left,
		&self->trackstate, SEQEDITOR_TRACKMODE_HEADER, workspace);
	self->trackheaders.skin = skin;
	psy_ui_component_setalign(&self->trackheaders.component,
		psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_height, &self->left);
	psy_ui_component_setalign(&self->zoombox_height.component,
		psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->zoombox_height.signal_changed, self,
		seqeditor_onzoomboxheightchanged);
	seqeditorruler_init(&self->ruler, &self->component, &self->trackstate,
		workspace);	
	psy_ui_component_setalign(seqeditorruler_base(&self->ruler),
		psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(seqeditorruler_base(&self->ruler), &topmargin);
	// tracks
	seqeditortracks_init(&self->tracks, &self->component,		
		&self->trackstate, SEQEDITOR_TRACKMODE_ENTRY, workspace);
	self->tracks.skin = skin;
	psy_ui_scroller_init(&self->scroller, &self->tracks.component,
		&self->component);
	psy_ui_component_setalign(&self->scroller.component,
		psy_ui_ALIGN_CLIENT);
	seqeditorbar_setdragmode(&self->bar, self->tracks.dragmode);
	// align
	psy_ui_component_resize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20.0),
			psy_ui_value_makeeh(6 * 1.4 + 2.5)));
	// use splitbar
	psy_ui_component_preventpreferredsize(&self->component);
	seqeditor_updatesong(self, workspace->song);
	psy_signal_connect(&self->workspace->signal_songchanged, self,
		seqeditor_onsongchanged);
	psy_signal_connect(&self->tracks.component.signal_scroll, self,
		seqeditor_onentryscroll);
	psy_signal_connect(&self->trackheaders.component.signal_scroll, self,
		seqeditor_onheaderscroll);
	psy_signal_connect(&psycleconfig_general(workspace_conf(workspace))->signal_changed,
		self, seqeditor_onconfigure);
	psy_signal_connect(&self->bar.move.signal_clicked, self,
		seqeditor_ondragmodemove);
	psy_signal_connect(&self->bar.reorder.signal_clicked, self,
		seqeditor_ondragmodereorder);
}

void seqeditor_onsongchanged(SeqEditor* self, Workspace* workspace, int flag,
	psy_audio_SongFile* songfile)
{
	seqeditor_updatesong(self, workspace->song);
}

void seqeditor_updatesong(SeqEditor* self, psy_audio_Song* song)
{
	if (song) {
		seqeditortracks_build(&self->tracks);
		seqeditortracks_build(&self->trackheaders);
		psy_signal_connect(&song->sequence.sequencechanged, self,
			seqeditor_onsequencechanged);
		seqeditor_updatescrollstep(self);
		seqeditor_updateoverflow(self);		
	}
}

void seqeditor_updatescrollstep(SeqEditor* self)
{	
	psy_ui_TextMetric tm;
	
	tm = psy_ui_component_textmetric(&self->tracks.component);
	self->tracks.component.scrollstepy = psy_ui_add_values(
		self->tracks.trackstate->lineheight,
		self->tracks.trackstate->linemargin,
		&tm);
	self->trackheaders.component.scrollstepy = self->tracks.component.scrollstepy;
}

void seqeditor_updateoverflow(SeqEditor* self)
{
	psy_ui_component_updateoverflow(seqeditortracks_base(&self->tracks));
	psy_ui_component_updateoverflow(seqeditortracks_base(&self->trackheaders));
	psy_ui_component_invalidate(seqeditortracks_base(&self->tracks));
	psy_ui_component_invalidate(seqeditortracks_base(&self->trackheaders));
}

void seqeditor_onsequencechanged(SeqEditor* self, psy_audio_Sequence* sender)
{
	//if (sender->lastchange != psy_audio_SEQUENCE_CHANGE_REPOSITION) {
		seqeditortracks_build(&self->tracks);
		seqeditortracks_build(&self->trackheaders);
	//} else {
	//	if (sender->lastchangedtrack != UINTPTR_MAX) {
//			psy_List* tracknode;
//			tracknode = psy_list_at(&self->tracks, sender->lastchangedtrack);
//			if (tracknode) {
//				SeqEditorTrack* track;
//
//				track = (SeqEditorTrack*)psy_list_entry(tracknode);
//				track->bitmapvalid = FALSE;				
//			}
//		}
//	}
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
}

void seqeditor_onentryscroll(SeqEditor* self, psy_ui_Component* sender)
{
	psy_ui_component_setscrollleft(&self->ruler.component,
		psy_ui_component_scrollleft(&self->tracks.component));
	psy_ui_component_setscrolltop(&self->trackheaders.component,
		psy_ui_component_scrolltop(&self->tracks.component));
}

void seqeditor_onheaderscroll(SeqEditor* self, psy_ui_Component* sender)
{	
	psy_ui_component_setscrolltop(&self->tracks.component,
		psy_ui_component_scrolltop(&self->trackheaders.component));
}

void seqeditor_onconfigure(SeqEditor* self, GeneralConfig* sender,
	psy_Property* property)
{
	seqeditortracks_invalidatebitmap(&self->tracks);
	psy_ui_component_invalidate(&self->tracks.component);
}

void seqeditor_onzoomboxbeatchanged(SeqEditor* self, ZoomBox* sender)
{
	self->trackstate.pxperbeat = (intptr_t)(sender->zoomrate * DEFAULT_PXPERBEAT);
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
	seqeditortracks_invalidatebitmap(&self->tracks);
}

void seqeditor_onzoomboxheightchanged(SeqEditor* self, ZoomBox* sender)
{
	self->trackstate.lineheight = psy_ui_mul_value_real(
		self->trackstate.defaultlineheight, zoombox_rate(sender));
	seqeditor_updatescrollstep(self);
	seqeditor_updateoverflow(self);
	seqeditortracks_invalidatebitmap(&self->tracks);
}

void seqeditor_ondragmodemove(SeqEditor* self, psy_ui_Component* sender)
{
	self->tracks.dragmode = SEQEDITORDRAG_MOVE;
	seqeditorbar_setdragmode(&self->bar, self->tracks.dragmode);
}

void seqeditor_ondragmodereorder(SeqEditor* self, psy_ui_Component* sender)
{
	self->tracks.dragmode = SEQEDITORDRAG_REORDER;
	seqeditorbar_setdragmode(&self->bar, self->tracks.dragmode);
}