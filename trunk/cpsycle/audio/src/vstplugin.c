// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "vstplugin.h"
#if defined(__GNUC__)
#define _inline inline
#endif
#include "aeffectx.h"
#include <stdlib.h>
#if defined DIVERSALIS__OS__MICROSOFT
#include <excpt.h>
#endif
#include <operations.h>
#include "pattern.h"
#include "songio.h"

static const VstInt32 kBlockSize = 512;
static const VstInt32 kNumProcessCycles = 5;

// vtable prototypes
static int mode(psy_audio_VstPlugin*);
static void work(psy_audio_VstPlugin* self, psy_audio_BufferContext*);
static const psy_audio_MachineInfo* info(psy_audio_VstPlugin*);
static void parametertweak(psy_audio_VstPlugin* self, int par, int val);
static int parameterlabel(psy_audio_VstPlugin*, char* txt, int param);
static int parametername(psy_audio_VstPlugin*, char* txt, int param);
static int describevalue(psy_audio_VstPlugin*, char* txt, int param, int value);
static int parametervalue(psy_audio_VstPlugin*, int param);
static int parametertype(psy_audio_VstPlugin*, int param);
static void parameterrange(psy_audio_VstPlugin*, int param, int* minval, int* maxval);
static unsigned int numparameters(psy_audio_VstPlugin*);
static unsigned int numparametercols(psy_audio_VstPlugin*);
static void dispose(psy_audio_VstPlugin* self);
static uintptr_t numinputs(psy_audio_VstPlugin*);
static uintptr_t numoutputs(psy_audio_VstPlugin*);
static void loadspecific(psy_audio_VstPlugin*, struct psy_audio_SongFile*, unsigned int slot);
static void savespecific(psy_audio_VstPlugin*, struct psy_audio_SongFile*, unsigned int slot);
static int haseditor(psy_audio_VstPlugin*);
static void seteditorhandle(psy_audio_VstPlugin*, void* handle);
static void editorsize(psy_audio_VstPlugin*, int* width, int* height);
static void editoridle(psy_audio_VstPlugin*);
// private
static void checkEffectProperties (AEffect* effect);
static void checkEffectProcessing (AEffect* effect);
static int machineinfo(AEffect* effect, psy_audio_MachineInfo* info, const char* path, int shellidx);
typedef AEffect* (*PluginEntryProc)(audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK hostcallback(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
static PluginEntryProc getmainentry(Library* library);
static void processevents(psy_audio_VstPlugin*, psy_audio_BufferContext*);
struct VstMidiEvent* allocinitmidievent(psy_audio_VstPlugin*, const psy_audio_PatternEntry*);

struct VstMidiEvent* allocnoteon(psy_audio_VstPlugin*, const psy_audio_PatternEntry*);
struct VstMidiEvent* allocnoteoff(psy_audio_VstPlugin*, int note, int channel);
static void vstplugin_onfileselect(psy_audio_VstPlugin*, struct VstFileSelect*);

// init vstplugin class vtable
static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_VstPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.mode = (fp_machine_mode) mode;
		vtable.work = (fp_machine_work) work;
		vtable.info = (fp_machine_info) info;
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.describevalue = (fp_machine_describevalue) describevalue;	
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;	
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.haseditor = (fp_machine_haseditor) haseditor;
		vtable.seteditorhandle = (fp_machine_seteditorhandle) seteditorhandle;
		vtable.editorsize = (fp_machine_editorsize) editorsize;
		vtable.editoridle = (fp_machine_editoridle) editoridle;
		vtable_initialized = 1;
	}
}

