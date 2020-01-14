// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIGRAPHICS_H)
#define UIGRAPHICS_H

#include <windows.h>
#include "uidef.h"
#include "uibitmap.h"

typedef struct {
  HDC hdc;  
  HFONT hfont;
  HFONT hFontPrev;
  psy_ui_Rectangle clip;
  HPEN pen;  
  HPEN penprev;
  HBRUSH brush;
  HBRUSH hBrushPrev;
} psy_ui_Graphics;

void ui_graphics_init(psy_ui_Graphics*, HDC);
void ui_graphics_dispose(psy_ui_Graphics*);
void ui_textout(psy_ui_Graphics*, int x, int y,  const char*, size_t len);
void ui_textoutrectangle(psy_ui_Graphics*, int x, int y, unsigned int options,
	psy_ui_Rectangle r, const char* text, size_t len);
void ui_drawrectangle(psy_ui_Graphics*, const psy_ui_Rectangle);
void ui_drawroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle,
	psy_ui_Size cornersize);
psy_ui_Size ui_textsize(psy_ui_Graphics*, const char*);
void ui_drawsolidrectangle(psy_ui_Graphics*, const psy_ui_Rectangle r,
	unsigned int color);
void ui_drawsolidroundrectangle(psy_ui_Graphics*, const psy_ui_Rectangle r,
	psy_ui_Size cornersize, unsigned int color);
void ui_drawsolidpolygon(psy_ui_Graphics*, psy_ui_Point*,
	unsigned int numpoints,  unsigned int inner, unsigned int outter);
void ui_drawline(psy_ui_Graphics*, int x1, int y1, int x2, int y2);
void ui_drawarc(psy_ui_Graphics*, int x1, int y1, int x2, int y2);
void ui_drawfullbitmap(psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y);
void ui_drawbitmap(psy_ui_Graphics*, psy_ui_Bitmap*, int x, int y, int width,
	int height, int xsrc, int ysrc);
void ui_setbackgroundcolor(psy_ui_Graphics*, unsigned int color);
void ui_setbackgroundmode(psy_ui_Graphics*, unsigned int mode);
void ui_settextcolor(psy_ui_Graphics*, unsigned int color);
void ui_setcolor(psy_ui_Graphics*, unsigned int color);
void ui_setfont(psy_ui_Graphics*, psy_ui_Font* font);
void ui_moveto(psy_ui_Graphics*, psy_ui_Point pt);
void ui_devcurveto(psy_ui_Graphics*, psy_ui_Point control_p1,
	psy_ui_Point control_p2, psy_ui_Point p);

#endif
