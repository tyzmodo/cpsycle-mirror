// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackerview.h"
#include <pattern.h>
#include "cmdsnotes.h"
#include <string.h>
#include <math.h>
#include "resources/resource.h"
#include "skingraphics.h"
#include <portable.h>

#define TIMERID_TRACKERVIEW 600
static const big_beat_t epsilon = 0.0001;

static void trackergrid_ondraw(TrackerGrid*, ui_component* sender, ui_graphics*);
static void trackergrid_drawbackground(TrackerGrid*, ui_graphics*, TrackerGridBlock* clip);
static void trackergrid_drawtrackbackground(TrackerGrid*, ui_graphics*, int track);
static void trackergrid_drawevents(TrackerGrid*, ui_graphics*,
	TrackerGridBlock* clip);
static void trackergrid_drawevent(TrackerGrid*, ui_graphics* g, PatternEvent*,
	int x, int y, int playbar, int cursor, int selection, int beat, int beat4,
	int mid);
static void trackergrid_onkeydown(TrackerGrid*, ui_component* sender,
	KeyEvent*);
static void trackergrid_onmousedown(TrackerGrid*, ui_component* sender,
	MouseEvent*);
static void trackergrid_onmousemove(TrackerGrid*, ui_component* sender,
	MouseEvent*);
static void trackergrid_onmouseup(TrackerGrid*, ui_component* sender,
	MouseEvent*);
static void trackergrid_onmousedoubleclick(TrackerGrid*, ui_component* sender,
	MouseEvent*);
static TrackerCursor trackergrid_makecursor(TrackerGrid* self, int x, int y);
static void trackergrid_onsize(TrackerGrid*, ui_component* sender, ui_size* size);
static void trackergrid_onscroll(TrackerGrid*, ui_component* sender,
	int stepx, int stepy);
static int trackergrid_testcursor(TrackerGrid*, unsigned int track,
	big_beat_t offset, unsigned int subline);
int trackergrid_testselection(TrackerGrid*, unsigned int track, double offset,
	unsigned int subline);
static int trackergrid_testplaybar(TrackerGrid* self, big_beat_t offset);
static void trackergrid_clipblock(TrackerGrid*, const ui_rectangle*,
	TrackerGridBlock*);
static void trackergrid_drawdigit(TrackerGrid*, ui_graphics*, int digit,
	int col, int x, int y);
static void trackergrid_adjustscroll(TrackerGrid*);
static double trackergrid_offset(TrackerGrid*, int y, unsigned int* lines,
	unsigned int* sublines, unsigned int* subline);
static void trackergrid_numtrackschanged(TrackerGrid*, Player*,
	unsigned int numtracks);
// static void trackerview_onsize(TrackerView*, ui_component* sender, ui_size*);
static void trackerview_ondestroy(TrackerView*, ui_component* sender);
static void trackerview_onkeydown(TrackerView*, ui_component* sender,
	KeyEvent*);
static void trackerview_ontimer(TrackerView*, ui_component* sender, 
	int timerid);
static void trackerview_onalign(TrackerView*, ui_component* sender);
static void trackerview_inputnote(TrackerView*, note_t);
static void trackerview_inputdigit(TrackerView*, int value);
static void enterdigitcolumn(PatternEvent*, int column, int value);
static void enterdigit(int digit, int newval, unsigned char* val);
static int chartoint(char c);
static void trackerview_setcentermode(TrackerView*, int mode);
static void trackerview_prevcol(TrackerView*);
static void trackerview_nextcol(TrackerView*);
static void trackerview_prevline(TrackerView*);
static void trackerview_prevlines(TrackerView*, uintptr_t lines, int wrap);
static void trackerview_advanceline(TrackerView*);
static void trackerview_advancelines(TrackerView*, uintptr_t lines, int wrap);
static void trackerview_home(TrackerView*);
static void trackerview_end(TrackerView*);
static void trackerview_selectall(TrackerView*);
static void trackerview_selectcol(TrackerView*);
static void trackerview_prevtrack(TrackerView*);
static void trackerview_nexttrack(TrackerView*);
static void trackerview_enablesync(TrackerView*);
static void trackerview_preventsync(TrackerView*);
static int trackerview_scrollleft(TrackerView*);
static int trackerview_scrollright(TrackerView*);
static int trackerview_scrollup(TrackerView*);
static int trackerview_scrolldown(TrackerView*);
static void trackerview_showlinenumbers(TrackerView*, int showstate);
static void trackerview_showlinenumbercursor(TrackerView*, int showstate);
static void trackerview_showlinenumbersinhex(TrackerView*, int showstate);
static void trackerview_showemptydata(TrackerView*, int showstate);
static int trackerview_numlines(TrackerView*);
static void trackerview_setclassicheadercoords(TrackerView*);
static void trackerview_setheadercoords(TrackerView*);
static void trackerview_setheadertextcoords(TrackerView*);
static void trackerview_onconfigchanged(TrackerView*, Workspace*, Properties*);
static void trackerview_readconfig(TrackerView*);
static void trackerview_oninput(TrackerView*, Player*, PatternEvent*);
static void trackerview_initinputs(TrackerView*);
static void trackerview_invalidatecursor(TrackerView*, const TrackerCursor*);
static void trackerview_invalidateline(TrackerView*, beat_t offset);
static void trackerview_initdefaultskin(TrackerView*);
static int trackerview_offsettoscreenline(TrackerView*, big_beat_t);
static void trackerview_onchangegenerator(TrackerView* self);
static void trackerview_onchangeinstrument(TrackerView* self);
static void trackerview_blockstart(TrackerView*);
static void trackerview_blockend(TrackerView*);
static void trackerview_blockunmark(TrackerView*);
static void trackerview_showblockmenu(TrackerView*);
static void trackerview_hideblockmenu(TrackerView*);
static void trackerview_toggleblockmenu(TrackerView*);
static void trackerview_centeroncursor(TrackerView*);
static void trackerview_onblockcut(TrackerView*);
static void trackerview_onblockcopy(TrackerView*);
static void trackerview_onblockpaste(TrackerView*);
static void trackerview_onblockmixpaste(TrackerView*);
static void trackerview_onblockdelete(TrackerView*);
static void trackerview_onblocktransposeup(TrackerView*);
static void trackerview_onblocktransposedown(TrackerView*);
static void trackerview_onblocktransposeup12(TrackerView*);
static void trackerview_onblocktransposedown12(TrackerView*);
static void trackerview_onlpbchanged(TrackerView*, Player* sender,
	uintptr_t lpb);
static void trackerview_onpatterneditpositionchanged(TrackerView*,
	Workspace* sender);
static void trackerview_onparametertweak(TrackerView*,
	Workspace* sender, int slot, int tweak, int value);

static void trackerheader_ondraw(TrackerHeader*, ui_component* sender,
	ui_graphics* g);
static void trackerheader_onmousedown(TrackerHeader*, ui_component* sender,
	MouseEvent*);

static void OnLineNumbersLabelDraw(TrackerLineNumbersLabel*,
	ui_component* sender, ui_graphics*);
static void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel*,
	ui_component* sender);

static void trackerlinenumbers_ondraw(TrackerLineNumbers*,
	ui_component* sender, ui_graphics*);
static void trackerlinenumbers_invalidatecursor(TrackerLineNumbers*,
	const TrackerCursor*);
static void trackerlinenumbers_invalidateline(TrackerLineNumbers*,
	beat_t offset);
static int testrange(big_beat_t position, big_beat_t offset, big_beat_t width);
static int testrange_e(big_beat_t position, big_beat_t offset, big_beat_t width);
static int colgroupstart(int col);

enum {
	CMD_NAVUP,
	CMD_NAVDOWN,
	CMD_NAVLEFT,
	CMD_NAVRIGHT,
	CMD_NAVPAGEUP,	///< pgup
	CMD_NAVPAGEDOWN,///< pgdn
	CMD_NAVTOP,		///< home
	CMD_NAVBOTTOM,	///< end

	CMD_COLUMNPREV,	///< tab
	CMD_COLUMNNEXT,	///< s-tab

	CMD_BLOCKSTART,
	CMD_BLOCKEND,
	CMD_BLOCKUNMARK,
	CMD_BLOCKCUT,
	CMD_BLOCKCOPY,
	CMD_BLOCKPASTE,
	CMD_BLOCKMIX,

	CMD_ROWINSERT,
	CMD_ROWDELETE,
	CMD_ROWCLEAR,
			
	CMD_TRANSPOSEBLOCKINC,
	CMD_TRANSPOSEBLOCKDEC,
	CMD_TRANSPOSEBLOCKINC12,
	CMD_TRANSPOSEBLOCKDEC12,

	CMD_SELECTALL,
	CMD_SELECTCOL,
	CMD_SELECTBAR,

	CMD_UNDO,
	CMD_REDO,

	CMD_FOLLOWSONG
};

enum {
	TRACKER_COLUMN_NOTE	 = 0,
	TRACKER_COLUMN_INST	 = 1,
	TRACKER_COLUMN_MACH	 = 3,
	TRACKER_COLUMN_CMD	 = 5,
	TRACKER_COLUMN_PARAM = 7,
	TRACKER_COLUMN_END	 = 9
};

/// Commands
typedef struct {
	Command command;
	TrackerCursor cursor;
	Pattern* pattern;
	double bpl;
	PatternNode* node;	
	PatternEvent event;
	PatternEvent oldevent;
	int insert;
} InsertCommand;

static void InsertCommandDispose(InsertCommand*);
static void InsertCommandExecute(InsertCommand*);
static void InsertCommandRevert(InsertCommand*);

InsertCommand* InsertCommandAlloc(Pattern* pattern, double bpl,
	TrackerCursor cursor, PatternEvent event)
{
	InsertCommand* rv;
	
	rv = malloc(sizeof(InsertCommand));
	rv->command.dispose = InsertCommandDispose;
	rv->command.execute = InsertCommandExecute;
	rv->command.revert = InsertCommandRevert;
	rv->cursor = cursor;	
	rv->bpl = bpl;
	rv->event = event;
	rv->node = 0;
	rv->insert = 0;
	rv->pattern = pattern;	
	return rv;
}

void InsertCommandDispose(InsertCommand* self) { }

void InsertCommandExecute(InsertCommand* self)
{		
	PatternNode* prev;
	self->node = 
		pattern_findnode(self->pattern,
			self->cursor.track,
			(beat_t)self->cursor.offset,
			self->cursor.subline,
			(beat_t)self->bpl, &prev);	
	if (self->node) {
		self->oldevent = pattern_event(self->pattern, self->node);
		pattern_setevent(self->pattern, self->node, &self->event);
		self->insert = 0;
	} else {
		self->node = pattern_insert(self->pattern,
			prev,
			self->cursor.track, 
			(beat_t)self->cursor.offset,
			&self->event);
		self->insert = 1;
	}
}

void InsertCommandRevert(InsertCommand* self)
{		
	if (self->insert) {
		pattern_remove(self->pattern, self->node);
	} else {
		pattern_setevent(self->pattern, self->node, &self->oldevent);		
	}
}

