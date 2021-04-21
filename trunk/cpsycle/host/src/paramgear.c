// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramgear.h"
// audio
#include <songio.h>
#include <exclusivelock.h>
// platform
#include "../../detail/portable.h"

// ParamRackBox
// prototypes
static void paramrackbox_ondestroy(ParamRackBox*, psy_ui_Component* sender);
static void paramrackbox_onmousedoubleclick(ParamRackBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void paramrackbox_onthemechanged(ParamRackBox*, MachineViewConfig*,
	psy_Property* theme);
static void paramrackbox_onaddeffect(ParamRackBox*, psy_ui_Button* sender);
// implementation
void paramrackbox_init(ParamRackBox* self, psy_ui_Component* parent,
	uintptr_t slot, Workspace* workspace)
{
	psy_Property* theme;
	psy_audio_Machine* machine;
	psy_ui_Margin margin;

	machine = psy_audio_machines_at(&workspace->song->machines, slot);
	psy_ui_component_init(&self->component, parent, NULL);
	psy_signal_connect(&self->component.signal_destroy, self,
		paramrackbox_ondestroy);
	self->workspace = workspace;
	self->slot = slot;
	theme = workspace->config.macparam.theme;
	if (theme) {
		psy_ui_component_setbackgroundcolour(&self->component, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlecolour", 0x00292929)));		
	}
	psy_ui_component_init(&self->header, &self->component, NULL);
	psy_ui_component_setalign(&self->header, psy_ui_ALIGN_TOP);
	if (theme) {
		psy_ui_component_setcolour(&self->header, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlefontcolour", 0x00B4B4B4)));
	}
	// title label
	psy_ui_label_init(&self->title, &self->header, NULL);
	psy_ui_label_preventtranslation(&self->title);	
	if (machine) {
		psy_ui_label_settext(&self->title, psy_audio_machine_editname(machine));
	}
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->title.component.signal_mousedoubleclick, self,
		paramrackbox_onmousedoubleclick);	
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.5, 0.5);		
	//psy_ui_component_setspacing(&self->title.component, &margin);
	// Insert Effect
	if (machine && slot != psy_audio_MASTER_INDEX) {
		psy_ui_button_init_text(&self->inserteffect, &self->header, NULL, "+");
		psy_ui_component_setalign(&self->inserteffect.component, psy_ui_ALIGN_RIGHT);
		psy_signal_connect(&self->inserteffect.signal_clicked, self,
			paramrackbox_onaddeffect);
	}
	// Parameter List
	parameterlistbox_init(&self->parameters, &self->component,
		psy_audio_machines_at(&workspace->song->machines, slot),
		psycleconfig_macparam(workspace_conf(workspace)));
	psy_ui_component_setalign(&self->parameters.component, psy_ui_ALIGN_LEFT);
	psy_signal_connect(
		&psycleconfig_macview(workspace_conf(workspace))->signal_themechanged,
		self, paramrackbox_onthemechanged);
	self->nextbox = NULL;
}

void paramrackbox_ondestroy(ParamRackBox* self, psy_ui_Component* sender)
{
	psy_signal_disconnect_context(&psycleconfig_macview(
		workspace_conf(self->workspace))->signal_themechanged, self);
}

void paramrackbox_select(ParamRackBox* self)
{
	psy_ui_component_invalidate(&self->header);
}

void paramrackbox_deselect(ParamRackBox* self)
{
	psy_ui_component_invalidate(&self->header);
}

void paramrackbox_onmousedoubleclick(ParamRackBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	workspace_showparameters(self->workspace, self->slot);	
}

void paramrackbox_onthemechanged(ParamRackBox* self, MachineViewConfig* config,
	psy_Property* theme)
{
	if (theme) {
		psy_ui_component_setbackgroundcolour(&self->component, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlecolour", 0x00292929)));	
		psy_ui_component_setcolour(&self->header, psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlefontcolour", 0x00B4B4B4)));
		psy_ui_component_setbackgroundcolour(&self->parameters.listbox.component,
			psy_ui_colour_make(
			psy_property_at_colour(theme, "machineguititlecolour", 0x00292929)));		
	}
	psy_ui_component_invalidate(&self->component);
}

void paramrackbox_onaddeffect(ParamRackBox* self, psy_ui_Button* sender)
{
	if (self->workspace && workspace_song(self->workspace)) {
		if (self->nextbox) {
			psy_audio_machines_selectwire(&self->workspace->song->machines,
				psy_audio_wire_make(self->slot, self->nextbox->slot));
			workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
				SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_ADDEFFECT);
		} else {
			workspace_selectview(self->workspace, VIEW_ID_MACHINEVIEW,
				SECTION_ID_MACHINEVIEW_NEWMACHINE, NEWMACHINE_APPEND);
		}
	}
}

// ParamRackPane
// implementation
static void paramrackpane_ondestroy(ParamRackPane*);
static void paramrackpane_build(ParamRackPane*);
static void paramrackpane_buildall(ParamRackPane*);
static void paramrackpane_buildinputs(ParamRackPane*);
static void paramrackpane_buildoutputs(ParamRackPane*);
static void paramrackpane_buildinchain(ParamRackPane*);
static void paramrackpane_buildoutchain(ParamRackPane*, uintptr_t slot);
static void paramrackpane_buildlevel(ParamRackPane*, uintptr_t level);
static void paramrackpane_insertbox(ParamRackPane*, uintptr_t slot);
static void paramrackpane_removebox(ParamRackPane*, uintptr_t slot);
static void paramrackpane_onsongchanged(ParamRackPane*, Workspace* sender,
	int flag, psy_audio_Song*);
static void paramrackpane_connectsong(ParamRackPane*);
static void paramrackpane_onmachinesinsert(ParamRackPane*,
	psy_audio_Machines*, uintptr_t slot);
static void paramrackpane_onmachinesremoved(ParamRackPane*,
	psy_audio_Machines*, uintptr_t slot);
static void paramrackpane_onconnected(ParamRackPane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void paramrackpane_ondisconnected(ParamRackPane*,
	psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void paramrackpane_clear(ParamRackPane*);

// ParamRackPane
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ParamRackPane* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)paramrackpane_ondestroy;		
		vtable_initialized = TRUE;
	}
}
// implementation
void paramrackpane_init(ParamRackPane* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->lastselected = psy_INDEX_INVALID;
	self->level = 2;	
	psy_ui_component_setdefaultalign(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_make(
		psy_ui_value_makeeh(0.0), psy_ui_value_makeew(0.1),
		psy_ui_value_makeeh(0.0), psy_ui_value_makeew(0.0)));	
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setscrollstep_width(&self->component, psy_ui_value_make_px(100));
	if (workspace_song(workspace)) {
		self->machines = &workspace->song->machines;
	}
	self->mode = PARAMRACK_OUTCHAIN;
	self->workspace = workspace;
	psy_signal_connect(&workspace->signal_songchanged, self,
		paramrackpane_onsongchanged);
	paramrackpane_connectsong(self);
	psy_table_init(&self->boxes);
	self->lastinserted = NULL;
	paramrackpane_build(self);
}

void paramrackpane_ondestroy(ParamRackPane* self)
{
	psy_table_dispose(&self->boxes);
}

void paramrackpane_setmode(ParamRackPane* self, ParamRackMode mode)
{
	if (self->mode != mode) {
		self->mode = mode;
		paramrackpane_build(self);
	}
}

void paramrackpane_onsongchanged(ParamRackPane* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{
	if (workspace_song(sender)) {
		self->machines = &sender->song->machines;
		self->lastselected = psy_audio_machines_selected(self->machines);		
		paramrackpane_connectsong(self);		
		paramrackpane_build(self);		
	} else {
		self->machines = NULL;
		self->lastselected = psy_INDEX_INVALID;
		psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
			psy_ui_component_destroy);
		psy_table_init(&self->boxes);
		psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	}
}

void paramrackpane_build(ParamRackPane* self)
{
	if (!self->machines) {
		return;
	}
	self->lastselected = psy_audio_machines_selected(self->machines);
	self->lastinserted = NULL;
	switch (self->mode) {
		case PARAMRACK_INPUTS:
			paramrackpane_clear(self);
			paramrackpane_buildinputs(self);
		break;
		case PARAMRACK_OUTPUTS:
			paramrackpane_clear(self);
			paramrackpane_buildoutputs(self);
			break;
		case PARAMRACK_INCHAIN:
			paramrackpane_clear(self);
			paramrackpane_buildinchain(self);
			break;
		case PARAMRACK_OUTCHAIN: {			
			paramrackpane_clear(self);
			paramrackpane_buildoutchain(self, self->lastselected);			
			break; }
		case PARAMRACK_LEVEL: {
			psy_ui_component_hide(&self->component);
			paramrackpane_clear(self);
			paramrackpane_buildlevel(self, self->level);
			psy_ui_component_show(&self->component);
			break; }
		// Fallthrough
		case PARAMRACK_ALL:
		default:
			paramrackpane_buildall(self);
			break;
	}
	psy_ui_component_align(&self->component);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void paramrackpane_clear(ParamRackPane* self)
{
	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	psy_ui_component_update(
		psy_ui_component_parent(&self->component));
}

void paramrackpane_buildall(ParamRackPane* self)
{
	psy_TableIterator it;	

	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);	
	psy_table_init(&self->boxes);
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	for (it = psy_audio_machines_begin(self->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		if (!psy_audio_machines_isvirtualgenerator(self->machines,
				psy_tableiterator_key(&it))) {
			paramrackpane_insertbox(self, psy_tableiterator_key(&it));
		}
	}
}

void paramrackpane_buildinputs(ParamRackPane* self)
{
	psy_audio_MachineSockets* sockets;

	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	paramrackpane_insertbox(self, self->lastselected);
	sockets = psy_audio_connections_at(&self->machines->connections,
		self->lastselected);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {
				if (!psy_table_exists(&self->boxes, socket->slot)) {
					paramrackpane_insertbox(self, socket->slot);
				}
			}
		}
	}
}

