// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "master.h"
#include "song.h"
#include "songio.h"
#include "machines.h"
#include "plugin_interface.h"

#include <convert.h>

#include <string.h>
#include <math.h>

#include "../../detail/portable.h"

static void master_generateaudio(psy_audio_Master*, psy_audio_BufferContext*);
static void master_seqtick(psy_audio_Master*, uintptr_t channel,
	const psy_audio_PatternEvent* ev);
static int master_mode(psy_audio_Master* self) { return MACHMODE_MASTER; }
static void master_dispose(psy_audio_Master*);
static const psy_audio_MachineInfo* info(psy_audio_Master*);
static void master_loadspecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void master_savespecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static const char* master_editname(psy_audio_Master* self)
{
	return "Psycle Master and Minimixer";
}
static uintptr_t numinputs(psy_audio_Master* self) { return 2; }
static uintptr_t numoutputs(psy_audio_Master* self) { return 2; }
static uintptr_t slot(psy_audio_Master* self) { return MASTER_INDEX; }
// Parameters
static psy_audio_MachineParam* parameter(psy_audio_Master* self,
	uintptr_t param);
static psy_audio_MachineParam* tweakparameter(psy_audio_Master* self,
	uintptr_t param);
static uintptr_t numparameters(psy_audio_Master*);
static uintptr_t numtweakparameters(psy_audio_Master*);
static unsigned int numparametercols(psy_audio_Master*);
static void master_describeeditname(psy_audio_Master*, char* text, uintptr_t slot);

static void master_title_name(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, char* text);
static void master_title_describe(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, int* active, char* text);
static void master_slider_tweak(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, float value);
static void master_slider_normvalue(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, float* rv);
static void master_level_normvalue(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, float* rv);
static void master_level_describe(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, int* active, char* text);

static psy_dsp_amp_range_t amprange(psy_audio_Master* self)
{
	return PSY_DSP_AMP_RANGE_IGNORE;
}

static psy_audio_MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
	MACHMODE_FX,
	"Master"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Master",
	"Psycledelics",
	"help",
	MACH_MASTER,
	0,
	0
};

const psy_audio_MachineInfo* master_info(void) { return &MacInfo; }

#define NUMROWS 3

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Master* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.generateaudio = (fp_machine_generateaudio) master_generateaudio;
		vtable.seqtick = (fp_machine_seqtick) master_seqtick;
		vtable.mode = (fp_machine_mode) master_mode;
		vtable.info = (fp_machine_info) info;
		vtable.dispose = (fp_machine_dispose) master_dispose;
		vtable.info = (fp_machine_info) info;
		
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.slot = (fp_machine_slot) slot;
		vtable.loadspecific = (fp_machine_loadspecific) master_loadspecific;
		vtable.savespecific = (fp_machine_savespecific) master_savespecific;
		vtable.editname = (fp_machine_editname) master_editname;
		// Parameter
		vtable.parameter = (fp_machine_parameter) parameter;
		vtable.tweakparameter = (fp_machine_tweakparameter) tweakparameter;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.numtweakparameters = (fp_machine_numparameters)numtweakparameters;
		vtable.amprange = (fp_machine_amprange) amprange;
		vtable_initialized = 1;
	}
}

void master_init(psy_audio_Master* self, MachineCallback callback)
{
	memset(self, 0, sizeof(psy_audio_Master));
	machine_init(&self->machine, callback);	
	vtable_init(self);
	self->machine.vtable = &vtable;
	self->volume = (psy_dsp_amp_t) 1.f;
	psy_audio_custommachineparam_init(&self->param_info,
		"", "", MPF_INFOLABEL | MPF_SMALL, 0, 0x1FFE);
	psy_signal_connect(&self->param_info.machineparam.signal_name, self,
		master_title_name);
	psy_signal_connect(&self->param_info.machineparam.signal_describe, self,
		master_title_describe);
	psy_audio_custommachineparam_init(&self->param_slider,
		"Vol", "Vol", MPF_SLIDER | MPF_SMALL, 0, 0x1FE);
	psy_signal_connect(&self->param_slider.machineparam.signal_tweak, self,
		master_slider_tweak);
	psy_signal_connect(&self->param_slider.machineparam.signal_normvalue, self,
		master_slider_normvalue);
	psy_signal_connect(&self->param_slider.machineparam.signal_describe, self,
		master_level_describe);
	psy_audio_custommachineparam_init(&self->param_level,
		"", "", MPF_SLIDERLEVEL | MPF_SMALL, 0, 0xFFFF);
	psy_signal_connect(&self->param_level.machineparam.signal_normvalue, self,
		master_level_normvalue);
}

void master_dispose(psy_audio_Master* self)
{		
	machine_dispose(&self->machine);
	psy_audio_custommachineparam_dispose(&self->param_info);
	psy_audio_custommachineparam_dispose(&self->param_slider);
	psy_audio_custommachineparam_dispose(&self->param_level);
}

const psy_audio_MachineInfo* info(psy_audio_Master* self)
{
	return &MacInfo;
}

void master_generateaudio(psy_audio_Master* self, psy_audio_BufferContext* bc)
{
	psy_audio_buffer_mulsamples(bc->output, bc->numsamples, self->volume);
}

