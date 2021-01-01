// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_LUABIND_MIDINOTES_H
#define psy_audio_LUABIND_MIDINOTES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

int psy_audio_luabind_midinotes_open(lua_State* L);

extern const char* luamidinotesbind_meta;

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_LUABIND_MIDINOTES_H */