void paramrackpane_buildoutputs(ParamRackPane* self)
{	
	psy_audio_MachineSockets* sockets;

	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	paramrackpane_insertbox(self, self->lastselected);
	sockets = psy_audio_connections_at(&self->machines->connections,
		self->lastselected);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {
				if (!psy_table_exists(&self->boxes, socket->slot)) {
					paramrackpane_insertbox(self, socket->slot);
				}
			}
		}
	}
}

void paramrackpane_buildinchain(ParamRackPane* self)
{
	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	psy_ui_component_invalidate(psy_ui_component_parent(&self->component));
	if (self->machines) {
		MachineList* path;
			
		path = psy_audio_compute_path(self->machines, self->lastselected,
			FALSE);
		for (; path != 0; path = path->next) {
			uintptr_t slot;

			slot = (size_t)path->entry;
			if (slot == psy_INDEX_INVALID) {
				// delimits the machines that could be processed parallel
				// todo: add thread functions
				continue;
			}
			paramrackpane_insertbox(self, slot);
		}
		psy_list_free(path);
	}
}

void paramrackpane_buildoutchain(ParamRackPane* self, uintptr_t slot)
{
	psy_audio_MachineSockets* sockets;
	
	paramrackpane_insertbox(self, slot);
	sockets = psy_audio_connections_at(&self->machines->connections, slot);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->outputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (socket->slot != psy_INDEX_INVALID) {
				if (!psy_table_exists(&self->boxes, socket->slot)) {
					paramrackpane_buildoutchain(self, socket->slot);
				}
			}
		}
	}
}

