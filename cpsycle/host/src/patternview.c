// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "patternview.h"
#include <pattern.h>
#include <string.h>
#include <math.h>
#include "resources/resource.h"

extern HINSTANCE appInstance;

static void OnDraw(PatternGrid* self, ui_component* sender, ui_graphics* g);
static void OnHeaderDraw(PatternHeader* self, ui_component* sender, ui_graphics* g);
static void DrawBackground(PatternGrid* self, ui_graphics* g, PatternGridBlock* clip);
static void DrawTrackBackground(PatternGrid* self, ui_graphics* g, int track);
static void DrawEvents(PatternGrid* self, ui_graphics* g, PatternGridBlock* clip);
static void DrawPatternEvent(PatternGrid* self, ui_graphics* g, PatternEvent* event, int x, int y, int playbar, int cursor, int beat, int beat4);
static void OnKeyDown(PatternView* self, ui_component* sender, int keycode, int keydata);
static void OnGridKeyDown(PatternGrid* self, ui_component* sender, int keycode, int keydata);
static void OnGridSize(PatternGrid* self, ui_component* sender, int width, int height);
static void OnViewSize(PatternView* self, ui_component* sender, int width, int height);
static void OnScroll(PatternGrid* self, ui_component* sende, int cx, int cy);
static void SetDefaultSkin(PatternGrid* self);
static void OnMouseDown(PatternGrid* self, ui_component* sender, int x, int y, int button);
static void OnEditPositionChanged(PatternGrid* self, Sequence* sender);
static void ClipBlock(PatternGrid* self, ui_graphics* g, PatternGridBlock* block);
static void DrawDigit(PatternGrid* self, ui_graphics* g, int digit, int col, int x, int y);
static void EnterDigit(int digit, int newval, unsigned char* val);
static void OnLineNumbersDraw(PatternLineNumbers* self, ui_component* sender, ui_graphics* g);
static void OnLineNumbersLabelDraw(PatternLineNumbers* self, ui_component* sender, ui_graphics* g);
static void InitDefaultSkin(PatternView* self);
static void BlitSkinPart(PatternHeader* self, ui_graphics* g, int x, int y, SkinCoord* coord);
static void LineNumbersDrawBackground(PatternLineNumbers* self, ui_graphics* g);
static void OnTimer(PatternView* self, ui_component* sender, int timerid);
static void OnPropertiesClose(PatternView* self, ui_component* sender);
static void OnPropertiesApply(PatternView* self, ui_component* sender);
static int NumLines(PatternView* self);
static void AdjustScrollranges(self, width, height);
static float Offset(PatternGrid* self, int y, int* lines, int* sublines, int* subline);

char* notes_tab_a440[256] = {
	"C-m","C#m","D-m","D#m","E-m","F-m","F#m","G-m","G#m","A-m","A#m","B-m", //0
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //1
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //2
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //3
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //4
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //5
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //6
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //7
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //8
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //9
	"off","twk","twf","mcm","tws","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};
char* notes_tab_a220[256] = {
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //0
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //1
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //2
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //3
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //4
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //5
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //6
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //7
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //8
	"C-9","C#9","D-9","D#9","E-9","F-9","F#9","G-9","G#9","A-9","A#9","B-9", //9
	"off","twk","twf","mcm","tws","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};

void InitPatternGrid(PatternGrid* self, ui_component* parent, PatternView* view, Player* player)
{		
	self->view = view;
	self->header = 0;
	if (self->view->skin.hfont == NULL) {
		self->view->skin.hfont = ui_createfont("Tahoma", 12);
	}
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_size, self, OnGridSize);
	signal_connect(&self->component.signal_keydown,self, OnGridKeyDown);
	signal_connect(&self->component.signal_mousedown, self,OnMouseDown);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_scroll, self, OnScroll);	
	ui_component_move(&self->component, 0, 0);

	self->player = player;
	self->numtracks = 16;	
	self->lpb = 4;
	self->bpl = 1.0f / self->lpb;
	self->notestab = notes_tab_a220;	
	self->cursor.track = 0;
	self->cursor.offset = 0;
	self->cursor.subline = 0;
	self->cursor.col = 0;
	self->cursorstep = 0.25;
	self->dx = 0;
	self->dy = 0;
		
	self->textwidth = 10;
	self->colx[0] = 0;
	self->colx[1] = (self->textwidth*3)+2;
	self->colx[2] = self->colx[1]+self->textwidth;
	self->colx[3] = self->colx[2]+self->textwidth+1;
	self->colx[4] = self->colx[3]+self->textwidth;
	self->colx[5] = self->colx[4]+self->textwidth+1;
	self->colx[6] = self->colx[5]+self->textwidth;
	self->colx[7] = self->colx[6]+self->textwidth;
	self->colx[8] = self->colx[7]+self->textwidth;
	self->colx[9] = self->colx[8]+self->textwidth+1;
	self->trackwidth = self->colx[9];	
	self->lineheight = 12;	
	self->component.scrollstepx = self->trackwidth;
	self->component.scrollstepy = self->lineheight;
	ui_component_showhorizontalscrollbar(&self->component);
	ui_component_showverticalscrollbar(&self->component);
	ui_component_sethorizontalscrollrange(&self->component, 0, 16);
	self->component.doublebuffered = 1;	
	signal_connect(&self->player->song->sequence.signal_editposchanged,
		self, OnEditPositionChanged);	
}