/// TrackerGrid
void trackergrid_init(TrackerGrid* self, ui_component* parent,
	TrackerView* view, Player* player)
{		
	self->view = view;
	self->header = 0;	
	self->hasselection = 0;
	self->midline = 1;	
	signal_connect(&player->signal_numsongtrackschanged, self,
		trackergrid_numtrackschanged);
	self->view->font = ui_createfont("Tahoma", 12);	
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_size, self, trackergrid_onsize);
	signal_connect(&self->component.signal_keydown,self, 
		trackergrid_onkeydown);
	signal_connect(&self->component.signal_mousedown, self,
		trackergrid_onmousedown);
	signal_connect(&self->component.signal_mousemove, self,
		trackergrid_onmousemove);
	signal_connect(&self->component.signal_mouseup, self,
		trackergrid_onmouseup);
	signal_connect(&self->component.signal_mousedoubleclick, self,
		trackergrid_onmousedoubleclick);
	signal_connect(&self->component.signal_draw, self,
		trackergrid_ondraw);
	signal_connect(&self->component.signal_scroll, self,
		trackergrid_onscroll);	
	self->player = player;	
	self->numtracks = player_numsongtracks(player);
	self->lpb = player_lpb(self->player);
	self->bpl = 1 / (big_beat_t) player_lpb(self->player);
	self->notestabmode = NOTESTAB_DEFAULT;	
	self->cursor.track = 0;
	self->cursor.offset = 0;
	self->cursor.subline = 0;
	self->cursor.col = 0;
	workspace_setpatterneditposition(self->view->workspace, self->cursor);
	self->cursorstep = 0.25;
	self->dx = 0;
	self->dy = 0;

	self->textheight = 12;
	self->textwidth = 9;
	self->textleftedge = 2;
	self->colx[0] = 0;
	self->colx[1] = (self->textwidth*3)+2;
	self->colx[2] = self->colx[1]+self->textwidth;
	self->colx[3] = self->colx[2]+self->textwidth + 1;
	self->colx[4] = self->colx[3]+self->textwidth;
	self->colx[5] = self->colx[4]+self->textwidth + 1;
	self->colx[6] = self->colx[5]+self->textwidth;
	self->colx[7] = self->colx[6]+self->textwidth;
	self->colx[8] = self->colx[7]+self->textwidth;
	self->colx[9] = self->colx[8]+self->textwidth + 1;
	self->trackwidth = self->colx[9];		
	
	self->lineheight = self->textheight + 1;
	self->component.scrollstepx = self->trackwidth;
	self->component.scrollstepy = self->lineheight;
	ui_component_showhorizontalscrollbar(&self->component);
	ui_component_showverticalscrollbar(&self->component);
	trackergrid_adjustscroll(self);
	self->component.doublebuffered = 1;
	self->component.wheelscroll = 4;
}

void trackergrid_numtrackschanged(TrackerGrid* self, Player* player,
	unsigned int numsongtracks)
{	
	self->numtracks = numsongtracks;	
	self->view->header.numtracks = numsongtracks;
	trackergrid_adjustscroll(self);
	ui_component_invalidate(&self->view->component);
}

void TrackerViewApplyProperties(TrackerView* self, Properties* p)
{
	self->skin.separator = properties_int(p, "pvc_separator", 0x00292929);
	self->skin.separator2 = properties_int(p, "pvc_separator2", 0x00292929);
	self->skin.background = properties_int(p, "pvc_background", 0x00292929);
	self->skin.background2 = properties_int(p, "pvc_background2", 0x00292929);
	self->skin.row4beat = properties_int(p, "pvc_row4beat", 0x00595959);
	self->skin.row4beat2 = properties_int(p, "pvc_row4beat2", 0x00595959);
	self->skin.rowbeat = properties_int(p, "pvc_rowbeat", 0x00363636);
	self->skin.rowbeat2 = properties_int(p, "pvc_rowbeat2", 0x00363636);
	self->skin.row = properties_int(p, "pvc_row", 0x003E3E3E);
	self->skin.row2 = properties_int(p, "pvc_row2", 0x003E3E3E);
	self->skin.font = properties_int(p, "pvc_font", 0x00CACACA);
	self->skin.font2 = properties_int(p, "pvc_font2", 0x00CACACA );
	self->skin.fontPlay = properties_int(p, "pvc_fontplay", 0x00FFFFFF);
	self->skin.fontCur2 = properties_int(p, "pvc_fontcur2", 0x00FFFFFF);
	self->skin.fontSel = properties_int(p, "pvc_fontsel", 0x00FFFFFF);
	self->skin.fontSel2 = properties_int(p, "pvc_fontsel2", 0x00FFFFFF);
	self->skin.selection = properties_int(p, "pvc_selection", 0x009B7800);
	self->skin.selection2 = properties_int(p, "pvc_selection2", 0x009B7800);
	self->skin.playbar = properties_int(p, "pvc_playbar", 0x009F7B00);
	self->skin.playbar2 = properties_int(p, "pvc_playbar2", 0x009F7B00);
	self->skin.cursor = properties_int(p, "pvc_cursor", 0x009F7B00);
	self->skin.cursor2 = properties_int(p, "pvc_cursor2", 0x009F7B00);
	self->skin.midline = properties_int(p, "pvc_midline", 0x007D6100);
	self->skin.midline2 = properties_int(p, "pvc_midline2", 0x007D6100);
	ui_component_setbackgroundcolor(
		&self->linenumbers.component, self->skin.background);	
	ui_component_invalidate(&self->component);
}

void trackergrid_ondraw(TrackerGrid* self, ui_component* sender, ui_graphics* g)
{	 
  	TrackerGridBlock clip;
	if (self->view->pattern) {
		ui_setfont(g, &self->view->font);
		trackergrid_clipblock(self, &g->clip, &clip);
		trackergrid_drawbackground(self, g, &clip);
		trackergrid_drawevents(self, g, &clip);
	} else {
		ui_drawsolidrectangle(g, g->clip, self->view->skin.background);	
	}
}

big_beat_t trackergrid_offset(TrackerGrid* self, int y, unsigned int* lines,
	unsigned int* sublines, unsigned int* subline)
{
	double offset = 0;	
	int cpy = 0;		
	int first = 1;
	unsigned int count;
	unsigned int remaininglines = 0;
	
	count = (y >= 0) ? y / self->lineheight : 0;
	if (self->view->pattern) {
		PatternNode* curr = self->view->pattern->events;
		*lines = 0;
		*sublines = 0;	
		*subline = 0;
		while (curr) {		
			PatternEntry* entry;		
			first = 1;
			do {
				entry = (PatternEntry*)curr->entry;			
				if ((entry->offset >= offset) && (entry->offset < offset + self->bpl))
				{
					if ((*lines + *sublines) >= count) {
						break;
					}
					if (entry->track == 0 && !first) {
						++(*sublines);
						++*subline;
					}							
					first = 0;
					curr = curr->next;
				} else {
					*subline = 0;
					break;
				}
				if ((int)(*lines + *sublines) >= count) {
					break;
				}
			} while (curr);
			if ((*lines + *sublines) >= count) {
				break;
			}
			++(*lines);
			*subline = 0;		
			offset += self->bpl;
		}	
		remaininglines =  (count - (*lines + *sublines));
		*lines += remaininglines;
	}
	return offset + remaininglines * self->bpl;
}

void trackergrid_clipblock(TrackerGrid* self, const ui_rectangle* clip,
	TrackerGridBlock* block)
{	
	int lines;
	int sublines;
	int subline;
	block->topleft.track = (clip->left - self->dx) / self->trackwidth;
	block->topleft.col = 0;
	block->topleft.offset =  trackergrid_offset(self, clip->top - self->dy,
		&lines, &sublines, &subline);
	block->topleft.line = lines;
	block->topleft.subline = subline;
	block->topleft.totallines = lines + sublines;
	block->bottomright.track = (clip->right - self->dx + self->trackwidth) / self->trackwidth;
	if (block->bottomright.track > self->numtracks) {
		block->bottomright.track = self->numtracks;
	}
	block->bottomright.col = 0;
	block->bottomright.offset = trackergrid_offset(self, clip->bottom - self->dy,
		&lines, &sublines, &subline);
	block->bottomright.line = lines;
	block->bottomright.totallines = lines + sublines;
	block->bottomright.subline = subline;
}

void trackergrid_drawbackground(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* clip)
{
	ui_rectangle r;
	unsigned int track;

	for (track = clip->topleft.track; track < clip->bottomright.track;
			++track) {
		trackergrid_drawtrackbackground(self, g, track);
	}
	ui_setrectangle(&r, self->numtracks * self->trackwidth + self->dx, 0, 
		self->cx - (self->numtracks * self->trackwidth + self->dx), self->cy);
	ui_drawsolidrectangle(g, r, self->view->skin.background);
}

void trackergrid_drawtrackbackground(TrackerGrid* self, ui_graphics* g, int track)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, track * self->trackwidth + self->dx, 0,
		self->trackwidth, self->cy);
	ui_drawsolidrectangle(g, r, self->view->skin.background);	
}

int trackergrid_testselection(TrackerGrid* self, unsigned int track, double offset,
	unsigned int subline)
{
	return self->hasselection &&
		track >= self->selection.topleft.track &&
		track < self->selection.bottomright.track &&
		offset >= self->selection.topleft.offset &&
		offset < self->selection.bottomright.offset;
}

int trackergrid_testcursor(TrackerGrid* self, unsigned int track, double offset,
	unsigned int subline)
{
	return self->cursor.track == track && 
		testrange(self->cursor.offset, offset, self->bpl) &&
		self->cursor.subline == subline;
}

int trackergrid_testplaybar(TrackerGrid* self, big_beat_t offset)
{
	return player_playing(self->player) && 
		testrange(self->view->lastplayposition -
				      self->view->sequenceentryoffset,
				  offset, self->bpl); 	
}

void trackergrid_drawevents(TrackerGrid* self, ui_graphics* g, TrackerGridBlock* clip)
{	
	unsigned int track;
	int cpx = 0;	
	int cpy;
	double offset;	
	int subline;	
	int line = 0;
	int mid = 0;
	PatternNode* node;
	ui_size size;
	int halfy;
	
	size = ui_component_size(&self->component);
	halfy = (size.height / self->lineheight / 2) * self->lineheight;
	cpy = (clip->topleft.totallines - clip->topleft.subline) * self->lineheight + self->dy;
	offset = clip->topleft.offset;	
	node = pattern_greaterequal(self->view->pattern, (beat_t)offset);
	subline = 0;	
	while (offset <= clip->bottomright.offset && offset < self->view->pattern->length) {	
		int beat;
		int beat4;
		int fill;
		
		beat = fabs(fmod(offset, 1.0f)) < 0.01f ;
		beat4 = fabs(fmod(offset, 4.0f)) < 0.01f;
		mid = self->midline && cpy >= halfy && cpy < halfy + self->lineheight;
		do {
			fill = 0;
			cpx = clip->topleft.track * self->trackwidth + self->dx;						
			for (track =  clip->topleft.track; track < clip->bottomright.track;
					++track) {
				int hasevent = 0;
				int cursor;
				int selection;
				int playbar;

				cursor = trackergrid_testcursor(self, track, offset, subline);
				selection = trackergrid_testselection(self, track, offset,
					subline);
				playbar = trackergrid_testplaybar(self, offset);
				while (!fill && node &&
					   ((PatternEntry*)(node->entry))->track <= track &&
					   testrange_e(((PatternEntry*)(node->entry))->offset,
						  offset,
						  self->bpl)) {
					PatternEntry* entry;
										
					entry = (PatternEntry*)(node->entry);					
					if (entry->track == track) {
						trackergrid_drawevent(self, g, &entry->event, 
							cpx, cpy, playbar, cursor, selection, beat, beat4, mid);
						node = node->next;
						hasevent = 1;
						break;
					}
					node = node->next;
				}
				if (!hasevent) {
					PatternEvent event;
					memset(&event, 0xFF, sizeof(PatternEvent));
					event.cmd = 0;
					event.parameter = 0;
					trackergrid_drawevent(self, g, &event, cpx, cpy, playbar,
						cursor, selection, beat, beat4, mid);
				} else
				if (node && ((PatternEntry*)(node->entry))->track <= track) {
					fill = 1;
				}
				cpx += self->trackwidth;			
			}
			// skip remaining tracks
			while (node && ((PatternEntry*)(node->entry))->track > 0 &&
					testrange_e(((PatternEntry*)(node->entry))->offset,
						offset, self->bpl)) {
				node = node->next;
			}
			cpy += self->lineheight;
			++line;
			++subline;
		} while (node &&
			((PatternEntry*)(node->entry))->offset + 2*epsilon < offset + self->bpl);
		offset += self->bpl;
		subline = 0;
	}
}

