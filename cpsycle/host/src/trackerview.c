// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// prefix file for each .c file
#include "../../detail/prefix.h"

#include "trackerview.h"
// local
#include "cmdsnotes.h"
#include "patterncmds.h"
#include "skingraphics.h"
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

static void setcmdall(psy_Property*, int cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext);
static void setcmd(psy_Property*, int cmd, uint32_t keycode,
	const char* key, const char* shorttext);
static void setcolumncolour(PatternViewSkin*, psy_ui_Graphics*,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks);
static const char* notetostr(psy_audio_PatternEvent ev, psy_dsp_NotesTabMode notestabmode,
	bool showemptydate)
{
	static const char* emptynotestr = "- - -";

	if (ev.note != psy_audio_NOTECOMMANDS_EMPTY || !showemptydate) {
		return psy_dsp_notetostr(ev.note, notestabmode);
	}
	return emptynotestr;
}

static int keycodetoint(uint32_t keycode)
{
	if (keycode >= '0' && keycode <= '9') {
		return keycode - '0';
	} else if (keycode >= 'A' && keycode <= 'Z') {
		return keycode - 'A' + 10;
	}
	return -1;
}

static void enterdigit(int digit, int newval, unsigned char* val)
{
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else if (digit == 1) {
		*val = (*val & 0xF0) | (newval & 0x0F);
	}
}

static void lohi(uint8_t* value, int digit, uint8_t* lo, uint8_t* hi)
{
	*lo = *value & 0x0F;
	*hi = (*value & 0xF0) >> 4;
}

static void digitlohi(int value, int digit, uintptr_t size, uint8_t* lo, uint8_t* hi)
{
	uintptr_t pos;

	pos = (size - 1) - digit / 2;
	lohi((uint8_t*)&value + pos, digit, lo, hi);
}

static void entervaluecolumn(psy_audio_PatternEntry* entry, intptr_t column, intptr_t value)
{
	psy_audio_PatternEvent* ev;

	assert(entry);

	ev = psy_audio_patternentry_front(entry);
	switch (column) {
		case TRACKER_COLUMN_INST:
			ev->inst = (uint16_t)value;
			break;
		case TRACKER_COLUMN_MACH:
			ev->mach = (uint8_t)value;
			break;
		case TRACKER_COLUMN_VOL:
			ev->vol = (uint16_t)value;
			break;
		case TRACKER_COLUMN_CMD:
			ev->cmd = (uint8_t)value;
			break;
		case TRACKER_COLUMN_PARAM:
			ev->parameter = (uint8_t)value;
			break;
		default:
			break;
	}
}

// TrackerGridColumn
// prototypes
static void trackergridcolumn_ondraw(TrackerGridColumn*, psy_ui_Graphics*);
static void trackergridcolumn_drawtrackevents(TrackerGridColumn*, psy_ui_Graphics*);
static  TrackerColumnFlags trackergridcolumn_columnflags(TrackerGridColumn*,
	uintptr_t line, uintptr_t track, psy_dsp_big_beat_t offset);
static void trackergridcolumn_drawentry(TrackerGridColumn*,
	psy_ui_Graphics* g, psy_audio_PatternEntry* entry, double y,
	TrackerColumnFlags columnflags);
static void trackergridcolumn_drawdigit(TrackerGridColumn*, psy_ui_Graphics*,
	psy_ui_RealPoint cp, const char* str);
static void trackergridcolumn_drawresizebar(TrackerGridColumn*, psy_ui_Graphics*);
static void trackergridcolumn_onmousedown(TrackerGridColumn*,
	psy_ui_MouseEvent*);
static void trackergridcolumn_onmousemove(TrackerGridColumn*,
	psy_ui_MouseEvent*);
static void trackergridcolumn_onmouseup(TrackerGridColumn*,
	psy_ui_MouseEvent*);
static psy_audio_PatternCursor trackergridcolumn_makecursor(TrackerGridColumn*,
	psy_ui_RealPoint);
static psy_dsp_big_beat_t trackergridcolumn_currseqoffset(TrackerGridColumn*);
static void trackergridcolumn_onpreferredsize(TrackerGridColumn*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

// vtable
static psy_ui_ComponentVtable trackergridcolumn_vtable;
static bool trackergridcolumn_vtable_initialized = FALSE;

static void trackergridcolumn_vtable_init(TrackerGridColumn* self)
{
	if (!trackergridcolumn_vtable_initialized) {
		trackergridcolumn_vtable = *(self->component.vtable);
		trackergridcolumn_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackergridcolumn_ondraw;
		trackergridcolumn_vtable.onpreferredsize =
			(psy_ui_fp_component_onpreferredsize)
			trackergridcolumn_onpreferredsize;
		trackergridcolumn_vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			trackergridcolumn_onmousedown;
		trackergridcolumn_vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			trackergridcolumn_onmousemove;
		trackergridcolumn_vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			trackergridcolumn_onmouseup;
		trackergridcolumn_vtable_initialized = TRUE;
	}
	self->component.vtable = &trackergridcolumn_vtable;
}

void trackergridcolumn_init(TrackerGridColumn* self, psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerGridState* gridstate,
	TrackerLineState* linestate, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	trackergridcolumn_vtable_init(self);
	psy_ui_component_setalign(trackergridcolumn_base(self), psy_ui_ALIGN_LEFT);
	self->gridstate = gridstate;
	self->linestate = linestate;
	self->workspace = workspace;	
	self->index = index;
	self->trackdef = NULL;	
}

TrackerGridColumn* trackergridcolumn_alloc(void)
{
	return (TrackerGridColumn*)malloc(sizeof(TrackerGridColumn));
}

TrackerGridColumn* trackergridcolumn_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, uintptr_t index, TrackerGridState* gridstate,
	TrackerLineState* linestate, Workspace* workspace)
{
	TrackerGridColumn* rv;

	rv = trackergridcolumn_alloc();
	if (rv) {
		trackergridcolumn_init(rv, parent, view, index, gridstate, linestate,
			workspace);
		psy_ui_component_deallocateafterdestroyed(trackergridcolumn_base(rv));
	}
	return rv;
}

void trackergridcolumn_ondraw(TrackerGridColumn* self, psy_ui_Graphics* g)
{	
	trackergridcolumn_drawtrackevents(self, g);
	if (self->gridstate->trackconfig->colresize &&
			self->gridstate->trackconfig->resizetrack == self->index) {
		trackergridcolumn_drawresizebar(self, g);		
	}
}

void trackergridcolumn_drawtrackevents(TrackerGridColumn* self, psy_ui_Graphics* g)
{	
	psy_List** events;
	psy_List* p;
	double cpy;
	double offset;
	uintptr_t line;	
	
	events = trackereventtable_track(&self->gridstate->trackevents,
		self->index);
	self->trackdef = trackergridstate_trackdef(self->gridstate, self->index);
	self->digitsize = psy_ui_realsize_make(
		self->gridstate->trackconfig->textwidth,
		self->linestate->lineheightpx - 1);
	for (p = *events,
			offset = self->gridstate->trackevents.clip.topleft.offset,
			cpy = trackerlinestate_beattopx(self->linestate, offset),
			line = self->gridstate->trackevents.clip.topleft.line;
			p != NULL;
			p = p->next, ++line, cpy += self->linestate->lineheightpx,
			offset += trackerlinestate_bpl(self->linestate)) {				
		trackergridcolumn_drawentry(self, g, (psy_audio_PatternEntry*)p->entry,
			cpy, trackergridcolumn_columnflags(self,
				line - trackerlinestate_beattoline(self->linestate,
					self->gridstate->trackevents.seqoffset),
				self->index, offset));

	}
	self->trackdef = NULL;
}

TrackerColumnFlags trackergridcolumn_columnflags(TrackerGridColumn* self,
	uintptr_t line, uintptr_t track, psy_dsp_big_beat_t offset)
{
	TrackerColumnFlags rv;	

	assert(self);
	
	if (self->gridstate->drawbeathighlights) {
		uintptr_t lpb;

		lpb = trackerlinestate_lpb(self->linestate);
		rv.beat = (line % lpb) == 0;
		rv.beat4 = (line % (lpb * 4)) == 0;		
		rv.mid = self->gridstate->midline &&
			(line == trackerlinestate_midline(self->linestate,
			psy_ui_component_scrolltoppx(psy_ui_component_parent(
				&self->component))));
	} else {
		rv.beat = 0;
		rv.beat4 = 0;
		rv.mid = 0;
	}
	rv.cursor = (self->gridstate->trackevents.currcursorline == line) &&
		(track == self->gridstate->cursor.track) &&
		self->linestate->drawcursor;
	rv.selection = psy_audio_patternselection_test(
		&self->gridstate->selection, track, offset);
	rv.playbar = trackergridstate_hasplaybar(self->gridstate) &&
		psy_audio_player_playing(workspace_player(self->workspace)) &&
		(self->gridstate->trackevents.currplaybarline == line);
	rv.focus = TRUE;
	return rv;
}

void trackergridcolumn_drawentry(TrackerGridColumn* self, psy_ui_Graphics* g,
	psy_audio_PatternEntry* entry, double y, TrackerColumnFlags columnflags)
{	
	psy_audio_PatternEvent* event;
	TrackerColumnFlags currcolumnflags;		
	uintptr_t column;
	const char* notestr;
	psy_ui_RealPoint cp;
	psy_ui_RealPoint cp_leftedge;	

	if (!entry) {
		entry = &self->gridstate->empty;
	}
	event = psy_audio_patternentry_front(entry);	
	currcolumnflags = columnflags;	
	currcolumnflags.cursor &= self->gridstate->cursor.column == 0;
	setcolumncolour(self->gridstate->skin, g, currcolumnflags, entry->track,
		trackergridstate_numsongtracks(self->gridstate));
	cp_leftedge = cp = psy_ui_realpoint_make(
		self->gridstate->trackconfig->patterntrackident, y);
	cp_leftedge.x += self->gridstate->trackconfig->textleftedge;
	// draw note
	notestr = notetostr(*event, psy_dsp_NOTESTAB_A440, FALSE);
	psy_ui_textoutrectangle(g,
		cp_leftedge, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		psy_ui_realrectangle_make(cp, psy_ui_realsize_make(
			self->digitsize.width * 3.0, self->digitsize.height)),
		notestr, psy_strlen(notestr));
	cp.x += trackdef_columnwidth(self->trackdef, 0, self->digitsize.width);
	// draw digit columns
	for (column = 1; column < trackdef_numcolumns(self->trackdef); ++column) {
		TrackColumnDef* coldef;
		uintptr_t digit;
		uintptr_t value;
		bool empty;
		uintptr_t num;
		const char* digitstr = NULL;

		coldef = trackdef_columndef(self->trackdef, column);
		if (!coldef) {
			continue;
		}
		value = trackdef_value(self->trackdef, column, entry);
		empty = coldef->emptyvalue == value;
		if (empty) {
			digitstr = (self->gridstate->showemptydata) ? "." : "";
		}
		if (column > TRACKER_COLUMN_VOL) {
			intptr_t cmd;
			psy_List* ev;

			cmd = (column - (int)TRACKER_COLUMN_VOL - 1) / 2;
			ev = psy_list_at(entry->events, cmd);
			if (ev && psy_audio_patternevent_tweakvalue(
					(psy_audio_PatternEvent*)(ev->entry)) != 0) {
				empty = 0;
			}
		}		
		currcolumnflags.cursor = columnflags.cursor && (column ==
			self->gridstate->cursor.column);
		for (num = coldef->numdigits, digit = 0; digit < num; ++digit) {			
			if (columnflags.cursor) {
				currcolumnflags.cursor = columnflags.cursor &&
					(self->gridstate->cursor.column == column) &&
					(self->gridstate->cursor.digit == digit);
				setcolumncolour(self->gridstate->skin, g, currcolumnflags,
					entry->track, trackergridstate_numsongtracks(self->gridstate));
			}
			if (!empty) {
				digitstr = hex_tab[((value >> ((num - digit - 1) * 4)) & 0x0F)];
			}
			trackergridcolumn_drawdigit(self, g, cp, digitstr);
			cp.x += self->digitsize.width;
		}
		cp.x += coldef->marginright;
	}	
}