void PatternViewApplyProperties(PatternView* self, Properties* properties)
{
	properties_readint(properties, "pvc_separator", &self->skin.separator, 0x00400000);
	properties_readint(properties, "pvc_separator2", &self->skin.separator2, 0x00004000);
	properties_readint(properties, "pvc_background", &self->skin.background, 0x009a887c);
	properties_readint(properties, "pvc_background2", &self->skin.background2, 0x00aa786c);
	properties_readint(properties, "pvc_row4beat", &self->skin.row4beat, 0x00d5ccc6);
	properties_readint(properties, "pvc_row4beat2", &self->skin.row4beat2, 0x00fdfcf6);
	properties_readint(properties, "pvc_rowbeat", &self->skin.rowbeat, 0x00c9beb8);
	properties_readint(properties, "pvc_rowbeat2", &self->skin.rowbeat2, 0x00f9eee8);
	properties_readint(properties, "pvc_row", &self->skin.row, 0x00c1b5aa);
	properties_readint(properties, "pvc_row2", &self->skin.row2, 0x00f1e5da);
	properties_readint(properties, "pvc_font", &self->skin.font, 0x00000000);
	properties_readint(properties, "pvc_font2", &self->skin.font2, 0x00000000);
	properties_readint(properties, "pvc_fontplay", &self->skin.fontPlay, 0x00ffffff);
	properties_readint(properties, "pvc_fontplay2", &self->skin.fontPlay2, 0x00ffffff);
	properties_readint(properties, "pvc_fontcur", &self->skin.fontCur, 0x00ffffff);
	properties_readint(properties, "pvc_fontcur2", &self->skin.fontCur2, 0x00ffffff);
	properties_readint(properties, "pvc_fontsel", &self->skin.fontSel, 0x00ffffff);
	properties_readint(properties, "pvc_fontsel2", &self->skin.fontSel2, 0x00ffffff);
	properties_readint(properties, "pvc_selection", &self->skin.selection, 0x00e00000);
	properties_readint(properties, "pvc_selection2", &self->skin.selection2, 0x00ff5050);
	properties_readint(properties, "pvc_playbar", &self->skin.playbar, 0x0000e000);
	properties_readint(properties, "pvc_playbar2", &self->skin.playbar2, 0x0000e000);
	properties_readint(properties, "pvc_cursor", &self->skin.cursor, 0x005050e0);
	properties_readint(properties, "pvc_cursor2", &self->skin.cursor2, 0x005050ff);
	ui_invalidate(&self->component);
}

void OnDraw(PatternGrid* self, ui_component* sender, ui_graphics* g)
{	 
  	PatternGridBlock clip;
	if (self->view->skin.hfont) {
		ui_setfont(g, self->view->skin.hfont);
	}	
	ClipBlock(self, g, &clip);
	DrawBackground(self, g, &clip);
	if (self->view->pattern) {		
		DrawEvents(self, g, &clip);
	}
}

float Offset(PatternGrid* self, int y, int* lines, int* sublines, int* subline)
{
	float offset = 0;	
	int cpy = 0;		
	int first = 1;
	int count = y / self->lineheight;
	int remaininglines = 0;
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
				if (*lines + *sublines >= count) {
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
			if (*lines + *sublines >= count) {
				break;
			}
		} while (curr);
		if (!curr || (*lines + *sublines >= count)) {
			break;
		}
		++(*lines);
		*subline = 0;		
		offset += self->bpl;
	}	
	remaininglines =  (count - (*lines + *sublines));
	*lines += remaininglines;
	return offset + remaininglines * self->bpl;
}

