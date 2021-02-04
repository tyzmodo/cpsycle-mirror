// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiprogressbar.h"
#include <stdlib.h>
#include <string.h>

static void ondestroy(psy_ui_ProgressBar*, psy_ui_Component* sender);
static void ondraw(psy_ui_ProgressBar*, psy_ui_Graphics*);
static void onpreferredsize(psy_ui_ProgressBar*, psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_ProgressBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw) ondraw;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize) onpreferredsize;
		vtable_initialized = 1;
	}
}

void psy_ui_progressbar_init(psy_ui_ProgressBar* self,
	psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);
	self->text = strdup("");
	self->progress = 0.f;
}

void psy_ui_progressbar_settext(psy_ui_ProgressBar* self, const char* text)
{
}

void ondestroy(psy_ui_ProgressBar* self, psy_ui_Component* sender)
{	
	free(self->text);	
}

void ondraw(psy_ui_ProgressBar* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;	
	psy_ui_RealRectangle r;
	int height;
		
	height = 10;
	size = psy_ui_component_sizepx(&self->component);	
	psy_ui_setrectangle(&r, 0, (size.height - height) / 2, 
		self->progress * size.width, height);
	psy_ui_drawsolidrectangle(g, r, psy_ui_colour_make(0x00D1C5B6));
}

void onpreferredsize(psy_ui_ProgressBar* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	if (rv) {
		rv->width = psy_ui_value_makeew(20);
		rv->height = psy_ui_value_makeeh(1);
	}
}

void psy_ui_progressbar_setprogress(psy_ui_ProgressBar* self, float progress)
{
	self->progress = progress;
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_update(&self->component);	
}

void psy_ui_progressbar_tick(psy_ui_ProgressBar* self)
{
	self->progress += 0.01f;
	psy_ui_component_invalidate(&self->component);
	psy_ui_component_update(&self->component);	
}