int testrange(big_beat_t position, big_beat_t offset, big_beat_t width)
{
	return position >= offset && position < offset + width; 
}

int testrange_e(big_beat_t position, big_beat_t offset, big_beat_t width)
{
	return position + 2*epsilon >= offset &&
		position < offset + width - epsilon;
}

void SetColColor(TrackerSkin* skin, ui_graphics* g, int col, int playbar, int cursor, int selection, int beat, int beat4, int mid)
{	
	if (cursor != 0) {
		ui_setbackgroundcolor(g, skin->cursor);
		ui_settextcolor(g, skin->fontCur);
	} else
	if (playbar) {
		ui_setbackgroundcolor(g, skin->playbar);
		ui_settextcolor(g, skin->fontPlay);		
	} else		
	if (selection != 0) {		
		ui_setbackgroundcolor(g, skin->cursor);
		ui_settextcolor(g, skin->fontCur);		
	} else 
	if (mid) {
		ui_setbackgroundcolor(g, skin->midline);
		if (cursor != 0) {
			ui_settextcolor(g, skin->fontCur);
		} else {
			ui_settextcolor(g, skin->font);
		}
	} else {	
		if (beat4) {
			ui_setbackgroundcolor(g, skin->row4beat);			
			ui_settextcolor(g, skin->font);
		} else
		if (beat) {
			ui_setbackgroundcolor(g, skin->rowbeat);
			ui_settextcolor(g, skin->font);
		} else {
			ui_setbackgroundcolor(g, skin->row);
			ui_settextcolor(g, skin->font);
		}
	}	
}

void trackergrid_drawevent(TrackerGrid* self, ui_graphics* g, PatternEvent* event,
	int x, int y, int playbar, int cursor, int selection, int beat, int beat4, int mid)
{					
	ui_rectangle r;
	static const char* emptynotestr = "- - -";
	const char* notestr;
		
	SetColColor(&self->view->skin, g, 0, playbar, cursor && self->cursor.col == 0, 
		selection, beat, beat4, mid);
	{	// draw note		
		ui_setrectangle(&r, x + self->colx[0], y, self->textwidth*3, self->textheight);
		notestr = (event->note != 255 || !self->view->showemptydata) 
			  ? notetostr(event->note, self->notestabmode)
			  : emptynotestr;		
		ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, r, notestr,
		 strlen(notestr));	
	}
	{	// draw inst
		int hi = (event->inst & 0xF0) >> 4;
		int lo = event->inst & 0x0F;
		if (event->inst == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 1, playbar, cursor && (self->cursor.col == 1), 
			selection, beat, beat4,mid);
		trackergrid_drawdigit(self, g, hi, 1, x, y);
		SetColColor(&self->view->skin, g, 2, playbar, cursor && (self->cursor.col == 2),
			selection, beat, beat4, mid);
		trackergrid_drawdigit(self, g, lo, 2, x, y);
	}
	{	// draw mach
		int hi = (event->mach & 0xF0) >> 4;
		int lo = event->mach & 0x0F;
		if (event->mach == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 3, playbar, cursor && (self->cursor.col == 3),
			selection, beat, beat4, mid);
		trackergrid_drawdigit(self, g, hi, 3, x, y);
		SetColColor(&self->view->skin, g, 4, playbar, cursor && (self->cursor.col == 4),
			selection, beat, beat4,mid);
		trackergrid_drawdigit(self, g, lo, 4, x, y);
	}
	{	// draw cmd
		int hi = (event->cmd & 0xF0) >> 4;
		int lo = event->cmd & 0x0F;				
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 5, playbar, cursor && (self->cursor.col == 5),
			selection, beat, beat4, mid);
		trackergrid_drawdigit(self, g, hi, 5, x, y);
		SetColColor(&self->view->skin, g, 6, playbar, cursor && (self->cursor.col == 6),
			selection, beat, beat4, mid);
		trackergrid_drawdigit(self, g, lo, 6, x, y);
	}
	{	// draw parameter
		int hi = (event->parameter & 0xF0) >> 4;
		int lo = event->parameter & 0x0F;		
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 7, playbar, cursor && (self->cursor.col == 7),
			selection, beat, beat4,mid);
		trackergrid_drawdigit(self, g, hi, 7, x, y);
		SetColColor(&self->view->skin, g, 8, playbar, cursor && (self->cursor.col == 8),
			selection, beat, beat4, mid);
		trackergrid_drawdigit(self, g, lo, 8, x, y);
	}			
}

void trackergrid_drawdigit(TrackerGrid* self, ui_graphics* g, int digit, int col, int x, int y)
{
	char buffer[20];	
	ui_rectangle r;
	ui_setrectangle(&r, x + self->colx[col], y, self->textwidth, self->textheight);
	if (digit != -1) {
		psy_snprintf(buffer, 2, "%X", digit);	
	} else {
		if (self->view->showemptydata) {
			psy_snprintf(buffer, 2, "%s", ".");
		} else {
			psy_snprintf(buffer, 2, "%s", "");	
		}
	}
	ui_textoutrectangle(g, r.left + self->textleftedge, r.top,
		ETO_OPAQUE | ETO_CLIPPED, r, buffer, strlen(buffer));	
}

void trackergrid_onsize(TrackerGrid* self, ui_component* sender, ui_size* size)
{
	self->cx = size->width;
	self->cy = size->height;
	trackergrid_adjustscroll(self);
}

void trackergrid_adjustscroll(TrackerGrid* self)
{
	ui_size size;
	int visitracks;
	int visilines;
	int vscrollmax;
	size = ui_component_size(&self->component);	
	visitracks = size.width / self->trackwidth;
	visilines = size.height / self->lineheight;
	ui_component_sethorizontalscrollrange(&self->component, 0,
		self->numtracks - visitracks);
	vscrollmax = trackerview_numlines(self->view);
	if (!self->midline) {
		vscrollmax -= visilines;
	} else {
		vscrollmax -= 1;
	}
	ui_component_setverticalscrollrange(&self->component, 0, vscrollmax);
	if (self->midline) {
		
		trackerview_centeroncursor(self->view);
	}
}

void trackerview_centeroncursor(TrackerView* self)
{
	int line;
	int visilines;
	ui_size size;

	size = ui_component_size(&self->grid.component);
	visilines = size.height / self->grid.lineheight;
	line = trackerview_offsettoscreenline(self, 
		self->grid.cursor.offset) + self->grid.cursor.subline;
	self->grid.dy = (visilines / 2 - line) * self->grid.lineheight;
	self->linenumbers.dy = self->grid.dy;
	ui_component_setverticalscrollposition(&self->grid.component,
		line);
}

unsigned int NumSublines(Pattern* pattern, double offset, double bpl)
{
	PatternNode* node = pattern_greaterequal(pattern, (beat_t)offset);	
	unsigned int currsubline = 0;
	int first = 1;

	while (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl) {			
			break;
		}				
		if (entry->track == 0 && !first) {
			++currsubline;			
		}
		node = node->next;
		first = 0;
	}
	return currsubline;
}

void trackergrid_onkeydown(TrackerGrid* self, ui_component* sender,
	KeyEvent* keyevent)
{
	sender->propagateevent = 1;	
}

void trackerview_prevcol(TrackerView* self)
{
	int invalidate = 1;
	TrackerCursor oldcursor;
	oldcursor = self->grid.cursor;

	if (self->grid.cursor.col == 0) {		
		if (self->wraparound) {
			self->grid.cursor.col = TRACKERGRID_numparametercols - 2;
		}
		if (self->grid.cursor.track > 0) {
			--self->grid.cursor.track;
			trackerview_scrollleft(self);
		} else 
		if (self->wraparound) {			
			self->grid.cursor.track = player_numsongtracks(
				&self->workspace->player) - 1;
			invalidate = trackerview_scrollright(self);
		}
	} else {
		--self->grid.cursor.col;
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);	
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

void trackerview_nextcol(TrackerView* self)
{
	int invalidate = 1;
	TrackerCursor oldcursor;
	oldcursor = self->grid.cursor;	

	if (self->grid.cursor.col == TRACKERGRID_numparametercols - 2) {
		if (self->wraparound) {
			self->grid.cursor.col = 0;
		}
		if (self->grid.cursor.track < player_numsongtracks(
			&self->workspace->player) - 1) {
			++self->grid.cursor.track;
			invalidate = trackerview_scrollright(self);
		} else 
		if (self->wraparound) {
			self->grid.cursor.track = 0;
			invalidate = trackerview_scrollleft(self);
		}
	} else {
		++self->grid.cursor.col;
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

void trackerview_prevline(TrackerView* self)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;
	if (self->grid.cursor.subline > 0) {
		--self->grid.cursor.subline;
	} else {
		trackerview_prevlines(self, workspace_cursorstep(self->workspace),
			self->wraparound);
	}	
}

void trackerview_prevlines(TrackerView* self, uintptr_t lines, int wrap)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;
	self->grid.cursor.subline = 0;
	self->grid.cursor.offset -= lines * self->grid.bpl;		
	if (self->grid.cursor.offset < 0) {
		if (wrap) {
			self->grid.cursor.offset += self->pattern->length;
			if (self->grid.cursor.offset < 0) {
				self->grid.cursor.offset = 0;
			}
			trackerview_scrolldown(self);
		} else {
			self->grid.cursor.offset = 0;
			trackerview_scrollup(self);
		}			
	} else {
		trackerview_scrollup(self);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);		
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);	
}

void trackerview_advanceline(TrackerView* self)
{	
	if (self->grid.cursor.subline < 
		NumSublines(self->pattern, self->grid.cursor.offset, self->grid.bpl)) {
		TrackerCursor oldcursor;
		oldcursor = self->grid.cursor;
		++self->grid.cursor.subline;
		workspace_setpatterneditposition(self->workspace, self->grid.cursor);
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);		
	} else {
		trackerview_advancelines(self, workspace_cursorstep(self->workspace),
			self->wraparound);
	}	
}

void trackerview_advancelines(TrackerView* self, uintptr_t lines, int wrap)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;		
	self->grid.cursor.offset += lines * self->grid.bpl;
	self->grid.cursor.subline = 0;
	if (self->grid.cursor.offset >= self->pattern->length) {
		if (wrap) {
			self->grid.cursor.offset = self->grid.cursor.offset -
				self->pattern->length;
			if (self->grid.cursor.offset > self->pattern->length - self->grid.bpl) {
				self->grid.cursor.offset = self->pattern->length - self->grid.bpl;
			}
			trackerview_scrollup(self);
		} else {
			self->grid.cursor.offset = self->pattern->length -
				self->grid.bpl;
			trackerview_scrolldown(self);
		}
	} else {
		trackerview_scrolldown(self);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);
}

void trackerview_home(TrackerView* self)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;		
	self->grid.cursor.offset = 0;
	self->grid.cursor.subline = 0;	
	trackerview_scrollup(self);	
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);
}

void trackerview_end(TrackerView* self)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;		
	self->grid.cursor.offset = self->pattern->length - self->grid.bpl;
	self->grid.cursor.subline = 0;	
	trackerview_scrolldown(self);	
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);
	trackerview_invalidatecursor(self, &oldcursor);
	trackerview_invalidatecursor(self, &self->grid.cursor);
}

void trackerview_selectall(TrackerView* self)
{
	if (self->workspace->song && self->pattern) {
		self->grid.selection.topleft.offset = 0;
		self->grid.selection.topleft.track = 0;
		self->grid.selection.bottomright.offset = self->pattern->length;
		self->grid.selection.bottomright.track = 
			self->workspace->song->patterns.songtracks;
		self->grid.hasselection = 1;
		ui_component_invalidate(&self->component);
	}			
}

void trackerview_selectcol(TrackerView* self)
{
	if (self->workspace->song && self->pattern) {
		self->grid.selection.topleft.offset = 0;
		self->grid.selection.topleft.track = self->grid.cursor.track;
		self->grid.selection.bottomright.offset = self->pattern->length;
		self->grid.selection.bottomright.track = self->grid.cursor.track + 1;
		self->grid.hasselection = 1;
		ui_component_invalidate(&self->component);
	}		
}