void ClipBlock(PatternGrid* self, ui_graphics* g, PatternGridBlock* block)
{	
	int lines;
	int sublines;
	int subline;
	block->topleft.track = (g->clip.left - self->dx) / self->trackwidth;
	block->topleft.col = 0;
	block->topleft.offset =  Offset(self, g->clip.top - self->dy, &lines, &sublines, &subline);
	block->topleft.line = lines;
	block->topleft.subline = subline;
	block->topleft.totallines = lines + sublines;
	block->bottomright.track = (g->clip.right - self->dx + self->trackwidth) / self->trackwidth;
	if (block->bottomright.track > self->numtracks) {
		block->bottomright.track = self->numtracks;
	}
	block->bottomright.col = 0;
	block->bottomright.offset = Offset(self, g->clip.bottom - self->dy, &lines, &sublines, &subline);
	block->bottomright.line = lines;
	block->bottomright.totallines = lines + sublines;
	block->bottomright.subline = subline;
}

void DrawBackground(PatternGrid* self, ui_graphics* g, PatternGridBlock* clip)
{
	int track;
	for (track = clip->topleft.track; track < clip->bottomright.track; ++track) {
		DrawTrackBackground(self, g, track);
	}
}

void DrawTrackBackground(PatternGrid* self, ui_graphics* g, int track)
{
	ui_rectangle r;
	
	ui_setrectangle(&r, track * self->trackwidth + self->dx, 0, self->trackwidth, self->cy);
	ui_drawsolidrectangle(g, r, self->view->skin.background);	
}

int TestCursor(PatternGrid* self, int track, float offset, int subline)
{
	return self->cursor.track == track && 
		self->cursor.offset >= offset && self->cursor.offset < offset + self->bpl
		&& self->cursor.subline == subline;
}

void DrawEvents(PatternGrid* self, ui_graphics* g, PatternGridBlock* clip)
{	
	int track;
	int cpx = 0;	
	int cpy;
	float offset;
	int subline;	
	PatternNode* prev;
	
	cpy = (clip->topleft.totallines - clip->topleft.subline) * self->lineheight + self->dy;	
	offset = clip->topleft.offset;	
	self->curr_event = pattern_greaterequal(self->view->pattern, offset, &prev);
	subline = 0;	
	while (offset <= clip->bottomright.offset && offset < self->view->pattern->length) {	
		int beat;
		int beat4;
		int fill;			
		beat = fmod(offset, 1.0f) == 0.0f;
		beat4 = fmod(offset, 4.0f) == 0.0f;		
		do {
			fill = 0;
			cpx = clip->topleft.track * self->trackwidth + self->dx;						
			for (track =  clip->topleft.track; track < clip->bottomright.track; ++track) {						
				int hasevent = 0;
				int cursor = TestCursor(self, track, offset, subline);
				int playbar = 0;
				playbar = (self->player->playing &&
						   self->player->sequencer.pos >= offset &&
						   self->player->sequencer.pos < offset + self->bpl) ? 1 : 0;

				while (!fill && self->curr_event &&
					((PatternEntry*)(self->curr_event->entry))->track <= track &&
					((PatternEntry*)(self->curr_event->entry))->offset >= offset && 				
					((PatternEntry*)(self->curr_event->entry))->offset < offset + self->bpl) {
					if (((PatternEntry*)(self->curr_event->entry))->track == track) {
						DrawPatternEvent(self, g, &((PatternEntry*)(self->curr_event->entry))->event, cpx, cpy, playbar, cursor, beat, beat4);
						hasevent = 1;
						self->curr_event = self->curr_event->next;
						break;
					}
					self->curr_event = self->curr_event->next;
				}
				if (!hasevent) {
					PatternEvent event;
					memset(&event, 0xFF, sizeof(PatternEvent));
					event.cmd = 0;
					event.parameter = 0;
					DrawPatternEvent(self, g, &event, cpx, cpy, playbar, cursor, beat, beat4);
				} else
				if (self->curr_event && ((PatternEntry*)(self->curr_event->entry))->track <= track) {
					fill = 1;					
				}
				cpx += self->trackwidth;			
			}
			cpy += self->lineheight;
			subline++;
		} while (self->curr_event &&
			((PatternEntry*)(self->curr_event->entry))->offset < offset + self->bpl);
		offset += self->bpl;
		subline = 0;
	}
}