void paramrackpane_buildlevel(ParamRackPane* self, uintptr_t level)
{
	psy_table_disposeall(&self->boxes, (psy_fp_disposefunc)
		psy_ui_component_destroy);
	if (self->machines) {
		MachineList* path;

		path = psy_audio_machines_level(self->machines, psy_audio_MASTER_INDEX, level);
		for (; path != 0; path = path->next) {
			uintptr_t slot;

			slot = (size_t)path->entry;
			if (slot == psy_INDEX_INVALID) {
				continue;
			}
			paramrackpane_insertbox(self, slot);
		}
		psy_list_free(path);
	}
}

 void paramrackpane_insertbox(ParamRackPane* self, uintptr_t slot)
{
	if (psy_audio_machines_at(self->machines, slot)) {
		ParamRackBox* box;

		if (psy_table_exists(&self->boxes, slot)) {
			paramrackpane_removebox(self, slot);
		}
		box = (ParamRackBox*)malloc(sizeof(ParamRackBox));
		if (box) {			
			paramrackbox_init(box, &self->component, slot, self->workspace);
			if (self->lastinserted) {
				self->lastinserted->nextbox = box;
			}
			self->lastinserted = box;
			psy_ui_component_setalign(&box->component, psy_ui_ALIGN_LEFT);
			psy_table_insert(&self->boxes, slot, box);			
		}		
	}
}

