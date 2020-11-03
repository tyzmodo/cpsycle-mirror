// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "../eventdriver.h"

#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <stdio.h>
#include <hashtbl.h>

#define DEVICE_NONE 0

#define INPUT_BUTTON_FIRST 0
#define INPUT_BUTTON_LAST 127
#define INPUT_MOVE_UP 128
#define INPUT_MOVE_DOWN 129
#define INPUT_MOVE_LEFT 130
#define INPUT_MOVE_RIGHT 131

typedef struct {
	psy_EventDriver driver;
	LPDIRECTINPUT8 di;
	LPDIRECTINPUTDEVICE8 joystick;
	int count;
	psy_Table inputs;	
	int deviceid;
	bool active;
	int (*error)(int, const char*);
	EventDriverData lastinput;	
	HANDLE hEvent;
	psy_Properties* devices;
	psy_Properties* cmddef;
	DIJOYSTATE2 state;
} DXJoystickDriver;

static void driver_free(psy_EventDriver*);
static int driver_init(psy_EventDriver*);
static int driver_open(psy_EventDriver*);
static int driver_close(psy_EventDriver*);
static int driver_dispose(psy_EventDriver*);
static void driver_configure(psy_EventDriver*, psy_Properties*);
static void driver_cmd(psy_EventDriver*, const char* section, EventDriverData input, EventDriverCmd*);
static EventDriverCmd driver_getcmd(psy_EventDriver*, const char* section);
static void driver_setcmddef(psy_EventDriver*, psy_Properties*);
static void driver_setcmddefaults(DXJoystickDriver*, psy_Properties*);
static void driver_idle(psy_EventDriver* self);

static void init_properties(psy_EventDriver* self);
static void apply_properties(DXJoystickDriver* self);

static CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg,
	DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

static HRESULT poll(DXJoystickDriver*, DIJOYSTATE2* js);

static BOOL CALLBACK
enumCallback(const DIDEVICEINSTANCE* instance, VOID* context);

static BOOL CALLBACK
staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef);

static int driver_setformat(DXJoystickDriver*);

int driver_onerror(int err, const char* msg)
{
	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT EventDriverInfo const * __cdecl GetPsycleEventDriverInfo(void)
{
	static EventDriverInfo info;
	info.Flags = 0;
	info.Name = "DirectX Joystick Driver";
	info.ShortName = "DX Joystick";
	info.Version = 0;
	return &info;
}

EXPORT psy_EventDriver* __cdecl eventdriver_create(void)
{
	DXJoystickDriver* dxjoystick = (DXJoystickDriver*) malloc(sizeof(DXJoystickDriver));
	if (dxjoystick) {
		memset(dxjoystick, 0, sizeof(DXJoystickDriver));
		dxjoystick->deviceid = DEVICE_NONE;
		dxjoystick->driver.open = driver_open;
		dxjoystick->driver.free = driver_free;
		dxjoystick->driver.open = driver_open;
		dxjoystick->driver.close = driver_close;
		dxjoystick->driver.dispose = driver_dispose;
		dxjoystick->driver.configure = driver_configure;
		dxjoystick->driver.error = driver_onerror;
		dxjoystick->driver.cmd = driver_cmd;
		dxjoystick->driver.getcmd = driver_getcmd;
		dxjoystick->driver.setcmddef = driver_setcmddef;
		dxjoystick->driver.idle = driver_idle;
		dxjoystick->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		driver_init(&dxjoystick->driver);
		return &dxjoystick->driver;
	}
	return NULL;
}

void driver_free(psy_EventDriver* driver)
{
	free(driver);
}

int driver_init(psy_EventDriver* driver)
{
	DXJoystickDriver* self = (DXJoystickDriver*) driver;
	HRESULT hr;

	self->active = 0;
	psy_table_init(&self->inputs);
	// Create a DirectInput device
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
		&IID_IDirectInput8, (VOID**)&self->di, NULL))) {
		return 1;
	}
	init_properties(&self->driver);
	psy_signal_init(&driver->signal_input);
	return 0;
}