void SetColColor(PatternSkin* skin, ui_graphics* g, int col, int playbar, int cursor, int beat, int beat4)
{
	if (cursor != 0) {
		if (beat4) {
			ui_setbackgroundcolor(g, skin->cursor);
			ui_settextcolor(g, skin->fontCur);			
		} else
		if (beat) {
			ui_setbackgroundcolor(g, skin->cursor);
			ui_settextcolor(g, skin->fontCur);			
		} else {
			ui_setbackgroundcolor(g, skin->cursor);
			ui_settextcolor(g, skin->fontCur);
		}
	} else 
	if (playbar) {
		ui_setbackgroundcolor(g, skin->playbar);
		ui_settextcolor(g, skin->fontPlay);		
	} else	{
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

void DrawPatternEvent(PatternGrid* self, ui_graphics* g, PatternEvent* event, int x, int y, int playbar, int cursor, int beat, int beat4)
{					
	ui_rectangle r;	

	SetColColor(&self->view->skin, g, 0, playbar, cursor && self->cursor.col == 0, beat, beat4);
	ui_setrectangle(&r, x + self->colx[0], y, self->textwidth*3, self->lineheight);
	ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r,
	self->notestab[event->note],
	strlen(self->notestab[event->note]));			
	
	{	// inst
		int hi = (event->inst & 0xF0) >> 4;
		int lo = event->inst & 0x0F;
		if (event->inst == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 1, playbar, cursor && (self->cursor.col == 1), beat, beat4);
		DrawDigit(self, g, hi, 1, x, y);
		SetColColor(&self->view->skin, g, 2, playbar, cursor && (self->cursor.col == 2), beat, beat4);
		DrawDigit(self, g, lo, 2, x, y);
	}
	{	// mach
		int hi = (event->mach & 0xF0) >> 4;
		int lo = event->mach & 0x0F;
		if (event->mach == 0xFF) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 3, playbar, cursor && (self->cursor.col == 3), beat, beat4);
		DrawDigit(self, g, hi, 3, x, y);
		SetColColor(&self->view->skin, g, 4, playbar, cursor && (self->cursor.col == 4), beat, beat4);
		DrawDigit(self, g, lo, 4, x, y);
	}
	{	// cmd
		int hi = (event->cmd & 0xF0) >> 4;
		int lo = event->cmd & 0x0F;				
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 5, playbar, cursor && (self->cursor.col == 5), beat, beat4);
		DrawDigit(self, g, hi, 5, x, y);
		SetColColor(&self->view->skin, g, 6, playbar, cursor && (self->cursor.col == 6), beat, beat4);
		DrawDigit(self, g, lo, 6, x, y);
	}
	{	// parameter
		int hi = (event->parameter & 0xF0) >> 4;
		int lo = event->parameter & 0x0F;		
		if (event->cmd == 0x00 && event->parameter == 0x00) {
			hi = -1;
			lo = -1;
		}
		SetColColor(&self->view->skin, g, 7, playbar, cursor && (self->cursor.col == 7), beat, beat4);
		DrawDigit(self, g, hi, 7, x, y);
		SetColColor(&self->view->skin, g, 8, playbar, cursor && (self->cursor.col == 8), beat, beat4);
		DrawDigit(self, g, lo, 8, x, y);
	}		
}

void DrawDigit(PatternGrid* self, ui_graphics* g, int digit, int col, int x, int y)
{
	char buffer[20];	
	ui_rectangle r;
	ui_setrectangle(&r, x + self->colx[col], y, self->textwidth, self->lineheight);
	if (digit != -1) {
		_snprintf(buffer, 2, "%X", digit);	
	} else {
		_snprintf(buffer, 2, "%s", "");	
	}
	ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer, strlen(buffer));	
}

void OnGridSize(PatternGrid* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;
	AdjustScrollranges(self, width, height);
}

void AdjustScrollranges(PatternGrid* self, int width, int height)
{
	int visibletracks;
	int visiblelines;
	
	visibletracks = width / self->trackwidth;
	visiblelines = height / self->lineheight;
	ui_component_sethorizontalscrollrange(&self->component, 0, self->numtracks - visibletracks);
	ui_component_setverticalscrollrange(&self->component, 0, NumLines(self->view) - visiblelines);	
}


int NumSublines(Pattern* pattern, float offset, float bpl)
{
	PatternNode* prev;
	PatternNode* node = pattern_greaterequal(pattern, offset, &prev);		
	int currsubline = -1;
	while (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl) {			
			break;
		}				
		if (entry->track == 0) {
			++currsubline;					
		}
		node = node->next;
	}
	return currsubline;
}

