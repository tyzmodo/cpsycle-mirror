// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUMETER_H)
#define VUMETER_H

#include "uicomponent.h"
#include "workspace.h"

typedef struct {	
	ui_component component;
	psy_dsp_amp_t leftavg;
	psy_dsp_amp_t rightavg;
} Vumeter;

void vumeter_init(Vumeter*, ui_component* parent, Workspace*);

#endif