void vstplugin_init(psy_audio_VstPlugin* self, MachineCallback callback, const char* path)
{	
	psy_audio_Machine* base = &self->custommachine.machine;
	PluginEntryProc mainproc;
	
	custommachine_init(&self->custommachine, callback);	
	vtable_init(self);
	base->vtable->setcallback(&self->custommachine.machine, callback);
	self->custommachine.machine.vtable = &vtable;
	self->info = 0;
	self->editorhandle = 0;
	self->events = 0;
	self->plugininfo = 0;
	psy_table_init(&self->tracknote);
	library_init(&self->library);
	library_load(&self->library, path);		
	mainproc = getmainentry(&self->library);
	if (mainproc) {
		self->effect = mainproc(hostcallback);
		if (self->effect) {			
			VstInt32 numInputs;
			VstInt32 numOutputs;
			unsigned int size;

			self->effect->user = self;
			self->eventcap = 1024;
			size = sizeof(struct VstEvents) + 
				sizeof(VstEvent*) * self->eventcap;
			self->events = (struct VstEvents*) malloc(size);			
			numInputs = self->effect->numInputs;
			numOutputs = self->effect->numOutputs;			
			self->effect->user = self;
			self->effect->dispatcher (self->effect, effOpen, 0, 0, 0, 0);
			self->effect->dispatcher (self->effect, effSetSampleRate, 0, 0, 0,
				(float)base->vtable->samplerate(base));
			self->effect->dispatcher (self->effect, effSetProcessPrecision, 0, kVstProcessPrecision32, 0, 0);
			self->effect->dispatcher (self->effect, effSetBlockSize, 0, kBlockSize, 0, 0);
			self->effect->dispatcher (self->effect, effMainsChanged, 0, 1, 0, 0);
			self->effect->dispatcher (self->effect, effStartProcess, 0, 0, 0, 0);
			self->plugininfo = machineinfo_allocinit();
			machineinfo(self->effect, self->plugininfo, self->library.path, 0);
			base->vtable->seteditname(base, self->plugininfo->ShortName);
		}
	}
	if (!base->vtable->editname(base)) {
		base->vtable->seteditname(base, "VstPlugin");
	}
} 

void dispose(psy_audio_VstPlugin* self)
{		
	if (self->library.module) {
		if (self->effect) {
			self->effect->dispatcher (self->effect, effClose, 0, 0, 0, 0);
			self->effect = 0;			
		}
		library_dispose(&self->library);		
		if (self->info) {
			free((char*)self->info->Author);
			free((char*)self->info->Name);
			free((char*)self->info->ShortName);
			free((char*)self->info->Command);
		}
		self->info = 0;
		free(self->events);		
	}	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}
	psy_table_dispose(&self->tracknote);
	custommachine_dispose(&self->custommachine);
}

PluginEntryProc getmainentry(Library* library)
{
	PluginEntryProc rv = 0;

	rv = (PluginEntryProc)library_functionpointer(library, "VSTPluginMain");
	if(!rv) {
		rv = (PluginEntryProc)library_functionpointer(library,"main");
	}
	return rv;
}

int plugin_vst_test(const char* path, psy_audio_MachineInfo* rv)
{
	int vst = 0;
	
	if (path && strcmp(path, "") != 0) {
		Library library;
		PluginEntryProc mainentry;	
		
		library_init(&library);		
		library_load(&library, path);
		if (!library_empty(&library)) {
			mainentry = getmainentry(&library);
			if (mainentry) {
				AEffect* effect;
				
				effect = mainentry(hostcallback);
				vst = effect && machineinfo(effect, rv, path, 0) == 0;
			}	
		}
		library_dispose(&library);	
	}
	return vst;
}