PatternNode* FindNode(Pattern* pattern, int track, float offset, int subline, float bpl, PatternNode** prev)
{
	PatternNode* node = pattern_greaterequal(pattern, offset, prev);	
	int currsubline = 0;
	int first = 1;	
	while (node) {
		PatternEntry* entry = (PatternEntry*)(node->entry);
		if (entry->offset >= offset + bpl) {			
			node = 0;
			break;
		}
		if (entry->track == 0 && !first) {
			++currsubline;				
		}
		if (subline < currsubline) {			
			node = 0;
			break;
		}
		if (entry->track > track && subline == currsubline) {			
			node = 0;
			break;
		}		
		if (entry->track == track && subline == currsubline) {
			break;
		}				
		*prev = node;		
		node = node->next;
		first = 0;
	}
	return node;
}

void OnGridKeyDown(PatternGrid* self, ui_component* sender, int keycode, int keydata)
{
	sender->propagateevent = 1;	
}

void AdvanceCursor(PatternView* self)
{
	if (self->grid.cursor.subline < NumSublines(self->pattern, self->grid.cursor.offset, self->grid.bpl)) {
		++self->grid.cursor.subline;
	} else {
		self->grid.cursor.offset += self->grid.bpl;
		self->grid.cursor.subline = 0;
	}
}

void OnKeyDown(PatternView* self, ui_component* sender, int keycode, int keydata)
{		
	int cmd;
	
	if (keycode == VK_UP) {
		if (self->grid.cursor.subline > 0) {
			--self->grid.cursor.subline;
		} else {
			self->grid.cursor.offset -= self->grid.bpl;
		}
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_DOWN) {		
		AdvanceCursor(self);
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_LEFT) {
		if (self->grid.cursor.col == 0) {
			self->grid.cursor.col = PatternGrid_NUMCOLS - 2;
			--self->grid.cursor.track;
		} else {
			--self->grid.cursor.col;
		}
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_RIGHT) {
		if (self->grid.cursor.col == PatternGrid_NUMCOLS - 2) {
			self->grid.cursor.col = 0;
			++self->grid.cursor.track;
		} else {
			++self->grid.cursor.col;
		}
		ui_invalidate(&self->component);
	} else
	if (keycode == VK_TAB && GetKeyState (VK_SHIFT) < 0) {
		self->grid.cursor.track -= 1;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_TAB) {
		self->grid.cursor.track += 1;
		ui_invalidate(&self->component);		
	} else
	if (keycode == VK_DELETE) {
		PatternNode* prev;
		PatternNode* node = FindNode(self->pattern, self->grid.cursor.track,
			self->grid.cursor.offset, self->grid.cursor.subline, self->grid.bpl, &prev);
		if (node) {
			pattern_remove(self->pattern, node);
			ui_invalidate(&self->component);
		}
	} else
	if (keycode == VK_RETURN) {
		PatternNode* prev;
		PatternNode* node = FindNode(self->pattern, 0,
			self->grid.cursor.offset, self->grid.cursor.subline + 1, self->grid.bpl, &prev);		
		if (prev && ((PatternEntry*)prev->entry)->offset >= self->grid.cursor.offset) {
			PatternEvent ev = { 255, 255, 255, 0, 0 };
			float offset;
			++self->grid.cursor.subline;
			offset = self->grid.cursor.offset + self->grid.cursor.subline*self->grid.bpl/4;
			pattern_insert(self->pattern, prev, 0, offset, &ev);			
			AdjustScrollranges(&self->grid, self->grid.cx, self->grid.cy);
			ui_invalidate(&self->component);			
		}		
	} else {
		if (self->grid.cursor.col == 0) {
			cmd = Cmd(&self->grid.noteinputs->map, keycode);
			if (cmd != -1) {		
				int base = 48;
				PatternNode* prev;
				PatternNode* node = FindNode(self->pattern, self->grid.cursor.track, self->grid.cursor.offset, self->grid.cursor.subline, self->grid.bpl, &prev);
				if (node) {					
					PatternEntry* entry = (PatternEntry*)(node->entry);
					entry->event.note = (unsigned char)(base + cmd);					
				} else {
					float offset;
					PatternEvent ev = { 0, 255, 255, 0, 0 };
					ev.note = (unsigned char)(base + cmd);
					ev.mach = machines_slot(&self->grid.player->song->machines);
					offset = self->grid.cursor.offset;					
					pattern_insert(self->pattern, prev, self->grid.cursor.track, offset, &ev);
				}
				AdvanceCursor(self);				
				ui_invalidate(&self->component);			
			}		
		} else {
			int val = -1;
			if (keycode >= '0' && keycode <='9') {
				val = keycode - '0';
			} else
			if (keycode >= 'A' && keycode <='Z') {
				val = keycode - 'A' + 10;
			}
			if (val != -1 && self->pattern) {
				PatternNode* prev;
				PatternEntry* entry = 0;
				PatternNode* node = FindNode(self->pattern, self->grid.cursor.track, self->grid.cursor.offset, self->grid.cursor.subline, self->grid.bpl, &prev);
				if (node) {					
					entry = (PatternEntry*)(node->entry);					
				} else {
					float offset;
					int base = 48;
					PatternEvent ev = { 255, 255, 255, 0, 0 };					
					offset = self->grid.cursor.offset;
					entry = pattern_write(self->pattern, self->grid.cursor.track, offset, ev);					
				}				
				if (entry) {					
					switch (self->grid.cursor.col) {
						case 1: 
							if ((entry->event.inst == 0xFF) && (val != 0x0F)) {
								entry->event.inst = 0;
							}
							EnterDigit(0, val, &entry->event.inst);
						break;
						case 2:
							if ((entry->event.inst == 0xFF) && (val != 0x0F)) {
								entry->event.inst = 0;
							}
							EnterDigit(1, val, &entry->event.inst);
						break;
						case 3:
							if ((entry->event.mach == 0xFF) && (val != 0x0F)) {
								entry->event.mach = 0;
							}
							EnterDigit(0, val, &entry->event.mach);
						break;
						case 4:
							if ((entry->event.mach == 0xFF) && (val != 0x0F)) {
								entry->event.mach = 0;
							}
							EnterDigit(1, val, &entry->event.mach);
						break;
						case 5:							
							EnterDigit(0, val, &entry->event.cmd);
						break;
						case 6:
							EnterDigit(1, val, &entry->event.cmd);
						break;
						case 7:
							EnterDigit(0, val, &entry->event.parameter);
						break;
						case 8:
							EnterDigit(1, val, &entry->event.parameter);
						break;
						default:
						break;
					}
					ui_invalidate(&self->component);
				}
			}
		}		
	}	
	ui_component_propagateevent(sender);
}