void trackerview_selectbar(TrackerView* self)
{
	if (self->workspace->song && self->pattern) {
		self->grid.selection.topleft.offset = self->grid.cursor.offset;
		self->grid.selection.topleft.track = self->grid.cursor.track;
		self->grid.selection.bottomright.offset = self->grid.cursor.offset + 4.0;
		if (self->grid.cursor.offset > self->pattern->length) {
			self->grid.cursor.offset = self->pattern->length;
		}
		self->grid.selection.bottomright.track = self->grid.cursor.track + 1;
		self->grid.hasselection = 1;
		ui_component_invalidate(&self->component);
	}		
}

int trackerview_scrollup(TrackerView* self)
{
	int line;
	int topline;
	int rv = 1;
	ui_rectangle r;		
	
	line = trackerview_offsettoscreenline(self, self->grid.cursor.offset)
		+ self->grid.cursor.subline;
	ui_setrectangle(&r,
		self->grid.cursor.track * self->grid.trackwidth,
		self->grid.lineheight * line,		
		self->grid.trackwidth,
		self->grid.lineheight);
	if (self->grid.midline) {
		topline = ui_component_size(&self->grid.component).height  
			/ self->grid.lineheight / 2;		
	} else {
		topline = 0;
	}
	if (-self->grid.dy + topline * self->grid.lineheight > r.top) {
		int dlines = (-self->grid.dy + topline * self->grid.lineheight - r.top) / (self->grid.lineheight);
		signal_emit(&self->grid.component.signal_scroll, &self->grid.component,
			2, 0, dlines);
		ui_component_scrollstep(&self->grid.component, 0, dlines);
		ui_component_setverticalscrollposition(
		&self->grid.component, 
		ui_component_verticalscrollposition(&self->grid.component) - dlines);
		rv = 0;
	}	
	return rv;
}

int trackerview_scrolldown(TrackerView* self)
{
	int line;
	int visilines;
	int rv = 1;	
	
	visilines = ui_component_size(&self->grid.component).height  / self->grid.lineheight;
	if (self->grid.midline) {
		visilines /= 2;
	} else {
		--visilines;
	}
	line = trackerview_offsettoscreenline(self, self->grid.cursor.offset)
		+ self->grid.cursor.subline;	
	if (visilines < line + self->grid.dy / self->grid.lineheight) {
		int dlines;

		dlines = line + self->grid.dy / self->grid.lineheight - visilines;
		signal_emit(&self->grid.component.signal_scroll, &self->grid.component,
			2, 0, -dlines);
		ui_component_scrollstep(&self->grid.component, 0, -dlines);
		ui_component_setverticalscrollposition(
			&self->grid.component, 
			ui_component_verticalscrollposition(&self->grid.component) + dlines);
		rv = 0;		
	}
	return rv;
}

void trackerview_prevtrack(TrackerView* self)
{
	TrackerCursor oldcursor;
	int invalidate = 1;

	oldcursor = self->grid.cursor;
	self->grid.cursor.col = 0;
	if (self->grid.cursor.track > 0) {
		--self->grid.cursor.track;
		trackerview_scrollleft(self);
	} else 
	if (self->wraparound) {
		self->grid.cursor.track = 
			player_numsongtracks(&self->workspace->player) - 1;
		invalidate = trackerview_scrollright(self);
	}
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);	
}

void trackerview_nexttrack(TrackerView* self)
{
	TrackerCursor oldcursor;
	int invalidate = 1;

	oldcursor = self->grid.cursor;
	self->grid.cursor.col = 0;
	if (self->grid.cursor.track < 
		player_numsongtracks(&self->workspace->player) - 1) {
		++self->grid.cursor.track;
		invalidate = trackerview_scrollright(self);
	} else
	if (self->wraparound) {
		self->grid.cursor.track = 0;
		invalidate = trackerview_scrollleft(self);
	}
	workspace_setpatterneditposition(self->workspace, self->grid.cursor);		
	if (invalidate) {
		trackerview_invalidatecursor(self, &oldcursor);
		trackerview_invalidatecursor(self, &self->grid.cursor);
	}
}

int trackerview_scrollleft(TrackerView* self)
{	
	int tracks;
	int invalidate = 1;
	
	tracks = self->grid.cursor.track;
	if (-self->grid.dx / self->grid.trackwidth > tracks) {
		self->grid.dx = -tracks * self->grid.trackwidth;
		self->header.dx = self->grid.dx;
		ui_component_invalidate(&self->grid.component);
		ui_component_update(&self->grid.component);
		ui_component_invalidate(&self->header.component);
		ui_component_update(&self->header.component);
		ui_component_sethorizontalscrollposition(&self->grid.component, 0);
		invalidate = 0;
	}
	return invalidate;
}

int trackerview_scrollright(TrackerView* self)
{
	int invalidate = 1;
	int visitracks;
	int tracks;
		
	visitracks = ui_component_size(&self->component).width /
		self->grid.trackwidth;
	tracks = self->grid.cursor.track + 2;
	if (visitracks - self->grid.dx / self->grid.trackwidth < tracks) {
		self->grid.dx = -(tracks - visitracks) * self->grid.trackwidth;
		self->header.dx = self->grid.dx;
		ui_component_invalidate(&self->header.component);		
		ui_component_update(&self->header.component);
		ui_component_invalidate(&self->grid.component);		
		ui_component_update(&self->grid.component);
		ui_component_sethorizontalscrollposition(&self->grid.component, 
			tracks - visitracks);
		invalidate = 0;
	}			
	return invalidate;
}

