// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineframe.h"

static void OnDestroy(MachineFrame* self, ui_component* frame);
static void OnSize(MachineFrame* self, ui_component* sender, ui_size* size);

void InitMachineFrame(MachineFrame* self, ui_component* parent)
{		
	self->view = 0;
	ui_frame_init(&self->component, parent);	
	ui_component_move(&self->component, 200, 150);
	ui_component_resize(&self->component, 400, 400);	
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);
	//ui_frame_init(self, &self->component, &self->component);	
	//ui_component_resize(&self->component, 400, 400);		
}

void MachineFrameSetParamView(MachineFrame* self, ui_component* view)
{
	self->view = (ui_component*) view;
}


void OnDestroy(MachineFrame* self, ui_component* frame)
{
	self->component.hwnd = 0;		
}

void OnSize(MachineFrame* self, ui_component* sender, ui_size* size)
{
	if (self->view) {
		ui_component_resize(self->view, size->width, size->height);
	}
}