void EnterDigit(int digit, int newval, unsigned char* val)
{	
	if (digit == 0) {
		*val = (*val & 0x0F) | ((newval & 0x0F) << 4);
	} else
	if (digit == 1) {
		*val = (*val & 0xF0) | (newval & 0x0F);
	}
}

void OnScroll(PatternGrid* self, ui_component* sender, int cx, int cy)
{	
	self->dx += cx;
	self->dy += cy;	

	if (self->header && cx != 0) {
		self->header->dx += cx;
		ui_invalidate(&self->header->component);
	}
	if (self->linenumbers && cy != 0) {
		self->linenumbers->dy += cy;
		ui_invalidate(&self->linenumbers->component);
		UpdateWindow(self->linenumbers->component.hwnd);
	}
}

void OnMouseDown(PatternGrid* self, ui_component* sender, int x, int y, int button)
{
	if (button == 1) {
		int lines;
		int sublines;
		int subline;		
		int coloffset;				
		self->cursor.offset = Offset(self, y - self->dy, &lines, &sublines, &subline);
		self->cursor.totallines = lines + sublines;
		self->cursor.subline = subline;
		self->cursor.track = (x - self->dx) / self->trackwidth;
		coloffset = (x - self->dx) - self->cursor.track * self->trackwidth;
		if (coloffset < 3*self->textwidth) {
			self->cursor.col = 0;
		} else {
			self->cursor.col = coloffset / self->textwidth - 2;
		}
		ui_invalidate(&self->component);
		ui_component_setfocus(&self->component);
	} else
	if (button == 2) {
		ui_size size = ui_component_size(&self->view->component);
		if (ui_component_visible(&self->view->properties.component)) {			
			ui_component_hide(&self->view->properties.component);			
		} else {
			ui_component_show(&self->view->properties.component);
		}
		OnViewSize(self->view, &self->view->component, size.width, size.height);
	}
}

void OnEditPositionChanged(PatternGrid* self, Sequence* sender)
{	
	if (sender->editpos.sequence) {
		SequenceEntry* entry = (SequenceEntry*)sender->editpos.sequence->entry;	
		PatternViewSetPattern(self->view, patterns_at(&self->player->song->patterns, entry->pattern));
	} else {
		PatternViewSetPattern(self->view, 0);		
	}
	ui_invalidate(&self->component);
}