void work(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{
	if (!self->custommachine.machine.vtable->bypassed(
			&self->custommachine.machine)) {
		if (!self->custommachine.machine.vtable->bypassed(
				&self->custommachine.machine)) {
			uintptr_t c;

			for (c = 0; c < bc->output->numchannels; ++c) {
				dsp.mul(bc->output->samples[c], bc->numsamples, 1/32768.f);
			}
		}
		processevents(self, bc);
		if (!self->custommachine.machine.vtable->bypassed(
				&self->custommachine.machine)) {
			uintptr_t c;

			self->effect->processReplacing(self->effect, bc->output->samples,
				bc->output->samples, bc->numsamples);
			for (c = 0; c < bc->output->numchannels; ++c) {
				dsp.mul(bc->output->samples[c], bc->numsamples, 32768.f);
			}
		}
	}
	if (bc->output) {
		psy_audio_Buffer* memory;
		psy_audio_Machine* base;

		base = &self->custommachine.machine;		
		memory = base->vtable->buffermemory(base);
		if (memory) {			
			buffer_insertsamples(memory, bc->output, 
				base->vtable->buffermemorysize(base),
				bc->numsamples);
		}
	}
}

void processevents(psy_audio_VstPlugin* self, psy_audio_BufferContext* bc)
{
	psy_audio_Machine* base = &self->custommachine.machine;
	psy_List* p;
	int count = 0;
	int i;
	
	for (p = bc->events; p != 0 && count < self->eventcap;
			p = p->next) {
		psy_audio_PatternEntry* entry = (psy_audio_PatternEntry*)p->entry;		
		if (entry->event.cmd == SET_PANNING) {
			// todo split work
			base->vtable->setpanning(base, entry->event.parameter / 255.f);
		} else
		if (entry->event.note == NOTECOMMANDS_TWEAK) {
			// todo translate to midi events 
//			self->parametertweak(self, entry->event.inst,
//				entry->event.parameter);
		} else 
		if (entry->event.note < NOTECOMMANDS_RELEASE) {
			VstNote* note = 0;

			if (psy_table_exists(&self->tracknote, entry->track)) {
				note = (VstNote*) psy_table_at(&self->tracknote, entry->track);
				self->events->events[count] = (VstEvent*)
					allocnoteoff(self, note->key, entry->track);
				++count;
			}
			self->events->events[count] = (VstEvent*)
				allocnoteon(self, entry);
			if (!note) {
				note = malloc(sizeof(VstNote));			
				psy_table_insert(&self->tracknote, entry->track, (void*) note);
			}
			note->key = entry->event.note;
			note->midichan = 0;
			++count;			
		} else
		if (entry->event.note == NOTECOMMANDS_RELEASE) {
			if (psy_table_exists(&self->tracknote, entry->track)) {
				VstNote* note;
				
				note = psy_table_at(&self->tracknote, entry->track);
				self->events->events[count] = (VstEvent*)
					allocnoteoff(self, note->key, entry->track);
				++count;
				psy_table_remove(&self->tracknote, entry->track);				
			}
		}
	}	
	self->events->numEvents = count;
	self->events->reserved = 0;
	self->effect->dispatcher(self->effect, effProcessEvents, 0, 0, self->events, 0);
	for (i = 0; i < count; ++i) {		
		free(self->events->events[i]);
	}
}

struct VstMidiEvent* allocnoteon(psy_audio_VstPlugin* self, const psy_audio_PatternEntry* entry)
{
	struct VstMidiEvent* rv;	

	rv = malloc(sizeof(struct VstMidiEvent));
	if (rv) {
		char note;

		memset(rv, 0, sizeof(struct VstMidiEvent));
		note = (char) entry->event.note;
		rv->type = kVstMidiType;
		rv->byteSize = sizeof(struct VstMidiEvent);
		rv->flags = kVstMidiEventIsRealtime;
		rv->midiData[0] = (char)(entry->track + 0x90);
		rv->midiData[1] = (char)note;
		rv->midiData[2] = (char)(127);
	}
	return rv;
}

struct VstMidiEvent* allocnoteoff(psy_audio_VstPlugin* self, int note, int channel)
{
	struct VstMidiEvent* rv;	

	rv = malloc(sizeof(struct VstMidiEvent));
	if (rv) {
		memset(rv, 0, sizeof(struct VstMidiEvent));
		rv->type = kVstMidiType;
		rv->byteSize = sizeof(struct VstMidiEvent);
		rv->flags = kVstMidiEventIsRealtime;
		rv->midiData[0] = (char)(channel | 0x80);
		rv->midiData[1] = (char) note;
		rv->midiData[2] = (char) 0;
	}
	return rv;
}

#if defined DIVERSALIS__OS__MICROSOFT
static int FilterException(int code, struct _EXCEPTION_POINTERS *ep) 
{
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int machineinfo(AEffect* effect, psy_audio_MachineInfo* info, const char* path,
	int shellidx)
{
	char effectName[256] = {0};
	char vendorString[256] = {0};
	char productString[256] = {0};
	int err = 0;

#if defined DIVERSALIS__OS__MICROSOFT    
	__try
#endif   
    {
		effect->dispatcher (effect, effGetEffectName, 0, 0, effectName, 0);
		effect->dispatcher (effect, effGetVendorString, 0, 0, vendorString, 0);
		effect->dispatcher (effect, effGetProductString, 0, 0, productString, 0);
		
		machineinfo_set(info,
			vendorString,
			"",
			(effect->flags & effFlagsIsSynth) == effFlagsIsSynth,
			effFlagsIsSynth ? MACHMODE_FX : MACHMODE_GENERATOR,
			effectName,
			effectName,
			0,
			0,
			MACH_VST,
			path,
			shellidx);		
	}
#if defined DIVERSALIS__OS__MICROSOFT        	
	__except(FilterException(GetExceptionCode(), GetExceptionInformation())) {		
		err = GetExceptionCode();
	}
#endif	
	return err;
}

const psy_audio_MachineInfo* info(psy_audio_VstPlugin* self)
{	
	return self->plugininfo;
}

void parametertweak(psy_audio_VstPlugin* self, int par, int val)
{
	self->effect->setParameter(self->effect, par, val / 65535.f);
}

int parameterlabel(psy_audio_VstPlugin* self, char* txt, int param)
{
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamLabel, param, 0, txt, 0);
	return *txt != '\0';
}

int parametername(psy_audio_VstPlugin* self, char* txt, int param)
{
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamName, param, 0, txt, 0);
	return *txt != '\0';
}