void trackergridcolumn_drawdigit(TrackerGridColumn* self, psy_ui_Graphics* g,
	psy_ui_RealPoint cp, const char* str)
{
	psy_ui_RealPoint cp_leftedge;

	cp_leftedge = cp;
	cp_leftedge.x += self->gridstate->trackconfig->textleftedge;
	psy_ui_textoutrectangle(g,
		cp_leftedge, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		psy_ui_realrectangle_make(cp, self->digitsize),
		str, psy_strlen(str));
}

void trackergridcolumn_drawresizebar(TrackerGridColumn* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	psy_ui_drawsolidrectangle(g,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(size.width - 3, 0),
			psy_ui_realsize_make(3.0, size.height)),
		psy_ui_colour_white());
}

void trackergridcolumn_onmousedown(TrackerGridColumn* self, psy_ui_MouseEvent* ev)
{
	if (ev->button == 1) {
		psy_ui_RealSize size;

		size = psy_ui_component_size_px(&self->component);
		if (ev->pt.x > size.width - 5) {
			self->gridstate->trackconfig->resizetrack = self->index;
			self->gridstate->trackconfig->colresize = TRUE;
			self->resizestartsize = size;
			self->gridstate->trackconfig->resizesize = size;
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_capture(&self->component);
		} else {
			if (psy_audio_patternselection_valid(&self->gridstate->selection)) {
				psy_audio_patternselection_disable(&self->gridstate->selection);
				psy_ui_component_invalidate(&self->component);
			}
			self->gridstate->dragselectionbase = trackergridcolumn_makecursor(
				self, ev->pt);
		}
	}
}

void trackergridcolumn_onmousemove(TrackerGridColumn* self, psy_ui_MouseEvent* ev)
{
	psy_ui_RealSize size;

	size = psy_ui_component_size_px(&self->component);
	if (self->gridstate->trackconfig->colresize) {
		double basewidth;

		self->trackdef = trackergridstate_trackdef(self->gridstate,
			self->index);
		basewidth = trackdef_basewidth(self->trackdef,
			self->gridstate->trackconfig->textwidth);
		if (ev->pt.x > basewidth) {
			self->gridstate->trackconfig->resizesize.width = ev->pt.x;
			psy_ui_component_align(psy_ui_component_parent(&self->component));
			psy_ui_component_invalidate(psy_ui_component_parent(
				&self->component));
		}
	} else {
		self->gridstate->dragcursor =
			trackergridstate_checkcursorbounds(self->gridstate,
			trackergridcolumn_makecursor(self, ev->pt));
	}	
	if (self->gridstate->showresizecursor &&
			(ev->pt.x > size.width - 5)) {
		psy_ui_component_setcursor(&self->component,
			psy_ui_CURSORSTYLE_COL_RESIZE);
	}
}

void trackergridcolumn_onmouseup(TrackerGridColumn* self, psy_ui_MouseEvent* ev)
{
	psy_ui_component_releasecapture(&self->component);
	if (self->gridstate->trackconfig->colresize &&
			self->gridstate->trackconfig->resizesize.width > 0.0) {
		double basewidth;
		uintptr_t numfx;

		self->trackdef = trackergridstate_trackdef(self->gridstate, self->index);
		basewidth = trackdef_basewidth(self->trackdef, self->gridstate->trackconfig->textwidth);		
		numfx = (uintptr_t)psy_max(1.0, 
			(psy_max(0.0, self->gridstate->trackconfig->resizesize.width - basewidth)) /
			(self->gridstate->trackconfig->textwidth * 4.0));		
		if (self->trackdef != &self->gridstate->trackconfig->trackdef) {
			self->trackdef->numfx = numfx;
		} else if (numfx > 1) {
			self->trackdef = malloc(sizeof(TrackDef));					
			if (self->trackdef) {
				trackdef_init(self->trackdef);
				self->trackdef->numfx = numfx;
				psy_table_insert(&self->gridstate->trackconfig->trackconfigs, self->index,
					self->trackdef);
			}
		}				
	}	
	self->gridstate->trackconfig->resizetrack = psy_INDEX_INVALID;
	psy_ui_component_align(psy_ui_component_parent(&self->component));
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
}

psy_audio_PatternCursor trackergridcolumn_makecursor(TrackerGridColumn* self,
	psy_ui_RealPoint pt)
{
	psy_audio_PatternCursor rv;
	TrackDef* trackdef;	
	double cpx;	

	rv.seqoffset = (self->linestate->singlemode)
		? 0.0
		: trackergridcolumn_currseqoffset(self);
	rv.offset = trackerlinestate_pxtobeat(self->linestate, pt.y) - rv.seqoffset;
	rv.line = trackerlinestate_beattoline(self->linestate, rv.offset);
	rv.lpb = trackerlinestate_lpb(self->linestate);
	if (trackergridstate_pattern(self->gridstate) &&
			rv.offset >= psy_audio_pattern_length(trackergridstate_pattern(self->gridstate))) {
		if (self->linestate->singlemode) {
			rv.offset = psy_audio_pattern_length(trackergridstate_pattern(self->gridstate)) -
				trackerlinestate_bpl(self->linestate);
		}
	}
	rv.track = self->index;		
	rv.column = 0;
	rv.digit = 0;
	rv.key = self->workspace->patterneditposition.key;
	trackdef = trackergridstate_trackdef(self->gridstate, rv.track);
	cpx = 0;	
	while (rv.column < trackdef_numcolumns(trackdef) &&
			cpx + trackdef_columnwidth(trackdef, rv.column,
				self->gridstate->trackconfig->textwidth) < pt.x) {
		cpx += trackdef_columnwidth(trackdef, rv.column,
			self->gridstate->trackconfig->textwidth);
		++rv.column;
	}
	rv.digit = (uintptr_t)((pt.x - cpx) /
		self->gridstate->trackconfig->textwidth);
	if (rv.digit >= trackdef_numdigits(trackdef, rv.column)) {
		rv.digit = trackdef_numdigits(trackdef, rv.column) - 1;
	}
	self->gridstate->cursor.patternid =
		workspace_patterncursor(self->workspace).patternid;
	return rv;
}

psy_dsp_big_beat_t trackergridcolumn_currseqoffset(TrackerGridColumn* self)
{
	psy_audio_SequenceEntry* entry;

	entry = psy_audio_sequence_entry(self->gridstate->sequence,
		workspace_sequenceeditposition(self->workspace));
	if (entry) {
		return entry->offset;
	}
	return 0.0;
}

void trackergridcolumn_onpreferredsize(TrackerGridColumn* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (self->gridstate->trackconfig->colresize &&
			self->gridstate->trackconfig->resizetrack == self->index) {
		rv->width = psy_ui_value_make_px(self->gridstate->trackconfig->resizesize.width);
	} else {
		rv->width = psy_ui_value_make_px(trackergridstate_trackwidth(
			self->gridstate, self->index));		
	}
	if (self->linestate->maxlines == psy_INDEX_INVALID) {
		rv->height = psy_ui_value_make_px(
			(trackerlinestate_numlines(self->linestate)) *
			self->linestate->lineheightpx);
	} else {
		rv->height = psy_ui_value_make_px((self->linestate->maxlines) *
			self->linestate->lineheightpx);
	}
}

// TrackerGrid
// prototypes
static void trackergrid_ondestroy(TrackerGrid*);
static void trackergrid_init_signals(TrackerGrid*);
static void trackergrid_dispose_signals(TrackerGrid*);
static void trackergrid_ondraw(TrackerGrid*, psy_ui_Graphics*);
static void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g,
	const psy_audio_PatternSelection* clip);
static void trackergrid_updatetrackevents(TrackerGrid*,
	const psy_audio_PatternSelection* clip);
static void trackergrid_onalign(TrackerGrid*);
static void trackergrid_onkeydown(TrackerGrid*, psy_ui_KeyEvent*);
static void trackergrid_onkeyup(TrackerGrid*, psy_ui_KeyEvent*);
static void trackergrid_onmousedown(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, psy_ui_MouseEvent*);
static void trackergrid_dragselection(TrackerGrid*, psy_audio_PatternCursor);
static void trackergrid_onscroll(TrackerGrid*, psy_ui_Component* sender);
static void trackergrid_clearmidline(TrackerGrid*);
static bool trackergrid_movecursorwhenpaste(TrackerGrid*);
static void trackergrid_inputevent(TrackerGrid*, const psy_audio_PatternEvent*,
	bool chordmode);
static void trackergrid_enterdigitcolumn(TrackerGrid*, psy_audio_PatternEntry*,
	psy_audio_PatternCursor, intptr_t digitvalue);
static void trackergrid_inputvalue(TrackerGrid*, intptr_t value, intptr_t digit);
static void trackergrid_prevtrack(TrackerGrid*);
static void trackergrid_nexttrack(TrackerGrid*);
static void trackergrid_prevline(TrackerGrid*);
static void trackergrid_advanceline(TrackerGrid*);
static void trackergrid_prevlines(TrackerGrid*, uintptr_t lines, bool wrap);
static void trackergrid_advancelines(TrackerGrid*, uintptr_t lines, bool wrap);
static void trackergrid_home(TrackerGrid*);
static void trackergrid_end(TrackerGrid*);
static void trackergrid_rowdelete(TrackerGrid*);
static void trackergrid_rowclear(TrackerGrid*);
static void trackergrid_prevcol(TrackerGrid*);
static void trackergrid_nextcol(TrackerGrid*);
static void trackergrid_selectall(TrackerGrid*);
static void trackergrid_selectcol(TrackerGrid*);
static void trackergrid_selectmachine(TrackerGrid*);
static void trackergrid_oninput(TrackerGrid*, psy_audio_Player*,
	psy_audio_PatternEvent*);
static void trackergrid_setdefaultevent(TrackerGrid*,
	psy_audio_Pattern* defaultpattern, psy_audio_PatternEvent*);
static void trackergrid_enablepatternsync(TrackerGrid*);
static void trackergrid_preventpatternsync(TrackerGrid*);
static void trackergrid_resetpatternsync(TrackerGrid*);
static void trackergrid_ongotocursor(TrackerGrid*, psy_audio_PatternCursor*);
static psy_dsp_big_beat_t trackergrid_currseqoffset(TrackerGrid*);
static psy_audio_OrderIndex trackergrid_checkupdatecursorseqoffset(
	TrackerGrid*, psy_audio_PatternCursor* rv);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(TrackerGrid* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			trackergrid_ondestroy;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			trackergrid_ondraw;
		vtable.onkeydown =
			(psy_ui_fp_component_onkeyevent)
			trackergrid_onkeydown;
		vtable.onkeyup =
			(psy_ui_fp_component_onkeyevent)
			trackergrid_onkeyup;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmousedown;
		vtable.onmousemove =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmousemove;
		vtable.onmouseup =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmouseup;
		vtable.onmousedoubleclick =
			(psy_ui_fp_component_onmouseevent)
			trackergrid_onmousedoubleclick;		
		vtable.onalign =
			(psy_ui_fp_component_onalign)
			trackergrid_onalign;		
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void trackergrid_init(TrackerGrid* self, psy_ui_Component* parent,
	TrackConfig* trackconfig, TrackerGridState* gridstate, TrackerLineState* linestate,
	Workspace* workspace)
{
	assert(self);

	// init base component
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setvtable(&self->component, vtable_init(self));
	// set references
	self->workspace = workspace;
	psy_table_init(&self->columns);
	trackergrid_setsharedgridstate(self, gridstate, trackconfig);
	trackergrid_setsharedlinestate(self, linestate);
	trackergrid_storecursor(self);
	// setup base component
	// psy_ui_component_setbackgroundmode(&self->component,
	//	psy_ui_NOBACKGROUND);
	psy_ui_component_doublebuffer(&self->component);	
	trackergrid_init_signals(self);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	// init internal data	
	self->syncpattern = TRUE;	
	self->gridstate->midline = FALSE;	
	self->chordmode = FALSE;
	self->chordbegin = 0;
	self->wraparound = TRUE;
	self->gridstate->showemptydata = FALSE;	
	self->movecursoronestep = FALSE;
	self->ft2home = TRUE;
	self->ft2delete = TRUE;
	self->effcursoralwaysdown = FALSE;	
	self->pgupdownstep = 4;
	self->preventscrolltop = FALSE;
	self->preventeventdriver = FALSE;
	self->notestabmode = psy_dsp_NOTESTAB_DEFAULT;
	psy_audio_patternselection_init(&self->gridstate->selection);
	// handle midline invalidation
	psy_signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);
	// receive notecommands from the player
	psy_signal_connect(&workspace_player(self->workspace)->signal_inputevent, self,
		trackergrid_oninput);
	psy_signal_connect(&self->workspace->signal_gotocursor, self,
		trackergrid_ongotocursor);	
}

