// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "pianoroll.h"
// local
#include "patterncmds.h"
// std
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

#define PIANOROLL_REFRESHRATE 50

enum {
	CMD_NAVUP = 1024,
	CMD_NAVDOWN,
	CMD_NAVLEFT,
	CMD_NAVRIGHT,
	CMD_ENTER,
	CMD_ROWCLEAR
};

static void definecmd(psy_Property* cmds, int cmd, uintptr_t keycode,
	bool shift, bool ctrl, const char* key, const char* shorttext);

// KeyboardState
void keyboardstate_init(KeyboardState* self, PatternViewSkin* skin)
{
	assert(self);

	self->keymin = 0;
	self->keymax = 88;	
	self->defaultkeyheight = psy_ui_value_makeeh(1.0);
	self->keyheight = self->defaultkeyheight;
	self->notemode = psy_dsp_NOTESTAB_A220;
	self->drawpianokeys = TRUE;
	self->skin = skin;
}

// PianoGridState
void gridstate_init(PianoGridState* self, PatternViewSkin* skin)
{
	assert(self);

	self->pattern = NULL;
	self->defaultbeatwidth = 90;
	self->pxperbeat = self->defaultbeatwidth;
	self->lpb = 4;	
	self->skin = skin;		
	psy_audio_patterncursor_init(&self->cursor);
}

// Header
static void pianoruler_ondraw(PianoRuler*, psy_ui_Graphics*);
static psy_audio_PatternSelection pianoruler_clipselection(PianoRuler*,
	psy_ui_Rectangle clip);
static void pianoruler_drawruler(PianoRuler*, psy_ui_Graphics*, psy_audio_PatternSelection);
void pianoruler_drawbeat(PianoRuler*, psy_ui_Graphics*, int beat, int x,
	int baseline, int tmheight);
static void pianoruler_onpreferredsize(PianoRuler*, const psy_ui_Size* limit,
	psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable pianoruler_vtable;
static bool pianoruler_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianoruler_vtable_init(PianoRuler* self)
{
	assert(self);

	if (!pianoruler_vtable_initialized) {
		pianoruler_vtable = *(self->component.vtable);
		pianoruler_vtable.ondraw = (psy_ui_fp_component_ondraw)
			pianoruler_ondraw;
		pianoruler_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			pianoruler_onpreferredsize;
		pianoruler_vtable_initialized = TRUE;
	}
	return &pianoruler_vtable;
}

void pianoruler_init(PianoRuler* self, psy_ui_Component* parent,
	PianoGridState* gridstate)
{
	assert(self);

	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(pianoruler_base(self),
		pianoruler_vtable_init(self));
	psy_ui_component_doublebuffer(pianoruler_base(self));
	pianoruler_setsharedgridstate(self, gridstate);			
}

void pianoruler_setsharedgridstate(PianoRuler* self, PianoGridState* gridstate)
{
	assert(self);

	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		gridstate_init(&self->defaultgridstate, NULL);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianoruler_onpreferredsize(PianoRuler* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	assert(self);

	rv->width = (pianogridstate_pattern(self->gridstate))
		? psy_ui_value_makepx(pianogridstate_beattopx(self->gridstate,
		  psy_audio_pattern_length(pianogridstate_pattern(self->gridstate))))
		: psy_ui_value_makepx(0);
	rv->height = psy_ui_value_makeeh(1.0);
}

void pianoruler_ondraw(PianoRuler* self, psy_ui_Graphics* g)
{	
	assert(self);

	pianoruler_drawruler(self, g, pianoruler_clipselection(self, g->clip));
}

psy_audio_PatternSelection pianoruler_clipselection(PianoRuler* self,
	psy_ui_Rectangle clip)
{
	psy_audio_PatternSelection rv;

	assert(self);

	rv.topleft.offset = pianogridstate_quantize(self->gridstate,
		pianogridstate_pxtobeat(self->gridstate, clip.left));
	rv.bottomright.offset = psy_min(
		psy_audio_pattern_length(pianogridstate_pattern(self->gridstate)),
		pianogridstate_pxtobeat(self->gridstate, clip.right));	
	return rv;
}

void pianoruler_drawruler(PianoRuler* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{	
	psy_ui_TextMetric tm;
	psy_ui_IntSize size;
	int baselinetop;
	int baseline;
	int currstep;
	psy_dsp_big_beat_t c;
	
	assert(self);

	tm = psy_ui_component_textmetric(&self->component);
	size = psy_ui_intsize_init_size(
		psy_ui_component_size(&self->component), &tm);	
	baseline = size.height - 1;
	baselinetop = baseline - tm.tmHeight / 3;
			
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
		0, 0));
	psy_ui_drawline(g, psy_ui_component_scrollleft(&self->component), baseline,
		size.width + psy_ui_component_scrollleft(&self->component),
		baseline);
	currstep = pianogridstate_steps(self->gridstate, clip.topleft.offset);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (c = clip.topleft.offset; c <= clip.bottomright.offset;
			c += pianogridstate_step(self->gridstate), ++currstep) {
		int cpx;
		
		cpx = pianogridstate_beattopx(self->gridstate, c);
		psy_ui_drawline(g, cpx, baseline, cpx, baselinetop);
		if ((currstep % self->gridstate->lpb) == 0) {
			pianoruler_drawbeat(self, g, (int)(c), cpx, baseline, tm.tmHeight);
		}
	}
}

void pianoruler_drawbeat(PianoRuler* self, psy_ui_Graphics* g, int beat,
	int x, int baseline, int tmheight)
{	
	char txt[40];

	assert(self);

	if ((beat % 4) == 0) {
		psy_ui_settextcolour(g, patternviewskin_row4beatcolour(self->gridstate->skin,
			0, 0));
	} else {
		psy_ui_settextcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
			0, 0));
	}
	psy_ui_drawline(g, x, baseline, x, baseline - tmheight / 2);
	psy_snprintf(txt, 40, "%d", beat);
	psy_ui_textout(g, x + 3, baseline - tmheight, txt, strlen(txt));
	psy_ui_setcolour(g, patternviewskin_rowbeatcolour(self->gridstate->skin,
		0, 0));
}

// PianoKeyboard
// prototypes
static void pianokeyboard_ondraw(PianoKeyboard*, psy_ui_Graphics*);
static void pianokeyboard_drawuncoveredbottombackground(PianoKeyboard*,
	psy_ui_Graphics*, psy_ui_IntSize);