int describevalue(psy_audio_VstPlugin* self, char* txt, int param, int value)
{ 		
	txt[0] = '\0';
	self->effect->dispatcher(self->effect, effGetParamDisplay, param, 0, txt, 0);
	return *txt != '\0';
}

int parametervalue(psy_audio_VstPlugin* self, int param)
{
	return (int)(self->effect->getParameter(self->effect, param) * 65535.f);
}

uintptr_t numinputs(psy_audio_VstPlugin* self)
{
	return (uintptr_t) self->effect->numInputs;
}

uintptr_t numoutputs(psy_audio_VstPlugin* self)
{
	return (uintptr_t) self->effect->numOutputs;
}

unsigned int numparameters(psy_audio_VstPlugin* self)
{
	return self->effect->numParams;	
}

unsigned int numparametercols(psy_audio_VstPlugin* self)
{
	return 6;
}

int parametertype(psy_audio_VstPlugin* self, int param)
{
	return MPF_STATE;
}

void parameterrange(psy_audio_VstPlugin* self, int param, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 65535;
}

void loadspecific(psy_audio_VstPlugin* self, struct psy_audio_SongFile* songfile, unsigned int slot)
{
	uint32_t size;
	unsigned char program;

	psyfile_read(songfile->file, &size, sizeof(size));
	if(size) {
		uint32_t count;

		psyfile_read(songfile->file, &program, sizeof program);
		psyfile_read(songfile->file, &count, sizeof count);
		size -= sizeof(program) + sizeof(count) + sizeof(float) * count;
		if(!size) {
			if (program < self->effect->numPrograms) {
				uint32_t i;				

				self->effect->dispatcher(self->effect,
					effBeginSetProgram, 0, 0, 0, 0);
				self->effect->dispatcher(self->effect,
					effSetProgram, 0, (VstIntPtr) program, 0, 0);
				for(i = 0; i < count; ++i) {
					float temp;
				
					psyfile_read(songfile->file, &temp, sizeof(temp));
					self->effect->setParameter(self->effect, (VstInt32) i,
						temp);
				}
				self->effect->dispatcher(self->effect,
					effEndSetProgram, 0, 0, 0, 0);
			}
		} else {
			self->effect->dispatcher(self->effect,
				effBeginSetProgram, 0, 0, 0, 0);
			self->effect->dispatcher(self->effect,
					effSetProgram, 0, (VstIntPtr) program, 0, 0);
			self->effect->dispatcher(self->effect,
				effEndSetProgram, 0, 0, 0, 0);
			psyfile_skip(songfile->file, sizeof(float) * count);
			if(self->effect->flags & effFlagsProgramChunks) {
				char * data;
				
				data = (char*) malloc(size);
				psyfile_read(songfile->file, data, size); // Number of parameters
				self->effect->dispatcher(self->effect,
					effSetChunk, 0, size, data, 0);					
				free(data);
				data = 0;					
			} else {
				// there is a data chunk, but this machine does not want one.
				psyfile_skip(songfile->file, size);
				return;
			}
		}	
	}
}