void trackergrid_ondestroy(TrackerGrid* self)
{
	assert(self);

	trackergrid_dispose_signals(self);
	psy_table_dispose(&self->columns);
}

void trackergrid_init_signals(TrackerGrid* self)
{
	assert(self);
	
	psy_signal_init(&self->signal_colresize);
}

void trackergrid_dispose_signals(TrackerGrid* self)
{
	assert(self);
	
	psy_signal_dispose(&self->signal_colresize);
}

void trackergrid_setsharedgridstate(TrackerGrid* self, TrackerGridState*
	gridstate, TrackConfig* trackconfig)
{
	assert(self);

	if (gridstate) {
		self->gridstate = gridstate;
	} else {
		trackergridstate_init(&self->defaultgridstate, trackconfig, NULL, NULL);
		self->gridstate = &self->defaultgridstate;
	}
}

void trackergrid_setsharedlinestate(TrackerGrid* self, TrackerLineState*
	linestate)
{
	assert(self);

	if (linestate) {
		self->linestate = linestate;
	} else {
		trackerlinestate_init(&self->defaultlinestate);
		self->linestate = &self->defaultlinestate;
	}
}

void trackergrid_ondraw(TrackerGrid* self, psy_ui_Graphics* g)
{
	psy_audio_PatternSelection clip;
		
	trackerlinestate_clip(self->linestate, &g->clip, &clip);
	trackergridstate_clip(self->gridstate, &g->clip, &clip);
	trackergrid_drawbackground(self, g, &clip);
	/* prepares entry draw done in trackergridcolumn */
	if (trackergridstate_pattern(self->gridstate)) {		
		trackergrid_updatetrackevents(self, &clip);	
	}
}

void trackergrid_drawbackground(TrackerGrid* self, psy_ui_Graphics* g,
	const psy_audio_PatternSelection* clip)
{
	uintptr_t track;
	psy_ui_RealSize size;
	double blankcpx;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);
	for (track = clip->topleft.track; track < clip->bottomright.track;
		++track) {
		double trackwidth;
		double cpx;

		cpx = trackergridstate_tracktopx(self->gridstate, track);
		trackwidth = trackergridstate_trackwidth(self->gridstate, track);
		psy_ui_drawsolidrectangle(g,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(
					cpx, psy_ui_component_scrolltoppx(&self->component)),
				psy_ui_realsize_make(trackwidth, size.height)),
			patternviewskin_separatorcolour(self->gridstate->skin, track,
				trackergridstate_numsongtracks(self->gridstate)));
	}
	if (trackergridstate_numsongtracks(self->gridstate) > 0) {
		blankcpx = trackergridstate_tracktopx(self->gridstate,
			trackergridstate_numsongtracks(self->gridstate) - 1) +
			trackergridstate_trackwidth(self->gridstate,
				trackergridstate_numsongtracks(self->gridstate) - 1);
		if (blankcpx - psy_ui_component_scrollleftpx(&self->component) < size.width) {
			psy_ui_drawsolidrectangle(g,
				psy_ui_realrectangle_make(
					psy_ui_realpoint_make(
						blankcpx,
						psy_ui_component_scrolltoppx(&self->component)),
					psy_ui_realsize_make(
						size.width - (blankcpx - psy_ui_component_scrollleftpx(&self->component)),
						size.height)),
				patternviewskin_separatorcolour(self->gridstate->skin, 1, 2));
		}
	}
}


void trackergrid_updatetrackevents(TrackerGrid* self,
	const psy_audio_PatternSelection* clip)
{
	uintptr_t track;
	uintptr_t maxlines;	
	double offset;
	double seqoffset;
	double length;	
	psy_audio_SequenceTrackIterator ite;	
	uintptr_t line;	

	assert(self);
	
	trackereventtable_clearevents(&self->gridstate->trackevents);
	ite.pattern = self->gridstate->pattern;
	ite.patternnode = NULL;
	ite.patterns = &self->workspace->song->patterns;
	seqoffset = 0.0;
	length = ite.pattern->length;
	offset = clip->topleft.offset;
	if (!self->linestate->singlemode && self->gridstate->sequence) {
		psy_audio_SequenceTrackNode* tracknode;

		tracknode = psy_list_at(self->gridstate->sequence->tracks,
			workspace_sequenceeditposition(self->workspace).track);
		if (!tracknode) {
			tracknode = self->gridstate->sequence->tracks;
		}
	 	ite = psy_audio_sequence_begin(self->gridstate->sequence,			
			tracknode, offset);
	 	if (ite.sequencentrynode) {
			seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);	 		
	 		if (ite.pattern) {
	 			length = ite.pattern->length;
	 		}		
	 	}
	} else {
		ite.sequencentrynode = NULL;
		ite.patternnode = psy_audio_pattern_greaterequal(
			trackergridstate_pattern(self->gridstate),
			(psy_dsp_big_beat_t)offset - seqoffset);
	}		
	line = trackerlinestate_beattoline(self->linestate, offset);	
	maxlines = trackerlinestate_numlines(self->linestate);
	while (offset <= clip->bottomright.offset && line < maxlines) {		
		bool fill;
		
		fill = !(offset >= seqoffset && offset < seqoffset + length) || !ite.patternnode;		
		// draw trackline
		for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
			bool hasevent = FALSE;
			
			while (!fill && ite.patternnode &&
					psy_audio_sequencetrackiterator_patternentry(&ite)->track <= track &&
					psy_dsp_testrange_e(
						psy_audio_sequencetrackiterator_offset(&ite),
						offset,
						trackerlinestate_bpl(self->linestate))) {
				psy_audio_PatternEntry* entry;

				entry = psy_audio_sequencetrackiterator_patternentry(&ite);
				if (entry->track == track) {
					psy_List** trackevents;

					trackevents = trackereventtable_track(&self->gridstate->trackevents, entry->track);
					psy_list_append(trackevents, entry);
					psy_list_next(&ite.patternnode);
					hasevent = TRUE;
					break;
				}
				psy_list_next(&ite.patternnode);
			}
			if (!hasevent) {				
				psy_List** trackevents;

				trackevents = trackereventtable_track(&self->gridstate->trackevents, track);
				psy_list_append(trackevents, NULL);
			} else if (ite.patternnode && ((psy_audio_PatternEntry*)(ite.patternnode->entry))->track <= track) {
				fill = TRUE;
			}			
		}
		// skip remaining events of the line
		while (ite.patternnode && (offset < seqoffset + length) && 
			(psy_audio_sequencetrackiterator_offset(&ite) + psy_dsp_epsilon * 2 <
				offset + trackerlinestate_bpl(self->linestate))) {
			psy_list_next(&ite.patternnode);
		}
		offset += trackerlinestate_bpl(self->linestate);
		if (offset >= seqoffset + length) {
			// go to next seqentry or end draw
			if (ite.sequencentrynode && ite.sequencentrynode->next) {
				psy_audio_sequencetrackiterator_inc_entry(&ite);
				seqoffset = psy_audio_sequencetrackiterator_seqoffset(&ite);				
				offset = seqoffset;
				if (ite.pattern) {
					length = ite.pattern->length;
				} else {
					break;
				}
			} else {
				break;
			}
		}		
	}
	self->gridstate->trackevents.seqoffset = seqoffset;
	self->gridstate->trackevents.clip = *clip;	
	self->gridstate->trackevents.currcursorline =
		trackerlinestate_beattoline(self->linestate,
		self->gridstate->cursor.offset + self->gridstate->cursor.seqoffset);
	self->gridstate->trackevents.currplaybarline =
		trackerlinestate_beattoline(self->linestate,
		self->linestate->lastplayposition -
		((self->linestate->singlemode)
			? self->linestate->sequenceentryoffset
			: 0.0));	
}

psy_audio_OrderIndex trackergrid_checkupdatecursorseqoffset(TrackerGrid* self,
		psy_audio_PatternCursor* rv) {
	if (rv->offset < 0 || rv->offset >=
			psy_audio_pattern_length(self->linestate->pattern)) {
		uintptr_t order;
		psy_audio_SequenceEntry* entry;

		order = psy_audio_sequence_order(self->gridstate->sequence,
			0, rv->offset + rv->seqoffset);
		if (order != psy_INDEX_INVALID) {
			entry = psy_audio_sequence_entry(self->gridstate->sequence,
				psy_audio_orderindex_make(
					workspace_sequenceeditposition(self->workspace).track,
					order));
			if (entry) {
				psy_dsp_big_beat_t seqdelta;

				seqdelta = (entry->offset - rv->seqoffset);
				rv->offset -= seqdelta;
				rv->seqoffset = entry->offset;
			}
			return psy_audio_orderindex_make(
				workspace_sequenceeditposition(self->workspace).track,
				order);
		}
	}
	return psy_audio_orderindex_make(psy_INDEX_INVALID,
		psy_INDEX_INVALID);
}

psy_dsp_big_beat_t trackergrid_currseqoffset(TrackerGrid* self)
{
	psy_audio_SequenceEntry* entry;

	entry = psy_audio_sequence_entry(self->gridstate->sequence,
		workspace_sequenceeditposition(self->workspace));
	if (entry) {
		return entry->offset;
	}
	return 0.0;
}

void setcolumncolour(PatternViewSkin* skin, psy_ui_Graphics* g,
	TrackerColumnFlags flags, uintptr_t track, uintptr_t numtracks)
{
	assert(skin);

	if (flags.cursor != 0) {		
		psy_ui_setbackgroundcolour(g, skin->cursor);
		psy_ui_settextcolour(g,
			patternviewskin_fontcurcolour(skin, track, numtracks));	
	} else if (flags.playbar) {
		psy_ui_setbackgroundcolour(g,
			patternviewskin_playbarcolour(skin, track, numtracks));
		psy_ui_settextcolour(g,
			patternviewskin_fontplaycolour(skin, track, numtracks));
	} else if (flags.selection) {
		if (flags.beat4) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selection4beatcolour(skin, track, numtracks));
		} else if (flags.beat) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selectionbeatcolour(skin, track, numtracks));
		} else {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_selectioncolour(skin, track, numtracks));
		}				
		psy_ui_settextcolour(g,
			patternviewskin_fontselcolour(skin, track, numtracks));
	} else if (flags.mid) {
		psy_ui_setbackgroundcolour(g,
			patternviewskin_midlinecolour(skin, track, numtracks));
		if (flags.cursor != 0) {
			psy_ui_settextcolour(g,
				patternviewskin_fontcurcolour(skin, track, numtracks));
		} else {
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		}
	} else {
		if (flags.beat4) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_row4beatcolour(skin, track, numtracks));
			psy_ui_settextcolour(g, skin->font);
		} else if (flags.beat) {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_rowbeatcolour(skin, track, numtracks));
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		} else {
			psy_ui_setbackgroundcolour(g,
				patternviewskin_rowcolour(skin, track, numtracks));
			psy_ui_settextcolour(g,
				patternviewskin_fontcolour(skin, track, numtracks));
		}
	}
}