void trackerview_onkeydown(TrackerView* self, ui_component* sender,
	KeyEvent* keyevent)
{		
	int cmd;

	cmd = inputs_cmd(&self->inputs, encodeinput(keyevent->keycode, 
		GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0));		
	if (cmd == CMD_NAVUP) {
		trackerview_prevline(self);
	} else
	if (cmd == CMD_NAVPAGEUP) {		
		trackerview_prevlines(self, 
			player_lpb(&self->workspace->player), 0);
	}
	if (cmd == CMD_NAVDOWN) {		
		trackerview_advanceline(self);
	} else
	if (cmd == CMD_NAVPAGEDOWN) {
		trackerview_advancelines(self, 
			player_lpb(&self->workspace->player), 0);
	} else
	if (cmd == CMD_NAVLEFT) {
		trackerview_prevcol(self);
	} else
	if (cmd == CMD_NAVRIGHT) {
		trackerview_nextcol(self);
	} else
	if (cmd == CMD_NAVTOP) {
		trackerview_home(self);
	} else
	if (cmd == CMD_NAVBOTTOM) {
		trackerview_end(self);
	} else
	if (cmd == CMD_COLUMNPREV) {
		trackerview_prevtrack(self);
	} else
	if (cmd == CMD_COLUMNNEXT) {
		trackerview_nexttrack(self);
	} else
	if (cmd == CMD_BLOCKSTART) {
		trackerview_blockstart(self);
	} else	
	if (cmd == CMD_BLOCKEND) {
		trackerview_blockend(self);
	} else
	if (cmd == CMD_BLOCKUNMARK) {
		trackerview_blockunmark(self);
	} else
	if (cmd == CMD_BLOCKCUT) {
		trackerview_onblockcut(self);
	} else
	if (cmd == CMD_BLOCKCOPY) {
		trackerview_onblockcopy(self);
	} else
	if (cmd == CMD_BLOCKPASTE) {
		trackerview_onblockpaste(self);
	} else
	if (cmd == CMD_BLOCKMIX) {
		trackerview_onblockmixpaste(self);
	} else
	if (cmd == CMD_BLOCKMIX) {
		trackerview_onblockmixpaste(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKINC) {
		trackerview_onblocktransposeup(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKDEC) {
		trackerview_onblocktransposedown(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKINC12) {
		trackerview_onblocktransposeup12(self);
	} else
	if (cmd == CMD_TRANSPOSEBLOCKDEC12) {
		trackerview_onblocktransposedown12(self);
	} else
	if (cmd == CMD_ROWDELETE) {
		if (self->grid.cursor.offset - self->grid.bpl >= 0 && self->grid.cursor.subline == 0) {
			PatternNode* prev;
			PatternNode* p;
			PatternNode* q;
			PatternNode* node;	

			trackerview_prevline(self);		
			node = pattern_findnode(self->pattern, self->grid.cursor.track,
				(beat_t)self->grid.cursor.offset, self->grid.cursor.subline, (beat_t)self->grid.bpl, &prev);		
			if (node) {			
				pattern_remove(self->pattern, node);
				ui_component_invalidate(&self->linenumbers.component);
			}
			if (prev) {
				p = prev->next;
			} else {
				p = self->pattern->events;
			}
			for (; p != 0; p = q) {
				PatternEntry* entry;

				q = p->next;
				entry = (PatternEntry*) p->entry;								
				if (entry->track == self->grid.cursor.track) {
					PatternEvent event;
					beat_t offset;
					uintptr_t track;
					PatternNode* node;
					PatternNode* prev;

					event = entry->event;
					offset = entry->offset;
					track = entry->track;
					pattern_remove(self->pattern, p);
					offset -= (beat_t) self->grid.bpl;
					node = pattern_findnode(self->pattern, track,
						offset, 
						self->grid.cursor.subline,
						(beat_t)self->grid.bpl,
						&prev);
					if (node) {
						PatternEntry* entry;

						entry = (PatternEntry*) node->entry;
						entry->event = event;
					} else {
						pattern_insert(self->pattern, prev, track, 
							(beat_t)offset, &event);
					}					
				}
			}
			ui_component_invalidate(&self->component);
		}
	} else
	if (cmd == CMD_ROWCLEAR) {
		PatternNode* prev;
		PatternNode* node = pattern_findnode(self->pattern, self->grid.cursor.track,
			(beat_t)self->grid.cursor.offset, self->grid.cursor.subline, (beat_t)self->grid.bpl, &prev);
		if (node) {
			pattern_remove(self->pattern, node);
			ui_component_invalidate(&self->linenumbers.component);
		}
		trackerview_advanceline(self);	
	} else
	if (cmd == CMD_SELECTALL) {	
		trackerview_selectall(self);
	} else
	if (cmd == CMD_SELECTCOL) {
		trackerview_selectcol(self);
	} else
	if (cmd == CMD_SELECTBAR) {
		trackerview_selectbar(self);
	} else
	if (cmd == CMD_UNDO) {
		workspace_undo(self->workspace);
	} else
	if (cmd == CMD_REDO) {
		workspace_redo(self->workspace);
	} else
	if (cmd == CMD_FOLLOWSONG) {
		self->workspace->followsong = !self->workspace->followsong;
	} else
	if (keyevent->keycode == VK_RETURN) {
		PatternNode* prev;
		PatternNode* node = pattern_findnode(self->pattern, 0,
			(beat_t)self->grid.cursor.offset, self->grid.cursor.subline + 1, (beat_t)self->grid.bpl, &prev);		
		if (prev && ((PatternEntry*)prev->entry)->offset >= self->grid.cursor.offset) {
			PatternEvent ev = { 255, 255, 255, 0, 0 };
			double offset;
			++self->grid.cursor.subline;
			offset = self->grid.cursor.offset + self->grid.cursor.subline*self->grid.bpl/4;
			pattern_insert(self->pattern, prev, 0, (beat_t)offset, &ev);			
			trackergrid_adjustscroll(&self->grid);
			workspace_setpatterneditposition(self->workspace, self->grid.cursor);
			ui_component_invalidate(&self->linenumbers.component);
		}	
	} else {
		if (self->grid.cursor.col != TRACKER_COLUMN_NOTE) {			
			trackerview_inputdigit(self, chartoint((char)keyevent->keycode));
		}		
	}	
	ui_component_propagateevent(sender);
}

void trackerview_oninput(TrackerView* self, Player* sender, PatternEvent* event)
{
	if (ui_component_hasfocus(&self->grid.component) &&
			self->grid.cursor.col == TRACKER_COLUMN_NOTE) {		
		trackerview_inputnote(self, event->note);		
	}
}

void trackerview_enablesync(TrackerView* self)
{
	self->opcount = self->pattern->opcount;
	self->syncpattern = 1;
}

void trackerview_preventsync(TrackerView* self)
{
	self->opcount = self->pattern->opcount;
	self->syncpattern = 0;
}

void trackerview_inputnote(TrackerView* self, note_t note)
{
	Machine* machine;
	PatternEvent event;
				
	patternevent_init(&event,
		note,
		255,
		(unsigned char)machines_slot(&self->workspace->song->machines),
		0,
		0);
	machine = machines_at(&self->workspace->song->machines, event.mach);
	if (machine && 
			machine_supports(machine, MACHINE_USES_INSTRUMENTS)) {
		event.inst = self->workspace->song->instruments.slot;
	}
	trackerview_preventsync(self);
	undoredo_execute(&self->workspace->undoredo,
		&InsertCommandAlloc(self->pattern, self->grid.bpl,
			self->grid.cursor, event)->command);	
	trackerview_advanceline(self);
	trackerview_enablesync(self);
}

void trackerview_inputdigit(TrackerView* self, int value)
{
	if (self->pattern && value != -1) {
		PatternNode* prev;	
		PatternEvent event;
		PatternNode* node;
		
		node = pattern_findnode(self->pattern,
					self->grid.cursor.track,
					(beat_t)self->grid.cursor.offset,
					self->grid.cursor.subline,
					(beat_t)self->grid.bpl,
					&prev);						
		event = pattern_event(self->pattern, node);
		enterdigitcolumn(&event, self->grid.cursor.col, value);
		trackerview_preventsync(self);
		undoredo_execute(&self->workspace->undoredo,
				&InsertCommandAlloc(self->pattern, self->grid.bpl,
					self->grid.cursor, event)->command);
		if (colgroupstart(self->grid.cursor.col + 1) != 
				colgroupstart(self->grid.cursor.col)) {
			self->grid.cursor.col = colgroupstart(self->grid.cursor.col);
			trackerview_advanceline(self);			
		} else {
			trackerview_nextcol(self);
		}
		trackerview_invalidatecursor(self, &self->grid.cursor);
		trackerview_enablesync(self);
	}
}

int colgroupstart(int col)
{
	if (col == 1 || col == 2) {
		return 1;
	} else
	if (col == 3 || col == 4) {
		return 3;
	} else
		if (col == 5 || col == 6 || col == 7 || col == 8) {
		return 5;
	} else {
		return 0;
	}
}

int chartoint(char c) {
	int rv = -1;

	if (c >= '0' && c <='9') {
		rv = c - '0';
	} else
	if (c >= 'A' && c <='Z') {
		rv = c - 'A' + 10;
	} else
	if (c >= 'a' && c <='z') {
		rv = c - 'a' + 10;
	}
	return rv;
}

void enterdigitcolumn(PatternEvent* event, int column, int value)
{
	switch (column) {
		case 1: 
			if ((event->inst == 0xFF) && (value != 0x0F)) {
				event->inst = 0;
			}
			enterdigit(0, value, &event->inst);
		break;
		case 2:
			if ((event->inst == 0xFF) && (value != 0x0F)) {
				event->inst = 0;
			}
			enterdigit(1, value, &event->inst);
		break;
		case 3:
			if ((event->mach == 0xFF) && (value != 0x0F)) {
				event->mach = 0;
			}
			enterdigit(0, value, &event->mach);
		break;
		case 4:
			if ((event->mach == 0xFF) && (value != 0x0F)) {
				event->mach = 0;
			}
			enterdigit(1, value, &event->mach);
		break;
		case 5:							
			enterdigit(0, value, &event->cmd);
		break;
		case 6:
			enterdigit(1, value, &event->cmd);
		break;
		case 7:
			enterdigit(0, value, &event->parameter);
		break;
		case 8:
			enterdigit(1, value, &event->parameter);
		break;
		default:
		break;
	}
}

void enterdigit(int digit, int newval, unsigned char* val)
{	
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else
	if (digit == 1) {
		*val = (*val & 0xF0) | (newval & 0x0F);
	}
}

void trackerview_invalidatecursor(TrackerView* self, const TrackerCursor* cursor)
{
	int line;		
	ui_rectangle r;		

	line = trackerview_offsettoscreenline(self, cursor->offset)
		+ cursor->subline;	
	ui_setrectangle(&r,
		cursor->track * self->grid.trackwidth + self->grid.dx,
		self->grid.lineheight * line + self->grid.dy,		
		self->grid.trackwidth,
		self->grid.lineheight);
	ui_component_invalidaterect(&self->grid.component, &r);
	trackerlinenumbers_invalidatecursor(&self->linenumbers, cursor);
}

void trackerview_invalidateline(TrackerView* self, beat_t offset)
{
	int line;	
	ui_rectangle r;	
	
	if (offset >= self->sequenceentryoffset &&
			offset < self->sequenceentryoffset + self->pattern->length) {
		line = (int) ((offset - self->sequenceentryoffset) 
			/ self->grid.bpl);
		ui_setrectangle(&r,
			self->grid.dx,
			self->grid.lineheight * line + self->grid.dy,
			ui_component_size(&self->component).width - self->grid.dx,
			self->grid.lineheight);
		ui_component_invalidaterect(&self->grid.component, &r);
	}
}


void trackergrid_onscroll(TrackerGrid* self, ui_component* sender, int stepx,
	int stepy)
{				
	self->dx += (stepx * sender->scrollstepx);
	self->dy += (stepy * sender->scrollstepy);

	if (self->header && stepx != 0) {
		self->header->dx = self->dx;
		ui_component_invalidate(&self->header->component);
	}
	if (self->linenumbers && stepy != 0) {
		self->linenumbers->dy = self->dy;
		ui_component_invalidate(&self->linenumbers->component);		
		ui_component_update(&self->linenumbers->component);
	}
	if (self->midline) {
		ui_size size;
		int halfvisilines;	
		int restoremidline;		
		ui_rectangle r;		
			
		size = ui_component_size(&self->component);		
		halfvisilines = size.height / self->lineheight / 2;
		ui_setrectangle(&r, 0, halfvisilines * self->lineheight, size.width,
			2 * self->lineheight);
		restoremidline = self->midline;
		self->midline = 0;
		ui_component_invalidaterect(&self->component, &r);
		self->midline = restoremidline;
	}
}

void trackergrid_onmousedown(TrackerGrid* self, ui_component* sender, MouseEvent* ev)
{
	if (ev->button == 2) {
		trackerview_toggleblockmenu(self->view);
	} else
	if (self->view->pattern) {
		if (ev->button == 1) {			
			TrackerCursor oldcursor;

			oldcursor = self->cursor;
			self->cursor = trackergrid_makecursor(self, ev->x, ev->y);			
			self->selection.topleft = self->cursor;
			workspace_setpatterneditposition(self->view->workspace, self->cursor);			
			if (self->hasselection) {				
				self->hasselection = 0;
				ui_component_invalidate(&self->component);
			}
			self->hasselection = 0;
			trackerview_invalidatecursor(self->view, &oldcursor);
			trackerview_invalidatecursor(self->view, &self->cursor);			
			ui_component_setfocus(&self->component);
		}		
	}
}

void trackergrid_onmousemove(TrackerGrid* self, ui_component* sender, MouseEvent* ev)
{
	TrackerCursor cursor;

	if (ev->button == 1) {
		cursor = trackergrid_makecursor(self, ev->x, ev->y);
		if (cursor.col != self->cursor.col ||
			cursor.offset != self->cursor.offset) {
			self->hasselection = 1;
			self->selection.bottomright = cursor;
			self->selection.bottomright.offset += self->bpl;
			self->selection.bottomright.track += 1;
			ui_component_invalidate(&self->component);		
		}
	}
}

void trackergrid_onmousedoubleclick(TrackerGrid* self, ui_component* sender,
	MouseEvent* ev)
{
	trackerview_selectcol(self->view);
}

TrackerCursor trackergrid_makecursor(TrackerGrid* self, int x, int y)
{
	TrackerCursor rv;
	int lines;
	int sublines;
	int subline;		
	int coloffset;
	

	rv.offset = trackergrid_offset(self, y - self->dy, &lines, &sublines, &subline);
	rv.totallines = lines + sublines;
	rv.subline = subline;
	rv.track = (x - self->dx) / self->trackwidth;
	coloffset = (x - self->dx) - rv.track * self->trackwidth;
	if (coloffset < 3*self->textwidth) {
		rv.col = 0;
	} else {
		rv.col = coloffset / self->textwidth - 2;
	}
	self->cursor.pattern = 
		workspace_patterneditposition(self->view->workspace).pattern;
	return rv;
}
	
void trackergrid_onmouseup(TrackerGrid* self, ui_component* sender, MouseEvent* ev)
{
	if (self->hasselection) {
		// ui_component_show(&self->view->blockmenu.component);
		// ui_component_align(&self->view->component);		
	}
}

void trackerview_init(TrackerView* self, ui_component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	self->opcount = 0;
	self->cursorstep = 1;
	self->syncpattern = 1;
	self->lastplayposition = -1.f;
	self->sequenceentryoffset = 0.f;
	self->wraparound = 1;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);	
	trackerview_initinputs(self);
	self->pattern = 0;
	ui_bitmap_loadresource(&self->skin.bitmap, IDB_HEADERSKIN);
	trackerview_initdefaultskin(self);	
	trackerheader_init(&self->header, &self->component, self);
	self->header.numtracks = player_numsongtracks(&workspace->player);
	self->header.trackwidth = self->skin.headercoords.background.destwidth;
	self->linenumbers.skin = &self->skin;
	trackergrid_init(&self->grid, &self->component, self, &workspace->player);
	InitTrackerLineNumbersLabel(&self->linenumberslabel, &self->component, self);	
	trackerlinenumbers_init(&self->linenumbers, &self->component, self);	
	self->grid.header = &self->header;
	self->grid.linenumbers = &self->linenumbers;	
	self->header.skin = &self->skin;
	self->linenumbers.lineheight = 13;
	self->showlinenumbers = 1;
	self->showlinenumbercursor = 1;
	self->showlinenumbersinhex = 1;
	self->showemptydata = 0;
	signal_connect(&self->component.signal_destroy, self, trackerview_ondestroy);
	signal_connect(&self->component.signal_align, self, trackerview_onalign);
	// signal_connect(&self->component.signal_size, self, trackerview_onsize);
	signal_connect(&self->component.signal_timer, self, trackerview_ontimer);
	signal_connect(&self->component.signal_keydown, self,
		trackerview_onkeydown);
	patternblockmenu_init(&self->blockmenu, &self->component);
	signal_connect(&self->blockmenu.changegenerator.signal_clicked, self,
		trackerview_onchangegenerator);
	signal_connect(&self->blockmenu.changeinstrument.signal_clicked, self,
		trackerview_onchangeinstrument);
	signal_connect(&self->blockmenu.cut.signal_clicked, self,
		trackerview_onblockcut);
	signal_connect(&self->blockmenu.copy.signal_clicked, self,
		trackerview_onblockcopy);
	signal_connect(&self->blockmenu.paste.signal_clicked, self,
		trackerview_onblockpaste);
	signal_connect(&self->blockmenu.mixpaste.signal_clicked, self,
		trackerview_onblockmixpaste);
	signal_connect(&self->blockmenu.del.signal_clicked, self,
		trackerview_onblockdelete);
	signal_connect(&self->blockmenu.blocktransposeup.signal_clicked, self,
		trackerview_onblocktransposeup);
	signal_connect(&self->blockmenu.blocktransposedown.signal_clicked, self,
		trackerview_onblocktransposedown);
	signal_connect(&self->blockmenu.blocktransposeup12.signal_clicked, self,
		trackerview_onblocktransposeup12);
	signal_connect(&self->blockmenu.blocktransposedown12.signal_clicked, self,
		trackerview_onblocktransposedown12);	
	ui_component_hide(&self->blockmenu.component);
	TrackerViewApplyProperties(self, 0);
	if (self->grid.trackwidth < self->header.trackwidth) {
		int i;
		int temp = (self->header.trackwidth - self->grid.trackwidth)/2;
		self->grid.trackwidth = self->header.trackwidth;
		for (i = 0; i < 10; ++i) {
			self->grid.colx[i] += temp;
		}				
	} else {
		self->header.trackwidth = self->grid.trackwidth;
	}
	self->grid.component.scrollstepx = self->grid.trackwidth;
	ui_component_starttimer(&self->component, TIMERID_TRACKERVIEW, 50);
	signal_connect(&workspace->player.signal_lpbchanged, self,
		trackerview_onlpbchanged);
	signal_connect(&workspace->signal_configchanged, self,
		trackerview_onconfigchanged);
	signal_connect(&workspace->player.signal_inputevent, self,
		trackerview_oninput);
	signal_connect(&workspace->signal_patterneditpositionchanged, self,
		trackerview_onpatterneditpositionchanged);	
	signal_connect(&workspace->signal_parametertweak, self,
		trackerview_onparametertweak);
	trackerview_readconfig(self);	
}

void trackerview_ondestroy(TrackerView* self, ui_component* sender)
{
	inputs_dispose(&self->inputs);
}

void trackerview_initdefaultskin(TrackerView* self)
{
	trackerview_setclassicheadercoords(self);
}

void trackerview_setheadercoords(TrackerView* self)
{
	static SkinCoord background = { 2, 0, 102, 23, 0, 0, 102, 23, 0 };	
	static SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	static SkinCoord mute = { 79, 40, 17, 17, 75, 66, 3, 17, 17 };
	static SkinCoord solo = { 92, 18, 11, 11, 97, 3, 11, 11, 0 };
	static SkinCoord digitx0 = { 0, 23, 9, 17, 15, 3, 9, 17, 0 };
	static SkinCoord digit0x = { 0, 23, 9, 17, 22, 3, 9, 17, 0 };	

	self->skin.headercoords.background = background;	
	self->skin.headercoords.record = record;
	self->skin.headercoords.mute = mute;
	self->skin.headercoords.solo = solo;
	self->skin.headercoords.digit0x = digit0x;
	self->skin.headercoords.digitx0 = digitx0;	
}

void trackerview_setclassicheadercoords(TrackerView* self)
{
	static SkinCoord background = { 2, 0, 102, 23, 0, 0, 102, 23, 0 };	
	static SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	static SkinCoord mute = { 79, 40, 17, 17, 66, 3, 17, 17, 0 };
	static SkinCoord solo = { 62, 40, 17, 17, 47, 3, 17, 17, 0 };
	static SkinCoord digitx0 = { 0, 23, 9, 17, 15, 3, 9, 17, 0 };
	static SkinCoord digit0x = { 0, 23, 9, 17, 22, 3, 9, 17, 0 };	

	self->skin.headercoords.background = background;	
	self->skin.headercoords.record = record;
	self->skin.headercoords.mute = mute;
	self->skin.headercoords.solo = solo;
	self->skin.headercoords.digit0x = digit0x;
	self->skin.headercoords.digitx0 = digitx0;	
}

void trackerview_setheadertextcoords(TrackerView* self)
{
	SkinCoord background = { 2, 57, 103, 23, 0, 0, 103, 23, 0 };
	SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12, 0 };
	SkinCoord mute = { 81, 18, 11, 11, 75, 3, 11, 11, 0 };
	SkinCoord solo = { 92, 18, 11, 11, 97, 3, 11, 11, 0 };
	SkinCoord digitx0 = { 0, 80, 6, 12, 5, 8, 6, 12, 0 };
	SkinCoord digit0x = { 0, 80, 6, 12, 11, 8, 6, 12, 0 };
	
	self->skin.headercoords.background = background;	
	self->skin.headercoords.record = record;
	self->skin.headercoords.mute = mute;
	self->skin.headercoords.solo = solo;
	self->skin.headercoords.digit0x = digit0x;
	self->skin.headercoords.digitx0 = digitx0;	
}

// void trackerview_onsize(TrackerView* self, ui_component* sender, ui_size* size)
// {
//	trackerview_align(self, sender);
// }

void trackerview_onalign(TrackerView* self, ui_component* sender)
{
	ui_size size;
	ui_size menusize;	
	int headerheight = 30;
	int linenumberwidth = self->showlinenumbers ? 45 :0;
	size = ui_component_size(&self->component);
	menusize.width = self->blockmenu.component.visible
		? ui_component_preferredsize(&self->blockmenu.component, &size).width
		: 0;	
	menusize.height = size.height;	
	ui_component_setposition(&self->blockmenu.component,
		size.width - menusize.width,
		0, menusize.width,
		size.height);
	ui_component_setposition(&self->header.component,
		linenumberwidth, 0,
		size.width - linenumberwidth - menusize.width, headerheight);
	ui_component_setposition(&self->grid.component,
		linenumberwidth,
		headerheight,
		size.width - linenumberwidth - menusize.width,
		size.height - headerheight);	
	ui_component_setposition(&self->linenumbers.component,
		0, headerheight, linenumberwidth, size.height - headerheight);
	ui_component_resize(&self->linenumberslabel.component,
		linenumberwidth, headerheight);	
}

void trackerheader_init(TrackerHeader* self, ui_component* parent,
	TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	signal_connect(&self->component.signal_draw, self, trackerheader_ondraw);
	signal_connect(&self->component.signal_mousedown, self,
		trackerheader_onmousedown);
	self->dx = 0;
	self->numtracks = 16;
	self->trackwidth = 102;
	self->classic = 1;
}

void trackerheader_ondraw(TrackerHeader* self, ui_component* sender, ui_graphics* g)
{	
	ui_size size;
	ui_rectangle r;
	int cpx = self->dx;
	uintptr_t track;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);		
	ui_drawsolidrectangle(g, r, self->skin->background);
	
	for (track = 0; track < self->numtracks; ++track) {		
		int trackx0 = track / 10;
		int track0x = track % 10;		
		SkinCoord digitx0 = self->skin->headercoords.digitx0;
		SkinCoord digit0x = self->skin->headercoords.digit0x;		
		digitx0.srcx += trackx0 * digitx0.srcwidth;
		digit0x.srcx += track0x * digit0x.srcwidth;
		skin_blitpart(g, &self->skin->bitmap, cpx, 0,
			&self->skin->headercoords.background);
		skin_blitpart(g, &self->skin->bitmap, cpx, 0, &digitx0);
		skin_blitpart(g, &self->skin->bitmap, cpx, 0, &digit0x);
		if (self->view->workspace->song) {
			if (patterns_istrackmuted(&self->view->workspace->song->patterns,
					track)) {
				skin_blitpart(g, &self->skin->bitmap, cpx, 0,
					&self->skin->headercoords.mute);
			}
			if (patterns_istracksoloed(&self->view->workspace->song->patterns,
					track)) {
				skin_blitpart(g, &self->skin->bitmap, cpx, 0,
					&self->skin->headercoords.solo);
			}
		}
		cpx += self->trackwidth;
	}		
}

