// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "master.h"
#include "machines.h"
#include <string.h>
#include <math.h>
#include "songio.h"
#include <portable.h>

static int master_mode(Master* self) { return MACHMODE_MASTER; }
static void master_dispose(Master*);

static int parametertype(Master*, int param);
static unsigned int numparameters(Master*);
static unsigned int numparametercols(Master*);
static void parametertweak(Master*, int par, int val);	
static void parameterrange(Master*, int numparam, int* minval, int* maxval);
static int parameterlabel(Master*, char* txt, int param);
static int parametername(Master*, char* txt, int param);
static int describevalue(Master*, char* txt, int param, int value);
static int parametervalue(Master*, int param);
static const MachineInfo* info(Master*);
static unsigned int numinputs(Master*);
static unsigned int numoutputs(Master*);
static int intparamvalue(float value);
static float floatparamvalue(int value);
static void master_loadspecific(Master*, struct SongFile*, unsigned int slot);
static void master_savespecific(Master*, struct SongFile*, unsigned int slot);

static MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
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

const MachineInfo* master_info(void) { return &MacInfo; }

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Master* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.mode = (fp_machine_mode) master_mode;
		vtable.info = (fp_machine_info) info;
		vtable.dispose = (fp_machine_dispose) master_dispose;
		vtable.info = (fp_machine_info) info;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) master_loadspecific;
		vtable.savespecific = (fp_machine_savespecific) master_savespecific;
		// Parameter
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;	
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable_initialized = 1;
	}
}

void master_init(Master* self, MachineCallback callback)
{
	memset(self, 0, sizeof(Master));
	machine_init(&self->machine, callback);	
	vtable_init(self);
	self->machine.vtable = &vtable;
}

void master_dispose(Master* self)
{		
	machine_dispose(&self->machine);
}

void parametertweak(Master* self, int param, int value)
{
	if (param == 0) {
		Machines* machines = self->machine.vtable->machines(self);
		if (machines) {			
			machines_setvolume(machines,
				floatparamvalue(value) * floatparamvalue(value) * 4.f);
		}
	} else {
		MachineSockets* sockets;
		WireSocket* p;
		int c = 1;
		Machines* machines = self->machine.vtable->machines(self);
		
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != 0 && c != param; p = p->next, ++c);
			if (p) {
				WireSocketEntry* input_entry;

				input_entry = (WireSocketEntry*) p->entry;
				input_entry->volume =
					floatparamvalue(value) * floatparamvalue(value) * 4.f;					
			}
		}		
	}
}

int describevalue(Master* self, char* txt, int param, int value)
{ 	
	if (param == 0) {
		Machines* machines = self->machine.callback.machines(
			self->machine.callback.context);

		amp_t db = (amp_t)(20 * log10(machines_volume(machines)));
		psy_snprintf(txt, 10, "%.2f dB", db);
		return 1;
	} else {
		MachineSockets* sockets;
		WireSocket* p;
		int c = 1;
		Machines* machines = self->machine.vtable->machines(self);
		
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != 0 && c != param; p = p->next, ++c);
			if (p) {				
				WireSocketEntry* input_entry;
				amp_t db;

				input_entry = (WireSocketEntry*) p->entry;
				db = (amp_t)(20 * log10(input_entry->volume));
				psy_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}			
		}
	}
	return 0;
}

int parametervalue(Master* self, int param)
{	
	if (param == 0) {
		Machines* machines = self->machine.callback.machines(
			self->machine.callback.context);

		if (machines) {
			return intparamvalue(
				(float)sqrt(machines_volume(machines)) * 0.5f);
		}
	} else {
		MachineSockets* sockets;
		WireSocket* input_socket;
		int c = 1;
		Machines* machines = self->machine.vtable->machines(self);
		
		sockets = connections_at(&machines->connections, MASTER_INDEX);
		if (sockets) {
			for (input_socket = sockets->inputs; input_socket != 0 && c != param;
					input_socket = input_socket->next, ++c);
			if (input_socket) {
				WireSocketEntry* input_entry;

				input_entry = (WireSocketEntry*) input_socket->entry;
				return intparamvalue(
					(float)sqrt(input_entry->volume) * 0.5f);
			}
		}
	}
	return 0;
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
}

const MachineInfo* info(Master* self)
{	
	return &MacInfo;
}

unsigned int numinputs(Master* self)
{
	return 2;
}

unsigned int numoutputs(Master* self)
{
	return 2;
}

int parametertype(Master* self, int par)
{
	return MPF_STATE;
}

void parameterrange(Master* self, int numparam, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 65535;
}

unsigned int numparameters(Master* self)
{
	return 13;
}

unsigned int numparametercols(Master* self)
{
	return 4;
}

int parameterlabel(Master* self, char* txt, int param)
{
	psy_snprintf(txt, 128, "%s", "Vol");
	return 1;
}

int parametername(Master* self, char* txt, int param)
{
	psy_snprintf(txt, 128, "%s", "Vol");
	return 1;
}

void master_loadspecific(Master* self, struct SongFile* songfile, unsigned int slot)
{	
	unsigned int size;
	int outdry = 256;
	unsigned char decreaseOnClip = 0;

	psyfile_read(songfile->file, &size, sizeof size ); // size of this part params to load
	psyfile_read(songfile->file, &outdry, sizeof outdry);
	psyfile_read(songfile->file, &decreaseOnClip, sizeof decreaseOnClip);

	machines_setvolume(&songfile->song->machines, outdry / (amp_t)256);
}

void master_savespecific(Master* self, struct SongFile* songfile, unsigned int slot)
{
	unsigned int size;
	int outdry = 256;
	unsigned char decreaseOnClip = 0;
				
	size = sizeof outdry + sizeof decreaseOnClip;
	psyfile_write(songfile->file, &size, sizeof size); // size of this part params to save
	psyfile_write(songfile->file, &outdry, sizeof outdry);
	psyfile_write(songfile->file, &decreaseOnClip, sizeof decreaseOnClip); 
}