void paramrackpane_removebox(ParamRackPane* self, uintptr_t slot)
{
	if (psy_table_exists(&self->boxes, slot)) {
		ParamRackBox* box;

		box = psy_table_at(&self->boxes, slot);
		psy_ui_component_deallocate(&box->component);		
		psy_table_remove(&self->boxes, slot);		
	}
}

void paramrackpane_connectsong(ParamRackPane* self)
{
	if (self->machines) {		
		psy_signal_connect(&self->machines->signal_insert, self,
			paramrackpane_onmachinesinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			paramrackpane_onmachinesremoved);
		psy_signal_connect(&self->machines->connections.signal_connected, self,
			paramrackpane_onconnected);
		psy_signal_connect(&self->machines->connections.signal_disconnected, self,
			paramrackpane_ondisconnected);
	}
}

void paramrackpane_onmachinesinsert(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	paramrackpane_insertbox(self, slot);
	psy_ui_component_align(psy_ui_component_parent(&self->component));
}

void paramrackpane_onmachinesremoved(ParamRackPane* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	paramrackpane_clear(self);
	paramrackpane_build(self);
}

void paramrackpane_onconnected(ParamRackPane* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	if (self->mode != PARAMRACK_ALL) {
		paramrackpane_build(self);
	}
}

void paramrackpane_ondisconnected(ParamRackPane* self,
	psy_audio_Connections* con, uintptr_t outputslot, uintptr_t inputslot)
{
	if (self->mode != PARAMRACK_ALL) {
		paramrackpane_build(self);
	}
}

// ParamRackBatchBar
void paramrackbatchbar_init(ParamRackBatchBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));	
	psy_ui_button_init_text(&self->select, &self->component, NULL, "Select in Gear");
}

// ParamRackModeBar
static void paramrackmodebar_ondestroy(ParamRackModeBar*);
static void paramrackmodebar_onmodeselect(ParamRackModeBar*, psy_ui_Button* sender);
// vtable
static psy_ui_ComponentVtable paramrackmodebar_vtable;
static bool paramrackmodebar_vtable_initialized = FALSE;

static void paramrackmodebar_vtable_init(ParamRackModeBar* self)
{
	if (!paramrackmodebar_vtable_initialized) {
		paramrackmodebar_vtable = *(self->component.vtable);
		paramrackmodebar_vtable.ondestroy =
			(psy_ui_fp_component_ondestroy)
			paramrackmodebar_ondestroy;
		paramrackmodebar_vtable_initialized = TRUE;
	}
	self->component.vtable = &paramrackmodebar_vtable;
}
// implementation
void paramrackmodebar_init(ParamRackModeBar* self, psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent, NULL);
	paramrackmodebar_vtable_init(self);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_signal_init(&self->signal_select);
	psy_ui_button_init_text_connect(&self->all, &self->component, NULL,
		"All", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->inputs, &self->component, NULL,
		"Inputs", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->outputs, &self->component, NULL,
		"Outputs", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->inchain, &self->component, NULL,
		"Inchain", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->outchain, &self->component, NULL,
		"Outchain", self, paramrackmodebar_onmodeselect);
	psy_ui_button_init_text_connect(&self->level, &self->component, NULL,
		"Level", self, paramrackmodebar_onmodeselect);
}

