// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiswitch.h"
// std
#include <stdlib.h>

// prototypes
static void ondestroy(psy_ui_Switch*);
static void ondraw(psy_ui_Switch*, psy_ui_Graphics*);
static void onmousedown(psy_ui_Switch*, psy_ui_MouseEvent*);
// vtable
static psy_ui_ComponentVtable vtable;
static int vtable_initialized = FALSE;

static void vtable_init(psy_ui_Switch* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			ondestroy;		
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			ondraw;
		vtable.onmousedown =
			(psy_ui_fp_component_onmouseevent)
			onmousedown;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
// implementation
void psy_ui_switch_init(psy_ui_Switch* self, psy_ui_Component* parent,
	psy_ui_Component* view)
{		
	psy_ui_component_init(&self->component, parent, view);
	psy_ui_component_setstyletypes(&self->component,
		psy_ui_STYLE_SWITCH, psy_ui_STYLE_SWITCH_HOVER, psy_ui_STYLE_SWITCH_SELECT,
		psy_INDEX_INVALID);
	vtable_init(self);			
	psy_signal_init(&self->signal_clicked);	
	self->state = FALSE;
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(4.0, 1.5));
}

void ondestroy(psy_ui_Switch* self)
{	
	psy_signal_dispose(&self->signal_clicked);	
}

psy_ui_Switch* psy_ui_switch_alloc(void)
{
	return (psy_ui_Switch*)malloc(sizeof(psy_ui_Switch));
}

psy_ui_Switch* psy_ui_switch_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view)
{
	psy_ui_Switch* rv;

	rv = psy_ui_switch_alloc();
	if (rv) {
		psy_ui_switch_init(rv, parent, view);
		psy_ui_component_deallocateafterdestroyed(&rv->component);		
	}
	return rv;
}

void ondraw(psy_ui_Switch* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	const psy_ui_TextMetric* tm;	
	psy_ui_RealSize switchsize;
	psy_ui_RealSize knobsize;
	double centery;
	psy_ui_RealRectangle switchrect;
	psy_ui_RealRectangle knobrect;
	psy_ui_RealSize corner;	
		
	size = psy_ui_component_scrollsize_px(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	switchsize = psy_ui_realsize_make(
		floor(5.0 * tm->tmAveCharWidth), 1.0 * tm->tmHeight);
	centery = floor((size.height - switchsize.height) / 2.0);
	switchrect = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, centery),
		switchsize);
	knobsize = psy_ui_realsize_make(
		floor(3.0 * tm->tmAveCharWidth), 1.0 * tm->tmHeight);
	centery = floor((size.height - knobsize.height) / 2.0);
	knobrect = psy_ui_realrectangle_make(
		psy_ui_realpoint_make(0.0, centery),
		knobsize);
	corner = psy_ui_realsize_make(6.0, 6.0);
	psy_ui_realrectangle_expand(&knobrect, -2.0, -1.0, -2.0, -1.0);	
	psy_ui_drawroundrectangle(g, switchrect, corner);
	if (self->state == FALSE) {		
		psy_ui_drawsolidroundrectangle(g, knobrect, corner,
			psy_ui_component_colour(&self->component));
	} else {
		psy_ui_realrectangle_settopleft(&knobrect,
			psy_ui_realpoint_make(switchsize.width - knobsize.width, knobrect.top));
		psy_ui_drawsolidroundrectangle(g, knobrect, corner,
			psy_ui_component_colour(&self->component));
	}
}

void onmousedown(psy_ui_Switch* self, psy_ui_MouseEvent* ev)
{
	psy_signal_emit(&self->signal_clicked, self, 0);
}

void psy_ui_switch_check(psy_ui_Switch* self)
{
	self->state = TRUE;
	psy_ui_component_addstylestate(&self->component,
		psy_ui_STYLESTATE_SELECT);	
}

void psy_ui_switch_uncheck(psy_ui_Switch* self)
{
	self->state = FALSE;
	psy_ui_component_removestylestate(&self->component,
		psy_ui_STYLESTATE_SELECT);	
}

bool psy_ui_switch_checked(const psy_ui_Switch* self)
{
	return self->state != FALSE;
}
