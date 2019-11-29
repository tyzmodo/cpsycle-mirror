// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../eventdriver.h"
#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <stdio.h>

#define DEVICE_NONE 0

typedef struct {
	EventDriver driver;	
	HMIDIIN hMidiIn;
	int deviceid;		
	int (*error)(int, const char*);
	HANDLE hEvent;
} MmeMidiDriver;

static void driver_free(EventDriver*);
static int driver_init(EventDriver*);
static void driver_connect(EventDriver*, void* context, EVENTDRIVERWORKFN callback, void* handle);
static int driver_open(EventDriver*);
static int driver_close(EventDriver*);
static int driver_dispose(EventDriver*);
static void updateconfiguration(EventDriver*);

static void init_properties(EventDriver* self);
static void apply_properties(MmeMidiDriver* self);

static CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

int onerror(int err, const char* msg)
{
	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT EventDriverInfo const * __cdecl GetPsycleEventDriverInfo(void)
{
	static EventDriverInfo info;
	info.Flags = 0;
	info.Name = "Windows MME Driver";
	info.ShortName = "MME";
	info.Version = 0;
	return &info;
}

EXPORT EventDriver* __cdecl eventdriver_create(void)
{
	MmeMidiDriver* mme = (MmeMidiDriver*) malloc(sizeof(MmeMidiDriver));
	memset(mme, 0, sizeof(MmeMidiDriver));
	mme->deviceid = DEVICE_NONE;
	mme->driver.open = driver_open;
	mme->driver.free = driver_free;
	mme->driver.connect = driver_connect;
	mme->driver.open = driver_open;
	mme->driver.close = driver_close;
	mme->driver.dispose = driver_dispose;
	mme->driver.updateconfiguration = updateconfiguration;
	mme->driver.error = onerror;
	mme->hEvent = CreateEvent
		(NULL, FALSE, FALSE, NULL);
	driver_init(&mme->driver);
	return &mme->driver;
}

void driver_free(EventDriver* driver)
{
	free(driver);
}

int driver_init(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;

	self->hMidiIn = 0;
	init_properties(&self->driver);
	return 0;
}

int driver_dispose(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;
	properties_free(self->driver.properties);
	self->driver.properties = 0;
	CloseHandle(self->hEvent);
	return 0;
}

void init_properties(EventDriver* self)
{		
	Properties* devices;
	int i;
	int n;	

	self->properties = properties_create();
		
	properties_append_string(self->properties, "name", "winmme midi");
	properties_append_string(self->properties, "version", "1.0");
	devices = properties_append_choice(self->properties, "device", 0);		 
	properties_append_int(devices, "0:None", 0, 0, 0);
	n = midiInGetNumDevs();	
	for (i = 0; i < n; i++)
	{
		char text[256];
		
		MIDIINCAPS caps;
		midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
#if defined _MSC_VER > 1200
		_snprintf_s(text, strlen(caps.szPname), 256, "%d:%s", i, caps.szPname);
#else
		_snprintf(text, 256, "%d:%s", i, caps.szPname);
#endif
		properties_append_int(devices, text, i, 0, 0);
	}
}

void apply_properties(MmeMidiDriver* self)
{	
	if (self->driver.properties) {
		Properties* p;

		p = properties_read(self->driver.properties, "device");
		if (p) {
			self->deviceid = properties_value(p);
		}
	}
}

void updateconfiguration(EventDriver* self)
{
	apply_properties((MmeMidiDriver*)self);
}

void driver_connect(EventDriver* driver, void* context, EVENTDRIVERWORKFN callback, void* handle)
{	
	driver->_callbackContext = context;
	driver->_pCallback = callback;	
}

int driver_open(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;	
	unsigned int success = 1;

	if (self->deviceid != 0) {
		if (midiInOpen (&self->hMidiIn, self->deviceid - 1, (DWORD_PTR)MidiCallback,
				(DWORD_PTR)driver, CALLBACK_FUNCTION)) {
			driver->error(0, "Cannot open MIDI device");
			success = 0;
		} else {		
			if (midiInStart(self->hMidiIn)) {
				driver->error(0, "Cannot start MIDI device");
				success = 0;
			}
		}
	}
	         
	return success;
}

int driver_close(EventDriver* driver)
{
	MmeMidiDriver* self = (MmeMidiDriver*) driver;
	unsigned int success = 1;


	if (self->deviceid != 0 && self->hMidiIn && midiInClose(self->hMidiIn)) {
		driver->error(0, "Cannot close MIDI device");
		success = 0;
	}

	return success;
}

CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{			
	EventDriver* driver = (EventDriver*)dwInstance;	

	switch(uMsg) {
		// normal data message
		case MIM_DATA: 			
			driver->_pCallback(driver->_callbackContext, 1,
				(unsigned char*)&dwParam1, 4);
		break;
		default:
		break;
	}
}