static void pianokeyboard_onmousedown(PianoKeyboard*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable pianokeyboard_vtable;
static bool pianokeyboard_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianokeyboard_vtable_init(PianoKeyboard* self)
{
	assert(self);

	if (!pianokeyboard_vtable_initialized) {
		pianokeyboard_vtable = *(self->component.vtable);
		pianokeyboard_vtable.ondraw = (psy_ui_fp_component_ondraw)
			pianokeyboard_ondraw;
		pianokeyboard_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianokeyboard_onmousedown;
		pianokeyboard_vtable_initialized = TRUE;
	}
	return &pianokeyboard_vtable;
}
// implementation
void pianokeyboard_init(PianoKeyboard* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate)
{
	psy_ui_component_init(pianokeyboard_base(self), parent);
	psy_ui_component_setvtable(pianokeyboard_base(self),
		pianokeyboard_vtable_init(self));
	pianokeyboard_setsharedkeyboardstate(self, keyboardstate);
	psy_ui_component_doublebuffer(pianokeyboard_base(self));
	psy_ui_component_setbackgroundmode(pianokeyboard_base(self),
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_setpreferredsize(pianokeyboard_base(self),
		psy_ui_size_makeem(8.0, 0.0));
	psy_ui_component_preventalign(pianokeyboard_base(self));
}

void pianokeyboard_setsharedkeyboardstate(PianoKeyboard* self,
	KeyboardState* keyboardstate)
{
	if (keyboardstate) {
		self->keyboardstate = keyboardstate;
	} else {
		keyboardstate_init(&self->defaultkeyboardstate, NULL);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
}

void pianokeyboard_ondraw(PianoKeyboard* self, psy_ui_Graphics* g)
{	
	int key;	
	psy_ui_IntSize size;
	psy_ui_TextMetric tm;

	assert(self);

	size = psy_ui_component_intsize(pianokeyboard_base(self));
	tm = psy_ui_component_textmetric(pianokeyboard_base(self));
	self->keyboardstate->keyboardheightpx = keyboardstate_height(self->keyboardstate, &tm);
	self->keyboardstate->keyheightpx = psy_ui_value_px(&self->keyboardstate->keyheight, &tm);
	psy_ui_setcolour(g, patternviewskin_keyseparatorcolour(self->keyboardstate->skin));
	if (self->keyboardstate->drawpianokeys) {
		psy_ui_settextcolour(g, patternviewskin_keyseparatorcolour(self->keyboardstate->skin));
	} else {
		psy_ui_settextcolour(g, patternviewskin_keywhitecolour(self->keyboardstate->skin));
	}
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	for (key = self->keyboardstate->keymin; key < self->keyboardstate->keymax; ++key) {
		int cpy;

		cpy = keyboardstate_keytopx(self->keyboardstate, key);
		psy_ui_drawline(g, 0, cpy, size.width, cpy);
		if (self->keyboardstate->drawpianokeys) {
			if (psy_dsp_isblack(key)) {
				psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					(int)(size.width * 0.75), cpy,
					(int)(size.width * 0.25),
					self->keyboardstate->keyheightpx),
					patternviewskin_keywhitecolour(self->keyboardstate->skin));
				psy_ui_drawline(g,
					0, cpy + self->keyboardstate->keyheightpx / 2,
					size.width, cpy + self->keyboardstate->keyheightpx / 2);
				psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
					0, cpy, (int)(size.width * 0.75),
					self->keyboardstate->keyheightpx),
					patternviewskin_keyblackcolour(self->keyboardstate->skin));
			} else {
				psy_ui_Rectangle r;

				r = psy_ui_rectangle_make(0, cpy, size.width,
					self->keyboardstate->keyheightpx);
				psy_ui_drawsolidrectangle(g, r,
					patternviewskin_keywhitecolour(self->keyboardstate->skin));
				if (psy_dsp_iskey_c(key) || psy_dsp_iskey_e(key)) {
					psy_ui_drawline(g,
						0, cpy + self->keyboardstate->keyheightpx,
						size.width, cpy + self->keyboardstate->keyheightpx);
					if (psy_dsp_iskey_c(key)) {
						psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_CLIPPED, r,
							psy_dsp_notetostr(key, self->keyboardstate->notemode),
							strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
					}
				}
			}
		} else {
			psy_ui_Rectangle r;

			r = psy_ui_rectangle_make(0, cpy, size.width,
				self->keyboardstate->keyheightpx);
			psy_ui_textoutrectangle(g, r.left, r.top, psy_ui_ETO_CLIPPED, r,
				psy_dsp_notetostr(key, self->keyboardstate->notemode),
				strlen(psy_dsp_notetostr(key, self->keyboardstate->notemode)));
			psy_ui_drawline(g,
				0, cpy + self->keyboardstate->keyheightpx,
				size.width,
				cpy + self->keyboardstate->keyheightpx);
		}
	}
	pianokeyboard_drawuncoveredbottombackground(self, g, size);
}

void pianokeyboard_drawuncoveredbottombackground(PianoKeyboard* self, psy_ui_Graphics*
	g, psy_ui_IntSize size)
{
	int blankstart;

	assert(self);

	blankstart = self->keyboardstate->keyboardheightpx;
	if (blankstart - psy_ui_component_scrolltop(&self->component) <
			size.height) {
		psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
			0, blankstart, size.width,
			size.height - (blankstart - psy_ui_component_scrolltop(
				pianokeyboard_base(self)))),
			patternviewskin_separatorcolour(self->keyboardstate->skin, 1, 2));
	}
}

void pianokeyboard_onmousedown(PianoKeyboard* self, psy_ui_MouseEvent* ev)
{	
	assert(self);

	psy_ui_mouseevent_stoppropagation(ev);
}

void pianokeyboard_setkeyboardtype(PianoKeyboard* self, KeyboardType
	keyboardtype)
{
	double width_em;

	assert(self);

	width_em = 8.0;
	switch (keyboardtype) {
		case KEYBOARDTYPE_KEYS:
			self->keyboardstate->drawpianokeys = TRUE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_A440;
			break;
		case KEYBOARDTYPE_NOTES:
			self->keyboardstate->drawpianokeys = FALSE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_A440;
			break;
		case KEYBOARDTYPE_DRUMS:
			self->keyboardstate->drawpianokeys = FALSE;
			self->keyboardstate->notemode = psy_dsp_NOTESTAB_GMPERCUSSION;
			width_em = 21.0;			
			break;
		default:
			break;
	}
	psy_ui_component_setpreferredsize(pianokeyboard_base(self),
		psy_ui_size_makeem(width_em, 0.0));
	psy_ui_component_invalidate(pianokeyboard_base(self));	
}

