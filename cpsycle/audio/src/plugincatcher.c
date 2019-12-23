// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "plugincatcher.h"

#include "dummy.h"
#include "duplicator.h"
#include "duplicator2.h"
#include "master.h"
#include "mixer.h"
#include "plugin.h"
#include "luaplugin.h"
#include "sampler.h"
#include "vstplugin.h"

#include <dir.h>

#include <stdlib.h>
#include <string.h>
#include <portable.h>


#if defined DIVERSALIS__OS__MICROSOFT
#define MODULEEXT ".dll"
#elif defined DIVERSALIS__OS__APPLE
#define	".dylib"
#define _MAX_PATH 4096
#else
#define MODULEEXT ".so"
#define _MAX_PATH 4096
#endif

static void plugincatcher_makeinternals(psy_audio_PluginCatcher*);
static void plugincatcher_makeplugininfo(psy_audio_PluginCatcher*,
	const char* name,
	const char* path,
	unsigned int type,
	const psy_audio_MachineInfo* info);
static void plugincatcher_makesampler(psy_audio_PluginCatcher*);
static void plugincatcher_makeduplicator(psy_audio_PluginCatcher*);
static int isplugin(int type);
static int onenumdir(psy_audio_PluginCatcher*, const char* path, int flag);
static int onpropertiesenum(psy_audio_PluginCatcher*, psy_Properties*, int level);
static int pathhasextension(const char* path);
static char* replace_char(char* str, char c, char r);

static const char* searchname;
static int searchtype;
static psy_Properties* searchresult;


void plugincatcher_init(psy_audio_PluginCatcher* self, psy_Properties* dirconfig)
{
	char inipath[_MAX_PATH];
	self->plugins = psy_properties_create();
	plugincatcher_makeinternals(self);
	
	workdir(inipath);		
	strcat(inipath, "\\psycle-plugin-scanner-cache.ini");
	self->inipath = strdup(inipath);
	self->dirconfig = dirconfig;
	psy_signal_init(&self->signal_changed);
	psy_signal_init(&self->signal_scanprogress);	
}

void plugincatcher_dispose(psy_audio_PluginCatcher* self)
{
	properties_free(self->plugins);
	self->plugins = 0;
	free(self->inipath);	
	self->dirconfig = 0;	
	psy_signal_dispose(&self->signal_changed);
	psy_signal_dispose(&self->signal_scanprogress);
}

void plugincatcher_clear(psy_audio_PluginCatcher* self)
{
	properties_free(self->plugins);
	self->plugins = psy_properties_create();
	plugincatcher_makeinternals(self);
}

void plugincatcher_makeinternals(psy_audio_PluginCatcher* self)
{		
	plugincatcher_makeplugininfo(self, "sampler", "", MACH_SAMPLER,
		sampler_info());
	plugincatcher_makeplugininfo(self, "dummy", "", MACH_DUMMY,
		dummymachine_info());
	plugincatcher_makeplugininfo(self, "master", "", MACH_MASTER,
		master_info());
	plugincatcher_makeplugininfo(self, "mixer", "", MACH_MIXER,
		mixer_info());
	plugincatcher_makeplugininfo(self, "duplicator", "", MACH_DUPLICATOR,
		duplicator_info());
	plugincatcher_makeplugininfo(self, "duplicator2", "", MACH_DUPLICATOR2,
		duplicator2_info());
}

void plugincatcher_makeplugininfo(psy_audio_PluginCatcher* self,
		const char* name,
		const char* path,
		unsigned int type,
		const psy_audio_MachineInfo* info) {

	if (info) {
		psy_Properties* p;

		p = psy_properties_create_section(self->plugins, name);
		psy_properties_append_int(p, "type", type, 0, 0);
		psy_properties_append_int(p, "flags", info->Flags, 0, 0);
		psy_properties_append_int(p, "mode", info->mode, 0, 0);
		psy_properties_append_string(p, "name", info->Name);
		psy_properties_append_string(p, "shortname", info->ShortName);
		psy_properties_append_string(p, "author", info->Author);
		psy_properties_append_string(p, "command", info->Command);
		psy_properties_append_string(p, "path", path);				
		if (type == MACH_PLUGIN) {
			char text[256];
			psy_snprintf(text, 256, "Psycle %s by %s ", 
				info->mode == MACHMODE_FX ? "effect" : "instrument", 
				info->Author);
			psy_properties_append_string(p, "desc", text);
		} else {
			psy_properties_append_string(p, "desc", "");
		}
	}
}

