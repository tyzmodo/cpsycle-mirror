// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SEQUENCE_H
#define psy_audio_SEQUENCE_H

// audio
#include "patterns.h"
// container
#include <list.h>
#include <signal.h>
#include <command.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

// psy_audio_Sequence
//
// contains the playorder list of a song (multisequence)

// psy_audio_SequenceEntry
//
// Entry inside a track of a sequence
typedef struct psy_audio_SequenceEntry {
	uintptr_t row;
	// playorder value (the pattern to be played)
	uintptr_t patternslot;
	// absolute start position (in beats) in the song
	psy_dsp_big_beat_t offset;
	// offset to the playlist index position allowing free positioning
	// needed to reposition the sequence
	psy_dsp_big_beat_t repositionoffset;
	bool selplay;
} psy_audio_SequenceEntry;

void psy_audio_sequenceentry_init(psy_audio_SequenceEntry*, uintptr_t
	patternslot, psy_dsp_big_beat_t offset);
psy_audio_SequenceEntry* psy_audio_sequenceentry_alloc(void);
psy_audio_SequenceEntry* psy_audio_sequenceentry_allocinit(uintptr_t
	patternslot, psy_dsp_big_beat_t offset);
psy_audio_SequenceEntry* psy_audio_sequenceentry_clone(psy_audio_SequenceEntry*);

INLINE void psy_audio_sequenceentry_setpatternslot(psy_audio_SequenceEntry*
	self, uintptr_t slot)
{
	assert(self);

	self->patternslot = slot;
}

INLINE uintptr_t psy_audio_sequenceentry_patternslot(const
	psy_audio_SequenceEntry* self)
{
	assert(self);

	return self->patternslot;
}

INLINE psy_audio_Pattern* psy_audio_sequenceentry_pattern(const
	psy_audio_SequenceEntry* self, psy_audio_Patterns* patterns)
{
	assert(self);

	if (patterns) {
		return (psy_audio_Pattern*)psy_audio_patterns_at(patterns,
			self->patternslot);		
	}
	return NULL;
}

typedef psy_List psy_audio_SequenceEntryNode;

// psy_audio_OrderIndex
//
// Addresses a SequenceEntry inside psy_audio_Sequence
typedef struct psy_audio_OrderIndex {
	uintptr_t track;
	uintptr_t order;
} psy_audio_OrderIndex;

INLINE psy_audio_OrderIndex psy_audio_orderindex_make(
	uintptr_t trackidx, uintptr_t orderidx)
{
	psy_audio_OrderIndex rv;

	rv.track = trackidx;
	rv.order = orderidx;
	return rv;
}

INLINE bool psy_audio_orderindex_valid(const psy_audio_OrderIndex* self)
{
	return (self->order != psy_INDEX_INVALID &&
		    self->track != psy_INDEX_INVALID);
}

// psy_audio_SequenceSelection
//
// Selection of SequenceEntries stored as OrderIndexes

typedef enum {
	psy_audio_SEQUENCESELECTION_SINGLE,
	psy_audio_SEQUENCESELECTION_MULTI
} psy_audio_SequenceSelectionMode;

typedef psy_List* psy_audio_SequenceSelectionIterator;

typedef struct psy_audio_SequenceSelection {
	psy_Signal signal_changed;
	psy_Signal signal_clear;
	psy_Signal signal_select;
	psy_Signal signal_deselect;
	psy_Signal signal_update;
	psy_audio_OrderIndex editposition;
	psy_List* entries; // Order Indexes
	psy_audio_SequenceSelectionMode mode;	
} psy_audio_SequenceSelection;

