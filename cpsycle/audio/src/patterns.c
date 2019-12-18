// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patterns.h"
#include <stdlib.h>

static void patterns_disposeslots(Patterns*);

void patterns_init(Patterns* self)
{
	table_init(&self->slots);
	self->songtracks = 16;	
	self->sharetracknames = 0;
	patternstrackstate_init(&self->trackstate);
}

void patterns_dispose(Patterns* self)
{	
	patterns_disposeslots(self);
	patternstrackstate_dispose(&self->trackstate);
}

void patterns_disposeslots(Patterns* self)
{	
	TableIterator it;

	for (it = table_begin(&self->slots);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		Pattern* pattern;
		
		pattern = (Pattern*)tableiterator_value(&it);
		pattern_dispose(pattern);
		free(pattern);
	}
	table_dispose(&self->slots);
}

void patterns_clear(Patterns* self)
{
	patterns_disposeslots(self);	
	table_init(&self->slots);	
}

void patterns_insert(Patterns* self, uintptr_t slot, Pattern* pattern)
{
	table_insert(&self->slots, slot, pattern);
}

int patterns_append(Patterns* self, Pattern* pattern)
{
	int slot = 0;
	
	while (table_at(&self->slots, slot)) {
		++slot;
	}
	table_insert(&self->slots, slot, pattern);	
	return slot;
}

Pattern* patterns_at(Patterns* self, uintptr_t slot)
{
	return table_at(&self->slots, slot);
}

void patterns_erase(Patterns* self, uintptr_t slot)
{

	table_remove(&self->slots, slot);
}

void patterns_remove(Patterns* self, uintptr_t slot)
{
	Pattern* pattern;
	
	pattern = (Pattern*) table_at(&self->slots, slot);
	table_remove(&self->slots, slot);
	pattern_dispose(pattern);
	free(pattern);	
}

uintptr_t patterns_size(Patterns* self)
{
	return self->slots.count;
}

void patterns_activatesolotrack(Patterns* self, uintptr_t track)
{
	patternstrackstate_activatesolotrack(&self->trackstate, track);
}

void patterns_deactivatesolotrack(Patterns* self)
{
	patternstrackstate_deactivatesolotrack(&self->trackstate);
}

void patterns_mutetrack(Patterns* self, uintptr_t track)
{
	patternstrackstate_mutetrack(&self->trackstate, track);
}

void patterns_unmutetrack(Patterns* self, uintptr_t track)
{
	patternstrackstate_unmutetrack(&self->trackstate, track);
}

int patterns_istrackmuted(Patterns* self, uintptr_t track)
{
	return patternstrackstate_istrackmuted(&self->trackstate, track);
}

int patterns_istracksoloed(Patterns* self, uintptr_t track)
{
	return patternstrackstate_istracksoloed(&self->trackstate, track);
}

void patterns_setsongtracks(Patterns* self, uintptr_t trackcount)
{
	self->songtracks = trackcount;
}

uintptr_t patterns_songtracks(Patterns* self)
{
	return self->songtracks;
}