int driver_dispose(psy_EventDriver* driver)
{
	DXJoystickDriver* self = (DXJoystickDriver*) driver;
	psy_properties_free(self->driver.properties);
	self->driver.properties = 0;
	CloseHandle(self->hEvent);
	psy_signal_dispose(&driver->signal_input);
	psy_table_disposeall(&self->inputs, (psy_fp_disposefunc)NULL);
	return 0;
}

void init_properties(psy_EventDriver* context)
{		
	DXJoystickDriver* self;
	HRESULT hr;

	self = (DXJoystickDriver*)context;
	context->properties = psy_properties_create();		
	psy_properties_append_string(context->properties, "name", "directx joystick");
	psy_properties_append_string(context->properties, "version", "1.0");
	self->devices = psy_properties_append_choice(context->properties, "device", 0);
	psy_properties_append_int(self->devices, "0:None", 0, 0, 0);
	self->count = 0;
	if (FAILED(hr = self->di->lpVtbl->EnumDevices(self->di,
			DI8DEVCLASS_GAMECTRL, enumCallback,
			self, DIEDFL_ATTACHEDONLY))) {
	}
	self->cmddef = psy_properties_append_section(context->properties, "cmds");
}

BOOL CALLBACK
enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	DXJoystickDriver* self;
	DIDEVICEINSTANCE* input;
	char text[256];

	self = (DXJoystickDriver*)context;
	
	_snprintf(text, 256, "%d:%s", self->count, instance->tszProductName);
	input = (DIDEVICEINSTANCE*)malloc(sizeof(DIDEVICEINSTANCE));
	*input = *instance;
	psy_table_insert(&self->inputs, self->count, input);
	psy_properties_append_int(self->devices, text, self->count, 0, 0);
	++self->count;
	return DIENUM_CONTINUE;
}

void apply_properties(DXJoystickDriver* self)
{	
	if (self->driver.properties) {
		psy_Properties* p;

		p = psy_properties_at(self->driver.properties, "device", PSY_PROPERTY_TYP_NONE);
		if (p) {
			self->deviceid = psy_properties_as_int(p);
		}
	}
}

void driver_configure(psy_EventDriver* self, psy_Properties* properties)
{
	apply_properties((DXJoystickDriver*)self);
}

int driver_open(psy_EventDriver* driver)
{
	HRESULT hr;
	DIDEVICEINSTANCE* instance;

	DXJoystickDriver* self = (DXJoystickDriver*) driver;
	unsigned int success = 1;

	self->lastinput.message = -1;
	// Obtain an interface to the enumerated joystick.
	
	instance = (DIDEVICEINSTANCE*)psy_table_at(&self->inputs, 0);
	if (!instance) {
		return 0;
	}
	hr = self->di->lpVtbl->CreateDevice(self->di, &instance->guidInstance,
		&self->joystick, NULL);

	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if (FAILED(hr)) {
		driver->error(0, "Cannot open DirectX Input device");
		return 0;
	}	
	driver_setformat(self);
	// set range and dead zone of joystick axes
	hr = self->joystick->lpVtbl->EnumObjects(self->joystick,
		&staticSetGameControllerAxesRanges, self->joystick, DIDFT_AXIS);
	if (FAILED(hr)) {
		driver->error(0, "Unable to set axis ranges of game controllers");
		return 0;
	}	
	self->active = TRUE;
	memset(&self->state, 0, sizeof(DIJOYSTATE2));
	return success;
}

int driver_setformat(DXJoystickDriver* self)
{
	DIDEVCAPS capabilities;
	HRESULT hr;

	// Set the data format to "simple joystick" - a predefined data format 
	//
	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
	if (FAILED(hr = self->joystick->lpVtbl->SetDataFormat(self->joystick,
		&c_dfDIJoystick2))) {
		return hr;
	}

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
	if (FAILED(hr = self->joystick->lpVtbl->SetCooperativeLevel(self->joystick,
		NULL, DISCL_EXCLUSIVE |
		DISCL_FOREGROUND))) {
		return hr;
	}

	// Determine how many axis the joystick has (so we don't error out setting
	// properties for unavailable axis)
	capabilities.dwSize = sizeof(DIDEVCAPS);
	if (FAILED(hr = self->joystick->lpVtbl->GetCapabilities(self->joystick, &capabilities))) {
		return hr;
	}
	return 0;
}