void trackergrid_prevtrack(TrackerGrid* self)
{	
	psy_audio_PatternCursorNavigator cursornavigator;

	assert(self);

	psy_audio_patterncursornavigator_init(&cursornavigator,
		&self->gridstate->cursor, trackergridstate_pattern(self->gridstate),
		trackerlinestate_bpl(self->linestate), self->wraparound, 0);
	if (psy_audio_patterncursornavigator_prevtrack(&cursornavigator,
			trackergridstate_numsongtracks(self->gridstate))) {
		trackergrid_scrollleft(self, self->gridstate->cursor);
		trackergrid_invalidatecursor(self);
	} else if (trackergrid_scrollright(self, self->gridstate->cursor)) {
		trackergrid_invalidatecursor(self);
	}	
}

void trackergrid_storecursor(TrackerGrid* self)
{
	assert(self);

	self->oldcursor = self->gridstate->cursor;
}

void trackergrid_nexttrack(TrackerGrid* self)
{
	psy_audio_PatternCursorNavigator cursornavigator;

	assert(self);

	psy_audio_patterncursornavigator_init(&cursornavigator,
		&self->gridstate->cursor, trackergridstate_pattern(self->gridstate),
		trackerlinestate_bpl(self->linestate), self->wraparound, 0);
	if (psy_audio_patterncursornavigator_nexttrack(&cursornavigator,
		trackergridstate_numsongtracks(self->gridstate))) {
		if (trackergrid_scrollleft(self, self->gridstate->cursor)) {
			trackergrid_invalidatecursor(self);
		}		
	} else {
		trackergrid_scrollright(self, self->gridstate->cursor);
		trackergrid_invalidatecursor(self);
	}
}

bool trackergrid_scrollup(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	intptr_t line;
	intptr_t topline;
	psy_ui_RealRectangle r;

	assert(self);

	line = trackerlinestate_beattoline(self->linestate,
		cursor.offset + cursor.seqoffset);
	psy_ui_setrectangle(&r,
		trackergridstate_tracktopx(self->gridstate, cursor.track),
		self->linestate->lineheightpx * line,
		trackergridstate_trackwidth(self->gridstate, cursor.track),
		self->linestate->lineheightpx);
	if (self->gridstate->midline) {
		psy_ui_Size gridsize;
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		gridsize = psy_ui_component_scrollsize(&self->component);
		topline = (intptr_t)(psy_ui_value_px(&gridsize.height, tm, NULL) / self->linestate->lineheightpx / 2);
	} else {
		topline = 0;
	}
	if (psy_ui_component_scrolltoppx(&self->component) +
			topline * self->linestate->lineheightpx > r.top) {
		intptr_t dlines;
		const psy_ui_TextMetric* tm;
		
		dlines = (intptr_t)((psy_ui_component_scrolltoppx(&self->component) +
			topline * self->linestate->lineheightpx - r.top) /
			(self->linestate->lineheightpx));				
		self->linestate->cursorchanging = TRUE;
		tm = psy_ui_component_textmetric(&self->component);
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_value_make_px(
				psy_ui_component_scrolltoppx(&self->component) -
				psy_ui_component_scrollstep_height_px(&self->component) *
				dlines));
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scrolldown(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	intptr_t line;
	intptr_t visilines;
	const psy_ui_TextMetric* tm;	

	assert(self);

	tm = psy_ui_component_textmetric(&self->component);
	visilines = self->linestate->visilines;
	if (self->gridstate->midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	line = trackerlinestate_beattoline(self->linestate,
		cursor.offset + cursor.seqoffset);
	if (visilines < line - psy_ui_component_scrolltoppx(&self->component) /
			self->linestate->lineheightpx) {
		intptr_t dlines;

		dlines = (intptr_t)
			(line - psy_ui_component_scrolltoppx(&self->component) /
			self->linestate->lineheightpx - visilines);
		self->linestate->cursorchanging = TRUE;
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_value_make_px(
				psy_ui_component_scrolltoppx(&self->component) +
				psy_ui_component_scrollstep_height_px(&self->component) *
				dlines));
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scrollleft(TrackerGrid* self, psy_audio_PatternCursor cursor)
{		
	assert(self);

	if (trackergridstate_pxtotrack(self->gridstate,
			psy_ui_component_scrollleftpx(&self->component)) > cursor.track) {
		psy_ui_component_setscrollleft(&self->component,
			psy_ui_value_make_px(trackergridstate_tracktopx(self->gridstate,
				cursor.track)));
		return FALSE;
	}
	return TRUE;
}

bool trackergrid_scrollright(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	uintptr_t visitracks;
	uintptr_t tracks;
	psy_ui_Size size;	
	const psy_ui_TextMetric* tm;	
	intptr_t trackright;
	intptr_t trackleft;

	assert(self);

	size = psy_ui_component_scrollsize(
		psy_ui_component_parent(&self->component));
	tm = psy_ui_component_textmetric(&self->component);	
	trackleft = trackergridstate_pxtotrack(self->gridstate,
		psy_ui_component_scrollleftpx(&self->component));
	trackright = trackergridstate_pxtotrack(self->gridstate,
		psy_ui_value_px(&size.width, tm, NULL) +
		psy_ui_component_scrollleftpx(&self->component));	
	visitracks = trackright - trackleft;
	tracks = cursor.track + 1;
	if (tracks > trackleft + visitracks) {
		psy_ui_component_setscrollleft(&self->component,
			psy_ui_value_make_px(
				trackergridstate_tracktopx(self->gridstate,
					tracks - visitracks)));
		return FALSE;
	}
	return TRUE;
}

void trackergrid_prevline(TrackerGrid* self)
{	
	assert(self);

	trackergrid_prevlines(self, workspace_cursorstep(self->workspace),
		self->wraparound);
}

void trackergrid_advanceline(TrackerGrid* self)
{
	assert(self);

	trackergrid_advancelines(self, workspace_cursorstep(self->workspace),
		self->wraparound);
}

void trackergrid_advancelines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;
		bool restorewrap;

		restorewrap = wrap;
		if (!self->linestate->singlemode) {
			wrap = TRUE;
		}
		psy_audio_patterncursornavigator_init(&cursornavigator,
			&self->gridstate->cursor,
			trackergridstate_pattern(self->gridstate),
			trackerlinestate_bpl(self->linestate), wrap, 0);		
		if (psy_audio_patterncursornavigator_advancelines(&cursornavigator, lines)) {
			trackergrid_scrolldown(self, self->gridstate->cursor);
		} else if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;

			self->gridstate->cursor.offset =
				psy_audio_pattern_length(self->linestate->pattern);
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->cursor);
			if (psy_audio_orderindex_valid(&index)) {
				self->preventscrolltop = TRUE;
				workspace_setsequenceeditposition(self->workspace,
					index);
				self->preventscrolltop = FALSE;
			} else if (restorewrap) {
				self->gridstate->cursor.offset = 0;
				self->preventscrolltop = TRUE;
				workspace_setsequenceeditposition(self->workspace,
					psy_audio_orderindex_make(
						workspace_sequenceeditposition(self->workspace).track, 0));
				self->preventscrolltop = FALSE;
				trackergrid_scrollup(self, self->gridstate->cursor);
				trackergrid_storecursor(self);
				return;
			} else {
				self->gridstate->cursor.offset =
					psy_audio_pattern_length(self->linestate->pattern) -
					trackerlinestate_bpl(self->linestate);				
			}
			trackergrid_scrolldown(self, self->gridstate->cursor);
		} else {
			trackergrid_scrollup(self, self->gridstate->cursor);
		}		
		trackergridstate_synccursor(self->gridstate);
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_prevlines(TrackerGrid* self, uintptr_t lines, bool wrap)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate)) {
		psy_audio_PatternCursorNavigator cursornavigator;
		bool restorewrap;

		restorewrap = wrap;
		if (!self->linestate->singlemode) {
			wrap = TRUE;
		}
		psy_audio_patterncursornavigator_init(&cursornavigator, &self->gridstate->cursor,
			trackergridstate_pattern(self->gridstate), trackerlinestate_bpl(self->linestate), wrap, 0);		
		if (!psy_audio_patterncursornavigator_prevlines(&cursornavigator, lines)) {
			trackergrid_scrollup(self, self->gridstate->cursor);
		} else if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;			
			
			self->gridstate->cursor.offset = -trackerlinestate_bpl(self->linestate);
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->cursor);
			if (psy_audio_orderindex_valid(&index)) {
				psy_audio_PatternCursor cursor;

				cursor = self->gridstate->cursor;
				self->preventscrolltop = TRUE;
				workspace_setsequenceeditposition(self->workspace, index);
				self->preventscrolltop = FALSE;
				self->gridstate->cursor = cursor;
			} else if (restorewrap) {				
				self->preventscrolltop = TRUE;
				workspace_setsequenceeditposition(self->workspace,
					psy_audio_orderindex_make(
						workspace_sequenceeditposition(self->workspace).track,
						psy_audio_sequence_track_size(
							self->gridstate->sequence, 0) - 1));
				self->preventscrolltop = FALSE;
				if (self->linestate->pattern) {
					self->gridstate->cursor.offset =
						psy_audio_pattern_length(self->linestate->pattern) -
						trackerlinestate_bpl(self->linestate);
				}
				trackergrid_scrolldown(self, self->gridstate->cursor);
				trackergrid_storecursor(self);
				return;
			} else {
				self->gridstate->cursor.offset =
					psy_audio_pattern_length(self->linestate->pattern) -
					trackerlinestate_bpl(self->linestate);
			}			
			trackergrid_scrollup(self, self->gridstate->cursor);
		} else {
			trackergrid_scrolldown(self, self->gridstate->cursor);
		}
		trackergridstate_synccursor(self->gridstate);
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_home(TrackerGrid* self)
{
	assert(self);

	if (self->ft2home) {
		self->gridstate->cursor.offset = 0.0;
		trackergrid_scrollup(self, self->gridstate->cursor);
	} else {
		if (self->gridstate->cursor.column != 0) {
			self->gridstate->cursor.column = 0;
		} else {
			self->gridstate->cursor.track = 0;
			self->gridstate->cursor.column = 0;
		}
		trackergrid_scrollleft(self, self->gridstate->cursor);
	}
	trackergridstate_synccursor(self->gridstate);
	trackergrid_invalidatecursor(self);
}

void trackergrid_end(TrackerGrid* self)
{
	assert(self);

	if (self->ft2home) {
		self->gridstate->cursor.offset = trackergridstate_pattern(self->gridstate)->length - trackerlinestate_bpl(self->linestate);
		trackergrid_scrolldown(self, self->gridstate->cursor);
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		if (self->gridstate->cursor.track != trackergridstate_numsongtracks(self->gridstate) - 1 ||
				self->gridstate->cursor.digit != columndef->numdigits - 1 ||
				self->gridstate->cursor.column != TRACKER_COLUMN_PARAM) {
			if (self->gridstate->cursor.column == TRACKER_COLUMN_PARAM &&
				self->gridstate->cursor.digit == columndef->numdigits - 1) {
				self->gridstate->cursor.track = trackergridstate_numsongtracks(self->gridstate) - 1;
				trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->gridstate->cursor.column = TRACKER_COLUMN_PARAM;
				self->gridstate->cursor.digit = columndef->numdigits - 1;
				trackergrid_scrollright(self, self->gridstate->cursor);
			} else {
				trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
				columndef = trackdef_columndef(trackdef, TRACKER_COLUMN_PARAM);
				self->gridstate->cursor.column = TRACKER_COLUMN_PARAM;
				self->gridstate->cursor.digit = columndef->numdigits - 1;
			}			
		}
	}
	trackergridstate_synccursor(self->gridstate);
	trackergrid_invalidatecursor(self);
}