void trackerheader_onmousedown(TrackerHeader* self, ui_component* sender,
	MouseEvent* ev)
{
	if (self->view->workspace->song) {
		ui_rectangle r;
		uintptr_t track;

		track = (ev->x - self->dx) / self->trackwidth;
		ui_setrectangle(&r,
			self->skin->headercoords.mute.destx + track * self->trackwidth,
			self->skin->headercoords.mute.desty,
			self->skin->headercoords.mute.destwidth,
			self->skin->headercoords.mute.destheight);
		if (ui_rectangle_intersect(&r, ev->x - self->dx, ev->y)) {
			if (patterns_istrackmuted(&self->view->workspace->song->patterns,
					track)) {
				patterns_unmutetrack(&self->view->workspace->song->patterns,
					track);
			} else {
				patterns_mutetrack(&self->view->workspace->song->patterns,
					track);
			}
			ui_component_invalidate(&self->component);
		}
		ui_setrectangle(&r,
			self->skin->headercoords.solo.destx + track * self->trackwidth,
			self->skin->headercoords.solo.desty,
			self->skin->headercoords.solo.destwidth,
			self->skin->headercoords.solo.destheight);

		if (ui_rectangle_intersect(&r, ev->x - self->dx, ev->y)) {
			if (patterns_istracksoloed(&self->view->workspace->song->patterns,
					track)) {
				patterns_deactivatesolotrack(
					&self->view->workspace->song->patterns);
			} else {
				patterns_activatesolotrack(
					&self->view->workspace->song->patterns, track);
			}
			ui_component_invalidate(&self->component);
		}
	}
}

void trackerlinenumbers_init(TrackerLineNumbers* self, ui_component* parent,
	TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 
		self->view->skin.background);
	signal_connect(&self->component.signal_draw, self,
		trackerlinenumbers_ondraw);	
	self->dy = 0;
	self->textheight = 12;
	self->lineheight = self->textheight + 1;
	self->component.doublebuffered = 1;
}

void trackerlinenumbers_ondraw(TrackerLineNumbers* self, ui_component* sender,
	ui_graphics* g)
{	
	if (self->view->pattern) {
		ui_size size;
		char buffer[20];		
		int cpy = self->dy;
		int line;		
		double offset;	
		TrackerGridBlock clip;

		size = ui_component_size(&self->component);
		trackergrid_clipblock(&self->view->grid, &g->clip, &clip);
		ui_setfont(g, &self->view->font);
		cpy = (clip.topleft.totallines - clip.topleft.subline) * self->lineheight + self->dy;
		offset = clip.topleft.offset;				
		line = clip.topleft.line;
		while (offset <= clip.bottomright.offset && offset < self->view->pattern->length) {
			ui_rectangle r;			
			int beat;
			int beat4;
			int subline = 0;
			int numsublines;
			int ystart;
			int cursor;			
			int playbar = trackergrid_testplaybar(&self->view->grid, offset);

			cursor = trackergrid_testcursor(&self->view->grid, 
				self->view->grid.cursor.track, offset, subline);
			beat = fmod(offset, 1.0f) == 0.0f;
			beat4 = fmod(offset, 4.0f) == 0.0f;
			SetColColor(self->skin, g, 0, playbar, self->view->showlinenumbercursor ?
				cursor : 0, 0, beat, beat4, 0);
			// %3i
			if (self->view->showlinenumbersinhex) {
				psy_snprintf(buffer, 10, "%.2X %.3f", line, offset);
			} else {
				psy_snprintf(buffer, 10, "%.2d %.3f", line, offset);
			}
			ui_setrectangle(&r, 0, cpy, size.width - 2, self->textheight);			
			ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer,
				strlen(buffer));		
			cpy += self->lineheight;
			ystart = cpy;
			numsublines = NumSublines(self->view->pattern, offset, self->view->grid.bpl);
			for (subline = 0; subline < numsublines; ++subline) {
				if (self->view->showlinenumbersinhex) {
					psy_snprintf(buffer, 10, "  %.2X", subline);
				} else {
					psy_snprintf(buffer, 10, "  %.2d", subline);
				}
				cursor = trackergrid_testcursor(&self->view->grid, 
					self->view->grid.cursor.track, offset, subline);
				SetColColor(self->skin, g, 0, playbar, cursor,
					0, beat, beat4, 0);
				ui_setrectangle(&r, 0, cpy, size.width - 2, self->textheight);
				ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer,
					strlen(buffer));		
				cpy += self->lineheight;
			}
			if (ystart != cpy) {
				ui_drawline(g, 1, ystart, 1, cpy - self->lineheight / 2);
				ui_drawline(g, 1, cpy - self->lineheight / 2, 4, cpy - self->lineheight / 2);
			}
			offset += self->view->grid.bpl;
			++line;
		}		
	}
}

