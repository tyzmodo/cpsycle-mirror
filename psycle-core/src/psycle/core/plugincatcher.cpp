// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "plugincatcher.h"
#include "file.h"
#include "fileio.h"
#include <universalis/os/fs.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream> // only for debug output
#include <sstream>
#include <cstring>

namespace psycle { namespace core {

namespace {
	boost::filesystem::path const & cache_path() {
		boost::filesystem::path const static once(
			universalis::os::fs::home_app_local("psycle") / "plugin-scan-v2.cache"
		);
		return once;
	}
}

PluginFinderCache::PluginFinderCache(bool delayedScan)
: PluginFinder(delayedScan)
{}

void PluginFinderCache::Initialize(bool clear) {
	if(clear) deleteCache();
	else loadCache();
}

void PluginFinderCache::EnablePlugin(const MachineKey & key, bool enable) {
	PluginFinder::EnablePlugin(key, enable);
	saveCache();
}

bool PluginFinderCache::loadCache(){
	///\todo: Implement this with a better structure (My plan was to use
	// the new riff classes on the helpers project, but they are unfinished)
	char temp[9];
	uint32_t version(0);
	uint32_t fileNumPlugs(0);
	RiffFile file;

	if(!file.Open(cache_path().file_string().c_str())) {
		return false;
	}

	file.ReadArray(temp, 8);
	temp[8] = 0;
	if(std::strcmp(temp, "PSYCACHE") != 0) {
		file.Close();
		deleteCache();
		return false;
	}

	file.Read(version);
	if(version != CURRENT_CACHE_MAP_VERSION) {
		file.Close();
		deleteCache();
		return false;
	}

	file.Read(fileNumPlugs);
	for (uint32_t i = 0; i < fileNumPlugs; ++i) {
		PluginInfo p;

		std::string fullPath;
		file.ReadString(fullPath); p.setLibName(fullPath);
		
		{ uint64_t filetime(0); file.Read(filetime); p.setFileTime((time_t )filetime); }
		{ std::string error_msg; file.ReadString(error_msg); p.setError(error_msg); }

		Hosts::type host;
		{ uint8_t host_int; file.Read(host_int); host = Hosts::type(host_int); }

		uint32_t index(0);
		file.Read(index);
		
		{ uint8_t allow; file.Read(allow); p.setAllow((bool)allow); }
		{ uint32_t role_int(0); file.Read(role_int); p.setRole(MachineRole::type(role_int)); }

		{ std::string s_temp;
			file.ReadString(s_temp); p.setName(s_temp);
			file.ReadString(s_temp); p.setAuthor(s_temp);
			file.ReadString(s_temp); p.setDesc(s_temp);
			file.ReadString(s_temp); p.setApiVersion(s_temp);
			file.ReadString(s_temp); p.setPlugVersion(s_temp);
		}

		// Temp here contains the full path to the .dll
		{
			boost::filesystem::path const path(fullPath);
			if(boost::filesystem::exists(path)) {
				std::time_t t_time = boost::filesystem::last_write_time(path);
				if(t_time != (std::time_t)(-1)) {
					// Only add the information to the cache if the dll hasn't been modified (say, a new version)
					// Else, we want to get the new information, and that will happen in the plugins scan.
					if(p.fileTime() == t_time) {
						MachineKey key(host, path.filename(), index);
						AddInfo(key, p);
					}
				}
				MachineKey key(host, path.filename(), index);
				if(!hasHost(host)) addHost(host);
				AddInfo(key, p);
			}
		}
	}

	file.Close();
	return true;
}

bool PluginFinderCache::saveCache(){
	deleteCache();

	RiffFile file;
	if(!file.Create(cache_path().file_string().c_str(), true)) {
		boost::filesystem::create_directory(cache_path().parent_path());
		if(!file.Create(cache_path().file_string().c_str(), true)) return false;
	}
	file.WriteArray("PSYCACHE", 8);
	uint32_t version = CURRENT_CACHE_MAP_VERSION;
	file.Write(version);
	
	uint32_t fileNumPlugs = 0;
	
	// We skip the internal host. It doesn't need to be cached, since it is autogenerated each time.
	for(uint32_t numHost = 1; hasHost(Hosts::type(numHost)); ++numHost) {
		fileNumPlugs += size(Hosts::type(numHost));
	}
	file.Write(fileNumPlugs);

	// We skip the internal host. It doesn't need to be cached, since it is autogenerated each time.
	for(uint32_t numHost = 1; hasHost(Hosts::type(numHost)); ++numHost) {
		PluginFinder::const_iterator iter = begin(Hosts::type(numHost));
		while(iter != end(Hosts::type(numHost))) {
			PluginInfo info =iter->second;
			file.WriteString(info.libName());
			file.Write((uint64_t)info.fileTime());
			file.WriteString(info.error());
			file.Write((uint8_t)iter->first.host());
			file.Write(iter->first.index());
			file.Write((uint8_t)info.allow());
			file.Write((uint32_t)info.role());
			file.WriteString(info.name());
			file.WriteString(info.author());
			file.WriteString(info.desc());
			file.WriteString(info.apiVersion());
			file.WriteString(info.plugVersion());
			++iter;
		}
	}
	file.Close();
	return true;
}

void PluginFinderCache::deleteCache(){
	boost::filesystem::remove(cache_path());
}

void PluginFinderCache::PostInitialization() {
	saveCache();
}

}}