void trackergrid_onkeydown(TrackerGrid* self, psy_ui_KeyEvent* ev)
{	
	assert(self);

	if (self->preventeventdriver) {
		psy_EventDriver* kbd;
		psy_EventDriverInput input;
		psy_EventDriverCmd cmd;

		kbd = workspace_kbddriver(self->workspace);
		input.message = psy_EVENTDRIVER_KEYDOWN;
		input.param1 = psy_audio_encodeinput(ev->keycode,
			self->chordmode ? 0 : ev->shift, ev->ctrl, ev->alt);
		input.param2 = workspace_octave(self->workspace) * 12;
		psy_eventdriver_cmd(kbd, "tracker", input, &cmd);
		if (cmd.id >= CMD_DIGIT0 && cmd.id <= CMD_DIGITF) {
			if (self->gridstate->cursor.column != TRACKER_COLUMN_NOTE) {
				intptr_t digit = keycodetoint((uint32_t)ev->keycode);
				if (digit != -1) {
					trackergrid_inputvalue(self, digit, 1);
					psy_ui_component_invalidate(&self->component);
					psy_ui_keyevent_stoppropagation(ev);
					return;
				}
			}
		}
		cmd.id = -1;
		input.message = psy_EVENTDRIVER_KEYDOWN;
		input.param1 = psy_audio_encodeinput(ev->keycode, 0, ev->ctrl, ev->alt);
		psy_eventdriver_cmd(kbd, "notes", input, &cmd);
		if (cmd.id != -1) {
			trackergrid_inputnote(self,
				(psy_dsp_note_t)(cmd.id + workspace_octave(self->workspace) * 12),
				1);
			psy_ui_component_invalidate(&self->component);
			psy_ui_keyevent_stoppropagation(ev);
		}				
	}	
}

void trackergrid_onkeyup(TrackerGrid* self, psy_ui_KeyEvent* ev)
{
	assert(self);

	if (self->chordmode && ev->keycode == psy_ui_KEY_SHIFT) {
		self->chordmode = FALSE;
		self->gridstate->cursor.track = self->chordbegin;
		trackergrid_scrollleft(self, self->gridstate->cursor);
		trackergrid_advanceline(self);
		psy_ui_keyevent_stoppropagation(ev);
	}
}

void trackergrid_prevcol(TrackerGrid* self)
{
	int invalidate = 1;

	assert(self);

	if (self->gridstate->cursor.column == 0 && self->gridstate->cursor.digit == 0) {
		if (self->gridstate->cursor.track > 0) {
			TrackDef* trackdef;

			--self->gridstate->cursor.track;
			trackdef = trackergridstate_trackdef(self->gridstate,
				self->gridstate->cursor.track);
			self->gridstate->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->gridstate->cursor.digit = trackdef_numdigits(trackdef,
				self->gridstate->cursor.column) - 1;
			trackergrid_scrollleft(self, self->gridstate->cursor);
		} else if (self->wraparound) {
			TrackDef* trackdef;

			self->gridstate->cursor.track = workspace_song(self->workspace)
				? psy_audio_song_numsongtracks(workspace_song(self->workspace)) - 1
				: 0;
			trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
			self->gridstate->cursor.column = trackdef_numcolumns(trackdef) - 1;
			self->gridstate->cursor.digit = trackdef_numdigits(trackdef,
				self->gridstate->cursor.column) - 1;
			invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
		}
	} else {
		if (self->gridstate->cursor.digit > 0) {
			--self->gridstate->cursor.digit;
		} else {
			TrackDef* trackdef;

			trackdef = trackergridstate_trackdef(self->gridstate,
				self->gridstate->cursor.track);
			--self->gridstate->cursor.column;
			self->gridstate->cursor.digit = trackdef_numdigits(trackdef,
				self->gridstate->cursor.column) - 1;
		}
	}
	trackergridstate_synccursor(self->gridstate);
	if (invalidate) {
		trackergrid_invalidatecursor(self);
	}
}

void trackergrid_nextcol(TrackerGrid* self)
{
	if (workspace_song(self->workspace)) {
		TrackDef* trackdef;
		int invalidate = 1;

		assert(self);

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		if (self->gridstate->cursor.column == trackdef_numcolumns(trackdef) - 1 &&
			self->gridstate->cursor.digit == trackdef_numdigits(trackdef,
				self->gridstate->cursor.column) - 1) {
			if (self->gridstate->cursor.track < psy_audio_song_numsongtracks(
				workspace_song(self->workspace)) - 1) {
				self->gridstate->cursor.column = 0;
				self->gridstate->cursor.digit = 0;
				++self->gridstate->cursor.track;
				invalidate = trackergrid_scrollright(self, self->gridstate->cursor);
			} else if (self->wraparound) {
				self->gridstate->cursor.column = 0;
				self->gridstate->cursor.digit = 0;
				self->gridstate->cursor.track = 0;
				invalidate = trackergrid_scrollleft(self, self->gridstate->cursor);
			}
		} else {
			++self->gridstate->cursor.digit;
			if (self->gridstate->cursor.digit >=
				trackdef_numdigits(trackdef, self->gridstate->cursor.column)) {
				++self->gridstate->cursor.column;
				self->gridstate->cursor.digit = 0;
			}
		}
		trackergridstate_synccursor(self->gridstate);
		if (invalidate) {
			trackergrid_invalidatecursor(self);
		}
	}
}

void trackergrid_selectall(TrackerGrid* self)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate)) {
		psy_audio_patternselection_init(&self->gridstate->selection);
		if (trackergridstate_pattern(self->gridstate)) {
			self->gridstate->selection.topleft.key = psy_audio_NOTECOMMANDS_B9;
			self->gridstate->selection.bottomright.offset = psy_audio_pattern_length(
				trackergridstate_pattern(self->gridstate));
			self->gridstate->selection.bottomright.track =
				trackergridstate_numsongtracks(self->gridstate);
			psy_audio_patternselection_enable(&self->gridstate->selection);
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_selectcol(TrackerGrid* self)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate)) {
		self->gridstate->selection.topleft.offset = 0;
		self->gridstate->selection.topleft.track = self->gridstate->cursor.track;
		self->gridstate->selection.bottomright.offset = trackergridstate_pattern(self->gridstate)->length;
		self->gridstate->selection.bottomright.track = self->gridstate->cursor.track + 1;
		psy_audio_patternselection_enable(&self->gridstate->selection);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_selectbar(TrackerGrid* self)
{
	assert(self);

	if (workspace_song(self->workspace) && trackergridstate_pattern(self->gridstate)) {
		self->gridstate->selection.topleft.offset = self->gridstate->cursor.offset;
		self->gridstate->selection.topleft.track = self->gridstate->cursor.track;
		self->gridstate->selection.bottomright.offset = self->gridstate->cursor.offset + 4.0;
		if (self->gridstate->cursor.offset > trackergridstate_pattern(self->gridstate)->length) {
			self->gridstate->cursor.offset = trackergridstate_pattern(self->gridstate)->length;
		}
		self->gridstate->selection.bottomright.track = self->gridstate->cursor.track + 1;
		psy_audio_patternselection_enable(&self->gridstate->selection);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_selectmachine(TrackerGrid* self)
{
	assert(self);

	if (workspace_song(self->workspace)) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		
		node = psy_audio_pattern_findnode_cursor(
			trackergridstate_pattern(self->gridstate),
			self->gridstate->cursor, &prev);
		if (node) {
			psy_audio_PatternEvent* ev;

			entry = (psy_audio_PatternEntry*)node->entry;
			ev = psy_audio_patternentry_front(entry);
			psy_audio_machines_select(&workspace_song(self->workspace)->machines,
				ev->mach);
			psy_audio_instruments_select(&workspace_song(self->workspace)->instruments,
				psy_audio_instrumentindex_make(0, ev->inst));			
		}		
	}
}

void trackergrid_oninput(TrackerGrid* self, psy_audio_Player* sender,
	psy_audio_PatternEvent* ev)
{
	assert(self);

	if (self->preventeventdriver) {
		return;
	}
	if (self->gridstate->cursor.column == TRACKER_COLUMN_NOTE &&
			ev->note != psy_audio_NOTECOMMANDS_RELEASE) {
		if (workspace_currview(self->workspace).id == VIEW_ID_PATTERNVIEW) {
			trackergrid_setdefaultevent(self, &sender->patterndefaults, ev);
			trackergrid_inputevent(self, ev, self->chordmode);
		}
	}
}

void trackergrid_setdefaultevent(TrackerGrid* self,
	psy_audio_Pattern* defaults,
	psy_audio_PatternEvent* ev)
{
	psy_audio_PatternNode* node;
	psy_audio_PatternNode* prev;

	assert(self);

	node = psy_audio_pattern_findnode_cursor(defaults, self->gridstate->cursor,
		&prev);
	if (node) {
		psy_audio_PatternEvent* defaultevent;

		defaultevent = psy_audio_patternentry_front(psy_audio_patternnode_entry(node));
		if (defaultevent->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
			ev->inst = defaultevent->inst;
		}
		if (defaultevent->mach != psy_audio_NOTECOMMANDS_psy_audio_EMPTY) {
			ev->mach = defaultevent->mach;
		}
		if (defaultevent->vol != psy_audio_NOTECOMMANDS_VOL_EMPTY) {
			ev->vol = defaultevent->vol;
		}
	}
}

void trackergrid_rowdelete(TrackerGrid* self)
{
	assert(self);

	if (self->gridstate->cursor.offset - trackerlinestate_bpl(self->linestate) >= 0) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternNode* p;
		psy_audio_PatternNode* q;
		psy_audio_PatternNode* node;

		if (self->ft2delete) {
			trackergrid_prevline(self);
		}
		node = psy_audio_pattern_findnode_cursor(
			trackergridstate_pattern(self->gridstate),
			self->gridstate->cursor, &prev);
		if (node) {
			psy_audio_pattern_remove(trackergridstate_pattern(self->gridstate), node);
			psy_audio_sequencer_checkiterators(
				&workspace_player(self->workspace)->sequencer,
				node);
		}
		p = (prev)
			? prev->next
			: psy_audio_pattern_begin(trackergridstate_pattern(self->gridstate));
		for (; p != NULL; p = q) {
			psy_audio_PatternEntry* entry;

			q = p->next;
			entry = psy_audio_patternnode_entry(p);
			if (entry->track == self->gridstate->cursor.track) {
				psy_audio_PatternEvent event;
				psy_dsp_big_beat_t offset;
				uintptr_t track;
				psy_audio_PatternNode* node;
				psy_audio_PatternNode* prev;

				event = *psy_audio_patternentry_front(entry);
				offset = entry->offset;
				track = entry->track;
				psy_audio_pattern_remove(trackergridstate_pattern(self->gridstate), p);
				psy_audio_sequencer_checkiterators(
					&workspace_player(self->workspace)->sequencer, p);
				offset -= (psy_dsp_big_beat_t)trackerlinestate_bpl(self->linestate);
				node = psy_audio_pattern_findnode(
					trackergridstate_pattern(self->gridstate), track, offset,
					(psy_dsp_big_beat_t)trackerlinestate_bpl(self->linestate),
					&prev);
				if (node) {
					psy_audio_PatternEntry* entry;

					entry = (psy_audio_PatternEntry*)node->entry;
					*psy_audio_patternentry_front(entry) = event;
				} else {
					psy_audio_pattern_insert(
						trackergridstate_pattern(self->gridstate), prev, track,
						offset, &event);
				}
			}
		}
	}
}

void trackergrid_rowclear(TrackerGrid* self)
{
	assert(self);

	if (self->gridstate->cursor.column == TRACKER_COLUMN_NOTE) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&removecommand_alloc(trackergridstate_pattern(self->gridstate),
				trackerlinestate_bpl(self->linestate),
			self->gridstate->cursor,
			(self->gridstate->synccursor)
				? self->workspace
				: NULL)->command);
		trackergrid_advanceline(self);		
	} else {
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		trackergrid_inputvalue(self, columndef->emptyvalue, 0);
	}
}

