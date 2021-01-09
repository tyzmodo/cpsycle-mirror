// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patterns.h"

#include <assert.h>
#include <stdlib.h>

// prototypes
static void psy_audio_patterns_init_signals(psy_audio_Patterns*);
static void psy_audio_patterns_dispose_signals(psy_audio_Patterns*);
static uintptr_t psy_audio_patterns_firstfreeslot(psy_audio_Patterns*);
static void psy_audio_patterns_disposeslots(psy_audio_Patterns*);
static void psy_audio_patterns_onpatternnamechanged(psy_audio_Patterns*,
	psy_audio_Pattern* sender);
static void psy_audio_patterns_onpatternlengthchanged(psy_audio_Patterns*,
	psy_audio_Pattern* sender);
// implementation
void psy_audio_patterns_init(psy_audio_Patterns* self)
{
	assert(self);

	psy_table_init(&self->slots);
	self->songtracks = 16;	
	self->sharetracknames = 0;
	psy_audio_trackstate_init(&self->trackstate);
	psy_audio_patterns_init_signals(self);
}

void psy_audio_patterns_init_signals(psy_audio_Patterns* self)
{
	assert(self);

	psy_signal_init(&self->signal_namechanged);
	psy_signal_init(&self->signal_lengthchanged);
}

void psy_audio_patterns_dispose(psy_audio_Patterns* self)
{	
	assert(self);

	psy_audio_patterns_disposeslots(self);
	psy_audio_trackstate_dispose(&self->trackstate);
	psy_audio_patterns_dispose_signals(self);
}

void psy_audio_patterns_dispose_signals(psy_audio_Patterns* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_namechanged);
	psy_signal_dispose(&self->signal_lengthchanged);
}

void psy_audio_patterns_disposeslots(psy_audio_Patterns* self)
{	
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->slots);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Pattern* pattern;
		
		pattern = (psy_audio_Pattern*)psy_tableiterator_value(&it);
		psy_audio_pattern_dispose(pattern);
		free(pattern);
	}
	psy_table_dispose(&self->slots);
	psy_signal_disconnectall(&self->signal_namechanged);
}

void psy_audio_patterns_clear(psy_audio_Patterns* self)
{
	assert(self);

	psy_audio_patterns_disposeslots(self);
	psy_table_init(&self->slots);	
}

void psy_audio_patterns_insert(psy_audio_Patterns* self, uintptr_t slot,
	psy_audio_Pattern* pattern)
{
	assert(self);
	
	psy_audio_patterns_remove(self, slot);
	psy_table_insert(&self->slots, slot, pattern);
	psy_signal_connect(&pattern->signal_namechanged, self,
		psy_audio_patterns_onpatternnamechanged);
	psy_signal_connect(&pattern->signal_lengthchanged, self,
		psy_audio_patterns_onpatternlengthchanged);
}

uintptr_t psy_audio_patterns_append(psy_audio_Patterns* self, psy_audio_Pattern* pattern)
{
	uintptr_t slot;
	
	assert(self);

	slot = psy_audio_patterns_firstfreeslot(self);
	psy_audio_patterns_insert(self, slot, pattern);
	return slot;
}

uintptr_t psy_audio_patterns_firstfreeslot(psy_audio_Patterns* self)
{
	uintptr_t rv = 0;

	assert(self);

	// find first free slot
	while (psy_table_exists(&self->slots, rv)) {
		++rv;
	}
	return rv;
}

psy_audio_Pattern* psy_audio_patterns_at(psy_audio_Patterns* self, uintptr_t slot)
{
	return psy_table_at(&self->slots, slot);
}

uintptr_t psy_audio_patterns_slot(psy_audio_Patterns* self, psy_audio_Pattern* pattern)
{
	uintptr_t rv;
	psy_TableIterator it;

	assert(self);

	rv = UINTPTR_MAX;
	for (it = psy_table_begin(&self->slots);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		if (pattern == psy_tableiterator_value(&it)) {
			rv = psy_tableiterator_key(&it);
			break;
		}
	}
	return rv;
}

void psy_audio_patterns_erase(psy_audio_Patterns* self, uintptr_t slot)
{
	psy_audio_Pattern* pattern;
	
	assert(self);

	pattern = psy_audio_patterns_at(self, slot);
	psy_table_remove(&self->slots, slot);
	if (pattern) {
		psy_signal_disconnect(&pattern->signal_namechanged, self,
			psy_audio_patterns_onpatternnamechanged);
		psy_signal_disconnect(&pattern->signal_lengthchanged, self,
			psy_audio_patterns_onpatternlengthchanged);
	}
}

void psy_audio_patterns_remove(psy_audio_Patterns* self, uintptr_t slot)
{
	psy_audio_Pattern* pattern;
	
	assert(self);

	pattern = psy_audio_patterns_at(self, slot);
	psy_table_remove(&self->slots, slot);
	if (pattern) {
		psy_audio_pattern_dispose(pattern);
		free(pattern);
	}
}

uintptr_t psy_audio_patterns_size(psy_audio_Patterns* self)
{
	assert(self);

	return psy_table_size(&self->slots);
}

void psy_audio_patterns_activatesolotrack(psy_audio_Patterns* self,
	uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_activatesolotrack(&self->trackstate, track);
}

void psy_audio_patterns_deactivatesolotrack(psy_audio_Patterns* self)
{
	assert(self);

	psy_audio_trackstate_deactivatesolotrack(&self->trackstate);
}

void psy_audio_patterns_mutetrack(psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_mutetrack(&self->trackstate, track);
}

void psy_audio_patterns_unmutetrack(psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	psy_audio_trackstate_unmutetrack(&self->trackstate, track);
}

int psy_audio_patterns_istrackmuted(const psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istrackmuted(&self->trackstate, track);
}

int psy_audio_patterns_istracksoloed(const psy_audio_Patterns* self, uintptr_t track)
{
	assert(self);

	return psy_audio_trackstate_istracksoloed(&self->trackstate, track);
}

void psy_audio_patterns_setsongtracks(psy_audio_Patterns* self,
	uintptr_t trackcount)
{
	assert(self);

	self->songtracks = trackcount;
}

uintptr_t psy_audio_patterns_songtracks(psy_audio_Patterns* self)
{
	assert(self);

	return self->songtracks;
}

void psy_audio_patterns_onpatternnamechanged(psy_audio_Patterns* self,
	psy_audio_Pattern* sender)
{
	uintptr_t slot;
	
	assert(self);

	slot = psy_audio_patterns_slot(self, sender);
	if (slot != UINTPTR_MAX) {
		psy_signal_emit(&self->signal_namechanged, self, 1, slot);
	}	
}

void psy_audio_patterns_onpatternlengthchanged(psy_audio_Patterns* self,
	psy_audio_Pattern* sender)
{
	uintptr_t slot;

	assert(self);

	slot = psy_audio_patterns_slot(self, sender);
	if (slot != UINTPTR_MAX) {
		psy_signal_emit(&self->signal_lengthchanged, self, 1, slot);
	}
}