void plugincatcher_scan(psy_audio_PluginCatcher* self)
{	
	psy_Properties* p;

	plugincatcher_clear(self);
	if (self->dirconfig) {
		p = psy_properties_findsection(self->dirconfig, "plugins");
		if (p) {		
			psy_dir_enumerate_recursive(self, psy_properties_valuestring(p), "*"MODULEEXT,
				MACH_PLUGIN, onenumdir);
		}
		p = psy_properties_findsection(self->dirconfig, "luascripts");
		if (p) {		
			psy_dir_enumerate(self, psy_properties_valuestring(p), "*.lua", MACH_LUA,
				onenumdir);
		}
		p = psy_properties_findsection(self->dirconfig, "vsts32");
		if (p) {		
			psy_dir_enumerate_recursive(self, psy_properties_valuestring(p), "*"MODULEEXT,
				MACH_VST, onenumdir);
		}
	}
	psy_signal_emit(&self->signal_changed, self, 0);
	psy_signal_emit(&self->signal_scanprogress, self, 1, 0);
}

int isplugin(int type)
{
	return 
		type == MACH_PLUGIN ||
		type == MACH_VST;
}

char* replace_char(char* str, char c, char r)
{
	char* p;
		
	for (p = strchr(str, c); p != 0; p = strchr(p + 1, c)) *p = r;
	return p;
}

int onenumdir(psy_audio_PluginCatcher* self, const char* path, int type)
{	
	psy_audio_MachineInfo macinfo;
	char name[_MAX_PATH];	

	machineinfo_init(&macinfo);
	plugincatcher_catchername(self, path, name);
	if (type == MACH_PLUGIN) {		
		if (plugin_psycle_test(path, &macinfo)) {
			plugincatcher_makeplugininfo(self, name, path, type, &macinfo);
			psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
		}	
	} else
	if (type == MACH_LUA) {
		if (plugin_luascript_test(path, &macinfo)) {
			plugincatcher_makeplugininfo(self, name, path, type, &macinfo);
			psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
		}
	} else
	if (type == MACH_VST) {
		if (plugin_vst_test(path, &macinfo)) {
			plugincatcher_makeplugininfo(self, name, path, type, &macinfo);
			psy_signal_emit(&self->signal_scanprogress, self, 1, 1);
		}
	}
	machineinfo_dispose(&macinfo);
	return 1;
}

void plugincatcher_catchername(psy_audio_PluginCatcher* self, const char* path, char* name)
{	
	char prefix[_MAX_PATH];
	char ext[_MAX_PATH];	

	psy_dir_extract_path(path, prefix, name, ext);
	strlwr(name);
	strlwr(ext);
	replace_char(name, ' ', '-');
}

int plugincatcher_load(psy_audio_PluginCatcher* self)
{
	int rv;

	plugincatcher_clear(self);	
	rv = propertiesio_load(self->plugins, self->inipath, 1);
	psy_signal_emit(&self->signal_changed, self, 0);
	return rv;
}

void plugincatcher_save(psy_audio_PluginCatcher* self)
{	
	propertiesio_save(self->plugins, self->inipath);
}

char* plugincatcher_modulepath(psy_audio_PluginCatcher* self, MachineType machtype,
	const char* path, char* fullpath)
{	
	if (!path) {
		*fullpath = '\0';
	} else
	if (pathhasextension(path)) {
		strcpy(fullpath, path);
	} else {		
		searchname = path;
		searchtype = machtype;
		searchresult = 0;
		psy_properties_enumerate(self->plugins, self, onpropertiesenum);
		if (!searchresult) {
			if (strstr(path, "blitz")) {
				searchname = "blitzn";
				searchtype = machtype;
				searchresult = 0;
				psy_properties_enumerate(self->plugins, self, onpropertiesenum);
			}
		}
		if (searchresult) {
			strcpy(fullpath, psy_properties_readstring(searchresult, "path", ""));
		} else {
			strcpy(fullpath, path);
		}
	}
	return fullpath;
}

int onpropertiesenum(psy_audio_PluginCatcher* self, psy_Properties* property, int level)
{
	if (psy_properties_type(property) == PSY_PROPERTY_TYP_SECTION) {
		const char* key = psy_properties_key(property);
		key = key;
		if ((strcmp(psy_properties_key(property), searchname) == 0) &&
				psy_properties_int(property, "type", 0) == searchtype) {
			searchresult = property;
			return 0;			
		}
	}
	return 1;
}

int pathhasextension(const char* path)
{
	return strrchr(path, '.') != 0;
}

const char* plugincatcher_searchpath(psy_audio_PluginCatcher* self, const char* name,
	int machtype)
{	
	searchname = name;
	searchtype = machtype;
	searchresult = 0;
	psy_properties_enumerate(self->plugins, self, onpropertiesenum);
	return psy_properties_readstring(searchresult, "path", 0);
}