// Pianogrid
// prototypes
static void pianogrid_ondestroy(Pianogrid*);
static void pianogrid_ondraw(Pianogrid*, psy_ui_Graphics*);
static void pianogrid_drawbackground(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
static void pianogrid_drawuncoveredrightbackground(Pianogrid*,
	psy_ui_Graphics*, psy_ui_IntSize);
static void pianogrid_drawuncoveredbottombackground(Pianogrid*,
	psy_ui_Graphics*, psy_ui_IntSize);
static void pianogrid_updatekeystate(Pianogrid*);
static void pianogrid_drawgrid(Pianogrid*, psy_ui_Graphics*, psy_audio_PatternSelection);
static psy_audio_PatternSelection pianogrid_clipselection(Pianogrid*, psy_ui_Rectangle
	clip);
static void pianogrid_drawgridcells(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
static void pianogrid_drawstepseparators(Pianogrid*, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
static void pianogrid_drawkeyseparators(Pianogrid* self, psy_ui_Graphics*,
	psy_audio_PatternSelection clip);
static void pianogrid_drawplaybar(Pianogrid*, psy_ui_Graphics*, psy_audio_PatternSelection);
static void pianogrid_drawentries(Pianogrid*, psy_ui_Graphics*, psy_audio_PatternSelection);
static bool pianogrid_hastrackdisplay(Pianogrid*, uintptr_t track);
static PianogridTrackEvent* pianogrid_lasttrackevent_at(Pianogrid* self, uintptr_t track);
static void pianogrid_lasttrackevent_clear(Pianogrid*);
static void pianogrid_drawevent(Pianogrid*, psy_ui_Graphics*,
	PianogridTrackEvent*, psy_dsp_big_beat_t length);
static void pianogrid_drawcursor(Pianogrid*, psy_ui_Graphics*, psy_audio_PatternSelection);
static psy_ui_Colour pianogrid_cellcolour(Pianogrid* self, uintptr_t step,
	intptr_t key);
static void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void pianogrid_onmousedown(Pianogrid*, psy_ui_MouseEvent*);
static bool pianogrid_keyhittest(Pianogrid*, psy_audio_PatternNode*,
	uintptr_t track, uint8_t cursorkey);
static void pianogrid_onmousemove(Pianogrid*, psy_ui_MouseEvent*);
static int pianogrid_scrollright(Pianogrid*, psy_audio_PatternCursor cursor);
static int pianogrid_scrollleft(Pianogrid*, psy_audio_PatternCursor cursor);
static void pianogrid_prevline(Pianogrid*);
static void pianogrid_prevlines(Pianogrid*, uintptr_t lines, int wrap);
static void pianogrid_advanceline(Pianogrid*);
static void pianogrid_advancelines(Pianogrid*, uintptr_t lines, int wrap);

static psy_ui_ComponentVtable pianogrid_vtable;
static bool pianogrid_vtable_initialized = FALSE;
// vtable
static psy_ui_ComponentVtable* pianogrid_vtable_init(Pianogrid* self)
{
	assert(self);

	if (!pianogrid_vtable_initialized) {
		pianogrid_vtable = *(self->component.vtable);
		pianogrid_vtable.ondraw = (psy_ui_fp_component_ondraw)pianogrid_ondraw;
		pianogrid_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianogrid_onmousedown;
		pianogrid_vtable.onmousemove = (psy_ui_fp_component_onmousemove)
			pianogrid_onmousemove;
		pianogrid_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			pianogrid_onpreferredsize;
		pianogrid_vtable_initialized = TRUE;
	}
	return &pianogrid_vtable;
}
// implementation
void pianogrid_init(Pianogrid* self, psy_ui_Component* parent,
	KeyboardState* keyboardstate, PianoGridState* gridstate,
	Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(pianogrid_base(self), parent);
	psy_ui_component_setvtable(pianogrid_base(self),
		pianogrid_vtable_init(self));
	psy_ui_component_setbackgroundmode(pianogrid_base(self),
		psy_ui_BACKGROUND_NONE);
	psy_ui_component_doublebuffer(pianogrid_base(self));
	psy_ui_component_setwheelscroll(pianogrid_base(self), 4);
	pianogrid_setsharedgridstate(self, gridstate);
	pianogrid_setsharedkeyboardstate(self, keyboardstate);
	self->trackdisplay = PIANOROLL_TRACK_DISPLAY_ALL;
	self->cursorchanging = FALSE;
	self->workspace = workspace;
	self->hoverpatternentry = NULL;
	self->lastplayposition = (psy_dsp_big_beat_t)0.0;
	self->sequenceentryoffset = (psy_dsp_big_beat_t)0.0;
	self->cursoronnoterelease = FALSE;
	psy_audio_patterncursor_init(&self->oldcursor);
	psy_table_init(&self->lasttrackevent);
	psy_signal_connect(&pianogrid_base(self)->signal_destroy, self,
		pianogrid_ondestroy);
	psy_ui_component_setoverflow(pianogrid_base(self), psy_ui_OVERFLOW_SCROLL);	
}

void pianogrid_ondestroy(Pianogrid* self)
{
	assert(self);

	psy_table_disposeall(&self->lasttrackevent, (psy_fp_disposefunc)NULL);
}

void pianogrid_setsharedgridstate(Pianogrid* self, PianoGridState* gridstate)
{
	assert(self);

	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		gridstate_init(&self->defaultgridstate, NULL);
		self->gridstate = &self->defaultgridstate;
	}
}

void pianogrid_setsharedkeyboardstate(Pianogrid* self, KeyboardState*
	keyboardstate)
{
	assert(self);

	if (keyboardstate) {
		self->keyboardstate = keyboardstate;
	} else {
		keyboardstate_init(&self->defaultkeyboardstate, NULL);
		self->keyboardstate = &self->defaultkeyboardstate;
	}
}

void pianogrid_onpatternchange(Pianogrid* self, psy_audio_Pattern* pattern)
{	
	assert(self);

	self->hoverpatternentry = NULL;
	pianogrid_lasttrackevent_clear(self);
	psy_ui_component_updateoverflow(pianogrid_base(self));
}

void pianogrid_settrackdisplay(Pianogrid* self, PianoTrackDisplay display)
{
	assert(self);

	self->trackdisplay = display;
	psy_ui_component_invalidate(pianogrid_base(self));
}

void pianogrid_ondraw(Pianogrid* self, psy_ui_Graphics* g)
{
	psy_audio_PatternSelection clip;

	assert(self);

	if (!pianogridstate_pattern(self->gridstate)) {
		return;
	}	
	pianogrid_updatekeystate(self);
	clip = pianogrid_clipselection(self, g->clip);
	self->cursoronnoterelease = FALSE;
	pianogrid_drawbackground(self, g, clip);
	pianogrid_drawgrid(self, g, clip);
	pianogrid_drawplaybar(self, g, clip);
	pianogrid_drawentries(self, g, clip);
	pianogrid_drawcursor(self, g, clip);
}

void pianogrid_updatekeystate(Pianogrid* self)
{
	psy_ui_TextMetric tm;

	assert(self);

	tm = psy_ui_component_textmetric(&self->component);
	self->keyboardstate->keyheightpx = psy_ui_value_px(
		&self->keyboardstate->keyheight, &tm);
	self->keyboardstate->keyboardheightpx = keyboardstate_height(
		self->keyboardstate, &tm);
}

psy_audio_PatternSelection pianogrid_clipselection(Pianogrid* self,
	psy_ui_Rectangle clip)
{
	psy_audio_PatternSelection rv;

	assert(self);

	pianogridstate_clip(self->gridstate, clip.left, clip.right,
		&rv.topleft.offset, &rv.bottomright.offset);
	pianokeyboardstate_clip(self->keyboardstate,
		clip.top, clip.bottom,
		&rv.bottomright.key, &rv.topleft.key);	
	return rv;
}

void pianogrid_drawbackground(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{	
	psy_ui_IntSize size;
		
	assert(self);
	assert(pianogridstate_pattern(self->gridstate));

	size = psy_ui_component_intsize(pianogrid_base(self));
	pianogrid_drawuncoveredrightbackground(self, g, size);
	pianogrid_drawuncoveredbottombackground(self, g, size);
}

void pianogrid_drawuncoveredrightbackground(Pianogrid* self, psy_ui_Graphics*
	g, psy_ui_IntSize size)
{
	int blankstart;

	assert(self);
	assert(pianogridstate_pattern(self->gridstate));

	blankstart = pianogridstate_beattopx(self->gridstate,
		psy_audio_pattern_length(pianogridstate_pattern(self->gridstate)));
	if (blankstart - psy_ui_component_scrollleft(&self->component) <
			size.width) {
		psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
				blankstart,
				psy_ui_component_scrolltop(&self->component),
				size.width - (blankstart - psy_ui_component_scrollleft(
					pianogrid_base(self))),
				size.height),
			patternviewskin_separatorcolour(self->gridstate->skin, 1, 2));
	}
}

void pianogrid_drawuncoveredbottombackground(Pianogrid* self, psy_ui_Graphics*
	g, psy_ui_IntSize size)
{
	int blankstart;

	assert(self);
	assert(pianogridstate_pattern(self->gridstate));

	blankstart = self->keyboardstate->keyboardheightpx;
	if (blankstart - psy_ui_component_scrolltop(&self->component) <
			size.height) {
		psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
				psy_ui_component_scrollleft(pianogrid_base(self)),
				blankstart,
				size.width,
				size.height - (blankstart - psy_ui_component_scrolltop(
					pianogrid_base(self)))),
			patternviewskin_separatorcolour(self->gridstate->skin, 1, 2));
	}
}

void pianogrid_drawgrid(Pianogrid* self, psy_ui_Graphics* g, psy_audio_PatternSelection clip)
{
	assert(self);

	pianogrid_drawgridcells(self, g, clip);	
	pianogrid_drawstepseparators(self, g, clip);
	pianogrid_drawkeyseparators(self, g, clip);
}

void pianogrid_drawgridcells(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{	
	uintptr_t key;

	assert(self);
	
	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {
		int cpy;
		uintptr_t steps;
		psy_dsp_big_beat_t c;

		cpy = keyboardstate_keytopx(self->keyboardstate, key);		
		c = clip.topleft.offset;
		steps = pianogridstate_steps(self->gridstate, c);
		for (; c <= clip.bottomright.offset;
				c += pianogridstate_step(self->gridstate), ++steps) {
			psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
				pianogridstate_beattopx(self->gridstate, c), cpy,
				pianogridstate_steppx(self->gridstate) + 1,
				self->keyboardstate->keyheightpx),
				pianogrid_cellcolour(self, steps, key));
		}
	}
}

psy_ui_Colour pianogrid_cellcolour(Pianogrid* self, uintptr_t step, intptr_t key)
{
	psy_ui_Colour rv;

	assert(self);

	if ((step % (self->gridstate->lpb * 4)) == 0) {
		rv = patternviewskin_row4beatcolour(self->gridstate->skin, 0, 0);
	} else if ((step % self->gridstate->lpb) == 0) {
		rv = patternviewskin_rowbeatcolour(self->gridstate->skin, 0, 0);
	} else {
		rv = patternviewskin_rowcolour(self->gridstate->skin, 0, 0);
	}
	if (psy_dsp_isblack(key)) {
		psy_ui_colour_add_rgb(&rv, -4, -4, -4);
	}
	return rv;
}

