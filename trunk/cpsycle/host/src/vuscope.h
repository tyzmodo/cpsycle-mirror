// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(VUSCOPE_H)
#define VUSCOPE_H

#include "uicomponent.h"
#include "workspace.h"

typedef struct {	
	ui_component component;
	Wire wire;
	amp_t leftavg;
	amp_t rightavg;
	float invol;
	float mult;
	float* pSamplesL;
	float* pSamplesR;
	int scope_peak_rate;
	int hold;	
	// memories for vu-meter
	amp_t peakL, peakR;
	int peakLifeL, peakLifeR;
	Workspace* workspace;	
} VuScope;

void vuscope_init(VuScope*, ui_component* parent, Wire wire, Workspace*);
void vuscope_stop(VuScope*);

#endif