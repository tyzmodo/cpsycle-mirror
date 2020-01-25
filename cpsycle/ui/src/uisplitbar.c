// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uisplitbar.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commctrl.h>

static void splitbar_onmousedown(psy_ui_SplitBar*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void splitbar_onmousemove(psy_ui_SplitBar*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void splitbar_onmouseup(psy_ui_SplitBar*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void splitbar_onmouseenter(psy_ui_SplitBar*, psy_ui_Component* sender);
static void splitbar_onmouseleave(psy_ui_SplitBar*, psy_ui_Component* sender);
static void splitbar_onpreferredsize(psy_ui_SplitBar*, psy_ui_Size* limit,
	psy_ui_Size* size);
static void splitbar_ondraw(psy_ui_SplitBar*, psy_ui_Graphics*);
static psy_ui_Component* splitbar_prevcomponent(psy_ui_SplitBar*);
static void splitbar_setcursor(psy_ui_SplitBar*);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_SplitBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			splitbar_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) splitbar_ondraw;
		vtable_initialized = 1;
	}
}

void psy_ui_splitbar_init(psy_ui_SplitBar* self, psy_ui_Component* parent)
{		
	self->resize = 0;
	self->hover = 0;
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_LEFT);
	psy_ui_component_resize(&self->component, 4, 5);
	psy_signal_connect(&self->component.signal_mousedown, self, splitbar_onmousedown);
		psy_signal_connect(&self->component.signal_mousemove, self, splitbar_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self, splitbar_onmouseup);
	psy_signal_connect(&self->component.signal_mouseenter, self, splitbar_onmouseenter);
	psy_signal_connect(&self->component.signal_mouseleave, self, splitbar_onmouseleave);	

}

void splitbar_onpreferredsize(psy_ui_SplitBar* self, psy_ui_Size* limit, psy_ui_Size* rv)
{		
	if (rv) {		
		psy_ui_TextMetric tm;

		tm = psy_ui_component_textmetric(&self->component);		
		rv->width = (int)(tm.tmAveCharWidth * 0.8);
		rv->height = (int) (tm.tmHeight * 1.5);
	} else {
		*rv = psy_ui_component_size(&self->component);
	}
}

void splitbar_onmousedown(psy_ui_SplitBar* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	psy_ui_component_capture(sender);
	self->resize = 1;
	splitbar_setcursor(self);
}

void splitbar_onmousemove(psy_ui_SplitBar* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->resize == 1) {		
		psy_ui_Rectangle position;
			
		position = psy_ui_component_position(sender);
		if (sender->align == psy_ui_ALIGN_LEFT) {
			psy_ui_component_move(sender, position.left + ev->x, position.top);
		} else {
			psy_ui_component_move(sender, position.left, position.top + ev->y);
		}
		psy_ui_component_invalidate(sender);
		psy_ui_component_update(sender);		
	} else {
		psy_ui_component_invalidate(sender);
	}
	splitbar_setcursor(self);
}

void splitbar_onmouseup(psy_ui_SplitBar* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{			
	psy_ui_Rectangle position;
	psy_ui_Component* prev;

	psy_ui_component_releasecapture(&self->component);
	self->resize = 0;
	prev = splitbar_prevcomponent(self);
	if (prev) {		
		position = psy_ui_component_position(sender);
		if (prev->align == psy_ui_ALIGN_LEFT) {
			psy_ui_component_resize(prev, position.left,
				psy_ui_component_size(prev).height);
		} else
		if (prev->align == psy_ui_ALIGN_TOP) {
			psy_ui_component_resize(prev, psy_ui_component_size(prev).width,
				position.top);
		}
		psy_ui_component_align(psy_ui_component_parent(sender));
	}
	splitbar_setcursor(self);
}

void splitbar_onmouseenter(psy_ui_SplitBar* self, psy_ui_Component* sender)
{	
	self->hover = 1;
//	ui_component_setbackgroundcolor(sender, 0x00666666);
	psy_ui_component_invalidate(sender);
	splitbar_setcursor(self);
}

void splitbar_onmouseleave(psy_ui_SplitBar* self, psy_ui_Component* sender)
{			
	self->hover = 0;
//	ui_component_setbackgroundcolor(sender, 0x00232323);
	psy_ui_component_invalidate(sender);
}

psy_ui_Component* splitbar_prevcomponent(psy_ui_SplitBar* self)
{
	psy_ui_Component* rv = 0;
	psy_List* c;

	c = psy_ui_component_children(psy_ui_component_parent(&self->component),
		0);
	while (c) {
		if (c->entry == &self->component) {
			c = c->prev;
			while (c && ((psy_ui_Component*)c->entry)->align != 
					self->component.align) {
				c = c->prev;
			}
			break;
		}
		c = c->next;
	}	
	if (c) {		
		rv = (psy_ui_Component*) c->entry;
	}
	psy_list_free(c);
	return rv;
}

void splitbar_setcursor(psy_ui_SplitBar* self)
{
	if (self->component.align == psy_ui_ALIGN_LEFT) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
	} else
	if (self->component.align == psy_ui_ALIGN_TOP) {
		psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_ROW_RESIZE);
	}
}

void splitbar_ondraw(psy_ui_SplitBar* self, psy_ui_Graphics* g)
{
	psy_ui_Rectangle r;
	psy_ui_Size size;
	
	size = psy_ui_component_size(&self->component);
	if (self->component.align == psy_ui_ALIGN_LEFT) {
		psy_ui_setrectangle(&r, (int) (size.width * 0.1), 0,
			(int) (size.width * 0.8), size.height);
	} else {
		psy_ui_setrectangle(&r, 0, (int) (size.height * 0.4),
			size.width, (int) (size.height * 0.2));
	}
	if (self->hover) {
		psy_ui_drawsolidrectangle(g, r, 0x00666666);
	} else {
		psy_ui_drawsolidrectangle(g, r, 0x00232323);
	}
}