void pianogrid_drawstepseparators(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{
	psy_dsp_big_beat_t c;

	assert(self);

	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, 0));
	for (c = clip.topleft.offset; c <= clip.bottomright.offset;
			c += pianogridstate_step(self->gridstate)) {
		int cpx;

		cpx = pianogridstate_beattopx(self->gridstate, c);
		psy_ui_drawline(g, cpx,
			psy_ui_component_scrolltop(pianogrid_base(self)),
			cpx, self->keyboardstate->keyboardheightpx);
	}
}

// draws key separators at C and E
void pianogrid_drawkeyseparators(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{	
	uint8_t key;	

	assert(self);

	psy_ui_setcolour(g, patternviewskin_separatorcolour(self->gridstate->skin,
		0, 0));	
	for (key = clip.bottomright.key; key <= clip.topleft.key; ++key) {		
		if (psy_dsp_iskey_c(key + 1) || psy_dsp_iskey_e(key + 1)) {
			int cpy;

			cpy = keyboardstate_keytopx(self->keyboardstate, key);
			psy_ui_drawline(g,
				pianogridstate_beattopx(self->gridstate, clip.topleft.offset),
				cpy,
				pianogridstate_beattopx(self->gridstate, clip.bottomright.offset),
				cpy);
		}
	}
}

void pianogrid_drawcursor(Pianogrid* self, psy_ui_Graphics* g, psy_audio_PatternSelection clip)
{
	assert(self);

	if (!self->cursorchanging && !self->cursoronnoterelease &&
			!(psy_audio_player_playing(&self->workspace->player) &&
			workspace_followingsong(self->workspace))) {
		psy_audio_PatternCursor cursor;		
		intptr_t key;
						
		cursor = workspace_patterncursor(self->workspace);
		if (cursor.key != psy_audio_NOTECOMMANDS_EMPTY) {
			key = cursor.key;
		} else {
			key = psy_audio_NOTECOMMANDS_MIDDLEC;
		}						
		psy_ui_drawsolidrectangle(g, psy_ui_rectangle_make(
			pianogridstate_beattopx(self->gridstate, cursor.offset),
			keyboardstate_keytopx(self->keyboardstate, key),
			pianogridstate_steppx(self->gridstate),
			self->keyboardstate->keyheightpx),
			patternviewskin_cursorcolour(self->gridstate->skin,
				0, 0));
	}
}

void pianogrid_drawplaybar(Pianogrid* self, psy_ui_Graphics* g, psy_audio_PatternSelection clip)
{
	assert(self);

	if (psy_audio_player_playing(&self->workspace->player)) {
		psy_dsp_big_beat_t offset;

		offset = psy_audio_player_position(&self->workspace->player) -
			self->sequenceentryoffset;
		if (offset >= 0 && offset < psy_audio_pattern_length(
				pianogridstate_pattern(self->gridstate))) {			
			psy_ui_drawsolidrectangle(g,
				psy_ui_rectangle_make(
					pianogridstate_beattopx(self->gridstate, pianogridstate_quantize(
						self->gridstate, offset)), 0,
					pianogridstate_steppx(self->gridstate),
					self->keyboardstate->keyboardheightpx),
				patternviewskin_playbarcolour(self->gridstate->skin,
					0, self->workspace->song->properties.tracks));
		}
	}
}

void pianogrid_drawentries(Pianogrid* self, psy_ui_Graphics* g,
	psy_audio_PatternSelection clip)
{
	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {		
		psy_audio_PatternNode* currnode;
		psy_TableIterator it;
				
		// iterates over the events of the pattern and draws events with notes. To
		// determine the length, the current event is stored for each track. In
		// the next turn the stored last event is drawn to the start of the
		// current. Finally the last events with no ending notereleases are drawn
		// till the end of the pattern.
		currnode = psy_audio_pattern_begin(pianogridstate_pattern(self->gridstate));
		while (currnode) {
			psy_audio_PatternEntry* curr;
			PianogridTrackEvent* last;

			curr = (psy_audio_PatternEntry*)psy_list_entry(currnode);
			if (pianogrid_hastrackdisplay(self, curr->track)) {
				psy_audio_PatternEvent* currevent;

				currevent = psy_audio_patternentry_front(curr);
				if (currevent->note <= psy_audio_NOTECOMMANDS_RELEASE) {					
					last = pianogrid_lasttrackevent_at(self, curr->track);
					// active tells if a last event exists or not
					if (last->active) {
						// draw last to start of current
						if (curr->offset >= clip.topleft.offset) {
							pianogrid_drawevent(self, g, last, curr->offset -
								last->offset);
						}
					}					
					if (currevent->note == psy_audio_NOTECOMMANDS_RELEASE) {
						// length is known: one grid step
						// draw with last key
						if (!last->active) {							
							last->note = psy_audio_NOTECOMMANDS_MIDDLEC;
						}
						last->offset = curr->offset;
						last->hover = (self->hoverpatternentry == curr);
						last->noterelease = TRUE;
						pianogrid_drawevent(self, g, last, pianogridstate_step(self->gridstate));
						// and skip draw last event
						last->noterelease = FALSE;
						last->active = FALSE;
					} else {
					// last = current
					last->active = TRUE;
					last->note = currevent->note;
					last->offset = curr->offset;
					last->hover = (self->hoverpatternentry == curr);
					}
				}
			}
			psy_audio_patternnode_next(&currnode);
		}
		// draws remaining events with no noterelease to the end of the pattern
		for (it = psy_table_begin(&self->lasttrackevent);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			PianogridTrackEvent* last;

			last = (PianogridTrackEvent*)psy_tableiterator_value(&it);
			if (last->active && pianogrid_hastrackdisplay(self, last->track)) {
				pianogrid_drawevent(self, g, last, psy_audio_pattern_length(
					pianogridstate_pattern(self->gridstate)) - last->offset);
			}
			last->active = FALSE;
		}
	}
}

bool pianogrid_hastrackdisplay(Pianogrid* self, uintptr_t track)
{
	assert(self);

	return (self->trackdisplay == PIANOROLL_TRACK_DISPLAY_ALL ||
		(self->trackdisplay == PIANOROLL_TRACK_DISPLAY_CURRENT &&
			track == self->gridstate->cursor.track) ||
		(self->trackdisplay == PIANOROLL_TRACK_DISPLAY_ACTIVE &&
			!patternstrackstate_istrackmuted(
				&self->workspace->song->patterns.trackstate, track)));
}

void pianogrid_lasttrackevent_clear(Pianogrid* self)
{
	assert(self);

	psy_table_disposeall(&self->lasttrackevent, (psy_fp_disposefunc)NULL);
	psy_table_init(&self->lasttrackevent);
}

PianogridTrackEvent* pianogrid_lasttrackevent_at(Pianogrid* self, uintptr_t track)
{
	PianogridTrackEvent* rv;

	assert(self);

	rv = psy_table_at(&self->lasttrackevent, track);
	if (!rv) {
		rv = (PianogridTrackEvent*)malloc(sizeof(PianogridTrackEvent));
		if (rv) {
			rv->offset = -1.0f;
			rv->active = FALSE;
			rv->track = track;
			rv->note = psy_audio_NOTECOMMANDS_EMPTY;
			rv->hover = FALSE;
			rv->noterelease = FALSE;
			psy_table_insert(&self->lasttrackevent, track, rv);
		}
	}
	return rv;
}

void pianogrid_drawevent(Pianogrid* self, psy_ui_Graphics* g,
	PianogridTrackEvent* ev, psy_dsp_big_beat_t length)
{
	psy_ui_Rectangle r;
	psy_ui_Colour colour;
	psy_audio_PatternCursor cursor;
	psy_ui_Size corner;

	assert(self);

	cursor = workspace_patterncursor(self->workspace);
	int left = (int)((ev->offset) * self->gridstate->pxperbeat);
	int width = (int)(length * self->gridstate->pxperbeat);
	corner = psy_ui_size_makepx(2, 2);
	psy_ui_setrectangle(&r, left,
		(self->keyboardstate->keymax - ev->note - 1) *
		self->keyboardstate->keyheightpx + 1, width,
		self->keyboardstate->keyheightpx - 2);
	if (ev->hover) {
		colour = patternviewskin_eventhovercolour(self->gridstate->skin, 0, 0);
	} else if (ev->track == cursor.track) {
		colour = patternviewskin_eventcurrchannelcolour(self->gridstate->skin,
			0, 0);
	} else {
		colour = patternviewskin_eventcolour(self->gridstate->skin, 0, 0);
	}
	if (!ev->noterelease) {
		psy_ui_drawsolidroundrectangle(g, r, corner, colour);
	} else {		
		if (self->gridstate->cursor.key == ev->note &&
				self->gridstate->cursor.offset ==
			pianogridstate_quantize(self->gridstate, ev->offset)) {
			self->cursoronnoterelease = TRUE;
			colour = patternviewskin_cursorcolour(self->gridstate->skin,
				0, 0);
		}
		psy_ui_setcolour(g, colour);
		psy_ui_drawroundrectangle(g, r, corner);		
	}
}

void pianogrid_onpreferredsize(Pianogrid* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	assert(self);

	pianogrid_updatekeystate(self);
	rv->height = psy_ui_value_makepx((self->keyboardstate->keymax -
		self->keyboardstate->keymin) * self->keyboardstate->keyheightpx);
	if (pianogridstate_pattern(self->gridstate)) {
		rv->width = psy_ui_value_makepx((int)(self->gridstate->pxperbeat *
			psy_audio_pattern_length(pianogridstate_pattern(self->gridstate))));
	} else {
		rv->width = psy_ui_value_makepx(0);
	}
}

void pianogrid_onmousedown(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternEvent patternevent;
		psy_audio_PatternCursor cursor;

		psy_ui_component_setfocus(&self->component);
		psy_audio_patternevent_clear(&patternevent);
		pianogrid_updatekeystate(self);		
		patternevent.note = keyboardstate_pxtokey(self->keyboardstate, ev->y);
		cursor = pianogridstate_cursor(self->gridstate);
		cursor.offset = pianogridstate_quantize(self->gridstate,
				pianogridstate_pxtobeat(self->gridstate, ev->x));
		cursor.key = patternevent.note;		
		if (ev->button == 1) {
			// left button			
			psy_undoredo_execute(&self->workspace->undoredo,
				&InsertCommandAlloc(
					pianogridstate_pattern(self->gridstate),
					pianogridstate_step(self->gridstate),
					cursor, patternevent,self->workspace)->command);
			workspace_setpatterncursor(self->workspace, cursor);			
		} else if (ev->button == 2) {
			// right button			
			psy_audio_PatternNode* node;
			psy_audio_PatternNode* prev;

			node = psy_audio_pattern_findnode(
				pianogridstate_pattern(self->gridstate),
				self->gridstate->cursor.track, cursor.offset,
					pianogridstate_step(self->gridstate), &prev);
			if (!node) {				
				if (prev) {
					psy_audio_PatternEntry* preventry;

					preventry = psy_audio_patternnode_entry(prev);
					if (preventry->track != cursor.track) {
						prev = psy_audio_pattern_prev_track(pianogridstate_pattern(self->gridstate),
							prev, cursor.track);
					}
				}
				node = prev;
			}
			if (node) {
				psy_audio_PatternNode* next;				
				psy_audio_PatternEntry* patternentry;

				patternentry = psy_audio_patternnode_entry(node);				
				if (!pianogrid_keyhittest(self, node, cursor.track, cursor.key)) {
					pianogrid_storecursor(self);
					workspace_setpatterncursor(self->workspace, cursor);
					pianogrid_invalidatecursor(self);
					return;
				}
				if (patternentry->offset == cursor.offset) {
					next = psy_audio_pattern_next_track(pianogridstate_pattern(self->gridstate),
						node, cursor.track);
					if (self->hoverpatternentry == psy_audio_patternnode_entry(node)) {
						self->hoverpatternentry = NULL;
					}
					psy_undoredo_execute(&self->workspace->undoredo,
						&RemoveCommandAlloc(self->gridstate->pattern,
							pianogridstate_step(self->gridstate),
							cursor, self->workspace)->command);
					if (next) {
						psy_audio_PatternEntry* nextentry;

						nextentry = psy_audio_patternnode_entry(next);
						if (psy_audio_patternentry_front(nextentry)->note
								== psy_audio_NOTECOMMANDS_RELEASE) {
							if (self->hoverpatternentry == psy_audio_patternnode_entry(next)) {
								self->hoverpatternentry = NULL;
							}
							cursor.offset = nextentry->offset;
							psy_undoredo_execute(&self->workspace->undoredo,
								&RemoveCommandAlloc(self->gridstate->pattern,
									pianogridstate_step(self->gridstate),
									cursor, self->workspace)->command);
						}
					}
					workspace_setpatterncursor(self->workspace, cursor);
				} else {
					if (psy_audio_patternentry_front(patternentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
						workspace_setpatterncursor(self->workspace, cursor);
						return;
					} else {
						psy_audio_PatternEvent release;

						next = psy_audio_pattern_next_track(pianogridstate_pattern(self->gridstate), node,
							cursor.track);
						if (next) {
							psy_audio_PatternEntry* nextentry;							

							nextentry = psy_audio_patternnode_entry(next);
							if (psy_audio_patternentry_front(nextentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
								psy_audio_PatternCursor release;

								release = cursor;
								release.offset = nextentry->offset;
								psy_undoredo_execute(&self->workspace->undoredo,
									&RemoveCommandAlloc(self->gridstate->pattern,
										pianogridstate_step(self->gridstate),
										release, self->workspace)->command);
							}
						}						
						psy_audio_patternevent_clear(&release);
						release.note = psy_audio_NOTECOMMANDS_RELEASE;
						psy_undoredo_execute(&self->workspace->undoredo,
							&InsertCommandAlloc(pianogridstate_pattern(self->gridstate),
								pianogridstate_step(self->gridstate),
								cursor, release,
								self->workspace)->command);
						cursor.key = psy_audio_patternentry_front(patternentry)->note;
						workspace_setpatterncursor(self->workspace, cursor);
					}
				}
			}
		}
	}
}

bool pianogrid_keyhittest(Pianogrid* self, psy_audio_PatternNode* node,
	uintptr_t track, uint8_t cursorkey)
{
	psy_audio_PatternEntry* patternentry;
	bool noteoff;

	assert(self);

	noteoff = FALSE;
	patternentry = psy_audio_patternnode_entry(node);
	if (psy_audio_patternentry_front(patternentry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
		psy_audio_PatternNode* prevtrack;

		// determine noterelease key
		// (note: noterelease event has no key but is drawn next to
		// the prev note or if there is none with middlec as key)
		prevtrack = psy_audio_pattern_prev_track(pianogridstate_pattern(
			self->gridstate), node, track);
		if (prevtrack) {
			// compare cursor key with the previous note key
			if (cursorkey == psy_audio_patternentry_front(
				psy_audio_patternnode_entry(prevtrack))->note) {
				noteoff = TRUE;
			}
		} else if (cursorkey == psy_audio_NOTECOMMANDS_MIDDLEC) {
			// compare cursor key with middlec key
			noteoff = TRUE;
		}
	}
	return !(psy_audio_patternentry_front(patternentry)->note != cursorkey && !noteoff);
}

void pianogrid_onmousemove(Pianogrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternEntry* oldhover;
		psy_audio_PatternNode* node;
		psy_audio_PatternNode* prev;		
		
		oldhover = self->hoverpatternentry;
		node = psy_audio_pattern_findnode(pianogridstate_pattern(self->gridstate),
			self->gridstate->cursor.track, pianogridstate_quantize(self->gridstate,
				pianogridstate_pxtobeat(self->gridstate, ev->x - self->component.scroll.x)),
			pianogridstate_step(self->gridstate), &prev);		
		if (!node ) {
			if (prev) {
				psy_audio_PatternEntry* preventry;

				preventry = psy_audio_patternnode_entry(prev);
				if (preventry->track != self->gridstate->cursor.track) {
					prev = psy_audio_pattern_prev_track(pianogridstate_pattern(self->gridstate),
						prev, self->gridstate->cursor.track);
				}
				if (psy_audio_patternentry_front(preventry)->note == psy_audio_NOTECOMMANDS_RELEASE) {
					prev = NULL;
				}
			}
			node = prev;
		}		
		if (node && pianogrid_keyhittest(self, node, self->gridstate->cursor.track,
				keyboardstate_pxtokey(self->keyboardstate, ev->y))) {
			self->hoverpatternentry = psy_audio_patternnode_entry(node);			
		} else {
			self->hoverpatternentry = NULL;
		}
		if (self->hoverpatternentry != oldhover) {
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void pianogrid_setcursor(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	psy_audio_PatternCursor oldcursor;

	assert(self);

	oldcursor = self->gridstate->cursor;
	self->gridstate->cursor = cursor;
	if (self->gridstate->cursor.track != oldcursor.track) {
		psy_ui_component_invalidate(&self->component);
	} else if (oldcursor.offset != self->gridstate->cursor.offset &&
			!self->cursorchanging) {
		pianogrid_invalidateline(self, oldcursor.offset);
		pianogrid_invalidateline(self, self->gridstate->cursor.offset);
	}
	self->cursorchanging = FALSE;
	if (psy_audio_player_playing(&self->workspace->player) && workspace_followingsong(self->workspace)) {
		bool scrolldown;

		scrolldown = self->lastplayposition <
			psy_audio_player_position(&self->workspace->player);
		pianogrid_invalidateline(self, self->lastplayposition - self->sequenceentryoffset);
		self->lastplayposition = psy_audio_player_position(&self->workspace->player);
		pianogrid_invalidateline(self, self->lastplayposition - self->sequenceentryoffset);		
		if (self->lastplayposition >= self->sequenceentryoffset &&
			self->lastplayposition < self->sequenceentryoffset +
			pianogridstate_pattern(self->gridstate)->length) {
			if (scrolldown != FALSE) {
				pianogrid_scrollright(self, self->gridstate->cursor);
			} else {
				pianogrid_scrollleft(self, self->gridstate->cursor);
			}
		}
		pianogrid_storecursor(self);
	}
}

int pianogrid_scrollright(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	int line;
	int visilines;
	psy_ui_IntSize size;

	assert(self);

	size = psy_ui_component_intsize(&self->component);
	visilines = (int)((size.width /
		(psy_dsp_big_beat_t)self->gridstate->pxperbeat) * self->gridstate->lpb);
	//if (self->midline) {
		//visilines /= 2;
	//} else {
	--visilines;
	//}
	line = pianogridstate_steps(self->gridstate, cursor.offset);
	if (visilines < line - psy_ui_component_scrollleft(&self->component) /
			pianogridstate_steppx(self->gridstate) + 2) {
		int dlines;

		dlines = line - psy_ui_component_scrollleft(&self->component) /
			pianogridstate_steppx(self->gridstate) - visilines + 16;
		self->cursorchanging = TRUE;
		psy_ui_component_setscrollleft(&self->component,
			psy_max(0, psy_ui_component_scrollleft(&self->component) +
			self->component.scrollstepx * dlines));
		return FALSE;
	}
	return TRUE;
}

void pianogrid_prevline(Pianogrid* self)
{
	assert(self);

	pianogrid_prevlines(self, workspace_cursorstep(self->workspace),
		TRUE);
}

void pianogrid_prevlines(Pianogrid* self, uintptr_t lines, int wrap)
{
	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->cursor,
			pianogridstate_pattern(self->gridstate),
			pianogridstate_step(self->gridstate), wrap);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_prevlines(&cursornavigator,
				lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace,
			pianogridstate_cursor(self->gridstate));
		pianogrid_invalidatecursor(self);
	}
}

void pianogrid_advanceline(Pianogrid* self)
{
	assert(self);

	pianogrid_advancelines(self, workspace_cursorstep(
		self->workspace), TRUE);
}

void pianogrid_advancelines(Pianogrid* self, uintptr_t lines, int wrap)
{
	assert(self);

	if (pianogridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;

		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			pianogridstate_pattern(self->gridstate),
			pianogridstate_step(self->gridstate), wrap);
		pianogrid_storecursor(self);
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			pianogrid_scrollright(self, self->gridstate->cursor);
		} else {
			pianogrid_scrollleft(self, self->gridstate->cursor);
		}
		workspace_setpatterncursor(self->workspace, self->gridstate->cursor);
		pianogrid_invalidatecursor(self);
	}
}

int pianogrid_scrollleft(Pianogrid* self, psy_audio_PatternCursor cursor)
{
	int line;
	int topline;
	psy_ui_Rectangle r;

	assert(self);

	line = pianogridstate_steps(self->gridstate, cursor.offset);
	psy_ui_setrectangle(&r,
		pianogridstate_steppx(self->gridstate) * line, 0,
		pianogridstate_steppx(self->gridstate), 200);
	/*if (self->midline) {
		psy_ui_Size gridsize;
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);
		gridsize = psy_ui_component_size(&self->component);
		topline = psy_ui_value_px(&gridsize.height, &tm) / self->linestate->lineheight / 2;
	} */
	// else {
		topline = 0;
	//}
	if (psy_ui_component_scrollleft(&self->component) + topline * pianogridstate_steppx(self->gridstate) > r.left) {
		int dlines = (psy_ui_component_scrollleft(&self->component) + topline * pianogridstate_steppx(self->gridstate) - r.left) /
			(pianogridstate_steppx(self->gridstate));
		self->cursorchanging = TRUE;
		psy_ui_component_setscrollleft (&self->component,
			psy_max(0, psy_ui_component_scrollleft(&self->component) -
			self->component.scrollstepx * dlines));
		return FALSE;
	}
	return TRUE;
}

void pianogrid_invalidateline(Pianogrid* self, psy_dsp_big_beat_t offset)
{	
	assert(self);

	if (offset >= 0.0 && offset < psy_audio_pattern_length(pianogridstate_pattern(
			self->gridstate))) {
		psy_ui_TextMetric tm;
		psy_ui_IntSize size;

		tm = psy_ui_component_textmetric(&self->component);
		size = psy_ui_intsize_init_size(
			psy_ui_component_size(&self->component), &tm);
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_rectangle_make(
				pianogridstate_beattopx(self->gridstate, pianogridstate_quantize(
					self->gridstate, offset)),
				psy_ui_component_scrolltop(&self->component),
				pianogridstate_steppx(self->gridstate),
				size.height));
	}
}

