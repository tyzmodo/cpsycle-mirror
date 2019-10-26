// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentsview.h"

static void OnSize(InstrumentsView* self, ui_component* sender, ui_size*);
static void OnShow(InstrumentsView* self, ui_component* sender);
static void OnHide(InstrumentsView* self, ui_component* sender);

void InitInstrumentsView(InstrumentsView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	ui_component_init(&self->component, parent);		
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_notebook_init(&self->notebook, &self->component);
	ui_component_setbackgroundmode(&self->notebook.component, BACKGROUND_SET);	
	signal_connect(&self->component.signal_size, self, OnSize);
	InitSampulseInstrumentView(&self->sampulseview, &self->notebook.component, workspace);
	InitSamplerInstrumentView(&self->samplerview, &self->notebook.component, workspace);	
	InitTabBar(&self->tabbar, tabbarparent);
	ui_component_setposition(&self->tabbar.component, 450, 0, 160, 20);	
	ui_component_hide(&self->tabbar.component);
	tabbar_append(&self->tabbar, "Sampulse");
	tabbar_append(&self->tabbar, "Sampler PS1");		
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	ui_notebook_setpage(&self->notebook, 0);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
	tabbar_select(&self->tabbar, 0);
}

void OnShow(InstrumentsView* self, ui_component* sender)
{	
	ui_component_show(&self->tabbar.component);
}

void OnHide(InstrumentsView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}

void OnSize(InstrumentsView* self, ui_component* sender, ui_size* size)
{
	ui_component_resize(&self->notebook.component, size->width, size->height);
}