void master_seqtick(psy_audio_Master* self, uintptr_t channel,
	const psy_audio_PatternEvent* ev)
{
	if (ev->cmd == SET_VOLUME) {
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		if (machines) {
			if (ev->inst == NOTECOMMANDS_INST_EMPTY &&
				ev->vol == MASTER_INDEX) {
				psy_dsp_amp_t nv;

				nv = ev->parameter / (psy_dsp_amp_t)0x1FE;
				self->volume = nv * nv * 4.f;
			} else
				if (ev->inst != NOTECOMMANDS_INST_EMPTY) {
					psy_audio_MachineSockets* sockets;

					sockets = connections_at(&machines->connections, ev->vol);
					if (sockets) {
						WireSocket* p;
						uintptr_t c = 0;

						for (p = sockets->outputs; p != NULL && c != ev->inst;
							p = p->next, ++c);
						if (p) {
							psy_audio_WireSocketEntry* output_entry;
							psy_dsp_amp_t nv;

							output_entry = (psy_audio_WireSocketEntry*)p->entry;
							nv = ev->parameter / (psy_dsp_amp_t)0x1FE;
							connections_setwirevolume(&machines->connections, ev->vol,
								output_entry->slot, nv * nv * 4.f);
						}
					}
				}
		}
	}
}

void master_describeeditname(psy_audio_Master* self, char* text, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = machines_at(psy_audio_machine_machines(&self->machine), slot);
	psy_snprintf(text, 128, "%s", (machine != NULL) ?
		psy_audio_machine_editname(machine) : "");
}

uintptr_t numparameters(psy_audio_Master* self)
{	
	return numparametercols(self) * NUMROWS;
}

unsigned int numparametercols(psy_audio_Master* self)
{
	psy_audio_MachineSockets* sockets;
	WireSocket* input_socket;
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(&self->machine);
	sockets = connections_at(&machines->connections, MASTER_INDEX);
	if (sockets) {
		input_socket = sockets->inputs;
		if (input_socket) {
			return psy_list_size(input_socket) + 1;
		}
	}
	return 1;
}

void master_title_name(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, char* text)
{
	if (sender->index == 0) {
		psy_snprintf(text, 128, "%s", "Master");
	} else {
		psy_snprintf(text, 10, "m%d", (int)sender->index);
	}
}

void master_title_describe(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, int* active, char* text)
{
	*active = 0;
	if (sender->index == 0) {
		*active = 0;
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != NULL && c != sender->index;
				p = p->next, ++c);
			if (p) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)p->entry;
				master_describeeditname(self, text, input_entry->slot);
				*active = 1;
			}
		}
	}
}

void master_slider_tweak(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, float value)
{
	if (sender->index == 0) {
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		if (machines) {
			self->volume = value * value * 4.f;
		}
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != NULL && c != sender->index; p = p->next, ++c);
			if (p) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)p->entry;
				input_entry->volume = value * value * 4.f;
			}
		}
	}
}

void master_slider_normvalue(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = 0;
	if (sender->index == 0) {		
		*rv = (float)sqrt(self->volume) * 0.5f;		
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* input_socket;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (input_socket = sockets->inputs; input_socket != 0 && c != sender->index;
				input_socket = input_socket->next, ++c);
			if (input_socket) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)input_socket->entry;
				*rv = (float)sqrt(input_entry->volume) * 0.5f;
			}
		}
	}
}

void master_level_normvalue(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = 0;
	if (sender->index == 0) {
		psy_audio_Buffer* buffer;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		buffer = machines_outputs(machines, MASTER_INDEX);
		*rv = buffer->volumedisplay;
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* input_socket;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (input_socket = sockets->inputs; input_socket != 0 &&
				c != sender->index;
				input_socket = input_socket->next, ++c);
			if (input_socket) {
				psy_audio_WireSocketEntry* input_entry;
				psy_audio_Buffer* buffer;

				input_entry = (psy_audio_WireSocketEntry*)input_socket->entry;
				buffer = machines_outputs(machines, input_entry->slot);
				*rv = buffer->volumedisplay;
			}
		}
	}
}

void master_level_describe(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, int* active, char* text)
{
	if (sender->index == 0) {		
		psy_snprintf(text, 10, "%.2f dB",
			(float)psy_dsp_convert_amp_to_db(self->volume));
		*active = 1;
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		*active = 0;
		if (sockets) {
			for (p = sockets->inputs; p != NULL && c != sender->index;
				p = p->next, ++c);
			if (p) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)p->entry;				
				psy_snprintf(text, 10, "%.2f dB", 
					(float) psy_dsp_convert_amp_to_db(input_entry->volume));
				*active = 1;
			}
		}		
	}
}

psy_audio_MachineParam* parameter(psy_audio_Master* self, uintptr_t id)
{
	int col = id / NUMROWS;
	int row = id % NUMROWS;

	if (row == 0) {
		self->param_info.index = col;
		return &self->param_info.machineparam;
	} else
	if (row == 1) {
		self->param_slider.index = col;
		return &self->param_slider.machineparam;
	} else
	if (row == 2) {
		self->param_level.index = col;
		return &self->param_level.machineparam;
	}
	return NULL;	
}

psy_audio_MachineParam* tweakparameter(psy_audio_Master* self, uintptr_t id)
{
	if (id == 0) {
		self->param_slider.index = 0;
		return &self->param_slider.machineparam;
	}
	return NULL;
}

uintptr_t numtweakparameters(psy_audio_Master* self)
{
	return numparametercols(self);
}

void master_loadspecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t size;
	int32_t outdry = 256;
	unsigned char decreaseOnClip = 0;

	// size of this part params to load
	psyfile_read(songfile->file, &size, sizeof size );
	psyfile_read(songfile->file, &outdry, sizeof outdry);
	psyfile_read(songfile->file, &decreaseOnClip, sizeof decreaseOnClip);

	self->volume = outdry / (psy_dsp_amp_t) 256;
}

void master_savespecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int32_t outdry = 256;
	unsigned char decreaseOnClip = 0;
				
	size = sizeof outdry + sizeof decreaseOnClip;
	// size of this part params to save
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &outdry, sizeof(outdry));
	psyfile_write(songfile->file, &decreaseOnClip, sizeof(decreaseOnClip));
}