void pianogrid_invalidatecursor(Pianogrid* self)
{
	assert(self);

	pianogrid_invalidateline(self, self->oldcursor.offset);
	pianogrid_invalidateline(self, self->gridstate->cursor.offset);
}

void pianogrid_storecursor(Pianogrid* self)
{
	assert(self);

	self->oldcursor = self->gridstate->cursor;
}

// PianoBar
// implenentation
void pianobar_init(PianoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	assert(self);
	
	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;
	psy_ui_label_init(&self->keys, pianobar_base(self));
	psy_ui_label_settext(&self->keys, "Keyboard");
	zoombox_init(&self->zoombox_keyheight, &self->component);
	psy_ui_combobox_init(&self->keytype, pianobar_base(self));
	psy_ui_combobox_setcharnumber(&self->keytype, 8);	
	psy_ui_combobox_addtext(&self->keytype, "Keys");	
	psy_ui_combobox_addtext(&self->keytype, "Notes");
	psy_ui_combobox_addtext(&self->keytype, "Drums");
	psy_ui_combobox_setcursel(&self->keytype, 0);
	psy_ui_label_init(&self->tracks, pianobar_base(self));
	psy_ui_label_settext(&self->tracks, "Show Tracks");
	psy_ui_button_init(&self->tracks_all, pianobar_base(self));
	psy_ui_button_settext(&self->tracks_all, "All");
	psy_ui_button_highlight(&self->tracks_all);
	psy_ui_button_init(&self->track_curr, pianobar_base(self));
	psy_ui_button_settext(&self->track_curr, "Current");
	psy_ui_button_init(&self->tracks_active, pianobar_base(self));
	psy_ui_button_settext(&self->tracks_active, "Active");	
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(0.25),
		psy_ui_value_makeew(1.0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(pianobar_base(self), psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT, &margin));
}

