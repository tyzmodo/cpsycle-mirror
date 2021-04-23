// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playlist.h"
// local
#include "dir.h"
#include "propertiesio.h"
// container
#include <hashtbl.h>
// file
// platform
#include "../../detail/portable.h"
// std
#include <assert.h>

static void playlist_initpath(psy_Playlist* self);

void psy_playlist_init(psy_Playlist* self)
{
	assert(self);
		
	self->recentsongs = psy_property_setcomment(
		psy_property_allocinit_key(NULL),
		"Psycle Recent Song ViewHistory created by\r\n; " PSYCLE__BUILD__IDENTIFIER("\r\n; "));
	self->recentfiles = psy_property_settext(
		psy_property_append_section(self->recentsongs, "files"),
		"Recent Songs");
	playlist_initpath(self);
}

void playlist_initpath(psy_Playlist* self)
{
	psy_Path path;

	assert(self);

	psy_path_init(&path, NULL);
	psy_path_setprefix(&path, psy_dir_config());
	psy_path_setname(&path, PSYCLE_RECENT_SONG_INI);
	self->path = psy_strdup(psy_path_full(&path));
	psy_path_dispose(&path);
}

void psy_playlist_dispose(psy_Playlist* self)
{
	assert(self);

	psy_property_deallocate(self->recentsongs);	
	free(self->path);
}

void psy_playlist_add(psy_Playlist* self, const char* filename)
{
	char hashkey[256];

	assert(self);
	
	psy_snprintf(hashkey, 256, "%u", (unsigned int)psy_strhash(filename));
	if (!psy_property_find(self->recentfiles, hashkey,
			PSY_PROPERTY_TYPE_NONE)) {
		psy_Path path;
		
		psy_path_init(&path, filename);		
		psy_property_preventtranslate(psy_property_settext(
			psy_property_setreadonly(psy_property_append_str(
				self->recentfiles, hashkey, filename), TRUE),
			psy_path_name(&path)));
		psy_playlist_save(self);
		psy_path_dispose(&path);
	}
}

void psy_playlist_clear(psy_Playlist* self)
{		
	assert(self);
	
	psy_property_clear(self->recentfiles);
	propertiesio_save(self->recentsongs, self->path);	
}

void psy_playlist_load(psy_Playlist* self)
{
	psy_List* p;
	psy_Path path;

	psy_path_init(&path, self->path);
	propertiesio_load(self->recentsongs, &path, 1);
	psy_path_dispose(&path);
	if (self->recentfiles) {
		for (p = psy_property_begin(self->recentfiles); p != NULL;
			psy_list_next(&p)) {
			psy_Property* property;
			psy_Path path;

			property = (psy_Property*)psy_list_entry(p);
			psy_path_init(&path, psy_property_item_str(property));
			psy_property_settext(property, psy_path_name(&path));
			psy_property_setreadonly(property, TRUE);
			psy_property_preventtranslate(property);
			psy_path_dispose(&path);
		}
	}
}

void psy_playlist_save(psy_Playlist* self)
{		
	assert(self);
	
	propertiesio_save(self->recentsongs, self->path);
}

void psy_playlist_setpath(psy_Playlist* self, const char* path)
{
	psy_strreset(&self->path, path);
}
