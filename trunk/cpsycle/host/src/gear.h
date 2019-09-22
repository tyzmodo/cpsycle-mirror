// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(GEAR_H)
#define GEAR_H

#include "uibutton.h"
#include "uilistbox.h"
#include "tabbar.h"
#include "workspace.h"


typedef struct {
	ui_component component;
	ui_button createreplace;
	ui_button del;
	ui_button parameters;
	ui_button properties;
	ui_button exchange;
	ui_button clone;
	ui_button showmaster;
} GearButtons;

typedef struct {
	ui_component component;
	TabBar tabbar;
	ui_listbox listbox;	
	GearButtons buttons;
	IntHashTable listboxslots;
	IntHashTable slotslistbox;
	Machines* machines;
} Gear;

void InitGear(Gear*, ui_component* parent, Workspace*);

#endif