void trackerlinenumbers_invalidatecursor(TrackerLineNumbers* self,
	const TrackerCursor* cursor)
{
	int line;		
	ui_rectangle r;		

	line = trackerview_offsettoscreenline(self->view, cursor->offset)
		+ cursor->subline;	
	ui_setrectangle(&r,
		0,
		self->view->grid.lineheight * line + self->view->grid.dy,
		ui_component_size(&self->component).width,
		self->view->grid.lineheight);
	ui_component_invalidaterect(&self->component, &r);	
}

void trackerlinenumbers_invalidateline(TrackerLineNumbers* self, beat_t offset)
{
	int line;	
	ui_rectangle r;	
	
	if (offset >= self->view->sequenceentryoffset &&
			offset < self->view->sequenceentryoffset + self->view->pattern->length) {
		line = (int) ((offset - self->view->sequenceentryoffset) 
			/ self->view->grid.bpl);	
		ui_setrectangle(&r,
			self->view->grid.dx,
			self->view->grid.lineheight * line + self->view->grid.dy,
			ui_component_size(&self->component).width - self->view->grid.dx,
			self->view->grid.lineheight);
		ui_component_invalidaterect(&self->component, &r);
	}
}

void InitTrackerLineNumbersLabel(TrackerLineNumbersLabel* self, ui_component* parent, TrackerView* view)
{		
	self->view = view;
	ui_component_init(&self->component, parent);
	signal_connect(&self->component.signal_draw, self, OnLineNumbersLabelDraw);
	signal_connect(&self->component.signal_mousedown, self, OnLineNumbersLabelMouseDown);
}

void OnLineNumbersLabelMouseDown(TrackerLineNumbersLabel* self, ui_component* sender)
{
	self->view->header.classic = !self->view->header.classic;
	if (self->view->header.classic) {
		trackerview_setclassicheadercoords(self->view);
	} else {
		trackerview_setheadercoords(self->view);
	}
	ui_component_invalidate(&self->view->header.component);
}

void OnLineNumbersLabelDraw(TrackerLineNumbersLabel* self, ui_component* sender, ui_graphics* g)
{	
	ui_size size;
	ui_rectangle r;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);	
	ui_setbackgroundcolor(g, self->view->skin.background);
	ui_settextcolor(g, self->view->skin.font);	
	ui_textoutrectangle(g, r.left, r.top, 0, r, "Line", strlen("Line"));
}

void trackerview_ontimer(TrackerView* self, ui_component* sender, int timerid)
{
	if (timerid == TIMERID_TRACKERVIEW && self->pattern) {		
		if (player_playing(self->grid.player)) {
			if (!workspace_followingsong(self->workspace)) {
				trackerview_invalidateline(self, self->lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);
				self->lastplayposition = player_position(&self->workspace->player);
				trackerview_invalidateline(self, self->lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);
			}
		} else {
			if (self->lastplayposition != -1) {				
				trackerview_invalidateline(self, self->lastplayposition);
				trackerlinenumbers_invalidateline(&self->linenumbers,
					self->lastplayposition);
				self->lastplayposition = -1;
			}
		}
		if (self->pattern && self->pattern->opcount != self->opcount &&
				self->syncpattern) {
			ui_component_invalidate(&self->grid.component);
			ui_component_invalidate(&self->linenumbers.component);
		}
		self->opcount = self->pattern ? self->pattern->opcount : 0;		
	}
}

int trackerview_numlines(TrackerView* self)
{
	int lines = 0;
	int sublines = 0;	
	int remaininglines = 0;
	double offset = 0;

	if (!self->pattern) {
		return 0;
	} else {		
		int first = 1;		
		PatternNode* curr = self->pattern->events;
		int subline = 0;

		while (curr && offset < self->pattern->length) {		
			PatternEntry* entry;		
			first = 1;
			do {
				entry = (PatternEntry*)curr->entry;			
				if ((entry->offset >= offset) && (entry->offset < offset + self->grid.bpl))
				{					
					if (entry->track == 0 && !first) {
						++(sublines);
						++subline;
					}							
					first = 0;
					curr = curr->next;
				} else {
					subline = 0;
					break;
				}	
			} while (curr);		
			++(lines);
			subline = 0;		
			offset += self->grid.bpl;
		}
	}			
	offset = self->pattern->length - offset;
	if (offset > 0) {
		remaininglines = (int)(offset * player_lpb(self->grid.player));
	}
	return lines + sublines + remaininglines;
}

int trackerview_offsettoscreenline(TrackerView* self, big_beat_t offs)
{
	int lines = 0;
	int sublines = 0;	
	int remaininglines = 0;
	double offset = 0;
	
	if (!self->pattern) {
		return 0;
	} else {		
		int first = 1;		
		PatternNode* curr = self->pattern->events;
		int subline = 0;

		while (curr && offset < self->pattern->length) {		
			PatternEntry* entry;		
			first = 1;
			do {
				entry = (PatternEntry*)curr->entry;			
				if ((entry->offset >= offset) && (entry->offset < offset + self->grid.bpl))
				{						
					if (entry->track == 0 && !first) {
						++(sublines);
						++subline;
					}
					if (entry->offset >= offs) {
						break;
					}
					first = 0;
					curr = curr->next;
				} else {
					subline = 0;
					break;
				}	
			} while (curr);			
			if (offset >= offs) {
				subline = 0;
				break;
			}
			++(lines);
			subline = 0;			
			offset += self->grid.bpl;			
		}
	}			
	offset = offs - offset;
	if (offset > 0) {		
		remaininglines = (int)(offset * player_lpb(self->grid.player));		
	}
	return lines + sublines + remaininglines;
}

void trackerview_onlpbchanged(TrackerView* self, Player* sender, uintptr_t lpb)
{
	self->grid.bpl = 1 / (big_beat_t) lpb;
}

void trackerview_onconfigchanged(TrackerView* self, Workspace* workspace,
	Properties* property)
{
	if (property == workspace->config) {
		trackerview_readconfig(self);
	} else
	if (strcmp(properties_key(property), "wraparound") == 0) {
		self->wraparound = properties_value(property);
		ui_component_invalidate(&self->component);
	} else
	if (strcmp(properties_key(property), "linenumbers") == 0) {
		trackerview_showlinenumbers(self, properties_value(property));
	} else
	if (strcmp(properties_key(property), "linenumberscursor") == 0) {
		trackerview_showlinenumbercursor(self, properties_value(property));
	} else
	if (strcmp(properties_key(property), "linenumbersinhex") == 0) {
		trackerview_showlinenumbersinhex(self, properties_value(property));
	} else
	if (strcmp(properties_key(property), "drawemptydata") == 0) {
		trackerview_showemptydata(self, properties_value(property));
	} else
	if (strcmp(properties_key(property), "centercursoronscreen") == 0) {
		trackerview_setcentermode(self, properties_value(property));
	}
}

void trackerview_readconfig(TrackerView* self)
{
	Properties* pv;
	
	pv = properties_findsection(self->workspace->config, "visual.patternview");
	if (pv) {		
		trackerview_showlinenumbers(self, properties_bool(pv, "linenumbers", 1));
		trackerview_showlinenumbercursor(self, properties_bool(pv, "linenumberscursor", 1));
		trackerview_showlinenumbersinhex(self, properties_bool(pv, "linenumbersinhex", 1));
		self->wraparound = properties_bool(pv, "wraparound", 1);
		trackerview_showemptydata(self, properties_bool(pv, "drawemptydata", 1));
		trackerview_setcentermode(self, properties_bool(pv, "centercursoronscreen", 1));
	}
}

void trackerview_showlinenumbers(TrackerView* self, int showstate)
{
	self->showlinenumbers = showstate;
	if (self->showlinenumbers != 0) {		
		ui_component_show(&self->linenumbers.component);
		ui_component_show(&self->linenumberslabel.component);
	} else {
		ui_component_hide(&self->linenumbers.component);
		ui_component_hide(&self->linenumberslabel.component);
	}
	ui_component_align(&self->component);
	ui_component_invalidate(&self->component);
}

void trackerview_showlinenumbercursor(TrackerView* self, int showstate)
{
	self->showlinenumbercursor = showstate;	
	ui_component_invalidate(&self->component);
}

void trackerview_showlinenumbersinhex(TrackerView* self, int showstate)
{
	self->showlinenumbersinhex = showstate;	
	ui_component_invalidate(&self->component);
}

void trackerview_showemptydata(TrackerView* self, int showstate)
{
	self->showemptydata = showstate;	
	ui_component_invalidate(&self->component);
}

void trackerview_setcentermode(TrackerView* self, int mode)
{
	self->grid.midline = mode;	
	trackergrid_adjustscroll(&self->grid);
	if (mode) {
		trackerview_centeroncursor(self);
	} else {
		self->grid.dy = 0;
		self->linenumbers.dy = 0;
		ui_component_setverticalscrollposition(&self->grid.component, 0);
	}
}

void trackerview_initinputs(TrackerView* self)
{
	inputs_init(&self->inputs);	
	inputs_define(&self->inputs, encodeinput(VK_UP, 0, 0), CMD_NAVUP);
	inputs_define(&self->inputs, encodeinput(VK_DOWN, 0, 0),CMD_NAVDOWN);
	inputs_define(&self->inputs, encodeinput(VK_LEFT, 0, 0),CMD_NAVLEFT);
	inputs_define(&self->inputs, encodeinput(VK_RIGHT, 0, 0),CMD_NAVRIGHT);
	inputs_define(&self->inputs, encodeinput(VK_PRIOR, 0, 0),CMD_NAVPAGEUP);
	inputs_define(&self->inputs, encodeinput(VK_NEXT, 0, 0),CMD_NAVPAGEDOWN);
	inputs_define(&self->inputs, encodeinput(VK_HOME, 0, 0), CMD_NAVTOP);
	inputs_define(&self->inputs, encodeinput(VK_END, 0, 0), CMD_NAVBOTTOM);	
	inputs_define(&self->inputs, encodeinput(VK_TAB, 1, 0), CMD_COLUMNPREV);
	inputs_define(&self->inputs, encodeinput(VK_TAB, 0, 0), CMD_COLUMNNEXT);	
		
	inputs_define(&self->inputs, encodeinput(VK_INSERT, 0, 0), CMD_ROWINSERT);
	inputs_define(&self->inputs, encodeinput(VK_BACK, 0, 0), CMD_ROWDELETE);
	inputs_define(&self->inputs, encodeinput(VK_DELETE, 0, 0), CMD_ROWCLEAR);

	inputs_define(&self->inputs, encodeinput('B', 0, 1), CMD_BLOCKSTART);
	inputs_define(&self->inputs, encodeinput('E', 0, 1), CMD_BLOCKEND);
	inputs_define(&self->inputs, encodeinput('U', 0, 1), CMD_BLOCKUNMARK);
	inputs_define(&self->inputs, encodeinput('X', 0, 1), CMD_BLOCKCUT);
	inputs_define(&self->inputs, encodeinput('C', 0, 1), CMD_BLOCKCOPY);
	inputs_define(&self->inputs, encodeinput('V', 0, 1), CMD_BLOCKPASTE);
	inputs_define(&self->inputs, encodeinput('M', 0, 1), CMD_BLOCKMIX);
	
	inputs_define(&self->inputs, encodeinput(VK_F12, 0, 1), CMD_TRANSPOSEBLOCKINC);
	inputs_define(&self->inputs, encodeinput(VK_F11, 0, 1), CMD_TRANSPOSEBLOCKDEC);
	inputs_define(&self->inputs, encodeinput(VK_F12, 1, 1), CMD_TRANSPOSEBLOCKINC12);
	inputs_define(&self->inputs, encodeinput(VK_F11, 1, 1), CMD_TRANSPOSEBLOCKDEC12);
	
	inputs_define(&self->inputs, encodeinput('A', 0, 1), CMD_SELECTALL);
	inputs_define(&self->inputs, encodeinput('R', 0, 1), CMD_SELECTCOL);
	inputs_define(&self->inputs, encodeinput('K', 0, 1), CMD_SELECTBAR);	
		
	inputs_define(&self->inputs, encodeinput('Z', 0, 1), CMD_UNDO);
	inputs_define(&self->inputs, encodeinput('Z', 1, 1), CMD_REDO);
	inputs_define(&self->inputs, encodeinput('F', 0, 1), CMD_FOLLOWSONG);
}

