// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineeditorview.h"

#include <uiwincomponentimp.h>

#include <string.h>
#include <stdlib.h>

#define TIMERID_MACHINEEDITORVIEW 420

static void machineeditorview_ondestroy(MachineEditorView* self, psy_ui_Component* sender);
static void onpreferredsize(MachineEditorView* self, psy_ui_Size* limit, psy_ui_Size* rv);
static void ontimer(MachineEditorView*, int id);

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentImp*)self->imp;
}
#endif

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(MachineEditorView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)onpreferredsize;
		vtable.ontimer = (psy_ui_fp_ontimer)ontimer;
	}
}

void machineeditorview_init(MachineEditorView* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{		
	self->machine = machine;
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	psy_audio_machine_seteditorhandle(machine,
		(void*) psy_ui_win_component_details(&self->component)->hwnd);
#endif
	psy_signal_connect(&self->component.signal_destroy, self,
		machineeditorview_ondestroy);
	psy_ui_component_starttimer(&self->component, TIMERID_MACHINEEDITORVIEW, 50);
}

void machineeditorview_ondestroy(MachineEditorView* self, psy_ui_Component* sender)
{
	if (self->machine) {
		psy_audio_machine_seteditorhandle(self->machine, NULL);
	}
}

MachineEditorView* machineeditorview_alloc(void)
{
	return (MachineEditorView*) malloc(sizeof(MachineEditorView));
}

MachineEditorView* machineeditorview_allocinit(psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{
	MachineEditorView* rv;

	rv = machineeditorview_alloc();
	if (rv) {
		machineeditorview_init(rv, parent, machine, workspace);
	}
	return rv;	
}

void ontimer(MachineEditorView* self, int timerid)
{	
	psy_audio_machine_editoridle(self->machine);
}

void onpreferredsize(MachineEditorView* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (rv) {		
		int width;
		int height;

		psy_audio_machine_editorsize(self->machine, &width, &height);
		rv->width = psy_ui_value_makepx(width);
		rv->height = psy_ui_value_makepx(height);
	}
}