void paramrackmodebar_ondestroy(ParamRackModeBar* self)
{
	psy_signal_dispose(&self->signal_select);
}

void paramrackmodebar_onmodeselect(ParamRackModeBar* self, psy_ui_Button* sender)
{
	ParamRackMode mode;
		
	mode = PARAMRACK_NONE;
	if (sender == &self->all) {		
		mode = PARAMRACK_ALL;
	} else if (sender == &self->inputs) {
		mode = PARAMRACK_INPUTS;
	} else if (sender == &self->outputs) {
		mode = PARAMRACK_OUTPUTS;
	} else if (sender == &self->inchain) {
		mode = PARAMRACK_INCHAIN;
	} else if (sender == &self->outchain) {
		mode = PARAMRACK_OUTCHAIN;
	} else if (sender == &self->level) {
		mode = PARAMRACK_LEVEL;
	}
	paramrackmodebar_setmode(self, mode);
}

void paramrackmodebar_setmode(ParamRackModeBar* self, ParamRackMode mode)
{
	psy_ui_button_disablehighlight(&self->all);
	psy_ui_button_disablehighlight(&self->inputs);
	psy_ui_button_disablehighlight(&self->outputs);
	psy_ui_button_disablehighlight(&self->inchain);
	psy_ui_button_disablehighlight(&self->outchain);
	psy_ui_button_disablehighlight(&self->level);
	switch (mode) {
	case PARAMRACK_ALL:
		psy_ui_button_highlight(&self->all);
		break;
	case PARAMRACK_INPUTS:
		psy_ui_button_highlight(&self->inputs);
		break;
	case PARAMRACK_OUTPUTS:
		psy_ui_button_highlight(&self->outputs);
		break;
	case PARAMRACK_INCHAIN:
		psy_ui_button_highlight(&self->inchain);
		break;
	case PARAMRACK_OUTCHAIN:
		psy_ui_button_highlight(&self->outchain);
		break;
	case PARAMRACK_LEVEL:
		psy_ui_button_highlight(&self->level);
		break;
	default:
		break;
	}
	if (mode != PARAMRACK_NONE) {
		psy_signal_emit(&self->signal_select, self, 1, mode);
	}
}


// ParamRack
// prototypes
static void paramrack_onmodeselected(ParamRack*, ParamRackModeBar* sender, intptr_t index);
static void paramrack_onalign(ParamRack*, psy_ui_Component* sender);
static void paramrack_onlevelchanged(ParamRack*, IntEdit* sender);
static void paramrack_onsolo(ParamRack*, psy_ui_Button* sender);
static void paramrack_onmute(ParamRack*, psy_ui_Button* sender);
static void paramrack_onremove(ParamRack*, psy_ui_Button* sender);
static void paramrack_onreplace(ParamRack*, psy_ui_Button* sender);
static void paramrack_onselect(ParamRack*, psy_ui_Button* sender);
static void paramrack_onmachineselected(ParamRack*,
	psy_audio_Machines*, uintptr_t slot);
static void paramrack_onsongchanged(ParamRack*, Workspace* sender,
	int flag, psy_audio_Song*);
static void paramrack_connectsong(ParamRack*);

