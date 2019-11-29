// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEFRAME_H)
#define MACHINEFRAME_H

#include "paramview.h"

typedef struct {
	ui_component component;
	ui_component* view;
} MachineFrame;

void InitMachineFrame(MachineFrame*, ui_component* parent);
void MachineFrameSetParamView(MachineFrame* self, ui_component* view);

#endif