int driver_close(psy_EventDriver* driver)
{
	DXJoystickDriver* self = (DXJoystickDriver*) driver;
	unsigned int success = 1;
	

	return success;
}

CALLBACK MidiCallback(HMIDIIN handle, unsigned int uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{			
	psy_EventDriver* driver = (psy_EventDriver*)dwInstance;	
	DXJoystickDriver* self = (DXJoystickDriver*) driver;	
	switch(uMsg) {
		// normal data message
		case MIM_DATA:
		{
			int cmd;
			int lsb;
			int msb;
			unsigned char* data;

			data = (unsigned char*) &dwParam1;
			lsb = data[0] & 0x0F;
			msb = (data[0] & 0xF0) >> 4;
			switch (msb) {
				case 0x9:
					// Note On/Off
					cmd = data[2] > 0 ? data[1] - 48 : 120;
					// channel = lsb;
					self->lastinput.param1 = cmd;
					self->lastinput.param2 = 48;
					psy_signal_emit(&self->driver.signal_input, self, 0);
				default:
				break;
			}
		}
		break;
		default:
		break;
	}
}

void driver_cmd(psy_EventDriver* driver, const char* sectionname,
	EventDriverData input, EventDriverCmd* cmd)
{		
	DXJoystickDriver* self = (DXJoystickDriver*)driver;
	psy_Properties* section;

	if (!sectionname) {
		return;
	}
	cmd->id = -1;
	section = psy_properties_findsection(driver->properties, sectionname);
	if (!section) {
		return;
	}
	if (input.message != FALSE) {
		psy_Properties* p;

		for (p = section->children; p != NULL; p = psy_properties_next(p)) {
			if (psy_properties_as_int(p) == input.param1) {
				break;
			}
		}
		if (p) {
			cmd->id = p->item.id;
			cmd->data.param1 = 0;
			cmd->data.param2 = 0;
		}		
	}
}

EventDriverCmd driver_getcmd(psy_EventDriver* driver, const char* section)
{
	EventDriverCmd cmd;
	DXJoystickDriver* self = (DXJoystickDriver*) driver;	
			
	driver_cmd(driver, section, self->lastinput, &cmd);	
	return cmd;
}

/*
void driver_configure(psy_EventDriver* driver)
{
	psy_Properties* notes;

	notes = psy_properties_findsection(driver->properties, "notes");
	if (notes) {
		driver_makenoteinputs((KbdDriver*)driver, notes);
	}
}
*/

void driver_setcmddef(psy_EventDriver* driver, psy_Properties* cmddef)
{	
	if (cmddef && cmddef->children) {
		DXJoystickDriver* self = (DXJoystickDriver*)driver;
		psy_Properties* cmds;

		cmds = psy_properties_clone(cmddef->children, TRUE);
		psy_properties_append_property(
			self->cmddef, cmds);
		driver_setcmddefaults(self, cmds);
	}
}

void driver_setcmddefaults(DXJoystickDriver* self, psy_Properties* cmddef)
{
	psy_Properties* section;

	section = psy_properties_find(cmddef, "general", PSY_PROPERTY_TYP_SECTION);
	if (section) {
		psy_properties_set_int(section, "cmd_editmachine", INPUT_BUTTON_FIRST + 0);
		psy_properties_set_int(section, "cmd_editpattern", INPUT_BUTTON_FIRST + 1);
		psy_properties_set_int(section, "cmd_help", INPUT_BUTTON_FIRST + 2);
	}
	section = psy_properties_find(cmddef, "trackercmds", PSY_PROPERTY_TYP_SECTION);
	if (section) {
		psy_properties_set_int(section, "cmd_navup", INPUT_MOVE_UP);
		psy_properties_set_int(section, "cmd_navdown", INPUT_MOVE_DOWN);
		psy_properties_set_int(section, "cmd_navleft", INPUT_MOVE_LEFT);
		psy_properties_set_int(section, "cmd_navright", INPUT_MOVE_RIGHT);
	}
}

void driver_idle(psy_EventDriver* driver)
{
	DIJOYSTATE2 state;
	DXJoystickDriver* self;
	int i;	

	self = (DXJoystickDriver*)driver;
	poll(self, &state);
	for (i = 0; i < 128; ++i) {
		if (self->state.rgbButtons[i] != state.rgbButtons[i]) {
			self->lastinput.message = state.rgbButtons[i];
			self->lastinput.param1 = INPUT_BUTTON_FIRST + i;
			psy_signal_emit(&self->driver.signal_input, self, 0);
		}
	}
	if (state.lY < 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_UP;
		self->lastinput.param2 = self->state.lY;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	} else if (state.lY > 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_DOWN;
		self->lastinput.param2 =  state.lY;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	}
	if (state.lX < 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_LEFT;
		self->lastinput.param2 = self->state.lX;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	} else if (state.lX > 0) {
		self->lastinput.message = TRUE;
		self->lastinput.param1 = INPUT_MOVE_RIGHT;
		self->lastinput.param2 = state.lX;
		psy_signal_emit(&self->driver.signal_input, self, 0);
	}
	self->state = state;
}

HRESULT poll(DXJoystickDriver* self, DIJOYSTATE2* js)
{
	HRESULT     hr;

	if (self->joystick == NULL) {
		return S_OK;
	}
	// Poll the device to read the current state
	hr = self->joystick->lpVtbl->Poll(self->joystick);
	if (FAILED(hr)) {
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = self->joystick->lpVtbl->Acquire(self->joystick);
		while (hr == DIERR_INPUTLOST) {
			hr = self->joystick->lpVtbl->Acquire(self->joystick);
		}

		// If we encounter a fatal error, return failure.
		if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
			return E_FAIL;
		}

		// If another application has control of this device, return successfully.
		// We'll just have to wait our turn to use the joystick.
		if (hr == DIERR_OTHERAPPHASPRIO) {
			return S_OK;
		}
	}

	// Get the input's device state
	if (FAILED(hr = self->joystick->lpVtbl->GetDeviceState(
		self->joystick,
		sizeof(DIJOYSTATE2), js))) {
		return hr; // The device should have been acquired during the Poll()
	}

	return S_OK;
}