void psy_audio_sequenceselection_init(psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_dispose(psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_clear(psy_audio_SequenceSelection*);
void psy_audio_sequenceselection_copy(psy_audio_SequenceSelection*,
	psy_audio_SequenceSelection* other);

INLINE psy_audio_SequenceSelectionIterator psy_audio_sequenceselection_begin(
	psy_audio_SequenceSelection* self)
{
	return self->entries;
}

void psy_audio_sequenceselection_seteditposition(psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);
void psy_audio_sequenceselection_select(psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);
void psy_audio_sequenceselection_deselect(psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);
bool psy_audio_sequenceselection_isselected(const psy_audio_SequenceSelection*,
	psy_audio_OrderIndex);
void psy_audio_sequenceselection_setmode(psy_audio_SequenceSelection*,
	psy_audio_SequenceSelectionMode);
void psy_audio_sequenceselection_update(psy_audio_SequenceSelection*);

// psy_audio_Order
// A pair of OrderIndex and SequenceEntrySequence

struct psy_audio_Sequence;

typedef struct {
	psy_audio_OrderIndex index;
	psy_audio_SequenceEntry entry;
} psy_audio_Order;

typedef struct psy_audio_SequencePaste {
	psy_List* entries;
} psy_audio_SequencePaste;

void psy_audio_sequencepaste_init(psy_audio_SequencePaste*);
void psy_audio_sequencepaste_dispose(psy_audio_SequencePaste*);

void psy_audio_sequencepaste_clear(psy_audio_SequencePaste*);
void psy_audio_sequencepaste_copy(psy_audio_SequencePaste*,
	struct psy_audio_Sequence*, psy_audio_SequenceSelection*);

// psy_audio_SequenceTrack
//
// A list of SequenceEntries sorted according to the playorder
typedef struct {
	psy_audio_SequenceEntryNode* entries;	
	char* name;
} psy_audio_SequenceTrack;

void psy_audio_sequencetrack_init(psy_audio_SequenceTrack*);
void psy_audio_sequencetrack_dispose(psy_audio_SequenceTrack*);

psy_audio_SequenceTrack* psy_audio_sequencetrack_alloc(void);
psy_audio_SequenceTrack* psy_audio_sequencetrack_allocinit(void);
psy_audio_SequenceTrack* psy_audio_sequencetrack_clone(psy_audio_SequenceTrack* src);

psy_dsp_big_beat_t psy_audio_sequencetrack_duration(psy_audio_SequenceTrack*,
	psy_audio_Patterns* patterns);

void psy_audio_sequencetrack_setname(psy_audio_SequenceTrack* self, const char* name);

INLINE const char* psy_audio_sequencetrack_name(const psy_audio_SequenceTrack* self)
{
	assert(self);

	return self->name;
}

// psy_audio_SequenceTrackIterator
//
// Iterator the player uses to advance through the track and pattern
typedef struct {
	psy_audio_Patterns* patterns;	
	psy_audio_SequenceEntryNode* sequencentrynode;
	psy_audio_PatternNode* patternnode;
	psy_audio_Pattern* pattern;
} psy_audio_SequenceTrackIterator;

void psy_audio_sequencetrackiterator_inc(psy_audio_SequenceTrackIterator*);
void psy_audio_sequencetrackiterator_inc_entry(psy_audio_SequenceTrackIterator*);
void psy_audio_sequencetrackiterator_dec_entry(psy_audio_SequenceTrackIterator*);

INLINE psy_audio_PatternNode* psy_audio_sequencetrackiterator_patternnode(
	psy_audio_SequenceTrackIterator* self)
{
	return self->patternnode;
}

INLINE psy_audio_SequenceEntry* psy_audio_sequencetrackiterator_entry(
	psy_audio_SequenceTrackIterator* self)
{
	return (self->sequencentrynode)
		? (psy_audio_SequenceEntry*)psy_list_entry(self->sequencentrynode)
		: NULL;
}

INLINE psy_audio_PatternEntry* psy_audio_sequencetrackiterator_patternentry(
	psy_audio_SequenceTrackIterator* self)
{
	return (self->patternnode)
		? (psy_audio_PatternEntry*)(self->patternnode)->entry
		: NULL;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencetrackiterator_offset(
	psy_audio_SequenceTrackIterator* self)
{
	return (psy_audio_sequencetrackiterator_patternentry(self))
		? psy_audio_sequencetrackiterator_entry(self)->offset +
		  psy_audio_sequencetrackiterator_patternentry(self)->offset
		: psy_audio_sequencetrackiterator_entry(self)->offset;
}

typedef psy_List psy_audio_SequenceTrackNode;

// psy_audio_SequencePosition
//
// defines the overall position in a song
typedef struct {
	psy_audio_SequenceTrackNode* tracknode;
	psy_audio_SequenceTrackIterator trackposition;	
} psy_audio_SequencePosition;

void psy_audio_sequenceposition_init(psy_audio_SequencePosition*);
psy_audio_SequenceEntry* psy_audio_sequenceposition_entry(
	psy_audio_SequencePosition*);

typedef struct psy_audio_Sequence {
	// signals
	psy_Signal signal_changed;
	psy_Signal signal_insert;
	psy_Signal signal_remove;
	psy_Signal signal_reorder;
	psy_Signal signal_trackinsert;
	psy_Signal signal_trackremove;
	psy_Signal signal_trackreposition;
	psy_Signal signal_clear;
	// internal data
	psy_audio_SequenceTrackNode* tracks;	
	psy_audio_TrackState trackstate;
	bool preventreposition;
	// references
	psy_audio_Patterns* patterns;	
} psy_audio_Sequence;

void psy_audio_sequence_init(psy_audio_Sequence*, psy_audio_Patterns*);
void psy_audio_sequence_dispose(psy_audio_Sequence*);
void psy_audio_sequence_copy(psy_audio_Sequence*, psy_audio_Sequence* other);

void psy_audio_sequence_clear(psy_audio_Sequence*);
// sequenceentry methods
void psy_audio_sequence_insert(psy_audio_Sequence*, psy_audio_OrderIndex,
	uintptr_t patidx);
void psy_audio_sequence_remove(psy_audio_Sequence*, psy_audio_OrderIndex);
void psy_audio_sequence_remove_selection(psy_audio_Sequence*,
	psy_audio_SequenceSelection*);
psy_audio_SequenceEntry* psy_audio_sequence_entry(psy_audio_Sequence*,
	psy_audio_OrderIndex);
const psy_audio_SequenceEntry* psy_audio_sequence_entry_const(const
	psy_audio_Sequence*, psy_audio_OrderIndex);
psy_audio_OrderIndex psy_audio_sequence_reorder(psy_audio_Sequence*,
	psy_audio_OrderIndex, psy_dsp_big_beat_t newposition);
void psy_audio_sequence_resetpatterns(psy_audio_Sequence*);
void psy_audio_sequence_fillpatterns(psy_audio_Sequence*);
void psy_audio_reposition(psy_audio_Sequence*);
uintptr_t psy_audio_sequence_order(psy_audio_Sequence*,
	uintptr_t trackidx, psy_dsp_big_beat_t position);
psy_audio_Pattern* psy_audio_sequence_pattern(psy_audio_Sequence*,
	psy_audio_OrderIndex);
void psy_audio_sequence_setpatternindex(psy_audio_Sequence*,
	psy_audio_OrderIndex, uintptr_t patidx);
uintptr_t psy_audio_sequence_patternindex(const psy_audio_Sequence*,
	psy_audio_OrderIndex);
bool psy_audio_sequence_patternused(psy_audio_Sequence*, uintptr_t patidx);
psy_audio_OrderIndex psy_audio_sequence_patternfirstused(psy_audio_Sequence*,
	uintptr_t patidx);
// track methods
void psy_audio_sequence_appendtrack(psy_audio_Sequence*,
	psy_audio_SequenceTrack*);
psy_audio_SequenceTrackNode* psy_audio_sequence_removetrack(
	psy_audio_Sequence*, psy_audio_SequenceTrackNode*);
uintptr_t psy_audio_sequence_sizetracks(psy_audio_Sequence*);
psy_audio_SequenceTrack* psy_audio_sequence_track_at(psy_audio_Sequence*,
	uintptr_t trackidx);
const psy_audio_SequenceTrack* psy_audio_sequence_track_at_const(const
	psy_audio_Sequence*, uintptr_t trackidx);
uintptr_t psy_audio_sequence_track_size(const psy_audio_Sequence*,
	uintptr_t trackindex);
uintptr_t psy_audio_sequence_maxtracksize(psy_audio_Sequence*);
void psy_audio_sequence_reposition_track(psy_audio_Sequence*,
	psy_audio_SequenceTrack*);
psy_dsp_big_beat_t psy_audio_sequence_offset(const psy_audio_Sequence*,
	psy_audio_OrderIndex);
// play selection
void psy_audio_sequence_setplayselection(psy_audio_Sequence*,
	struct psy_audio_SequenceSelection*);
void psy_audio_sequence_clearplayselection(psy_audio_Sequence*);
// methods for the sequencer
psy_audio_SequenceTrackIterator psy_audio_sequence_begin(psy_audio_Sequence*,
	psy_List* track, psy_dsp_big_beat_t position);
psy_audio_SequencePosition psy_audio_sequence_at(psy_audio_Sequence*,
	uintptr_t trackindex, uintptr_t position);
psy_audio_SequencePosition psy_audio_sequence_makeposition(psy_audio_Sequence*,
	psy_audio_SequenceTrackNode*, psy_List* entries);
psy_audio_SequenceEntryNode* psy_audio_sequence_node(psy_audio_Sequence* self,
	psy_audio_OrderIndex index, psy_audio_SequenceTrack** rv);
// duration
psy_dsp_big_beat_t psy_audio_sequence_duration(psy_audio_Sequence*);
psy_dsp_big_seconds_t psy_audio_sequence_calcdurationinms(psy_audio_Sequence*);
// trackstate
void psy_audio_sequence_activatesolotrack(psy_audio_Sequence*, uintptr_t trackidx);
void psy_audio_sequence_deactivatesolotrack(psy_audio_Sequence*);
void psy_audio_sequence_mutetrack(psy_audio_Sequence*, uintptr_t trackidx);
void psy_audio_sequence_unmutetrack(psy_audio_Sequence*, uintptr_t trackidx);
int psy_audio_sequence_istrackmuted(const psy_audio_Sequence*, uintptr_t trackidx);
int psy_audio_sequence_istracksoloed(const psy_audio_Sequence*, uintptr_t trackidx);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCE_H */