void trackergrid_inputevent(TrackerGrid* self,
	const psy_audio_PatternEvent* ev, bool chordmode)
{
	assert(self);

	trackergrid_preventpatternsync(self);
	psy_undoredo_execute(&self->workspace->undoredo,
		&insertcommand_alloc(trackergridstate_pattern(self->gridstate),
			trackerlinestate_bpl(self->linestate),
			self->gridstate->cursor, *ev,
			(self->gridstate->synccursor)
			? self->workspace : NULL)->command);
	if (chordmode != FALSE) {
		trackergrid_nexttrack(self);
	} else {
		trackergrid_advanceline(self);
	}
	if (ev->note < psy_audio_NOTECOMMANDS_RELEASE) {
		self->gridstate->cursor.key = ev->note;
		trackergridstate_synccursor(self->gridstate);
	}
	trackergrid_enablepatternsync(self);
}

void trackergrid_inputnote(TrackerGrid* self, psy_dsp_note_t note,
	bool chordmode)
{
	psy_audio_Machine* machine;
	psy_audio_PatternEvent ev;

	assert(self);

	psy_audio_patternevent_init_all(&ev,
		note,
		psy_audio_NOTECOMMANDS_INST_EMPTY,
		(unsigned char)psy_audio_machines_selected(&workspace_song(self->workspace)->machines),
		psy_audio_NOTECOMMANDS_VOL_EMPTY,
		0,
		0);
	machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, ev.mach);
	if (machine &&
		machine_supports(machine, MACHINE_USES_INSTRUMENTS)) {
		ev.inst = (uint16_t)psy_audio_instruments_selected(
			&workspace_song(self->workspace)->instruments).subslot;
	}
	trackergrid_inputevent(self, &ev, chordmode);
}

void trackergrid_inputvalue(TrackerGrid* self, intptr_t value, intptr_t digit)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate) && value != -1) {
		psy_audio_PatternNode* prev;
		psy_audio_PatternEntry* entry;
		psy_audio_PatternNode* node;
		psy_audio_PatternEntry newentry;
		TrackDef* trackdef;
		TrackColumnDef* columndef;

		trackdef = trackergridstate_trackdef(self->gridstate, self->gridstate->cursor.track);
		columndef = trackdef_columndef(trackdef, self->gridstate->cursor.column);
		psy_audio_patternentry_init(&newentry);
		node = psy_audio_pattern_findnode_cursor(
			trackergridstate_pattern(self->gridstate),
			self->gridstate->cursor, &prev);
		if (node) {
			entry = (psy_audio_PatternEntry*)node->entry;
		} else {
			entry = &newentry;
		}
		if (digit) {
			trackergrid_enterdigitcolumn(self, entry, self->gridstate->cursor,
				value);
		} else {
			entervaluecolumn(entry, self->gridstate->cursor.column, value);
		}
		trackergrid_preventpatternsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_alloc(trackergridstate_pattern(self->gridstate),
				trackerlinestate_bpl(self->linestate),
				self->gridstate->cursor,
				*psy_audio_patternentry_front(entry),
				self->workspace)->command);
		if (self->effcursoralwaysdown) {
			trackergrid_advanceline(self);
		} else {
			if (!digit) {
				if (columndef->wrapclearcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->gridstate->cursor.digit = 0;
					self->gridstate->cursor.column = columndef->wrapclearcolumn;
					trackergrid_advanceline(self);
				}
			} else if (self->gridstate->cursor.digit + 1 >= columndef->numdigits) {
				if (columndef->wrapeditcolumn == TRACKER_COLUMN_NONE) {
					trackergrid_nextcol(self);
				} else {
					self->gridstate->cursor.digit = 0;
					self->gridstate->cursor.column = columndef->wrapeditcolumn;
					trackergrid_advanceline(self);
				}
			} else {
				trackergrid_nextcol(self);
			}
		}
		trackergrid_invalidatecursor(self);
		trackergrid_enablepatternsync(self);
		psy_audio_patternentry_dispose(&newentry);
	}
}

void trackergrid_invalidatecursor(TrackerGrid* self)
{	
	assert(self);

	trackergrid_invalidateinternalcursor(self, self->oldcursor);
	trackergrid_invalidateinternalcursor(self, self->gridstate->cursor);
	trackergrid_storecursor(self);	
}

void trackergrid_invalidateinternalcursor(TrackerGrid* self,
	psy_audio_PatternCursor cursor)
{
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				trackergridstate_tracktopx(self->gridstate, cursor.track),
				trackerlinestate_beattopx(self->linestate, cursor.offset +
					cursor.seqoffset)),
			psy_ui_realsize_make(
				trackergridstate_trackwidth(self->gridstate, cursor.track),
				trackerlinestate_lineheight(self->linestate))));
}

void trackergrid_invalidateline(TrackerGrid* self, psy_dsp_big_beat_t position)
{
	assert(self);

	if (!trackergridstate_pattern(self->gridstate)) {
		return;
	}
	if (!self->gridstate->singlemode ||
			psy_dsp_testrange(position, self->linestate->sequenceentryoffset,
			psy_audio_pattern_length(trackergridstate_pattern(
				self->gridstate)))) {
		psy_ui_RealSize size;		
		
		size = psy_ui_component_scrollsize_px(&self->component);		
		psy_ui_component_invalidaterect(&self->component,
			psy_ui_realrectangle_make(
				psy_ui_realpoint_make(					
					psy_ui_component_scrollleftpx(&self->component),
					trackerlinestate_beattopx(self->linestate,
						position - ((self->gridstate->singlemode)
						? self->linestate->sequenceentryoffset
						: 0.0))),
				psy_ui_realsize_make(size.width, self->linestate->lineheightpx)));
		if (trackergridstate_pattern(
			self->gridstate)) {
			trackergrid_preventpatternsync(self);
			trackergridstate_pattern(
				self->gridstate)->opcount++;
			trackergrid_resetpatternsync(self);
		}
	}
}

void trackergrid_onscroll(TrackerGrid* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_ui_component_scrollleftpx(&self->component) < 0) {
		psy_ui_component_setscrollleft(&self->component, psy_ui_value_zero());
	}
	if (self->gridstate->midline) {
		trackergrid_clearmidline(self);
	}
}

void trackergrid_clearmidline(TrackerGrid* self)
{
	psy_ui_RealSize size;

	assert(self);

	size = psy_ui_component_scrollsize_px(&self->component);	
	self->gridstate->midline = FALSE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleftpx(&self->component),
				((self->linestate->visilines) / 2 - 1) * self->linestate->lineheightpx +
					psy_ui_component_scrolltoppx(&self->component)),
			psy_ui_realsize_make(size.width, self->linestate->lineheightpx)));
	psy_ui_component_update(&self->component);
	self->gridstate->midline = TRUE;
	psy_ui_component_invalidaterect(&self->component,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(
				psy_ui_component_scrollleftpx(&self->component),
				(self->linestate->visilines / 2 - 2) * self->linestate->lineheightpx +
				psy_ui_component_scrolltoppx(&self->component)),
			psy_ui_realsize_make(size.width, self->linestate->lineheightpx * 4)));		
}

void trackergrid_centeroncursor(TrackerGrid* self)
{
	intptr_t line;

	assert(self);

	line = trackerlinestate_beattoline(self->linestate,
		self->gridstate->cursor.offset);
	psy_ui_component_setscrolltop(&self->component,
		psy_ui_value_make_px(
			-(self->linestate->visilines / 2 - line) *
			self->linestate->lineheightpx));
}

void trackergrid_setcentermode(TrackerGrid* self, int mode)
{
	assert(self);

	self->gridstate->midline = mode;
	if (mode) {
		psy_ui_component_setoverflow(&self->component,
			psy_ui_OVERFLOW_SCROLL | psy_ui_OVERFLOW_VSCROLLCENTER);
		trackergrid_centeroncursor(self);
	} else {
		psy_ui_component_setoverflow(&self->component,
			psy_ui_OVERFLOW_SCROLL);
		psy_ui_component_setscrolltop(&self->component,
			psy_ui_value_make_px(0));
	}
}

void trackergrid_onmousedown(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (self->gridstate->trackconfig->colresize) {
		psy_signal_emit(&self->signal_colresize, self, 0);
	} else if (trackergridstate_pattern(self->gridstate) && ev->button == 1) {		
		if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;

			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->dragselectionbase);
			if (psy_audio_orderindex_valid(&index)) {
				self->preventscrolltop = TRUE;
				workspace_setsequenceeditposition(self->workspace,
					index);
				self->preventscrolltop = FALSE;
			}
		}
		self->lastdragcursor = self->gridstate->dragselectionbase;
		psy_audio_patternselection_init_all(&self->gridstate->selection,
			self->gridstate->dragselectionbase, self->gridstate->dragselectionbase);
		psy_audio_patternselection_disable(&self->gridstate->selection);
		if (!psy_ui_component_hasfocus(&self->component)) {
			psy_ui_component_setfocus(&self->component);
		}
		psy_ui_component_capture(&self->component);		
	}
}

void trackergrid_onmousemove(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (ev->button == 1) {
		if (self->gridstate->trackconfig->colresize) {
			psy_signal_emit(&self->signal_colresize, self, 0);
		} else {					
			if (!psy_audio_patterncursor_equal(&self->gridstate->dragcursor,
					&self->lastdragcursor)) {
				if (!psy_audio_patternselection_valid(&self->gridstate->selection)) {
					trackergridstate_startdragselection(self->gridstate,
						self->gridstate->dragcursor,
						trackerlinestate_bpl(self->linestate));
				} else {					
					trackergrid_dragselection(self, self->gridstate->dragcursor);
				}
				psy_ui_component_invalidate(&self->component);
				self->lastdragcursor = self->gridstate->dragcursor;
			}			
		}
	}
}

void trackergrid_dragselection(TrackerGrid* self, psy_audio_PatternCursor cursor)
{
	int restoremidline = self->gridstate->midline;
	
	self->gridstate->midline = 0;
	trackergridstate_dragselection(self->gridstate, cursor,
		trackerlinestate_bpl(self->linestate));
	if (cursor.offset < self->lastdragcursor.offset) {
		trackergrid_scrollup(self, cursor);
	} else {
		trackergrid_scrolldown(self, cursor);
	}
	if (cursor.track < self->lastdragcursor.track) {
		trackergrid_scrollleft(self, cursor);
	} else {
		trackergrid_scrollright(self, cursor);
	}
	self->gridstate->midline = restoremidline;
}

