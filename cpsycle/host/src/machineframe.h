// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEFRAME_H)
#define MACHINEFRAME_H

#include "paramview.h"
#include "paramlistbox.h"
#include "presetsbar.h"
#include "workspace.h"
#include "zoombox.h"

#include <dir.h>
#include <presets.h>
#include <uinotebook.h>
#include <uibutton.h>
#include <uieditor.h>
#include <uiedit.h>
#include <uilabel.h>
#include <uicombobox.h>
#include <presets.h>
#include <machine.h>

// aim: Wraps a Parameterview or Machineeditorview inside a frame. A Frame
//      can be docked at the bottom of the Machineview or floated to
//      a window.

typedef struct {
	psy_ui_Component component;
	psy_ui_Component titlerow;
	psy_ui_Component row0;
	ZoomBox zoombox;
	psy_ui_Label title;
	psy_ui_Button floatview;
	psy_ui_Button close;
	psy_ui_Button mute;
	psy_ui_Button parameters;
	psy_ui_Button command;
	psy_ui_Button help;	
	psy_ui_Button dock;
	PresetsBar presetsbar;	
	psy_audio_Machine* machine;
} ParameterBar;

typedef struct {
	psy_ui_Component component;
	ParameterBar parameterbar;
	ParameterListBox parameterbox;
	psy_ui_Notebook notebook;
	psy_ui_Editor help;
	psy_ui_Component* view;
	psy_ui_Component* machineview;	
	psy_audio_Machine* machine;	
	int dofloat;
	int dodock;
	int doclose;
	int floated;
	Workspace* workspace;
	ParamView* paramview;
} MachineFrame;

void parameterbar_init(ParameterBar*, psy_ui_Component* parent, Workspace* workspace);

void machineframe_init(MachineFrame*, psy_ui_Component* parent, bool floated, Workspace* workspace);
MachineFrame* machineframe_alloc(void);
MachineFrame* machineframe_allocinit(psy_ui_Component* parent, bool floated, Workspace* workspace);

void machineframe_setview(MachineFrame* self, psy_ui_Component* view, psy_audio_Machine*);
void machineframe_setparamview(MachineFrame* self, ParamView* view, psy_audio_Machine*);

#endif