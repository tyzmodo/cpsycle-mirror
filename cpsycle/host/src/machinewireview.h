// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEWIREVIEW_H)
#define MACHINEWIREVIEW_H

// host
#include "machineframe.h"
#include "machineeditorview.h" // vst view
#include "machineviewskin.h"
#include "machineui.h"
#include "machinestackview.h"
#include "newmachine.h"
#include "paramview.h"
#include "tabbar.h"
#include "workspace.h"
// ui
#include <uiedit.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineWireView
//	
// Displays and edits machines with their wires

typedef struct MachineWireView {
	// inherits
	psy_ui_Component component;
	// internal data
	psy_Table machineuis;
	psy_List* wireframes;
	psy_ui_RealPoint dragpt;	
	uintptr_t dragslot;
	MachineUi* dragmachineui;
	MachineViewDragMode dragmode;
	uintptr_t selectedslot;	
	psy_audio_Wire dragwire;	
	psy_audio_Wire selectedwire;
	psy_audio_Wire hoverwire;	
	psy_audio_PluginCatcher plugincatcher;	
	psy_ui_Edit editname;
	int randominsert;
	int addeffect;
	bool mousemoved;
	bool showwirehover;
	bool vudrawupdate;
	bool drawvirtualgenerators;
	uintptr_t opcount;
	// references
	struct MachineViewBar* statusbar;
	psy_audio_Machines* machines;	
	Workspace* workspace;	
	MachineViewSkin* skin;
} MachineWireView;

void machinewireview_init(MachineWireView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, MachineViewSkin*, Workspace*);
void machinewireview_centermaster(MachineWireView*);
void machinewireview_configure(MachineWireView*, MachineViewConfig*);
void machinewireview_showvirtualgenerators(MachineWireView*);
void machinewireview_hidevirtualgenerators(MachineWireView*);
void machinewireview_updateskin(MachineWireView*);

INLINE psy_ui_Component* machinewireview_base(MachineWireView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* MACHINEWIREVIEW_H */