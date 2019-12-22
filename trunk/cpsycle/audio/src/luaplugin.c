// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "luaplugin.h"
#include "lauxlib.h"
#include "lualib.h"
#include "dummy.h"

#include <stdlib.h>
#include <string.h>

static void work(psy_audio_LuaPlugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_LuaPlugin*, int channel, const psy_audio_PatternEvent*);
static void sequencerlinetick(psy_audio_LuaPlugin*);
static psy_audio_MachineInfo* info(psy_audio_LuaPlugin*);
static void parametertweak(psy_audio_LuaPlugin*, int par, int val);
static int describevalue(psy_audio_LuaPlugin*, char* txt, int param, int value);
static int parametervalue(psy_audio_LuaPlugin*, int param);
static void dispose(psy_audio_LuaPlugin*);
static int mode(psy_audio_LuaPlugin*);

int luascript_setmachine(lua_State* L);

static int luamachine_open(lua_State *L);
static const char* luamachine_meta = "psypluginmeta";
static int luamachine_create(lua_State* L);
static int luamachine_gc(lua_State* L);
static int luamachine_work(lua_State* L);

static MachineVtable vtable;
static int vtable_initialized = 0;

static const luaL_Reg psycle_methods[] = {
	{"setmachine", luascript_setmachine},
	{NULL, NULL}
};

static void vtable_init(psy_audio_LuaPlugin* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;		
		vtable.seqtick = (fp_machine_seqtick) seqtick;
		vtable.sequencerlinetick = (fp_machine_sequencerlinetick)
			sequencerlinetick;
		vtable.info = (fp_machine_info) info;		
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;		
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.dispose =(fp_machine_dispose) dispose;
		vtable_initialized = 1;
	}
}
		
void luaplugin_init(psy_audio_LuaPlugin* self, MachineCallback callback,
	const char* path)
{
	int err = 0;	

	self->plugininfo = 0;
	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	psyclescript_init(&self->script);
	if (err = psyclescript_load(&self->script, path)) {
		return;	
	}
	if (err = psyclescript_preparestate(&self->script, psycle_methods,
		self)) {
		return;
	}
	psyclescript_require(&self->script, "psycle.machine", luamachine_open);
	if (err = psyclescript_run(&self->script)) {
		return;
	}
	if (err = psyclescript_start(&self->script)) {
		return;
	}
	self->plugininfo = machineinfo_allocinit();
	psyclescript_machineinfo(&self->script, self->plugininfo);
	self->custommachine.machine.vtable->seteditname(
		&self->custommachine.machine, self->plugininfo->ShortName);
}

void dispose(psy_audio_LuaPlugin* self)
{	
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}	
    psyclescript_dispose(&self->script);
	custommachine_dispose(&self->custommachine);
}

int plugin_luascript_test(const char* path, psy_audio_MachineInfo* machineinfo)
{		
	psy_audio_PsycleScript script;
	int err = 0;	
	
	psyclescript_init(&script);
	err = psyclescript_load(&script, path);
	if (err) {
		return 0;
	}
	err = psyclescript_preparestate(&script, psycle_methods, 0);
	if (err) {
		return 0;
	}	
	psyclescript_require(&script, "psycle.machine", luamachine_open);
	err = psyclescript_run(&script);
	if (err) {
		return 0;
	}
	err = psyclescript_machineinfo(&script, machineinfo);
	if (err) {
		return 0;
	}	
	psyclescript_dispose(&script);
	return 1;
}

void work(psy_audio_LuaPlugin* self, psy_audio_BufferContext* bc)
{
	
}

void seqtick(psy_audio_LuaPlugin* self, int channel, const psy_audio_PatternEvent* event)
{
	
}

void sequencerlinetick(psy_audio_LuaPlugin* self)
{
}

psy_audio_MachineInfo* info(psy_audio_LuaPlugin* self)
{
	return self->plugininfo;
}

void parametertweak(psy_audio_LuaPlugin* self, int par, int val)
{

}

int describevalue(psy_audio_LuaPlugin* self, char* txt, int param, int value)
{ 
	return 0;
}

int parametervalue(psy_audio_LuaPlugin* self, int param)
{
	return 0;
}

int mode(psy_audio_LuaPlugin* self)
{
	return MACHMODE_FX;		
}

int luamachine_open(lua_State *L)
{
	static const luaL_Reg methods[] = {
		{"new", luamachine_create},
		{"work", luamachine_work},		
		{NULL, NULL}
	  };
	psyclescript_open(L, luamachine_meta, methods, 
		luamachine_gc, 0);  
  return 1;
}

int luamachine_create(lua_State* L)
{	
	int n;
	int self = 1;
	psy_audio_Machine** ud;	
	MachineCallback callback;

	memset(&callback, 0, sizeof(MachineCallback));
	lua_pushvalue(L, self);
	n = lua_gettop(L);
	luaL_checktype(L, -1, LUA_TTABLE);  // self
	lua_newtable(L);  // new
	lua_pushvalue(L, self);
	lua_setmetatable(L, -2);
	lua_pushvalue(L, self);
	lua_setfield(L, self, "__index");		
	ud = (void*)lua_newuserdata(L, sizeof(psy_audio_Machine*));	
	*ud = malloc(sizeof(psy_audio_DummyMachine));	
	dummymachine_init((psy_audio_DummyMachine*)*ud, callback); 
	luaL_getmetatable(L, luamachine_meta);
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "__self");
	lua_remove(L, n);
	psyclescript_register_weakuserdata(L, ud);
	return 1;
}

int luamachine_gc(lua_State* L)
{
	psy_audio_Machine** ud = (psy_audio_Machine**) luaL_checkudata(L, 1, luamachine_meta);	
	machine_dispose(*ud);
	free(*ud);
	return 0;
}

int luamachine_work(lua_State* L)
{
	return 0;	
}

int luascript_setmachine(lua_State* L)
{
	psy_audio_LuaPlugin* proxy;
	psy_audio_Machine** ud;

	lua_getglobal(L, "psycle");
	lua_getfield(L, -1, "__self");
	proxy = *(psy_audio_LuaPlugin**)luaL_checkudata(L, -1, "psyhostmeta");
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_getfield(L, 1, "__self");
	ud = (psy_audio_Machine**) luaL_checkudata(L, -1, "psypluginmeta");	
	if (*ud) {
		proxy->client = *ud;
	}
	lua_pushvalue(L, 1);	
	lua_setfield(L, 2, "proxy");
	return 0;
}