void InitPatternView(PatternView* self, ui_component* parent, Player* player)
{	
	ui_component_init(&self->component, parent);
	self->skin.skinbmp.hBitmap = LoadBitmap (appInstance, MAKEINTRESOURCE(IDB_HEADERSKIN));
	InitDefaultSkin(self);
	InitPatternHeader(&self->header, &self->component);	
	self->linenumbers.skin = &self->skin;
	InitPatternGrid(&self->grid, &self->component, self, player);
	InitPatternLineNumbersLabel(&self->linenumberslabel, &self->component);	
	InitPatternLineNumbers(&self->linenumbers, &self->component);
	self->linenumbers.view = self;
	self->grid.header = &self->header;
	self->grid.linenumbers = &self->linenumbers;
	self->header.trackwidth = self->grid.trackwidth;
	self->header.skin = &self->skin;
	self->linenumbers.lineheight = 12;
	signal_connect(&self->component.signal_size, self, OnViewSize);
	InitPatternProperties(&self->properties, &self->component, 0);	
	ui_component_hide(&self->properties.component);	
	signal_connect(&self->properties.closebutton.signal_clicked, self, OnPropertiesClose);
	signal_connect(&self->properties.applybutton.signal_clicked, self, OnPropertiesApply);
	signal_connect(&self->component.signal_timer, self, OnTimer);
	signal_connect(&self->component.signal_keydown,self, OnKeyDown);
	PatternViewApplyProperties(self, 0);
	SetTimer(self->component.hwnd, 200, 50, 0);
}

void PatternViewSetPattern(PatternView* self, Pattern* pattern)
{
	ui_size size;
	self->pattern = pattern;
	PatternPropertiesSetPattern(&self->properties, pattern);
	size = ui_component_size(&self->grid.component);
	AdjustScrollranges(&self->grid, size.width, size.height);
}

void InitDefaultSkin(PatternView* self)
{
	SkinCoord background = { 0, 0, 109, 18, 0, 0, 109, 18 };	
	SkinCoord record = { 0, 18, 7, 12, 52, 3, 7, 12 };
	SkinCoord mute = { 81, 18, 11, 11, 75, 3, 11, 11 };
	SkinCoord solo = { 92, 18, 11, 11, 97, 3, 11, 11 };
	SkinCoord digitx0 = { 0, 18, 7, 12, 24, 3, 7, 12 };
	SkinCoord digit0x = { 0, 18, 7, 12, 31, 3, 7, 12 };
		
	self->skin.headercoords.background = background;	
	self->skin.headercoords.record = record;
	self->skin.headercoords.mute = mute;
	self->skin.headercoords.solo = solo;
	self->skin.headercoords.digit0x = digit0x;
	self->skin.headercoords.digitx0 = digitx0;	
}

void OnViewSize(PatternView* self, ui_component* sender, int width, int height)
{
	int headerheight = 20;
	int linenumberwidth = 40;
	int hscrollbarheight = 20;
	int propertyheight = ui_component_visible(&self->properties.component) ? 60 : 0;
	ui_component_move(&self->header.component, linenumberwidth, 0);
	ui_component_resize(&self->header.component, width - linenumberwidth, headerheight);
	ui_component_move(&self->grid.component, linenumberwidth, headerheight);
	ui_component_resize(&self->linenumberslabel.component, linenumberwidth, headerheight);
	ui_component_resize(&self->grid.component, width - linenumberwidth, height - headerheight -  propertyheight);
	ui_component_move(&self->linenumbers.component, 0, headerheight);
	ui_component_resize(&self->linenumbers.component, linenumberwidth, height - headerheight -  propertyheight);
	ui_component_move(&self->properties.component, 0, height - propertyheight);
	ui_component_resize(&self->properties.component, width, propertyheight);
}

void InitPatternHeader(PatternHeader* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnHeaderDraw);
	self->dx = 0;
	self->trackwidth = 100;	
}