// Pianoroll
// protoypes
static void pianoroll_ontimer(Pianoroll*, uintptr_t timerid);
static void pianoroll_onlpbchanged(Pianoroll*, psy_audio_Player*,
	uintptr_t lpb);
static void pianoroll_onalign(Pianoroll*);
static void pianoroll_onmousedown(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_ongridscroll(Pianoroll*, psy_ui_Component* sender);
static void pianoroll_oneventdriverinput(Pianoroll* self, psy_EventDriver* sender);
static void pianoroll_onzoomboxbeatwidthchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onzoomboxkeyheightchanged(Pianoroll*, ZoomBox* sender);
static void pianoroll_onkeytypeselchange(Pianoroll*, psy_ui_Component* sender, int sel);
static void pianoroll_ondisplayalltracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_ondisplaycurrenttrack(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_ondisplayactivetracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_updatetrackdisplaybuttons(Pianoroll*);
static void pianoroll_onskinchanged(Pianoroll*, Workspace*);
static bool pianoroll_handlecommand(Pianoroll*, int cmd);
static void pianoroll_updateskin(Pianoroll*);
static void pianoroll_onpatterncursorchanged(Pianoroll*, Workspace* sender);

// vtable
static psy_ui_ComponentVtable pianoroll_vtable;
static bool pianoroll_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianoroll_vtable_init(Pianoroll* self)
{
	assert(self);

	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onalign = (psy_ui_fp_component_onalign)
			pianoroll_onalign;
		pianoroll_vtable.onmousedown = (psy_ui_fp_component_onmousedown)
			pianoroll_onmousedown;		
		pianoroll_vtable.ontimer = (psy_ui_fp_component_ontimer)
			pianoroll_ontimer;
		pianoroll_vtable_initialized = TRUE;
	}
	return &pianoroll_vtable;
}
// implenentation
void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	PatternViewSkin* skin, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, pianoroll_vtable_init(self));
	self->workspace = workspace;
	self->opcount = 0;
	self->syncpattern = 1;	
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_BACKGROUND_NONE);		
	// shared states
	keyboardstate_init(&self->keyboardstate, skin);
	gridstate_init(&self->gridstate, skin);	
	// left area (keyboardheader, keyboard)
	psy_ui_component_init(&self->left, &self->component);
	psy_ui_component_enablealign(&self->left);
	psy_ui_component_setalign(&self->left, psy_ui_ALIGN_LEFT);
	zoombox_init(&self->zoombox_beatwidth, &self->left);
	psy_signal_connect(&self->zoombox_beatwidth.signal_changed, self,
		pianoroll_onzoomboxbeatwidthchanged);
	psy_ui_component_setalign(zoombox_base(&self->zoombox_beatwidth), psy_ui_ALIGN_TOP);
	// Keyboard
	pianokeyboard_init(&self->keyboard, &self->left, &self->keyboardstate);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_CLIENT);	
	// top area (beatruler)
	psy_ui_component_init(&self->top, &self->component);
	psy_ui_component_enablealign(&self->top);
	psy_ui_component_setalign(&self->top, psy_ui_ALIGN_TOP);
	pianoruler_init(&self->header, &self->top, &self->gridstate);
	psy_ui_component_setalign(pianoruler_base(&self->header), psy_ui_ALIGN_TOP);
	// client area (event grid)
	pianogrid_init(&self->grid, &self->component, &self->keyboardstate,
		&self->gridstate, self->workspace);
	psy_ui_scroller_init(&self->scroller, pianogrid_base(&self->grid),
		&self->component);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	// bar
	pianobar_init(&self->bar, &self->scroller.component, self->workspace);	
	psy_ui_component_setalign(&self->bar.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->bar.zoombox_keyheight.signal_changed, self,
		pianoroll_onzoomboxkeyheightchanged);
	psy_ui_component_setalign(&self->bar.zoombox_keyheight.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_onlpbchanged);
	psy_signal_connect(&pianogrid_base(&self->grid)->signal_scroll, self,
		pianoroll_ongridscroll);
	psy_signal_connect(&self->bar.keytype.signal_selchanged, self,
		pianoroll_onkeytypeselchange);
	psy_signal_connect(&self->bar.tracks_all.signal_clicked, self,
		pianoroll_ondisplayalltracks);
	psy_signal_connect(&self->bar.track_curr.signal_clicked, self,
		pianoroll_ondisplaycurrenttrack);
	psy_signal_connect(&self->bar.tracks_active.signal_clicked, self,
		pianoroll_ondisplayactivetracks);
	psy_signal_connect(&self->workspace->player.eventdrivers.signal_input, self,
		pianoroll_oneventdriverinput);
	psy_signal_connect(&self->workspace->signal_skinchanged, self,
		pianoroll_onskinchanged);
	psy_signal_connect(&self->workspace->signal_patterncursorchanged,
		self, pianoroll_onpatterncursorchanged);
	pianoroll_updateskin(self);
	psy_ui_component_starttimer(&self->component, 0, PIANOROLL_REFRESHRATE);
}