// implementation
void paramrack_init(ParamRack* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	// Bottom
	psy_ui_component_init_align(&self->bottom, &self->component,
		psy_ui_ALIGN_BOTTOM);	
	// IntEdit
	intedit_init(&self->leveledit, &self->bottom, "", 2, 0, INT32_MAX);
	psy_ui_component_setalign(&self->leveledit.component, psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->leveledit.signal_changed, self, paramrack_onlevelchanged);
	// ChainMode
	paramrackmodebar_init(&self->modebar, &self->bottom);		
	psy_ui_component_setalign(&self->modebar.component,
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->modebar.signal_select, self,
		paramrack_onmodeselected);
	// BatchBar
	paramrackbatchbar_init(&self->batchbar, &self->bottom);
	psy_ui_component_setalign(&self->batchbar.component, psy_ui_ALIGN_RIGHT);
	//psy_signal_connect(&self->batchbar.solo.signal_clicked, self,
		//paramrack_onsolo);
	//psy_signal_connect(&self->batchbar.mute.signal_clicked, self,
		//paramrack_onmute);
	//psy_signal_connect(&self->batchbar.remove.signal_clicked, self,
		//paramrack_onremove);
	//psy_signal_connect(&self->batchbar.replace.signal_clicked, self,
		//paramrack_onreplace);
	psy_signal_connect(&self->batchbar.select.signal_clicked, self,
		paramrack_onselect);
	// Pane
	paramrackpane_init(&self->pane, &self->component, workspace);
	psy_ui_component_setalign(&self->pane.component,
		psy_ui_ALIGN_VCLIENT);
	psy_ui_component_setoverflow(&self->pane.component,
		psy_ui_OVERFLOW_HSCROLL);
	psy_ui_component_setmaximumsize(&self->component,
		psy_ui_size_make_em(0.0, 10.0));
	psy_ui_component_setminimumsize(&self->component,
		psy_ui_size_make_em(0.0, 10.0));	
	// connect scrollbar
	psy_ui_scroller_init(&self->scroller, &self->pane.component,
		&self->component, NULL);
	psy_ui_component_setalign(&self->scroller.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setbackgroundmode(&self->scroller.pane, psy_ui_SETBACKGROUND);
	psy_signal_connect(&self->component.signal_align, self,
		paramrack_onalign);		
	psy_signal_connect(&workspace->signal_songchanged, self,
		paramrack_onsongchanged);
	paramrack_connectsong(self);
	paramrackmodebar_setmode(&self->modebar, self->pane.mode);
}

void paramrack_onmodeselected(ParamRack* self, ParamRackModeBar* sender,
	intptr_t index)
{		
	paramrackpane_setmode(&self->pane, (ParamRackMode)index);
}

void paramrack_onalign(ParamRack* self, psy_ui_Component* sender)
{
	if (psy_ui_component_at(&self->pane.component, 0)) {		
		psy_ui_Size limit;
		psy_ui_Size preferredboxsize;

		// update scroll step
		limit = psy_ui_component_offsetsize(&self->component);
		preferredboxsize = psy_ui_component_preferredsize(
			psy_ui_component_at(&self->pane.component, 0), &limit);
		psy_ui_component_setscrollstep_width(&self->pane.component,
			preferredboxsize.width);
	}
	psy_ui_component_updateoverflow(&self->pane.component);	
}

void paramrack_onlevelchanged(ParamRack* self, IntEdit* sender)
{
	if (self->pane.mode == PARAMRACK_LEVEL) {
		self->pane.level = intedit_value(&self->leveledit);
		paramrackpane_build(&self->pane);
	}
}

void paramrack_onsolo(ParamRack* self, psy_ui_Button* sender)
{
	
}

void paramrack_onmute(ParamRack* self, psy_ui_Button* sender)
{
	if (self->pane.machines) {
		psy_TableIterator it;

		for (it = psy_table_begin(&self->pane.boxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			ParamRackBox* box;
			psy_audio_Machine* machine;

			box = (ParamRackBox*)psy_tableiterator_value(&it);
			machine = psy_audio_machines_at(self->pane.machines, box->slot);
			if (machine) {
				psy_audio_machine_mute(machine);
				++self->pane.machines->opcount;
			}
		}
	}
}

void paramrack_onremove(ParamRack* self, psy_ui_Button* sender)
{
	if (self->pane.machines) {
		psy_TableIterator it;
		psy_List* removelist;
		psy_List* p;

		removelist = NULL;
		// first copy machines to a temp list, box table iterator becomes
		// invalid at machine remove		
		for (it = psy_table_begin(&self->pane.boxes);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			ParamRackBox* box;

			box = (ParamRackBox*)psy_tableiterator_value(&it);
			psy_list_append(&removelist, (void*)box->slot);
		}
		// remove machines
		psy_audio_exclusivelock_enter();
		for (p = removelist; p != NULL; psy_list_next(&p)) {
			uintptr_t slot;

			slot = (uintptr_t)psy_list_entry(p);
			psy_audio_machines_remove(self->pane.machines, slot, TRUE);
		}
		psy_audio_exclusivelock_leave();
		// clean up		
		psy_list_free(removelist);
	}
}

void paramrack_onreplace(ParamRack* self, psy_ui_Button* sender)
{
	if (self->pane.machines) {
		psy_TableIterator it;
		psy_List* removelist;
		psy_List* p;

		removelist = NULL;
		// first copy machines to a temp list, box table iterator becomes
		// invalid at machine remove
		for (it = psy_table_begin(&self->pane.boxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			ParamRackBox* box;

			box = (ParamRackBox*)psy_tableiterator_value(&it);
			psy_list_append(&removelist, (void*)box->slot);
		}
		psy_audio_exclusivelock_enter();
		// remove machines
		for (p = removelist; p != NULL; psy_list_next(&p)) {
			uintptr_t slot;

			slot = (uintptr_t)psy_list_entry(p);
			psy_audio_machines_remove(self->pane.machines, slot, FALSE);
		}
		// create dummy
		for (p = removelist; p != NULL; psy_list_next(&p)) {
			uintptr_t slot;

			slot = (uintptr_t)psy_list_entry(p);
			psy_audio_machines_insert(self->pane.machines, slot,
				psy_audio_machinefactory_makemachine(
					&self->workspace->machinefactory,
					psy_audio_DUMMY, NULL, 0));
		}
		psy_audio_exclusivelock_leave();
		// clean up
		psy_list_free(removelist);
	}
}

void paramrack_onselect(ParamRack* self, psy_ui_Button* sender)
{
	psy_TableIterator it;
	psy_List* slotlist;	

	if (!workspace_gearvisible(self->workspace)) {
		workspace_togglegear(self->workspace);
	}
	slotlist = NULL;		
	for (it = psy_table_begin(&self->pane.boxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		ParamRackBox* box;

		box = (ParamRackBox*)psy_tableiterator_value(&it);
		psy_list_append(&slotlist, (void*)box->slot);
	}
	workspace_multiselectgear(self->workspace, slotlist);
	psy_list_free(slotlist);	
}

void paramrack_onsongchanged(ParamRack* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{
	if (workspace_song(sender)) {
		paramrack_connectsong(self);
	}
}

void paramrack_connectsong(ParamRack* self)
{
	if (workspace_song(self->workspace)) {	
		psy_signal_connect(&self->workspace->song->machines.signal_slotchange, self,
			paramrack_onmachineselected);
	}
}

void paramrack_onmachineselected(ParamRack* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (self->pane.lastselected == slot) {
		return;
	}
	if (self->pane.mode == PARAMRACK_LEVEL) {
		uintptr_t level;

		level = psy_audio_machines_levelofmachine(self->pane.machines, slot);
		if (level != psy_INDEX_INVALID) {
			paramrackpane_clear(&self->pane);
			paramrackpane_build(&self->pane);
			intedit_setvalue(&self->leveledit, (int32_t)level);
		}
	} else if (self->pane.mode == PARAMRACK_ALL) {
		if (psy_table_exists(&self->pane.boxes, self->pane.lastselected)) {
			paramrackbox_deselect((ParamRackBox*)
				psy_table_at(&self->pane.boxes, self->pane.lastselected));
		}
		if (psy_table_exists(&self->pane.boxes, slot)) {
			paramrackbox_select((ParamRackBox*)
				psy_table_at(&self->pane.boxes, slot));
			self->pane.lastselected = slot;
		} else {
			self->pane.lastselected = psy_INDEX_INVALID;
		}
	} else {
		self->pane.lastselected = slot;
		paramrackpane_clear(&self->pane);
		paramrackpane_build(&self->pane);
	}
}