void OnHeaderDraw(PatternHeader* self, ui_component* sender, ui_graphics* g)
{	
	int cpx = self->dx;
	int track;	
	for (track = 0; track < 16; ++track) {	
		ui_rectangle r;
		int trackx0 = track / 10;
		int track0x = track % 10;		
		SkinCoord digitx0 = self->skin->headercoords.digitx0;
		SkinCoord digit0x = self->skin->headercoords.digit0x;

		ui_setrectangle(&r, cpx, 0, self->trackwidth, 20);
		ui_drawsolidrectangle(g, r, self->skin->background);
		digitx0.srcx += trackx0 * digitx0.srcwidth;
		digit0x.srcx += track0x * digit0x.srcwidth;
		BlitSkinPart(self, g, cpx, 0, &self->skin->headercoords.background);
		BlitSkinPart(self, g, cpx, 0, &digitx0);
		BlitSkinPart(self, g, cpx, 0, &digit0x);
		cpx += self->trackwidth;
	}
}

void BlitSkinPart(PatternHeader* self, ui_graphics* g, int x, int y, SkinCoord* coord)
{
	ui_drawbitmap(g, &self->skin->skinbmp, x + coord->destx, y + coord->desty,
		coord->destwidth, coord->destheight, coord->srcx, coord->srcy);
}

void InitPatternLineNumbers(PatternLineNumbers* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnLineNumbersDraw);
	self->dy = 0;
	self->lineheight = 12;
	self->component.doublebuffered = 1;
}

void OnLineNumbersDraw(PatternLineNumbers* self, ui_component* sender, ui_graphics* g)
{
	char buffer[20];		
	int cpy = self->dy;
	int line;		
	float offset;
	PatternGridBlock clip;
	ClipBlock(&self->view->grid, g, &clip);
	LineNumbersDrawBackground(self, g);
	if (self->view->pattern) {		
		ui_setfont(g, self->skin->hfont);				
		cpy = (clip.topleft.totallines - clip.topleft.subline) * self->lineheight + self->dy;
		offset = clip.topleft.offset;				
		line = clip.topleft.line;
		while (offset <= clip.bottomright.offset && offset < self->view->pattern->length) {
			ui_rectangle r;			
			int beat;
			int beat4;
			int subline;
			int numsublines;
			int ystart;
			beat = fmod(offset, 1.0f) == 0.0f;
			beat4 = fmod(offset, 4.0f) == 0.0f;
			SetColColor(self->skin, g, 0, 0, 0, beat, beat4);
			// %3i
			_snprintf(buffer, 10, "%.2X %.2f", line, offset);
			ui_setrectangle(&r, 0, cpy, 40-2, self->lineheight);
			ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, buffer,
				strlen(buffer));		
			cpy += self->lineheight;
			ystart = cpy;
			numsublines = NumSublines(self->view->pattern, offset, self->view->grid.bpl);
			for (subline = 0; subline < numsublines; ++subline) {
				_snprintf(buffer, 10, "  %.2X", subline);	
				ui_setrectangle(&r, 0, cpy, 40-2, self->lineheight);
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

void LineNumbersDrawBackground(PatternLineNumbers* self, ui_graphics* g)
{
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_drawsolidrectangle(g, r, self->view->skin.background);
}

void InitPatternLineNumbersLabel(PatternLineNumbersLabel* self, ui_component* parent)
{		
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnLineNumbersLabelDraw);
}

void OnLineNumbersLabelDraw(PatternLineNumbers* self, ui_component* sender, ui_graphics* g)
{	
	ui_rectangle r;
	ui_setrectangle(&r, 0, 0, 40, 20);
	ui_textoutrectangle(g, r.left, r.top, ETO_OPAQUE, r, "Line",
		strlen("Line"));	
}

void OnTimer(PatternView* self, ui_component* sender, int timerid)
{
	if (self->grid.player->playing) {
		ui_invalidate(&self->grid.component);
	}
}

void OnPropertiesClose(PatternView* self, ui_component* sender)
{
	ui_size size = ui_component_size(&self->component);
	OnViewSize(self, &self->component, size.width, size.height);	
}

void OnPropertiesApply(PatternView* self, ui_component* sender)
{
	SCROLLINFO		si;	
	ui_size size = ui_component_size(&self->component);	
	OnViewSize(self, &self->grid.component, size.width, size.height);	
	size = ui_component_size(&self->grid.component);
	AdjustScrollranges(&self->grid, size.width, size.height);	
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (self->grid.component.hwnd, SB_VERT, &si) ;	
	self->grid.dy = -si.nPos * self->grid.lineheight;	
	self->linenumbers.dy = self->grid.dy;
	ui_invalidate(&self->component);
}

int NumLines(PatternView* self)
{
	int lines = 0;
	int sublines = 0;	
	int remaininglines = 0;
	float offset = 0;

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
		remaininglines = (int)(offset * self->grid.lpb);
	}
	return lines + sublines + remaininglines;
}