void pianoroll_setpattern(Pianoroll* self, psy_audio_Pattern* pattern)
{
	assert(self);

	pianogridstate_setpattern(&self->gridstate, pattern);
	pianogrid_onpatternchange(&self->grid, pattern);
	psy_ui_component_setscroll(pianogrid_base(&self->grid),
		psy_ui_intpoint_make(0, psy_ui_component_scrolltop(
			pianogrid_base(&self->grid))));
	psy_ui_component_setscroll(pianoruler_base(&self->header),
		psy_ui_intpoint_zero());
	pianoroll_updatescroll(self);
}

void pianoroll_ontimer(Pianoroll* self, uintptr_t timerid)
{
	assert(self);

	if (pianogridstate_pattern(&self->gridstate) &&
			psy_ui_component_visible(&self->component)) {
		if (psy_audio_player_playing(&self->workspace->player)) {
			int line;
			line = (int)(psy_audio_player_position(&self->workspace->player) /
				self->gridstate.lpb);
			if (self->grid.lastplayposition / self->gridstate.lpb != line) {
				pianogrid_invalidateline(&self->grid,
					self->grid.lastplayposition - self->grid.sequenceentryoffset);
				self->grid.lastplayposition =
					psy_audio_player_position(&self->workspace->player);
				pianogrid_invalidateline(&self->grid,
					self->grid.lastplayposition - self->grid.sequenceentryoffset);
			}
		} else if (self->grid.lastplayposition != -1) {
			pianogrid_invalidateline(&self->grid,
				self->grid.lastplayposition - self->grid.sequenceentryoffset);
			self->grid.lastplayposition = -1;
		}		
		if (pianogridstate_pattern(&self->gridstate) &&
				pianogridstate_pattern(&self->gridstate)->opcount != self->opcount &&
				self->syncpattern) {
			psy_ui_component_invalidate(pianogrid_base(&self->grid));
		}
		self->opcount = pianogridstate_pattern(&self->gridstate)
			? pianogridstate_pattern(&self->gridstate)->opcount
			: 0;
	}
}

void pianoroll_onalign(Pianoroll* self)
{
	assert(self);

	pianoroll_updatescroll(self);
}

void pianoroll_onmousedown(Pianoroll* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_setfocus(pianogrid_base(&self->grid));
}

void pianoroll_updatescroll(Pianoroll* self)
{	
	assert(self);

	pianogrid_updatekeystate(&self->grid);
	psy_ui_component_setscrollstep(pianogrid_base(&self->grid),
		pianogridstate_steppx(&self->gridstate),
		self->keyboardstate.keyheightpx);	
	psy_ui_component_setscrollstepy(pianokeyboard_base(&self->keyboard),
		self->keyboardstate.keyheightpx);
	psy_ui_component_setscrollstepx(pianoruler_base(&self->header),
		pianogridstate_steppx(&self->gridstate));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianokeyboard_base(&self->keyboard));
}