void trackerview_setpattern(TrackerView* self, Pattern* pattern)
{	
	self->pattern = pattern;
	if (pattern) {
		self->opcount = pattern->opcount;
	}
	self->grid.cursor.track = 0;
	self->grid.cursor.offset = 0;
	self->grid.cursor.subline = 0;
	self->grid.cursor.col = 0;	
	self->grid.dx = 0;
	self->grid.dy = 0;
	self->header.dx = 0;
	self->linenumbers.dy = 0;
	trackergrid_adjustscroll(&self->grid);
	ui_component_invalidate(&self->linenumbers.component);
	ui_component_invalidate(&self->header.component);	
}

void trackerview_onchangegenerator(TrackerView* self)
{
	if (self->pattern && self->workspace->song) {
			pattern_changemachine(self->pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright,
			self->workspace->song->machines.slot);
		ui_component_invalidate(&self->component);
	}
}

void trackerview_onchangeinstrument(TrackerView* self)
{
	if (self->pattern && self->workspace->song) {
			pattern_changeinstrument(self->pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright,
			self->workspace->song->instruments.slot);
		ui_component_invalidate(&self->component);
	}
}

void trackerview_onblockcut(TrackerView* self)
{
	if (self->grid.hasselection) {

		trackerview_onblockcopy(self);
		trackerview_onblockdelete(self);
	}
}

void trackerview_onblockcopy(TrackerView* self)
{
	if (self->grid.hasselection) {
		PatternNode* begin;		
		PatternNode* p;
		PatternNode* q;
		PatternNode* prev = 0;
		beat_t offset;
		int trackoffset;

		begin = pattern_greaterequal(self->pattern, 
			(beat_t) self->grid.selection.topleft.offset);
		offset = (beat_t) self->grid.selection.topleft.offset;
		trackoffset = self->grid.selection.topleft.track;
		pattern_dispose(&self->workspace->patternpaste);
		pattern_init(&self->workspace->patternpaste);
		p = begin;
		while (p != 0) {			
			PatternEntry* entry;
			q = p->next;

			entry = (PatternEntry*) p->entry;
			if (entry->offset < self->grid.selection.bottomright.offset) {
				if (entry->track >= self->grid.selection.topleft.track &&
						entry->track < self->grid.selection.bottomright.track) {						
					prev = pattern_insert(&self->workspace->patternpaste,
						prev, entry->track - trackoffset, 
						entry->offset - offset, &entry->event);
				}
			} else {
				break;
			}
			p = q;
		}
		pattern_setmaxsongtracks(&self->workspace->patternpaste, 
			self->grid.selection.bottomright.track -
			self->grid.selection.topleft.track);
		pattern_setlength(&self->workspace->patternpaste,
			(beat_t)(self->grid.selection.bottomright.offset -
			self->grid.selection.topleft.offset));

	}
	ui_component_invalidate(&self->component);
}

void trackerview_onblockpaste(TrackerView* self)
{
	PatternNode* p;
	PatternNode* prev = 0;
	beat_t offset;
	int trackoffset;
	PatternEditPosition begin;
	PatternEditPosition end;

	offset = (beat_t) self->grid.cursor.offset;
	trackoffset = self->grid.cursor.track;
	p = self->workspace->patternpaste.events;	

	begin = end = self->grid.cursor;
	end.track += self->workspace->patternpaste.maxsongtracks;
	end.offset += self->workspace->patternpaste.length;
	pattern_blockremove(self->pattern, begin, end);
	while (p != 0) {			
		PatternEntry* pasteentry;
		PatternNode* node;

		pasteentry = (PatternEntry*) p->entry;
		node = pattern_findnode(self->pattern, 
			pasteentry->track + trackoffset,
			pasteentry->offset + offset,
			0,
			(beat_t) self->grid.bpl,
			&prev);
		if (node) {
			PatternEntry* entry;

			entry = (PatternEntry*) node->entry;
			entry->event = pasteentry->event;
		} else {
			pattern_insert(self->pattern,
					prev, 
					pasteentry->track + trackoffset, 
					pasteentry->offset + offset, 
					&pasteentry->event);
		}						
		p = p->next;
	}	
	ui_component_invalidate(&self->component);
}

void trackerview_onblockmixpaste(TrackerView* self)
{
	PatternNode* p;
	PatternNode* prev = 0;
	beat_t offset;
	int trackoffset;

	offset = (beat_t) self->grid.cursor.offset;
	trackoffset = self->grid.cursor.track;
	p = self->workspace->patternpaste.events;
	while (p != 0) {			
		PatternEntry* pasteentry;		

		pasteentry = (PatternEntry*) p->entry;
		if (!pattern_findnode(self->pattern, pasteentry->track + trackoffset,
				pasteentry->offset + offset, 0, (beat_t) self->grid.bpl,
				&prev)) {
			pattern_insert(self->pattern,
					prev, 
					pasteentry->track + trackoffset, 
					pasteentry->offset + offset, 
					&pasteentry->event);
		}						
		p = p->next;
	}	
	ui_component_invalidate(&self->component);
}

void trackerview_onblockdelete(TrackerView* self)
{
	if (self->grid.hasselection) {
		pattern_blockremove(self->pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright);
		ui_component_invalidate(&self->component);
	}	
}

void trackerview_blockstart(TrackerView* self)
{
	self->grid.selection.topleft = self->grid.cursor;
	self->grid.selection.bottomright = self->grid.cursor;
	++self->grid.selection.bottomright.track;
	self->grid.selection.bottomright.offset += self->grid.bpl;
	self->grid.hasselection = 1;
	ui_component_invalidate(&self->component);
}

void trackerview_blockend(TrackerView* self)
{
	self->grid.selection.bottomright = self->grid.cursor;
	++self->grid.selection.bottomright.track;
	self->grid.selection.bottomright.offset += self->grid.bpl;
	ui_component_invalidate(&self->component);
}

void trackerview_blockunmark(TrackerView* self)
{
	self->grid.hasselection = 0;
	ui_component_invalidate(&self->component);
}

void trackerview_onblocktransposeup(TrackerView* self)
{
	if (self->grid.hasselection) {
		pattern_blocktranspose(self->pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright, 1);
		ui_component_invalidate(&self->component);
	}	
}

void trackerview_onblocktransposedown(TrackerView* self)
{
	if (self->grid.hasselection) {
		pattern_blocktranspose(self->pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright, -1);
		ui_component_invalidate(&self->component);
	}
}

void trackerview_onblocktransposeup12(TrackerView* self)
{
	if (self->grid.hasselection) {
		pattern_blocktranspose(self->pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright, 12);
		ui_component_invalidate(&self->component);
	}
}

void trackerview_onblocktransposedown12(TrackerView* self)
{
	if (self->grid.hasselection) {
		pattern_blocktranspose(self->pattern, 
			self->grid.selection.topleft,
			self->grid.selection.bottomright, -12);
		ui_component_invalidate(&self->component);
	}
}

void trackerview_showblockmenu(TrackerView* self)
{
	ui_component_show(&self->blockmenu.component);
	ui_component_align(&self->component);
	ui_component_invalidate(&self->linenumbers.component);
}

void trackerview_hideblockmenu(TrackerView* self)
{
	ui_component_hide(&self->blockmenu.component);
	ui_component_align(&self->component);
	ui_component_invalidate(&self->linenumbers.component);
}

void trackerview_toggleblockmenu(TrackerView* self)
{
	if (self->blockmenu.component.visible) {
		trackerview_hideblockmenu(self);
	} else {		
		trackerview_showblockmenu(self);
	}
}

void patternblockmenu_init(PatternBlockMenu* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
		ui_button_init(&self->cut, &self->component);
	ui_button_settext(&self->cut, "Cut");
	ui_button_init(&self->copy, &self->component);	
	ui_button_settext(&self->copy, "Copy");
	ui_button_init(&self->paste, &self->component);	
	ui_button_settext(&self->paste, "Paste");
	ui_button_init(&self->mixpaste, &self->component);
	ui_button_settext(&self->mixpaste, "MixPaste");
	ui_button_init(&self->del, &self->component);
	ui_button_settext(&self->del, "Delete");

	ui_button_init(&self->changegenerator, &self->component);
	ui_button_settext(&self->changegenerator, "Change Generator");
	ui_button_init(&self->changeinstrument, &self->component);
	ui_button_settext(&self->changeinstrument, "Change Instrument");

	ui_button_init(&self->blocktransposeup, &self->component);
	ui_button_settext(&self->blocktransposeup, "Transpose +1");
	ui_button_init(&self->blocktransposedown, &self->component);
	ui_button_settext(&self->blocktransposedown, "Transpose -1");
	ui_button_init(&self->blocktransposeup12, &self->component);
	ui_button_settext(&self->blocktransposeup12, "Transpose +12");	
	ui_button_init(&self->blocktransposedown12, &self->component);
	ui_button_settext(&self->blocktransposedown12, "Transpose -12");
	list_free(ui_components_setalign(ui_component_children(&self->component, 0), 
		UI_ALIGN_TOP, 0));	
}

void trackerview_onpatterneditpositionchanged(TrackerView* self, Workspace* sender)
{
	TrackerCursor oldcursor;

	oldcursor = self->grid.cursor;
	self->grid.cursor = workspace_patterneditposition(sender);
	trackerview_invalidatecursor(self, &oldcursor);
	if (player_playing(&sender->player) && workspace_followingsong(sender)) {
		int scrolldown;

		scrolldown = self->lastplayposition < 
			player_position(&self->workspace->player);
		trackerview_invalidateline(self, self->lastplayposition);
		trackerlinenumbers_invalidateline(&self->linenumbers,
				self->lastplayposition);
		self->lastplayposition = player_position(&self->workspace->player);
		trackerview_invalidateline(self, self->lastplayposition);
		trackerlinenumbers_invalidateline(&self->linenumbers,
				self->lastplayposition);
		if (self->lastplayposition >= self->sequenceentryoffset &&
				self->lastplayposition < self->sequenceentryoffset +
				self->pattern->length) {
			if (scrolldown) {
				trackerview_scrolldown(self);
			} else {
				trackerview_scrollup(self);
			}
		}
	}
}

void trackerview_onparametertweak(TrackerView* self, Workspace* sender,
	int slot, int tweak, int value)
{
	if (workspace_recordingtweaks(sender)) {		
		PatternEvent event;				
		
		patternevent_init(&event,
			(unsigned char) (
				workspace_recordtweaksastws(sender)
				? NOTECOMMANDS_TWEAKSLIDE
				: NOTECOMMANDS_TWEAK),
			255,
			(unsigned char) machines_slot(&self->workspace->song->machines),
			(unsigned char) ((value & 0xFF00) >> 8),
			(unsigned char) (value & 0xFF));
		event.inst = (unsigned char) tweak;
		trackerview_preventsync(self);
		undoredo_execute(&self->workspace->undoredo,
			&InsertCommandAlloc(self->pattern, self->grid.bpl,
				self->grid.cursor, event)->command);		
		if (workspace_advancelineonrecordtweak(sender) &&
				!(workspace_followingsong(sender) && 
				  player_playing(&sender->player))) {			
			trackerview_advanceline(self);
		} else {
			trackerview_invalidatecursor(self, &self->grid.cursor);
		}
		trackerview_enablesync(self);
	}
}