void trackergrid_onmouseup(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_releasecapture(&self->component);
	if (ev->button != 1) {
		return;
	}	
	if (self->gridstate->trackconfig->colresize) {
		self->gridstate->trackconfig->colresize = FALSE;
		psy_signal_emit(&self->signal_colresize, self, 0);		
	} else if (!psy_audio_patternselection_valid(&self->gridstate->selection)) {
		// set cursor only, if no selection was made
		if (!self->linestate->singlemode) {
			psy_audio_OrderIndex index;
			
			index = trackergrid_checkupdatecursorseqoffset(self,
				&self->gridstate->dragselectionbase);
			if (psy_audio_orderindex_valid(&index)) {
				self->preventscrolltop = TRUE;
				workspace_setsequenceeditposition(self->workspace,
					index);
				self->preventscrolltop = FALSE;
			}			
		}
		self->gridstate->cursor = self->gridstate->dragselectionbase;
		if (!psy_audio_patterncursor_equal(&self->oldcursor, &self->gridstate->cursor)) {
			trackergrid_invalidatecursor(self);
		}
		trackergridstate_synccursor(self->gridstate);
	}	
}

void trackergrid_onmousedoubleclick(TrackerGrid* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (ev->button == 1) {
		trackergrid_selectcol(self);
	}
}

void trackergrid_setpattern(TrackerGrid* self, psy_audio_Pattern* pattern)
{
	assert(self);
		
	trackergridstate_setpattern(self->gridstate, pattern);	
	trackerlinestate_setpattern(self->linestate, pattern);
	trackergrid_resetpatternsync(self);
	psy_ui_component_updateoverflow(trackergrid_base(self));	
	if (psy_audio_player_playing(workspace_player(self->workspace)) ||
			!trackergridstate_cursorposition_valid(self->gridstate)) {		
		if (!self->gridstate->singlemode) {
			self->gridstate->cursor.seqoffset =
				trackergrid_currseqoffset(self);
			if (!workspace_followingsong(self->workspace)) {
				psy_ui_component_setscrolltop(&self->component,
					psy_ui_value_make_px(trackerlinestate_beattopx(self->linestate,
						self->gridstate->cursor.offset +
						self->gridstate->cursor.seqoffset)));
			} else {
				self->gridstate->cursor.offset = 0;
			}
		} else {
			self->gridstate->cursor.offset = 0;
			psy_ui_component_setscrolltop(&self->component, psy_ui_value_zero());
		}		
	} else if (!self->gridstate->singlemode) {
		self->gridstate->cursor.offset = 0;
		self->gridstate->cursor.seqoffset =
			trackergrid_currseqoffset(self);		
		if (!self->preventscrolltop) {
			psy_ui_component_setscrolltop(&self->component,
				psy_ui_value_make_px(trackerlinestate_beattopx(self->linestate,
					self->gridstate->cursor.offset +
					self->gridstate->cursor.seqoffset)));
		}
	}
	if (self->gridstate->midline) {
		trackergrid_centeroncursor(self);
	}
}

void trackergrid_enablepatternsync(TrackerGrid* self)
{
	assert(self);
	
	trackergrid_resetpatternsync(self);
	self->syncpattern = TRUE;
}

void trackergrid_preventpatternsync(TrackerGrid* self)
{
	assert(self);

	trackergrid_resetpatternsync(self);
	self->syncpattern = FALSE;
}

void trackergrid_resetpatternsync(TrackerGrid* self)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate)) {
		self->component.opcount = trackergridstate_pattern(
			self->gridstate)->opcount;
	} else {
		self->component.opcount = 0;
	}
}

void trackergrid_changegenerator(TrackerGrid* self)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate) && workspace_song(self->workspace)) {
		psy_audio_pattern_changemachine(trackergridstate_pattern(self->gridstate),
			self->gridstate->selection.topleft,
			self->gridstate->selection.bottomright,
			psy_audio_machines_selected(&workspace_song(self->workspace)->machines));
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_changeinstrument(TrackerGrid* self)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate) && workspace_song(self->workspace)) {
		psy_audio_pattern_changeinstrument(trackergridstate_pattern(self->gridstate),
			self->gridstate->selection.topleft,
			self->gridstate->selection.bottomright,
			psy_audio_instruments_selected(&workspace_song(self->workspace)->instruments).subslot);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_blockcut(TrackerGrid* self)
{
	assert(self);

	trackergrid_blockcopy(self);
	trackergrid_blockdelete(self);	
}

void trackergrid_blockcopy(TrackerGrid* self)
{
	assert(self);

	if (trackergridstate_pattern(self->gridstate) &&
			psy_audio_patternselection_valid(&self->gridstate->selection)) {
		psy_audio_pattern_blockcopy(&self->workspace->patternpaste,
			trackergridstate_pattern(self->gridstate), self->gridstate->selection);
	}	
}

void trackergrid_blockpaste(TrackerGrid* self)
{
	assert(self);

	if (!psy_audio_pattern_empty(&self->workspace->patternpaste)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blockpastecommand_alloc(trackergridstate_pattern(self->gridstate),
				&self->workspace->patternpaste, self->gridstate->cursor,
				trackerlinestate_bpl(self->linestate), FALSE, self->workspace)->command);
		trackergrid_movecursorwhenpaste(self);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_blockmixpaste(TrackerGrid* self)
{
	assert(self);

	if (!psy_audio_pattern_empty(&self->workspace->patternpaste)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blockpastecommand_alloc(trackergridstate_pattern(self->gridstate),
				&self->workspace->patternpaste, self->gridstate->cursor,
				trackerlinestate_bpl(self->linestate), TRUE, self->workspace)->command);
		trackergrid_movecursorwhenpaste(self);
		psy_ui_component_invalidate(&self->component);
	}
}

bool trackergrid_movecursorwhenpaste(TrackerGrid* self)
{
	assert(self);

	if (patternviewconfig_ismovecursorwhenpaste(psycleconfig_patview(
			workspace_conf(self->workspace)))) {
		psy_audio_PatternCursor begin;
		psy_audio_PatternCursor end;

		begin = end = self->gridstate->cursor;
		end.track += self->workspace->patternpaste.maxsongtracks;
		end.offset += self->workspace->patternpaste.length;
		end.track = begin.track;
		if (end.offset >= psy_audio_pattern_length(trackergridstate_pattern(self->gridstate))) {
			end.offset = psy_audio_pattern_length(trackergridstate_pattern(self->gridstate)) -
				trackerlinestate_bpl(self->linestate);
		}		
		self->gridstate->cursor = end;
		trackergridstate_synccursor(self->gridstate);
		self->oldcursor = end;
		return TRUE;
	}
	return FALSE;
}

void trackergrid_blockdelete(TrackerGrid* self)
{
	assert(self);

	if (psy_audio_patternselection_valid(&self->gridstate->selection)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blockremovecommand_alloc(trackergridstate_pattern(self->gridstate),
				self->gridstate->selection,
				self->workspace)->command);		
		//		sequencer_checkiterators(&workspace_player(self->workspace).sequencer,
		//			node);
		psy_ui_component_invalidate(&self->component);
	}
}

void trackergrid_blockstart(TrackerGrid* self)
{
	assert(self);

	self->gridstate->dragselectionbase = self->gridstate->cursor;
	trackergridstate_startdragselection(self->gridstate,
		self->gridstate->cursor,
		trackerlinestate_bpl(self->linestate));
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockend(TrackerGrid* self)
{
	assert(self);

	trackergrid_dragselection(self, self->gridstate->cursor);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blockunmark(TrackerGrid* self)
{
	assert(self);

	psy_audio_patternselection_disable(&self->gridstate->selection);
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_blocktransposeup(TrackerGrid* self)
{
	assert(self);

	if (psy_audio_patternselection_valid(&self->gridstate->selection)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blocktransposecommand_alloc(trackergridstate_pattern(self->gridstate),
				self->gridstate->selection,
				self->gridstate->cursor, +1, self->workspace)->command);
	}
}

void trackergrid_blocktransposedown(TrackerGrid* self)
{
	assert(self);

	if (psy_audio_patternselection_valid(&self->gridstate->selection)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blocktransposecommand_alloc(trackergridstate_pattern(self->gridstate),
				self->gridstate->selection,
				self->gridstate->cursor, -1, self->workspace)->command);
	}
}

void trackergrid_blocktransposeup12(TrackerGrid* self)
{
	assert(self);

	if (psy_audio_patternselection_valid(&self->gridstate->selection)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blocktransposecommand_alloc(trackergridstate_pattern(self->gridstate),
				self->gridstate->selection,
				self->gridstate->cursor, 12, self->workspace)->command);
	}
}

void trackergrid_blocktransposedown12(TrackerGrid* self)
{
	assert(self);

	if (psy_audio_patternselection_valid(&self->gridstate->selection)) {
		psy_undoredo_execute(&self->workspace->undoredo,
			&blocktransposecommand_alloc(trackergridstate_pattern(self->gridstate),
				self->gridstate->selection,
				self->gridstate->cursor, -12, self->workspace)->command);
	}
}

void trackergrid_enterdigitcolumn(TrackerGrid* self,
	psy_audio_PatternEntry* entry, psy_audio_PatternCursor cursor,
	intptr_t digitvalue)
{
	TrackDef* trackdef;

	assert(self);

	trackdef = trackergridstate_trackdef(self->gridstate, cursor.track);
	if (trackdef) {
		uintptr_t value;
		uintptr_t num;
		ptrdiff_t pos;
		uint8_t* data;

		value = trackdef_value(trackdef, cursor.column, entry);
		if (cursor.digit != 0xF && value == trackdef_emptyvalue(trackdef,
				cursor.column)) {
			value = 0;
		}
		num = trackdef_numdigits(trackdef, cursor.column);
		pos = num / 2 - cursor.digit / 2 - 1;
		data = (uint8_t*)&value + pos;
		enterdigit(cursor.digit % 2, (uint8_t)digitvalue, data);
		trackdef_setvalue(trackdef, cursor.column, entry, *((uintptr_t*)&value));
	}
}

bool trackergrid_handlecommand(TrackerGrid* self, intptr_t cmd)
{
	bool handled = TRUE;

	assert(self);

	switch (cmd) {
		case CMD_NAVUP:
			if (self->movecursoronestep) {
				trackergrid_prevlines(self, 1, 0);
			} else {
				trackergrid_prevline(self);
			}
			break;
		case CMD_NAVPAGEUP:
			trackergrid_prevlines(self, self->pgupdownstep, 0);
			break;
		case CMD_NAVDOWN:
			if (self->movecursoronestep) {
				trackergrid_advancelines(self, 1, 0);
			} else {
				trackergrid_advanceline(self);
			}
			break;
		case CMD_NAVPAGEDOWN:
			trackergrid_advancelines(self, self->pgupdownstep, 0);
			break;
		case CMD_NAVLEFT:
			trackergrid_prevcol(self);
			break;
		case CMD_NAVRIGHT:
			trackergrid_nextcol(self);
			break;
		case CMD_NAVTOP:
			trackergrid_home(self);
			break;
		case CMD_NAVBOTTOM:
			trackergrid_end(self);
			break;
		case CMD_COLUMNPREV:
			trackergrid_prevtrack(self);
			break;
		case CMD_COLUMNNEXT:
			trackergrid_nexttrack(self);
			break;
		case CMD_BLOCKSTART:
			trackergrid_blockstart(self);
			break;
		case CMD_BLOCKEND:
			trackergrid_blockend(self);
			break;
		case CMD_BLOCKUNMARK:
			trackergrid_blockunmark(self);
			break;
		case CMD_BLOCKCUT:
			trackergrid_blockcut(self);
			break;
		case CMD_BLOCKCOPY:
			trackergrid_blockcopy(self);
			break;
		case CMD_BLOCKPASTE:
			trackergrid_blockpaste(self);
			break;
		case CMD_BLOCKMIX:
			trackergrid_blockmixpaste(self);
			break;
		case CMD_BLOCKDELETE:
			trackergrid_blockdelete(self);
			break;
		case CMD_TRANSPOSEBLOCKINC:
			trackergrid_blocktransposeup(self);
			break;
		case CMD_TRANSPOSEBLOCKDEC:
			trackergrid_blocktransposedown(self);
			break;
		case CMD_TRANSPOSEBLOCKINC12:
			trackergrid_blocktransposeup12(self);
			break;
		case CMD_TRANSPOSEBLOCKDEC12:
			trackergrid_blocktransposedown12(self);
			break;
		case CMD_ROWDELETE:
			trackergrid_rowdelete(self);			
			break;
		case CMD_ROWCLEAR:
			trackergrid_rowclear(self);			
			break;
		case CMD_SELECTALL:
			trackergrid_selectall(self);
			break;
		case CMD_SELECTCOL:
			trackergrid_selectcol(self);
			break;
		case CMD_SELECTBAR:
			trackergrid_selectbar(self);
			break;
		case CMD_SELECTMACHINE:
			trackergrid_selectmachine(self);
			psy_ui_component_setfocus(&self->component);
			break;
		case CMD_UNDO:
			workspace_undo(self->workspace);
			break;
		case CMD_REDO:
			workspace_redo(self->workspace);
			break;
		case CMD_DIGIT0:
		case CMD_DIGIT1:
		case CMD_DIGIT2:
		case CMD_DIGIT3:
		case CMD_DIGIT4:
		case CMD_DIGIT5:
		case CMD_DIGIT6:
		case CMD_DIGIT7:
		case CMD_DIGIT8:
		case CMD_DIGIT9:
		case CMD_DIGITA:
		case CMD_DIGITB:
		case CMD_DIGITC:
		case CMD_DIGITD:
		case CMD_DIGITE:
		case CMD_DIGITF:
			if (self->gridstate->cursor.column != TRACKER_COLUMN_NOTE) {
				int digit = (int)cmd - CMD_DIGIT0;
				if (digit != -1) {
					trackergrid_inputvalue(self, digit, 1);
				}
			}
			break;
		default:
			handled = FALSE;
			break;
	}
	return handled;
}

