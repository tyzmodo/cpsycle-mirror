// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiicondraw.h"
// platform
#include "../../detail/trace.h"
#include "../../detail/portable.h"

static void makearrow(psy_ui_RealPoint*, psy_ui_ButtonIcon icon, double x, double y);
static void psy_ui_icondraw_drawarrow(psy_ui_IconDraw*, psy_ui_RealPoint*,
psy_ui_Graphics*);

void psy_ui_icondraw_init(psy_ui_IconDraw* self, psy_ui_ButtonIcon icon,
	const psy_ui_Style* style)
{
	assert(self);
	assert(style);

	self->icon = icon;
	self->style = style;
}

void psy_ui_icondraw_draw(psy_ui_IconDraw* self, psy_ui_Graphics* g,
	psy_ui_RealPoint pt)
{
	psy_ui_RealPoint arrow[4];	
	
	if (self->icon == psy_ui_ICON_LESSLESS) {
		makearrow(arrow, psy_ui_ICON_LESS, pt.x - 4, pt.y);
		psy_ui_icondraw_drawarrow(self, arrow, g);
		makearrow(arrow, psy_ui_ICON_LESS, pt.x + 4, pt.y);
		psy_ui_icondraw_drawarrow(self, arrow, g);
	} else if (self->icon == psy_ui_ICON_MOREMORE) {
		makearrow(arrow, psy_ui_ICON_MORE, pt.x - 6, pt.y);
		psy_ui_icondraw_drawarrow(self, arrow, g);
		makearrow(arrow, psy_ui_ICON_MORE, pt.x + 2, pt.y);
		psy_ui_icondraw_drawarrow(self, arrow, g);
	} else {
		makearrow(arrow, self->icon, pt.x - 2, pt.y);
		psy_ui_icondraw_drawarrow(self, arrow, g);
	}
}

void psy_ui_icondraw_drawarrow(psy_ui_IconDraw* self, psy_ui_RealPoint* arrow, psy_ui_Graphics* g)
{
	uint32_t arrowcolour;
	uint32_t arrowhighlightcolour;

	arrowcolour = self->style->colour.value;
	arrowhighlightcolour = self->style->colour.value;	
	psy_ui_drawsolidpolygon(g, arrow, 4, arrowhighlightcolour,
		arrowhighlightcolour);	
}

void makearrow(psy_ui_RealPoint* arrow, psy_ui_ButtonIcon icon, double x, double y)
{
	switch (icon) {
	case psy_ui_ICON_LESS:
		arrow[0].x = 4 + x;
		arrow[0].y = 0 + y;
		arrow[1].x = 4 + x;
		arrow[1].y = 8 + y;
		arrow[2].x = 0 + x;
		arrow[2].y = 4 + y;
		arrow[3] = arrow[0];
		break;
	case psy_ui_ICON_MORE:
		arrow[0].x = 0 + x;
		arrow[0].y = 0 + y;
		arrow[1].x = 0 + x;
		arrow[1].y = 8 + y;
		arrow[2].x = 4 + x;
		arrow[2].y = 4 + y;
		arrow[3] = arrow[0];
		break;
	case psy_ui_ICON_UP:
		arrow[0].x = 0 + x;
		arrow[0].y = 4 + y;
		arrow[1].x = 8 + x;
		arrow[1].y = 4 + y;
		arrow[2].x = 4 + x;
		arrow[2].y = 0 + y;
		arrow[3] = arrow[0];
		break;
	case psy_ui_ICON_DOWN:
		arrow[0].x = 0 + x;
		arrow[0].y = 0 + y;
		arrow[1].x = 8 + x;
		arrow[1].y = 0 + y;
		arrow[2].x = 4 + x;
		arrow[2].y = 4 + y;
		arrow[3] = arrow[0];
		break;
	default:
		break;
	}
}