void pianoroll_onlpbchanged(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	assert(self);

	pianogridstate_setlpb(&self->gridstate, lpb);
	psy_ui_component_setscrollstepx(pianogrid_base(&self->grid),
		pianogridstate_steppx(&self->gridstate));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoruler_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_update(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_onpatterncursorchanged(Pianoroll* self,
	Workspace* sender)
{	
	assert(self);

	pianogrid_setcursor(&self->grid, workspace_patterncursor(self->workspace));
}

void pianoroll_ongridscroll(Pianoroll* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_ui_component_scrollleft(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrollleft(pianoruler_base(&self->header))) {
		psy_ui_component_setscrollleft(pianoruler_base(&self->header),
			psy_ui_component_scrollleft(pianogrid_base(&self->grid)));		
	}
	if (psy_ui_component_scrolltop(pianogrid_base(&self->grid)) !=
			psy_ui_component_scrolltop(&self->keyboard.component)) {
		psy_ui_component_setscrolltop(&self->keyboard.component,
			psy_ui_component_scrolltop(pianogrid_base(&self->grid)));		
	}
}

void pianoroll_onzoomboxbeatwidthchanged(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	pianogridstate_setzoom(&self->gridstate, zoombox_rate(sender));
	psy_ui_component_setscrollstepx(pianogrid_base(&self->grid),
		pianogridstate_steppx(&self->gridstate));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrollleft(pianoruler_base(&self->header),
		psy_ui_component_scrollleft(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_update(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_onzoomboxkeyheightchanged(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	self->keyboardstate.keyheight = psy_ui_mul_value_real(
		self->keyboardstate.defaultkeyheight, zoombox_rate(sender));
	pianogrid_updatekeystate(&self->grid);
	psy_ui_component_setscrollstepy(pianogrid_base(&self->grid),
		self->keyboardstate.keyheightpx);
	psy_ui_component_setscrollstepy(pianokeyboard_base(&self->keyboard),
		self->keyboardstate.keyheightpx);
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_setscrolltop(&self->keyboard.component,
		psy_ui_component_scrolltop(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_update(&self->keyboard.component);
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_onkeytypeselchange(Pianoroll* self, psy_ui_Component* sender, int sel)
{
	assert(self);

	if (sel >= 0 && sel < KEYBOARDTYPE_NUM) {
		pianokeyboard_setkeyboardtype(&self->keyboard, (KeyboardType)sel);
		psy_ui_component_align(pianoroll_base(self));
	}	
}

void pianoroll_ondisplayalltracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid,
		PIANOROLL_TRACK_DISPLAY_ALL);	
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_ondisplaycurrenttrack(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid,
		PIANOROLL_TRACK_DISPLAY_CURRENT);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_ondisplayactivetracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid,
		PIANOROLL_TRACK_DISPLAY_ACTIVE);
	pianoroll_updatetrackdisplaybuttons(self);
}

void pianoroll_updatetrackdisplaybuttons(Pianoroll* self)
{
	assert(self);

	psy_ui_button_disablehighlight(&self->bar.tracks_all);
	psy_ui_button_disablehighlight(&self->bar.track_curr);
	psy_ui_button_disablehighlight(&self->bar.tracks_active);
	switch (pianogrid_trackdisplay(&self->grid)) {
		case PIANOROLL_TRACK_DISPLAY_ALL:
			psy_ui_button_highlight(&self->bar.tracks_all);			
			break;
		case PIANOROLL_TRACK_DISPLAY_CURRENT:
			psy_ui_button_highlight(&self->bar.track_curr);
			break;
		case PIANOROLL_TRACK_DISPLAY_ACTIVE:			
			psy_ui_button_highlight(&self->bar.tracks_active);
			break;
		default:			
			break;
	}	
}

void pianoroll_onskinchanged(Pianoroll* self, Workspace* sender)
{
	assert(self);

	pianoroll_updateskin(self);
}

void pianoroll_updateskin(Pianoroll* self)
{
	assert(self);

	if (self->gridstate.skin) {		
		psy_ui_component_setbackgroundcolour(pianoruler_base(&self->header),
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		psy_ui_component_setbackgroundcolour(zoombox_base(&self->zoombox_beatwidth),
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		psy_ui_component_setbackgroundcolour(&self->scroller.hscroll.sliderpane.component,
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		psy_ui_component_setbackgroundcolour(&self->scroller.vscroll.sliderpane.component,
			patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
		// psy_ui_component_setbackgroundcolour(pianobar_base(&self->bar),
		// patternviewskin_backgroundcolour(self->gridstate.skin, 0, 0));
	}
}

// Handle EventDriver Input
void pianoroll_oneventdriverinput(Pianoroll* self, psy_EventDriver* sender)
{
	assert(self);

	if (psy_ui_component_hasfocus(pianogrid_base(&self->grid))) {
		psy_EventDriverCmd cmd;

		cmd = psy_eventdriver_getcmd(sender, "pianoroll");
		if (cmd.id != -1) {
			pianoroll_handlecommand(self, cmd.id);
		}
	}
}

void pianoroll_makecmds(psy_Property* parent)
{
	psy_Property* cmds;

	assert(parent);

	cmds = psy_property_settext(psy_property_append_section(parent,
		"pianoroll"), "Pianoroll");
	definecmd(cmds, CMD_NAVUP,
		psy_ui_KEY_UP, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navup", "up");
	definecmd(cmds, CMD_NAVDOWN,
		psy_ui_KEY_DOWN, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navdown", "down");
	definecmd(cmds, CMD_NAVLEFT,
		psy_ui_KEY_LEFT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navleft", "left");
	definecmd(cmds, CMD_NAVRIGHT,
		psy_ui_KEY_RIGHT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navright", "right");
	definecmd(cmds, CMD_ENTER,
		psy_ui_KEY_SPACE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"enter", "enter");
	definecmd(cmds, CMD_ROWCLEAR,
		psy_ui_KEY_DELETE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"rowclear", "clr row");
}

bool pianoroll_handlecommand(Pianoroll* self, int cmd)
{
	bool handled;

	assert(self);
	
	handled = TRUE;
	switch (cmd) {
		case CMD_NAVUP:
			if (self->gridstate.cursor.key < self->keyboardstate.keymax - 1) {
				pianogrid_storecursor(&self->grid);
				++self->gridstate.cursor.key;
				workspace_setpatterncursor(self->workspace,
					self->gridstate.cursor);
				pianogrid_invalidatecursor(&self->grid);
			}
			break;
		case CMD_NAVDOWN:
			if (self->gridstate.cursor.key > self->keyboardstate.keymin) {
				pianogrid_storecursor(&self->grid);
				--self->gridstate.cursor.key;
				workspace_setpatterncursor(self->workspace,
					self->gridstate.cursor);
				pianogrid_invalidatecursor(&self->grid);
			}
			break;
		case CMD_NAVLEFT:
			pianogrid_prevline(&self->grid);		
			break;
		case CMD_NAVRIGHT:
			pianogrid_advanceline(&self->grid);
			break;
		case CMD_ENTER: {
			psy_audio_PatternEvent patternevent;

			psy_audio_patternevent_clear(&patternevent);
			patternevent.note = self->gridstate.cursor.key;
			psy_undoredo_execute(&self->workspace->undoredo,
				&InsertCommandAlloc(pianogridstate_pattern(&self->gridstate),
					pianogridstate_step(&self->gridstate),
					self->gridstate.cursor, patternevent,
					self->workspace)->command);
			pianogrid_advanceline(&self->grid);
			break; }
		case CMD_ROWCLEAR:		
			psy_undoredo_execute(&self->workspace->undoredo,
				&RemoveCommandAlloc(self->gridstate.pattern,
					pianogridstate_step(&self->gridstate),
					self->gridstate.cursor, self->workspace)->command);
				pianogrid_advanceline(&self->grid);
			break;		
		default:
			handled = FALSE;
			break;
	}
	return handled;
}

// Defines a property with shortcut defaults for the keyboard driver
// key		: cmd id used by the trackerview
// text		: "cmds.key" language dictionary key used by the translator
// shorttext: short description for the keyboard help view
// value	: encoded key shortcut (keycode/shift/ctrl)
void definecmd(psy_Property* cmds, int cmd, uintptr_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	assert(cmds);

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_sethint(psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(cmds, key,
			psy_audio_encodeinput(keycode, shift, ctrl), 0, 0),
			cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}