BOOL CALLBACK staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef)
{
	// the game controller
	LPDIRECTINPUTDEVICE8 gameController = (LPDIRECTINPUTDEVICE8)pvRef;
	gameController->lpVtbl->Acquire(gameController);

	// structure to hold game controller range properties
	DIPROPRANGE gameControllerRange;

	// set the range to -100 and 100
	gameControllerRange.lMin = -100;
	gameControllerRange.lMax = 100;

	// set the size of the structure
	gameControllerRange.diph.dwSize = sizeof(DIPROPRANGE);
	gameControllerRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	// set the object that we want to change		
	gameControllerRange.diph.dwHow = DIPH_BYID;
	gameControllerRange.diph.dwObj = devObjInst->dwType;

	// now set the range for the axis		
	if (FAILED(gameController->lpVtbl->SetProperty(gameController,
		DIPROP_RANGE, &gameControllerRange.diph))) {
		return DIENUM_STOP;
	}

	// structure to hold game controller axis dead zone
	DIPROPDWORD gameControllerDeadZone;

	// set the dead zone to 10%
	gameControllerDeadZone.dwData = 1000;

	// set the size of the structure
	gameControllerDeadZone.diph.dwSize = sizeof(DIPROPDWORD);
	gameControllerDeadZone.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	// set the object that we want to change
	gameControllerDeadZone.diph.dwHow = DIPH_BYID;
	gameControllerDeadZone.diph.dwObj = devObjInst->dwType;

	// now set the dead zone for the axis
	if (FAILED(gameController->lpVtbl->SetProperty(gameController,
		DIPROP_DEADZONE, &gameControllerDeadZone.diph)))
		return DIENUM_STOP;

	return DIENUM_CONTINUE;
}