void savespecific(psy_audio_VstPlugin* self, struct psy_audio_SongFile* songfile, unsigned int slot)
{	
	uint32_t count;
	unsigned char program=0;
	uint32_t size;
	uint32_t chunksize = 0;
	char * data = 0;

	count = self->effect->numParams;
	size = sizeof(program) + sizeof(count);
	if(self->effect->flags & effFlagsProgramChunks) {
		count = 0;
		chunksize = self->effect->dispatcher(self->effect,
				effGetChunk, 0, 0, &data, 0);				
		size += chunksize;
	} else {
		size += (sizeof(float) * count);
	}
	psyfile_write(songfile->file, &size, sizeof(size));
	program = (unsigned char) self->effect->dispatcher(self->effect,
		effGetProgram, 0, 0, 0, 0);	
	psyfile_write(songfile->file, &program, sizeof(program));
	psyfile_write(songfile->file, &count, sizeof count);

	if(self->effect->flags & effFlagsProgramChunks) {
		psyfile_write(songfile->file, data, chunksize);
	} else {
		uint32_t i;

		for (i = 0; i < count; ++i) {
			float temp;
			
			temp = self->effect->getParameter(self->effect, (VstInt32) i);								
			psyfile_write(songfile->file, &temp, sizeof(temp));
		}
	}
}

int haseditor(psy_audio_VstPlugin* self)
{
	return (self->effect->flags & effFlagsHasEditor) == effFlagsHasEditor;
}

void seteditorhandle(psy_audio_VstPlugin* self, void* handle)
{		
	self->editorhandle = handle;
	self->effect->dispatcher(self->effect, effEditOpen, 0, 0, handle, 0);	
}

void editorsize(psy_audio_VstPlugin* self, int* width, int* height)
{
	struct ERect* r = 0;

	self->effect->dispatcher(self->effect, effEditGetRect, 0, 0,  &r, 0);
	if (r != 0) {
		*width = r->right - r->left;
		*height = r->bottom - r->top;
	} else {
		*width = 0;
		*height = 0;
	}
}

void editoridle(psy_audio_VstPlugin* self)
{
	if(self->editorhandle) {
		self->effect->dispatcher(self->effect, effEditIdle, 0, 0, 0, 0);
	}
}

int mode(psy_audio_VstPlugin* self)
{ 
	return (self->effect->flags & effFlagsIsSynth) == effFlagsIsSynth
		? MACHMODE_GENERATOR
		: MACHMODE_FX;
}

VstIntPtr VSTCALLBACK hostcallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
	VstIntPtr result = 0;
	psy_audio_VstPlugin* self;
	psy_audio_Machine* base;
	
	if (effect) {
		self = (psy_audio_VstPlugin*) effect->user;
	} else
	if (opcode == audioMasterVersion) {
		return kVstVersion;
	} else {
		return 0;
	}
	base = &self->custommachine.machine;
	
	switch(opcode)
	{
		case audioMasterVersion :
			result = kVstVersion;
		break;		
		case audioMasterIdle:            
        break;
		case audioMasterGetSampleRate:
			result = base->vtable->samplerate(base);
		break;
		case audioMasterOpenFileSelector :
			vstplugin_onfileselect(self, (struct VstFileSelect*) ptr);
		break;		
		default:
		break;
	}

	return result;
}

void vstplugin_onfileselect(psy_audio_VstPlugin* self,
	struct VstFileSelect* select)
{
	switch (select->command) {
		case kVstFileLoad:
			self->custommachine.machine.callback.fileselect_load(
				self->custommachine.machine.callback.context);
		break;
		case kVstFileSave:
			self->custommachine.machine.callback.fileselect_save(
				self->custommachine.machine.callback.context);
		break;
		case kVstDirectorySelect:
			self->custommachine.machine.callback.fileselect_directory(
				self->custommachine.machine.callback.context);
		break;
		default:
		break;
	}
}