void trackergrid_tweak(TrackerGrid* self, int slot, uintptr_t tweak,
	float normvalue)
{
	assert(self);

	if (workspace_recordingtweaks(self->workspace)) {
		psy_audio_PatternEvent event;
		psy_audio_Machine* machine;
		int value;

		machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, slot);
		assert(machine);
		value = 0; // machine_parametervalue_scaled(machine, tweak, normvalue);
		psy_audio_patternevent_init_all(&event,
			(unsigned char)(
				(keyboardmiscconfig_recordtweaksastws(&self->workspace->config.misc))
				? psy_audio_NOTECOMMANDS_TWEAKSLIDE
				: psy_audio_NOTECOMMANDS_TWEAK),
			psy_audio_NOTECOMMANDS_INST_EMPTY,
			(unsigned char)psy_audio_machines_selected(&workspace_song(self->workspace)->machines),
			psy_audio_NOTECOMMANDS_VOL_EMPTY,
			(unsigned char)((value & 0xFF00) >> 8),
			(unsigned char)(value & 0xFF));
		event.inst = (unsigned char)tweak;
		trackergrid_preventpatternsync(self);
		psy_undoredo_execute(&self->workspace->undoredo,
			&insertcommand_alloc(trackergridstate_pattern(self->gridstate), trackerlinestate_bpl(self->linestate),
				self->gridstate->cursor, event, self->workspace)->command);
		if (keyboardmiscconfig_advancelineonrecordtweak(&self->workspace->config.misc) &&
			!(workspace_followingsong(self->workspace) &&
				psy_audio_player_playing(workspace_player(self->workspace)))) {
			trackergrid_advanceline(self);
		}
		trackergrid_enablepatternsync(self);
	}
}

void trackergrid_onalign(TrackerGrid* self)
{
	const psy_ui_TextMetric* tm;
	assert(self);
	
	tm = psy_ui_component_textmetric(&self->component);
	self->linestate->lineheightpx =
		psy_max(1.0, floor(psy_ui_value_px(&self->linestate->lineheight,
			tm, NULL)));
	if (trackergrid_midline(self)) {
		trackergrid_centeroncursor(self);
	}
}

void trackergrid_showemptydata(TrackerGrid* self, int showstate)
{
	assert(self);

	self->gridstate->showemptydata = showstate;
	psy_ui_component_invalidate(&self->component);
}

void trackergrid_ongotocursor(TrackerGrid* self, psy_audio_PatternCursor* cursor)
{
	psy_ui_component_setscrolltop(&self->component,
		psy_ui_value_make_px(trackerlinestate_beattopx(self->linestate,
			self->gridstate->cursor.offset +
			self->gridstate->cursor.seqoffset)));
}

void trackergrid_build(TrackerGrid* self)
{
	uintptr_t t;

	psy_ui_component_clear(&self->component);
	psy_table_clear(&self->columns);
	for (t = 0; t < trackergridstate_numsongtracks(self->gridstate); ++t) {
		psy_table_insert(&self->columns, t, (void*)
			trackergridcolumn_allocinit(&self->component, &self->component,
				t, self->gridstate, self->linestate, self->workspace));
	}
}

void maketrackercmds(psy_Property* parent)
{
	psy_Property* cmds;

	assert(parent);

	cmds = psy_property_settext(
		psy_property_append_section(parent, "tracker"),
		"Tracker");
	setcmd(cmds, CMD_NAVUP, psy_ui_KEY_UP, "navup", "up");
	setcmd(cmds, CMD_NAVDOWN, psy_ui_KEY_DOWN, "navdown", "down");
	setcmd(cmds, CMD_NAVLEFT, psy_ui_KEY_LEFT, "navleft", "left");
	setcmd(cmds, CMD_NAVRIGHT, psy_ui_KEY_RIGHT, "navright", "right");
	setcmd(cmds, CMD_NAVPAGEUP, psy_ui_KEY_PRIOR, "navpageup", "pageup");
	setcmd(cmds, CMD_NAVPAGEDOWN, psy_ui_KEY_NEXT, "navpagedown", "pagedown");
	setcmd(cmds, CMD_NAVTOP, psy_ui_KEY_HOME, "navtop", "track top");
	setcmd(cmds, CMD_NAVBOTTOM, psy_ui_KEY_END, "navbottom", "track bottom");
	setcmdall(cmds, CMD_COLUMNPREV, psy_ui_KEY_TAB, psy_SHIFT_ON, psy_CTRL_OFF,
		"columnprev", "prev col");
	setcmd(cmds, CMD_COLUMNNEXT, psy_ui_KEY_TAB, "columnnext", "next col");
	setcmd(cmds, CMD_ROWINSERT, psy_ui_KEY_INSERT, "rowinsert", "ins row");
	setcmd(cmds, CMD_ROWDELETE, psy_ui_KEY_BACK, "rowdelete", "del row");
	setcmd(cmds, CMD_ROWCLEAR, psy_ui_KEY_DELETE, "rowclear", "clr row");
	setcmdall(cmds, CMD_BLOCKSTART, psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	setcmdall(cmds, CMD_BLOCKEND, psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
	setcmdall(cmds, CMD_BLOCKUNMARK, psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	setcmdall(cmds, CMD_BLOCKCUT, psy_ui_KEY_X, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcut", "cut");
	setcmdall(cmds, CMD_BLOCKCOPY, psy_ui_KEY_C, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcopy", "copy");
	setcmdall(cmds, CMD_BLOCKPASTE, psy_ui_KEY_V, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockpaste", "paste");
	setcmdall(cmds, CMD_BLOCKMIX, psy_ui_KEY_M, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockmix", "mix");
	setcmdall(cmds, CMD_BLOCKDELETE, psy_ui_KEY_X, psy_SHIFT_ON, psy_CTRL_ON,
		"blockdelete", "blkdel");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKINC,
		psy_ui_KEY_F12, psy_SHIFT_OFF, psy_CTRL_ON,
		"transposeblockinc", "Trsp+");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKDEC,
		psy_ui_KEY_F11, psy_SHIFT_OFF, psy_CTRL_ON,
		"transposeblockdec", "Trsp-");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKINC12,
		psy_ui_KEY_F12, psy_SHIFT_ON, psy_CTRL_ON,
		"transposeblockinc12", "Trsp+12");
	setcmdall(cmds, CMD_TRANSPOSEBLOCKDEC12,
		psy_ui_KEY_F11, psy_SHIFT_ON, psy_CTRL_ON,
		"transposeblockdec12", "Trsp-12");

	setcmdall(cmds, CMD_SELECTALL, psy_ui_KEY_A, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectall", "sel all");
	setcmdall(cmds, CMD_SELECTCOL, psy_ui_KEY_R, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectcol", "sel col");
	setcmdall(cmds, CMD_SELECTBAR, psy_ui_KEY_K, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectbar", "sel bar");
	setcmd(cmds, CMD_SELECTMACHINE, psy_ui_KEY_RETURN, "selectmachine",
		"Sel Mac/Ins");
	setcmdall(cmds, CMD_UNDO, psy_ui_KEY_Z, psy_SHIFT_OFF, psy_CTRL_ON,
		"undo", "undo");
	setcmdall(cmds, CMD_REDO, psy_ui_KEY_Z, psy_SHIFT_ON, psy_CTRL_ON,
		"redo", "redo");
	setcmd(cmds, CMD_DIGIT0, psy_ui_KEY_DIGIT0, "digit0", "0");
	setcmd(cmds, CMD_DIGIT1, psy_ui_KEY_DIGIT1, "digit1", "1");
	setcmd(cmds, CMD_DIGIT2, psy_ui_KEY_DIGIT2, "digit2", "2");
	setcmd(cmds, CMD_DIGIT3, psy_ui_KEY_DIGIT3, "digit3", "3");
	setcmd(cmds, CMD_DIGIT4, psy_ui_KEY_DIGIT4, "digit4", "4");
	setcmd(cmds, CMD_DIGIT5, psy_ui_KEY_DIGIT5, "digit5", "5");
	setcmd(cmds, CMD_DIGIT6, psy_ui_KEY_DIGIT6, "digit6", "6");
	setcmd(cmds, CMD_DIGIT7, psy_ui_KEY_DIGIT7, "digit7", "7");
	setcmd(cmds, CMD_DIGIT8, psy_ui_KEY_DIGIT8, "digit8", "8");
	setcmd(cmds, CMD_DIGIT9, psy_ui_KEY_DIGIT9, "digit9", "9");
	setcmd(cmds, CMD_DIGITA, psy_ui_KEY_A, "digitA", "A");
	setcmd(cmds, CMD_DIGITB, psy_ui_KEY_B, "digitB", "B");
	setcmd(cmds, CMD_DIGITC, psy_ui_KEY_C, "digitC", "C");
	setcmd(cmds, CMD_DIGITD, psy_ui_KEY_D, "digitD", "D");
	setcmd(cmds, CMD_DIGITE, psy_ui_KEY_E, "digitE", "E");
	setcmd(cmds, CMD_DIGITF, psy_ui_KEY_F, "digitF", "F");
}

// Appends a property with shortcut defaults for the keyboard driver
// key		: cmd id used by the trackerview
// text		: "cmds.key" language dictionary key used by the translator
// shorttext: short description for the keyboard help view
// value	: encoded key shortcut (keycode/shift/ctrl)
void setcmdall(psy_Property* cmds, int cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	assert(cmds);

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_sethint( psy_property_settext(psy_property_setshorttext(
		psy_property_setid(psy_property_append_int(cmds, key,
		psy_audio_encodeinput(keycode, shift, ctrl, 0), 0, 0),
		cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}

void setcmd(psy_Property* cmds, int cmd, uint32_t keycode,
	const char* key, const char* shorttext)
{
	assert(cmds);

	setcmdall(cmds, cmd, keycode, psy_SHIFT_OFF, psy_CTRL_OFF, key, shorttext);	
}
