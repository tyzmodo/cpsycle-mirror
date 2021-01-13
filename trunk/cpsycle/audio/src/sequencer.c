// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sequencer.h"
// local
#include "exclusivelock.h"
#include "instruments.h"
#include "pattern.h"
// container
#include <qsort.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#include <math.h>

#define QSORTARRAYRESIZE 1024

static void psy_audio_sequencerjump_init(psy_audio_SequencerJump* self)
{
	assert(self);
	
	self->dostop = 0;
	self->active = 0;
	self->offset = (psy_dsp_big_beat_t) 0.f;
}

static void psy_audio_sequencerjump_activate(psy_audio_SequencerJump* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	self->active = 1;
	self->offset = offset;
}

static void psy_audio_sequencerloop_init(psy_audio_SequencerLoop* self)
{
	assert(self);

	self->active = 0;
	self->count = 0;
	self->offset = (psy_dsp_big_beat_t) 0.f;
}

static void psy_audio_sequencerrowdelay_init(psy_audio_SequencerRowDelay* self)
{
	assert(self);

	self->active = 0;
	// line delay
	self->rowspeed = (psy_dsp_big_beat_t) 1.f;
}


static void psy_audio_sequencer_init_qsortarray(psy_audio_Sequencer*);
static void psy_audio_sequencer_reset_common(psy_audio_Sequencer*,
	psy_audio_Sequence*, psy_audio_Machines*, psy_dsp_big_hz_t samplerate);
static void psy_audio_sequencer_clearevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_cleardelayed(psy_audio_Sequencer*);
static void psy_audio_sequencer_clearinputevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_makecurrtracks(psy_audio_Sequencer*,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_setbarloop(psy_audio_Sequencer*);
static psy_dsp_big_beat_t psy_audio_sequencer_skiptrackiterators(
	psy_audio_Sequencer*, psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_clearcurrtracks(psy_audio_Sequencer*);
static void psy_audio_sequencer_advanceposition(psy_audio_Sequencer*,
	psy_dsp_big_beat_t width);
static void psy_audio_sequencer_updateplaymodeposition(psy_audio_Sequencer*);
static void psy_audio_sequencer_addsequenceevent(psy_audio_Sequencer*, 
	psy_audio_PatternEntry*,  psy_audio_SequencerTrack*,
	psy_dsp_big_beat_t offset);
static void psy_audio_addgate(psy_audio_Sequencer*, psy_audio_PatternEntry*);
static void psy_audio_sequencer_maketweakslideevents(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, uintptr_t channeloffset,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_makeretriggerevents(psy_audio_Sequencer*,
	psy_audio_SequencerTrack*, psy_audio_PatternEntry*, psy_dsp_big_beat_t
	offset);
static void psy_audio_sequencer_makeretriggercontinueevents(
	psy_audio_Sequencer*, psy_audio_SequencerTrack*, psy_audio_PatternEntry*,
	psy_dsp_big_beat_t offset);
static int psy_audio_sequencer_isoffsetinwindow(psy_audio_Sequencer*,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_insertevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_insertinputevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_insertdelayedevents(psy_audio_Sequencer*);
static int psy_audio_sequencer_sequencerinsert(psy_audio_Sequencer*);
static void psy_audio_sequencer_compute_beatspersample(psy_audio_Sequencer*);
static void psy_audio_sequencer_notifysequencertick(psy_audio_Sequencer*,
	psy_dsp_big_beat_t width);
static uintptr_t psy_audio_sequencer_currframesperline(psy_audio_Sequencer*);
static void psy_audio_sequencer_sortevents(psy_audio_Sequencer*);
static void psy_audio_sequencer_resizeqsortarray(psy_audio_Sequencer*,
	uintptr_t numevents);
static int psy_audio_sequencer_comp_events(psy_audio_PatternNode* lhs,
	psy_audio_PatternNode* rhs);
static void psy_audio_sequencer_assertorder(psy_audio_Sequencer*);
static void psy_audio_sequencer_jumpto(psy_audio_Sequencer*,
	psy_dsp_big_beat_t position);
static void psy_audio_sequencer_stopat(psy_audio_Sequencer*,
	psy_dsp_big_beat_t position);
static void psy_audio_sequencer_executejump(psy_audio_Sequencer*);
static void psy_audio_sequencer_notedelay(psy_audio_Sequencer*,
	psy_audio_PatternEntry*,
	uintptr_t channeloffset, 
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_retrigger(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_audio_SequencerTrack*,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_retriggercont(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_audio_SequencerTrack*,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_note(psy_audio_Sequencer*,
	psy_audio_PatternEntry* patternentry,
	uintptr_t channeloffset,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_tweak(psy_audio_Sequencer*,
	psy_audio_PatternEntry*,
	uintptr_t channeloffset,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_executeline(psy_audio_Sequencer*,
	psy_audio_SequencerTrack*, psy_dsp_big_beat_t offset);
static bool psy_audio_sequencer_executeglobalcommands(psy_audio_Sequencer*,
	psy_audio_PatternEntry*, psy_audio_SequencerTrack*,
	psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_patterndelay(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*);
void psy_audio_sequencer_finepatterndelay(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*);
static void psy_audio_sequencer_patternloop(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*, psy_dsp_big_beat_t offset);
static void psy_audio_sequencer_jumptoorder(psy_audio_Sequencer*,
	const psy_audio_PatternEvent*);
static void psy_audio_sequencer_breaktoline(psy_audio_Sequencer*,
	psy_audio_SequenceTrackIterator*, const psy_audio_PatternEvent*);

void psy_audio_sequencer_init(psy_audio_Sequencer* self, psy_audio_Sequence*
	sequence, psy_audio_Machines* machines)
{
	assert(self);

	self->events = NULL;
	self->delayedevents = NULL;
	self->inputevents = NULL;
	self->currtracks = NULL;	
	psy_audio_sequencer_init_qsortarray(self);
	psy_table_init(&self->lastmachine);
	psy_signal_init(&self->signal_newline);	
	psy_audio_sequencer_reset_common(self, sequence, machines,
		(psy_dsp_big_hz_t)44100.0);
	psy_audio_sequencertime_init(&self->seqtime);
}

void psy_audio_sequencer_init_qsortarray(psy_audio_Sequencer* self)
{
	self->qsortarray = malloc(sizeof(psy_audio_PatternNode*) *
		QSORTARRAYRESIZE);	
	self->qsortarraysize = QSORTARRAYRESIZE;
}

void psy_audio_sequencer_dispose(psy_audio_Sequencer* self)
{
	assert(self);

	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_cleardelayed(self);
	psy_audio_sequencer_clearinputevents(self);
	psy_audio_sequencer_clearcurrtracks(self);	
	psy_table_dispose(&self->lastmachine);
	psy_signal_dispose(&self->signal_newline);
	free(self->qsortarray);
	self->qsortarray = NULL;
	self->sequence = NULL;
	self->machines = NULL;
}

void psy_audio_sequencer_reset(psy_audio_Sequencer* self, psy_audio_Sequence*
	sequence, psy_audio_Machines* machines, psy_dsp_big_hz_t samplerate)
{
	assert(self);

	psy_audio_sequencer_reset_common(self, sequence, machines, samplerate);	
	psy_table_clear(&self->lastmachine);
}

void psy_audio_sequencer_reset_common(psy_audio_Sequencer* self,
	psy_audio_Sequence* sequence, psy_audio_Machines* machines,
	psy_dsp_big_hz_t samplerate)
{
	assert(self);
	
	self->sequence = sequence;
	self->machines = machines;
	psy_audio_sequencertime_init(&self->seqtime);
	self->seqtime.samplerate = samplerate;
	self->numsongtracks = 16;	
	self->lpb = 4;
	self->lpbspeed = (psy_dsp_big_beat_t)1.0;
	self->playing = FALSE;
	self->looping = TRUE;
	self->numplaybeats = (psy_dsp_big_beat_t)4.0;	
	self->window = (psy_dsp_big_beat_t)0.0;
	self->linetickcount = (psy_dsp_big_beat_t)0.0;
	self->mode = psy_audio_SEQUENCERPLAYMODE_PLAYALL;
	self->calcduration = FALSE;
	self->currrowposition = psy_INDEX_INVALID;
	self->extraticks = 0;
	self->tpb = 24;
	self->playtrack = psy_INDEX_INVALID;
	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_cleardelayed(self);
	psy_audio_sequencer_clearinputevents(self);
	psy_audio_sequencer_clearcurrtracks(self);
	psy_audio_sequencer_makecurrtracks(self, (psy_dsp_big_beat_t)0.0);
	psy_audio_sequencer_compute_beatspersample(self);
	psy_audio_sequencerjump_init(&self->jump);
	psy_audio_sequencerrowdelay_init(&self->rowdelay);
	psy_audio_sequencerloop_init(&self->loop);
}

void psy_audio_sequencer_setposition(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_cleardelayed(self);
	psy_audio_sequencer_clearcurrtracks(self);
	self->seqtime.position = offset;
	// todo only estimated sample pos on current bpm
	self->seqtime.playcounter = psy_audio_sequencer_frames(self,
		offset);
	self->seqtime.lastbarposition = floor(offset / 4) * 4;
	self->linetickcount = 0.0;		
	self->window = (psy_dsp_big_beat_t)0.0;
	psy_audio_sequencer_makecurrtracks(self, offset);
}

void psy_audio_sequencer_start(psy_audio_Sequencer* self)
{	
	assert(self);

	psy_audio_sequencerjump_init(&self->jump);
	psy_audio_sequencerrowdelay_init(&self->rowdelay);
	psy_audio_sequencerloop_init(&self->loop);
	self->lpbspeed = (psy_dsp_big_beat_t)1.0;
	psy_audio_sequencer_compute_beatspersample(self);
	self->linetickcount = 0.0;
	if (self->mode == psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS) {
		psy_audio_sequencer_setbarloop(self);		
	}	
	self->playing = TRUE;
}

void psy_audio_sequencer_setbarloop(psy_audio_Sequencer* self)
{	
	assert(self);

	self->playbeatloopstart = self->seqtime.position;
	self->playbeatloopend = self->playbeatloopstart +
		(psy_dsp_big_beat_t)self->numplaybeats;
	psy_audio_sequencer_setposition(self, self->playbeatloopstart);
}

void psy_audio_sequencer_setnumplaybeats(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t num)
{
	assert(self);

	self->numplaybeats = num;
	self->playbeatloopend = self->playbeatloopstart + num;
}

void psy_audio_sequencer_setsamplerate(psy_audio_Sequencer* self,
	psy_dsp_big_hz_t samplerate)
{
	assert(self);

	self->seqtime.samplerate = samplerate;
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_setbpm(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t bpm)
{	
	if (bpm < 32) {
		self->seqtime.bpm = 32;
	} else
	if (bpm > 999) {
		self->seqtime.bpm = 999;
	} else {
		self->seqtime.bpm = bpm;
	}
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_setlpb(psy_audio_Sequencer* self, uintptr_t lpb)
{	
	self->lpb = lpb;
	self->lpbspeed = (psy_dsp_big_beat_t)1.0;
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_setticksperbeat(psy_audio_Sequencer* self,
	uintptr_t ticks)
{
	self->tpb = ticks;
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_setextraticksperbeat(psy_audio_Sequencer* self,
	uintptr_t ticks)
{
	self->extraticks = ticks;
	psy_audio_sequencer_compute_beatspersample(self);
}

psy_audio_PatternNode* psy_audio_sequencer_machinetickevents(
	psy_audio_Sequencer* self, uintptr_t slot)
{
	psy_audio_PatternNode* rv = 0;
	psy_audio_PatternNode* p;

	assert(self);
		
	for (p = self->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry = psy_audio_patternnode_entry(p);
		
		if (psy_audio_patternentry_front(entry)->mach == slot) {
			if (self->playtrack == psy_INDEX_INVALID ||
					self->playtrack == entry->track) {
				psy_list_append(&rv, entry);
			}
		}		
	}
	return rv;
}

void psy_audio_sequencer_clearcurrtracks(psy_audio_Sequencer* self)
{
	psy_List* p;

	assert(self);

	for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
		psy_audio_SequencerTrack* track;

		track = (psy_audio_SequencerTrack*) p->entry;
		free(track->iterator);
		free(track);
	}
	psy_list_free(self->currtracks);
	self->currtracks = 0;
}

void psy_audio_sequencer_makecurrtracks(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t offset)
{
	psy_audio_SequenceTrackNode* p;
	uintptr_t trackindex;

	assert(self);

	for (p = self->sequence->tracks, trackindex = 0; p != NULL;
			psy_list_next(&p), ++trackindex) {
		psy_audio_SequencerTrack* track;

		track = malloc(sizeof(psy_audio_SequencerTrack));
		if (track) {			
			track->track = (psy_audio_SequenceTrack*)p->entry;
			track->channeloffset = trackindex * 64;
			track->iterator = (psy_audio_SequenceTrackIterator*)
				malloc(sizeof(psy_audio_SequenceTrackIterator));
			if (track->iterator) {
				*track->iterator = psy_audio_sequence_begin(self->sequence, p,
					offset);
				track->state.retriggeroffset = 0;
				track->state.retriggerstep = 0;
				psy_list_append(&self->currtracks, track);
			}
		}
	}
}

psy_dsp_big_beat_t psy_audio_sequencer_skiptrackiterators(
	psy_audio_Sequencer* self, psy_dsp_big_beat_t offset)
{
	psy_List* p;
	int first;
	psy_dsp_big_beat_t newplayposition;

	assert(self);
		
	first = 1;
	newplayposition = offset;
	for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
		psy_audio_SequencerTrack* track;
		psy_audio_SequenceTrackIterator* it;
		int skip = 0;

		track = (psy_audio_SequencerTrack*) p->entry;
		it = track->iterator;
		while (it->sequencentrynode &&
				!psy_audio_sequencetrackiterator_entry(it)->selplay) {
			psy_audio_sequencetrackiterator_inc_entry(it);			
			skip = 1;
		}
		if (first && it->sequencentrynode && skip) {
			newplayposition = psy_audio_sequencetrackiterator_offset(it);
		}		
	}
	return newplayposition;
}

void psy_audio_sequencer_clearevents(psy_audio_Sequencer* self)
{
	assert(self);

	psy_list_deallocate(&self->events, (psy_fp_disposefunc)
		psy_audio_patternentry_dispose);	
}

void psy_audio_sequencer_cleardelayed(psy_audio_Sequencer* self)
{	
	assert(self);

	psy_list_deallocate(&self->delayedevents, (psy_fp_disposefunc)
		psy_audio_patternentry_dispose);	
}

void psy_audio_sequencer_clearinputevents(psy_audio_Sequencer* self)
{
	psy_list_deallocate(&self->inputevents, (psy_fp_disposefunc)
		psy_audio_patternentry_dispose);	
}

void psy_audio_sequencer_frametick(psy_audio_Sequencer* self, uintptr_t
	numframes)
{	
	assert(self);

	if (self->playing) {		
		self->seqtime.playcounter += numframes;		
	}	
	psy_audio_sequencer_tick(self, psy_audio_sequencer_frametooffset(self,
		numframes));
}

void psy_audio_sequencer_tick(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t width)
{
	assert(self);

	if (psy_audio_sequencer_playing(self)) {		
		self->seqtime.lastbarposition = floor(self->seqtime.position / 4) * 4;
		psy_audio_sequencer_advanceposition(self, width);		
	}
	psy_audio_sequencer_clearevents(self);
	psy_audio_sequencer_insertinputevents(self);
	if (psy_audio_sequencer_playing(self)) {
		psy_audio_sequencer_updateplaymodeposition(self);
		psy_audio_sequencer_insertevents(self);
		psy_audio_sequencer_insertdelayedevents(self);
	}
	psy_audio_sequencer_notifysequencertick(self, width);
	if (psy_audio_sequencer_playing(self) &&
			psy_audio_sequencer_sequencerinsert(self)) {
		psy_audio_sequencer_insertdelayedevents(self);
	}	
	psy_audio_sequencer_sortevents(self);	
}

void psy_audio_sequencer_advanceposition(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t width)
{	
	assert(self);

	self->seqtime.position += self->window;
	self->window = width;	
}

void psy_audio_sequencer_updateplaymodeposition(psy_audio_Sequencer* self)
{
	assert(self);

	switch (psy_audio_sequencer_playmode(self)) {
		case psy_audio_SEQUENCERPLAYMODE_PLAYSEL:
			self->seqtime.position =
				psy_audio_sequencer_skiptrackiterators(self,
					self->seqtime.position);
			break;
		case psy_audio_SEQUENCERPLAYMODE_PLAYNUMBEATS: {
			if (self->seqtime.position >= self->playbeatloopend -
				1.f / (psy_dsp_big_beat_t)self->lpb) {
				if (self->looping) {
					psy_audio_sequencer_jumpto(self, self->playbeatloopstart);
				} else {
					psy_audio_sequencer_stopat(self, self->playbeatloopstart);
				}
			}
			break; }
		default:
			break;
	}
}

void psy_audio_sequencer_notifysequencertick(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t width)
{
	assert(self);

	if (self->machines) {
		psy_TableIterator it;

		for (it = psy_audio_machines_begin(self->machines);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_machine_sequencertick((psy_audio_Machine*)
				psy_tableiterator_value(&it));
		}
	}
}

uintptr_t psy_audio_sequencer_updatelinetickcount(psy_audio_Sequencer* self,
	uintptr_t amount)
{
	uintptr_t rv;

	assert(self);

	self->linetickcount -= psy_audio_sequencer_frametooffset(self, amount);
	if (self->linetickcount <= 0) {
		rv = psy_audio_sequencer_frames(self, -self->linetickcount);
	} else {
		rv = amount;
	}
	return rv;
}

void psy_audio_sequencer_clockstart(psy_audio_Sequencer* self)
{	
	// todo lock
	psy_audio_sequencer_stop(self);
	psy_audio_sequencer_setposition(self, (psy_dsp_big_beat_t)0.0);
	psy_audio_sequencer_start(self);
}

void psy_audio_sequencer_clock(psy_audio_Sequencer* self)
{	
	// todo sync
}

void psy_audio_sequencer_clockcontinue(psy_audio_Sequencer* self)
{	
	if (!psy_audio_sequencer_playing(self)) {
		psy_audio_sequencer_start(self);
	}
}

void psy_audio_sequencer_clockstop(psy_audio_Sequencer* self)
{
	psy_audio_sequencer_stop(self);
}

void psy_audio_sequencer_onnewline(psy_audio_Sequencer* self)
{
	assert(self);

	if (self->jump.dostop) {
		self->jump.active = 0;
		psy_audio_sequencer_stop(self);
	}
	if (self->jump.active) {
		psy_audio_sequencer_executejump(self);
	}
	if (self->rowdelay.active) {
		self->rowdelay.active = 0;
		psy_audio_sequencer_compute_beatspersample(self);
	}
	psy_signal_emit(&self->signal_newline, self, 0);	
	self->linetickcount += psy_audio_sequencer_currbeatsperline(self);
}

void psy_audio_sequencer_executejump(psy_audio_Sequencer* self)
{
	assert(self);

	self->jump.active = 0;
	psy_audio_sequencer_setposition(self, self->jump.offset);
}

int psy_audio_sequencer_sequencerinsert(psy_audio_Sequencer* self) {
	psy_audio_PatternNode* p;
	int rv = 0;

	assert(self);
	
	for (p = psy_audio_sequencer_tickevents(self); p != NULL;
			psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* ev;
		
		entry = psy_audio_patternnode_entry(p);
		ev = psy_audio_patternentry_front(entry);
		if (ev->mach != psy_audio_NOTECOMMANDS_EMPTY && self->machines) {
			psy_audio_Machine* machine;				
				
			machine = psy_audio_machines_at(self->machines, ev->mach);
			if (machine) {
				psy_audio_PatternNode* events;

				events = psy_audio_sequencer_machinetickevents(self, ev->mach);
				if (events) {					
					psy_audio_PatternNode* insert;
					
					insert = psy_audio_machine_sequencerinsert(machine,
						events);
					if (insert) {
						psy_audio_sequencer_append(self, insert);
						psy_list_free(insert);
						rv = 1;
					}
					psy_list_free(events);
				}					
			}									
		}
	}
	return rv;
}

void psy_audio_sequencer_insertevents(psy_audio_Sequencer* self)
{	
	bool continueplaying = FALSE;
	bool work = TRUE;

	assert(self);

	while (work) {
		psy_List* p;

		work = FALSE;		
		for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
			psy_audio_SequencerTrack* track;
			psy_dsp_big_beat_t offset;

			track = (psy_audio_SequencerTrack*) p->entry;
			if (track->iterator->sequencentrynode) {
				psy_audio_SequenceEntry* sequenceentry;
				psy_audio_Pattern* pattern;

				sequenceentry = psy_audio_sequencetrackiterator_entry(
					track->iterator);
				pattern = psy_audio_patterns_at(track->iterator->patterns,
					sequenceentry->patternslot);
				if (pattern) {
					if (self->seqtime.position + self->window <
							sequenceentry->offset + pattern->length) {
						self->currrowposition = sequenceentry->row;
						continueplaying = TRUE;
					} else if (track->iterator->sequencentrynode->next) {
						psy_audio_sequencetrackiterator_inc_entry(
							track->iterator);
						sequenceentry = psy_audio_sequencetrackiterator_entry(
							track->iterator);
						if (self->seqtime.position <= sequenceentry->offset
								+ pattern->length) {
							self->currrowposition = sequenceentry->row;
							continueplaying = TRUE;
						}
					}
				}				
				if (track->iterator->patternnode) {					
					offset = psy_audio_sequencetrackiterator_offset(
						track->iterator);
					if (psy_audio_sequencer_isoffsetinwindow(self, offset)) {
						psy_audio_sequencer_executeline(self, track, offset);						
						work = TRUE;
					}
				}
			}
		}		
	}	
	if (self->looping && !continueplaying) {
		psy_audio_sequencer_setposition(self, 0.f);
	} else {
		self->playing = continueplaying;
	}
}

void psy_audio_sequencer_executeline(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track,
	psy_dsp_big_beat_t offset)
{
	psy_dsp_big_beat_t rowoffset;
	psy_audio_PatternNode* events;
	psy_audio_PatternNode* p;

	assert(self);

	// First execute global events on the same offset (e.g. tracker row start)
	// before adding them to the current event list that time changes are
	// applied to all of them independend of the channel order
	rowoffset = offset;
	events = NULL;
	while (track->iterator->patternnode && rowoffset ==
			psy_audio_sequencetrackiterator_offset(track->iterator)) {
		psy_audio_PatternEntry* entry;

		entry = psy_audio_sequencetrackiterator_patternentry(track->iterator);
		if (entry && 
				!psy_audio_sequence_istrackmuted(self->sequence,
					entry->track) &&
				!psy_audio_patterns_istrackmuted(self->sequence->patterns,
					entry->track)) {
			psy_audio_sequencer_executeglobalcommands(self,
				entry, track, offset);
			psy_list_append(&events, entry);
		}
		psy_audio_sequencetrackiterator_inc(track->iterator);		
	}
	// global events are applied, now execute the events
	for (p = events; p != NULL; psy_list_next(&p)) {
		psy_audio_sequencer_addsequenceevent(self,
			psy_audio_patternnode_entry(p), track, rowoffset);
	}
	psy_list_free(events);
}

bool psy_audio_sequencer_executeglobalcommands(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_audio_SequencerTrack* track,
	psy_dsp_big_beat_t offset)
{
	psy_audio_PatternNode* p;
	bool done;

	assert(self);
	
	done = FALSE;
	for (p = patternentry->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*) p->entry;
		if (ev->note < psy_audio_NOTECOMMANDS_TWEAK ||
				ev->note == psy_audio_NOTECOMMANDS_EMPTY) {
			switch (ev->cmd) {
				case psy_audio_PATTERNCMD_EXTENDED:
					if ((ev->parameter & 0xF0) ==
							psy_audio_PATTERNCMD_PATTERN_DELAY) {
						psy_audio_sequencer_patterndelay(self, ev);
						done = TRUE;
					} else
					if ((ev->parameter & 0xF0) ==
							psy_audio_PATTERNCMD_FINE_PATTERN_DELAY) {
						psy_audio_sequencer_finepatterndelay(self, ev);
						done = TRUE;
					} else
					if (ev->parameter <
							psy_audio_PATTERNCMD_SET_LINESPERBEAT1) {
						self->lpbspeed = ev->parameter /
							(psy_dsp_big_beat_t)self->lpb;						
						psy_audio_sequencer_compute_beatspersample(self);
						done = TRUE;
					}
				break;
				case psy_audio_PATTERNCMD_SET_TEMPO:
					self->seqtime.bpm = ev->parameter;
					psy_audio_sequencer_compute_beatspersample(self);
					done = TRUE;
				break;
				case psy_audio_PATTERNCMD_JUMP_TO_ORDER:
					if (!self->calcduration) {
						psy_audio_sequencer_jumptoorder(self, ev);
					}
					done = TRUE;
				break;
				case psy_audio_PATTERNCMD_BREAK_TO_LINE:
					if (!self->calcduration) {
						psy_audio_sequencer_breaktoline(self,
							track->iterator, ev);						
					}
					done = TRUE;
				break;
				default:
				break;
			}
		}
	}	
	return done;
}

void psy_audio_sequencer_patterndelay(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev)
{
	psy_dsp_big_beat_t rows;

	assert(self);

	rows = (psy_dsp_big_beat_t)(ev->parameter & 0x0F);
	if (rows > 0) {
		self->rowdelay.active = 1;
		self->rowdelay.rowspeed = (psy_dsp_big_beat_t)1.f / rows;		
	}
	else {
		self->rowdelay.rowspeed = (psy_dsp_big_beat_t)1.f;
		self->rowdelay.active = 0;
	}
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_finepatterndelay(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev)
{
	psy_dsp_big_beat_t ticks;

	assert(self);

	ticks = (psy_dsp_big_beat_t)(ev->parameter & 0x0F);
	self->rowdelay.active = 1;
	self->rowdelay.rowspeed =
		(psy_dsp_big_beat_t) 0.5 / 15 * (psy_dsp_big_beat_t)(30 - ticks);	
	psy_audio_sequencer_compute_beatspersample(self);
}

void psy_audio_sequencer_patternloop(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev, psy_dsp_big_beat_t offset)
{
	assert(self);

	if (!self->loop.active) {
		self->loop.count = ev->parameter & 0x0F;
		if (self->loop.count > 0) {
			self->loop.active = 1;
			psy_audio_sequencer_jumpto(self, self->loop.offset);
		} else {
			self->loop.offset = offset;
		}
	} else if (self->loop.count > 0 && offset != self->loop.offset) {
		--self->loop.count;
		if (self->loop.count > 0) {
			psy_audio_sequencer_jumpto(self, self->loop.offset);
		} else {
			self->loop.active = 0;
			self->loop.offset = offset +
				(psy_dsp_big_beat_t)1.f / self->lpb;
		}
	} else if (self->loop.count == 0) {
		self->loop.active = 0;
		self->loop.offset = offset;
	}
}

void psy_audio_sequencer_jumptoorder(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev)
{
	psy_audio_SequencePosition position;

	assert(self);

	position = psy_audio_sequence_at(self->sequence, 0, ev->parameter);
	if (position.trackposition.sequencentrynode) {
		psy_audio_SequenceEntry* orderentry;

		orderentry = (psy_audio_SequenceEntry*)
			position.trackposition.sequencentrynode->entry;
		psy_audio_sequencer_jumpto(self, orderentry->offset);
	}
}

void psy_audio_sequencer_breaktoline(psy_audio_Sequencer* self,
	psy_audio_SequenceTrackIterator* it, const psy_audio_PatternEvent* ev)
{
	psy_audio_SequenceEntryNode* next = it->sequencentrynode->next;

	assert(self);

	if (next) {
		psy_audio_SequenceEntry* orderentry;

		orderentry = (psy_audio_SequenceEntry*) next->entry;
		psy_audio_sequencer_jumpto(self, orderentry->offset + ev->parameter *
			((psy_dsp_big_beat_t)1.f / self->lpb));
	}
}

void psy_audio_sequencer_append(psy_audio_Sequencer* self, psy_List* events)
{
	psy_audio_PatternNode* p;

	assert(self);

	for (p = events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;

		entry = psy_audio_patternnode_entry(p);
		entry->delta += self->seqtime.position;
		psy_list_append(&self->delayedevents, entry);		
	}
}

INLINE int psy_audio_sequencer_isoffsetinwindow(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t offset)
{
	assert(self);

	return psy_dsp_testrange(offset, self->seqtime.position, self->window);		
}

void psy_audio_sequencer_addsequenceevent(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry,
	psy_audio_SequencerTrack* track, psy_dsp_big_beat_t offset)
{	
	psy_audio_PatternEvent* ev;

	assert(self);	

	ev = psy_audio_patternentry_front(patternentry);
	if (ev->note == psy_audio_NOTECOMMANDS_TWEAKSLIDE) {
		psy_audio_sequencer_maketweakslideevents(self, patternentry,
			track->channeloffset, offset);
	} else if (ev->note < psy_audio_NOTECOMMANDS_TWEAK ||
			ev->note == psy_audio_NOTECOMMANDS_EMPTY) {
		if (self->machines) {
			psy_audio_Machine* machine;

			machine = psy_audio_machines_at(self->machines, ev->mach);
			if (!machine) {
				if (ev->mach == psy_audio_NOTECOMMANDS_MACH_EMPTY) {
					uintptr_t lastmachine;

					lastmachine = (uintptr_t)psy_table_at(&self->lastmachine,
						patternentry->track);
					machine = psy_audio_machines_at(self->machines,
						lastmachine);
				}
			}
			// Does this machine really exist and is not muted?
			if (machine && !psy_audio_machine_muted(machine)) {
				if (ev->cmd == psy_audio_PATTERNCMD_NOTE_DELAY) {
					psy_audio_sequencer_notedelay(self, patternentry,
						track->channeloffset, offset);
				} else if (ev->cmd == psy_audio_PATTERNCMD_RETRIGGER) {
					psy_audio_sequencer_retrigger(self, patternentry, track,
						offset);
				} else if (ev->cmd == psy_audio_PATTERNCMD_RETR_CONT) {
					psy_audio_sequencer_retriggercont(self, patternentry,
						track, offset);
				} else if (ev->cmd != psy_audio_PATTERNCMD_SET_VOLUME) {
					psy_audio_sequencer_note(self, patternentry,
						track->channeloffset, offset);
				}
			} 
		}
		if (ev->cmd == psy_audio_PATTERNCMD_SET_VOLUME) {
			psy_audio_PatternEntry* entry;
			psy_audio_PatternEvent* newev;
			
			entry = psy_audio_patternentry_clone(patternentry);
			newev = psy_audio_patternentry_front(entry);
			entry->track += track->channeloffset;
			// volume column used to store mach
			newev->vol = (newev->mach == psy_audio_NOTECOMMANDS_MACH_EMPTY)
				? psy_audio_MASTER_INDEX
				: newev->mach;
			// master handles all wire's volumes
			newev->mach = psy_audio_MASTER_INDEX;
			psy_audio_patternentry_setbpm(entry, self->seqtime.bpm);
			entry->delta = offset - self->seqtime.position;
			psy_list_append(&self->events, entry);		
		}		
	} else if (ev->note == psy_audio_NOTECOMMANDS_TWEAK) {
		psy_audio_sequencer_tweak(self, patternentry, track->channeloffset,
			offset);
	} else if ((ev->note == psy_audio_NOTECOMMANDS_MIDICC) &&
			(ev->inst < 0x80)) {
		psy_audio_PatternEntry* entry;
		psy_audio_PatternEvent* newev;

		entry = psy_audio_patternentry_clone(patternentry);
		newev = psy_audio_patternentry_front(entry);
		psy_audio_patternentry_setbpm(entry, self->seqtime.bpm);
		entry->delta = offset - self->seqtime.position;
		entry->track = newev->inst;
		entry->track += track->channeloffset;
		newev->note = psy_audio_NOTECOMMANDS_EMPTY;
		newev->inst = psy_audio_NOTECOMMANDS_INST_EMPTY;
		psy_list_append(&self->events, entry);
	}
}

void psy_audio_sequencer_notedelay(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry,
	uintptr_t channeloffset,
	psy_dsp_big_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	assert(self);

	entry = psy_audio_patternentry_clone(patternentry);
	entry->track += channeloffset;
	psy_audio_patternentry_setbpm(entry, self->seqtime.bpm);
	entry->delta = offset + psy_audio_patternentry_front(entry)->parameter *
		(psy_audio_sequencer_currbeatsperline(self) / 256.f);
	psy_list_append(&self->delayedevents, entry);
}

void psy_audio_sequencer_retrigger(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_audio_SequencerTrack* track,
	psy_dsp_big_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	assert(self);

	entry = psy_audio_patternentry_clone(patternentry);
	entry->track += track->channeloffset;
	psy_audio_patternentry_setbpm(entry, self->seqtime.bpm);
	entry->delta = offset - self->seqtime.position;
	psy_list_append(&self->events, entry);
	psy_audio_sequencer_makeretriggerevents(self, track, entry, offset);
}

void psy_audio_sequencer_retriggercont(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry, psy_audio_SequencerTrack* track,
	psy_dsp_big_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	assert(self);

	entry = psy_audio_patternentry_clone(patternentry);
	entry->track += track->channeloffset;
	psy_audio_patternentry_setbpm(entry, self->seqtime.bpm);
	entry->delta = offset + track->state.retriggeroffset;
	psy_list_append(&self->delayedevents, entry);
	psy_audio_sequencer_makeretriggercontinueevents(self, track, entry,
		offset);
}

void psy_audio_sequencer_note(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry,
	uintptr_t channeloffset,
	psy_dsp_big_beat_t offset)
{
	psy_audio_PatternEntry* entry;
	psy_audio_PatternEvent* ev;

	assert(self);

	entry = psy_audio_patternentry_clone(patternentry);
	ev = psy_audio_patternentry_front(entry);
	entry->track += channeloffset;
	if (ev->mach == psy_audio_NOTECOMMANDS_MACH_EMPTY) {
		uintptr_t lastmachine;

		lastmachine = (uintptr_t)psy_table_at(&self->lastmachine,
			patternentry->track);
		ev->mach = (uint8_t)lastmachine;
	}
	psy_audio_patternentry_setbpm(entry, self->seqtime.bpm);
	entry->delta = offset - self->seqtime.position;
	if (ev->note < psy_audio_NOTECOMMANDS_RELEASE) {
		psy_table_insert(&self->lastmachine, entry->track,
			(void*)(uintptr_t)ev->mach);
	}
	psy_list_append(&self->events, entry);
}

void psy_audio_sequencer_tweak(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* patternentry,
	uintptr_t channeloffset,
	psy_dsp_big_beat_t offset)
{
	psy_audio_PatternEntry* entry;

	assert(self);

	entry = psy_audio_patternentry_clone(patternentry);
	entry->track += channeloffset;
	psy_audio_patternentry_setbpm(entry, self->seqtime.bpm);
	entry->delta = offset - self->seqtime.position;
	psy_audio_patternentry_front(entry)->vol = 0;
	psy_list_append(&self->events, entry);
}

void psy_audio_addgate(psy_audio_Sequencer* self, psy_audio_PatternEntry*
	entry)
{
	uint8_t gate;
	psy_audio_PatternNode* p;	

	assert(self);
			
	gate = (uint8_t) psy_audio_NOTECOMMANDS_GATE_EMPTY;
	p = entry->events;
	while (p != NULL) {
		psy_audio_PatternEvent* event;

		event = (psy_audio_PatternEvent*)p->entry;
		if (event->cmd == psy_audio_PATTERNCMD_GATE) {
			gate = event->parameter;
			break;
		}
		psy_list_next(&p);
	}
	if (gate != psy_audio_NOTECOMMANDS_GATE_EMPTY) {		
		psy_audio_PatternEntry* noteoff;
		psy_audio_PatternEvent* noteoffev;

		noteoff = psy_audio_patternentry_allocinit();
		noteoffev = psy_audio_patternentry_front(noteoff);
		noteoffev->note = psy_audio_NOTECOMMANDS_RELEASE;
		noteoffev->mach = psy_audio_patternentry_front(entry)->mach;
		noteoff->delta += entry->offset + gate / 
			(self->lpb * psy_audio_sequencer_speed(self) * 128.f);
		psy_list_append(&self->delayedevents, noteoff);
	}
}

void psy_audio_sequencer_maketweakslideevents(psy_audio_Sequencer* self,
	psy_audio_PatternEntry* entry, uintptr_t channeloffset,
	psy_dsp_big_beat_t offset)
{
	psy_audio_PatternEvent* ev;
	psy_audio_Machine* machine;
	uintptr_t tweak;
	psy_audio_MachineParam* param;
	uintptr_t slide;
	uintptr_t framesperslide = 64;
	uintptr_t numslides;
	
	assert(self);

	if (!self->machines) {
		return;
	}
	ev = psy_audio_patternentry_front(entry);
	machine = psy_audio_machines_at(self->machines, ev->mach);
	if (!machine) {
		return;
	}
	tweak = ev->inst;
	if (tweak >= psy_audio_machine_numtweakparameters(machine)) {
		return;	
	}
	param = psy_audio_machine_tweakparameter(machine, tweak);
	if (!param) {
		return;
	}	
	numslides = psy_audio_sequencer_currframesperline(self) / framesperslide;
	if (numslides == 0) {
		return;
	}
	if (numslides > UINT16_MAX) {
		numslides = UINT16_MAX;
	}			
	for (slide = 0; slide < numslides; ++slide) {
		psy_audio_PatternEntry* slideentry;
		psy_audio_PatternEvent* slideev;
				
		slideentry = psy_audio_patternentry_clone(entry);
		slideev = psy_audio_patternentry_front(slideentry);
		slideentry->track += channeloffset;
		slideev->note = psy_audio_NOTECOMMANDS_TWEAK;
		slideev->vol = (uint16_t)(numslides - slide);
		psy_audio_patternentry_setbpm(slideentry, self->seqtime.bpm);
		slideentry->delta = offset +
			psy_audio_sequencer_frametooffset(self, slide * framesperslide);
		slideentry->priority = 1; // not used right now
		psy_list_append(&self->delayedevents, slideentry);
	}			
}

void psy_audio_sequencer_makeretriggerevents(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track, psy_audio_PatternEntry* entry,
	psy_dsp_big_beat_t offset)
{
	psy_dsp_big_beat_t retriggerstep;
	psy_dsp_big_beat_t retriggeroffset;

	assert(self);
	
	retriggerstep = psy_audio_patternentry_front(entry)->parameter *
		(psy_audio_sequencer_currbeatsperline(self) / 256.f);		
	retriggeroffset = retriggerstep;
	while (retriggeroffset < psy_audio_sequencer_currbeatsperline(self)) {
		psy_audio_PatternEntry* retriggerentry;
		
		retriggerentry = psy_audio_patternentry_clone(entry);
		psy_audio_patternentry_front(retriggerentry)->cmd = 0;
		psy_audio_patternentry_front(retriggerentry)->parameter = 0;
		retriggerentry->delta = offset + entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset = retriggeroffset -
		psy_audio_sequencer_currbeatsperline(self);
	track->state.retriggerstep = retriggerstep;
}

void psy_audio_sequencer_makeretriggercontinueevents(psy_audio_Sequencer* self,
	psy_audio_SequencerTrack* track, psy_audio_PatternEntry* entry,
	psy_dsp_big_beat_t offset)
{
	psy_dsp_big_beat_t retriggerstep;
	psy_dsp_big_beat_t retriggeroffset;

	assert(self);
	
	if ((psy_audio_patternentry_front(entry)->parameter & 0xf0) != 0) {
		uint8_t retriggerrate; // x / 16 = row duration per trigger

		retriggerrate = psy_audio_patternentry_front(entry)->parameter & 0xf0;
		// convert retriggerrate to beat unit
		retriggerstep = retriggerrate *
			(psy_audio_sequencer_currbeatsperline(self) / 256.f);
	} else {
		// use current retriggerrate	
		retriggerstep = track->state.retriggerstep;		
	}
	retriggeroffset = track->state.retriggeroffset + retriggerstep;
	while (retriggeroffset < psy_audio_sequencer_currbeatsperline(self)) {
		psy_audio_PatternEntry* retriggerentry;

		retriggerentry = psy_audio_patternentry_clone(entry);
		retriggerentry->track += track->channeloffset;
		psy_audio_patternentry_front(retriggerentry)->cmd = 0;
		psy_audio_patternentry_front(retriggerentry)->parameter = 0;
		retriggerentry->delta = entry->delta + retriggeroffset;
		psy_list_append(&self->delayedevents, retriggerentry);		
		retriggeroffset += retriggerstep;
	}
	track->state.retriggeroffset = retriggeroffset -
		psy_audio_sequencer_currbeatsperline(self);
	track->state.retriggerstep = retriggerstep;
}

void psy_audio_sequencer_insertdelayedevents(psy_audio_Sequencer* self)
{
	psy_audio_PatternNode* p;	
	
	assert(self);

	p = self->delayedevents;
	while (p != NULL) {	
		psy_audio_PatternEntry* delayed;
		
		delayed = psy_audio_patternnode_entry(p);		
		if (psy_audio_sequencer_isoffsetinwindow(self, delayed->delta)) {
			delayed->delta -= self->seqtime.position;
			psy_list_append(&self->events, delayed);
			p = psy_list_remove(&self->delayedevents, p);
		} else {
			psy_list_next(&p);
		}
	}	
}

void psy_audio_sequencer_insertinputevents(psy_audio_Sequencer* self)
{
	psy_List* p;
	psy_List* q;
	
	assert(self);

	for (p = self->inputevents; p != NULL; p = q) {
		psy_audio_PatternEntry* entry = psy_audio_patternnode_entry(p);
		
		q = p->next;
		entry->delta = 0;
		psy_list_append(&self->events, entry);
		if (q) {
			q->tail = self->inputevents->tail;
			q->prev = 0;
		}
		self->inputevents = q;
		free(p);		
	}
}

void psy_audio_sequencer_compute_beatspersample(psy_audio_Sequencer* self)
{
	assert(self);
	assert(self->seqtime.samplerate != 0.0);
	
	self->beatspersample =
		(self->seqtime.bpm * psy_audio_sequencer_speed(self)) /
		(self->seqtime.samplerate * 60.0);
}

psy_audio_PatternNode* psy_audio_sequencer_timedevents(
	psy_audio_Sequencer* self, uintptr_t slot, uintptr_t amount)
{
	psy_audio_PatternNode* rv;
	psy_audio_PatternNode* p;

	assert(self);

	rv = psy_audio_sequencer_machinetickevents(self, slot);
	for (p = rv ; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;
		psy_dsp_big_beat_t beatspersample;
		uintptr_t deltaframes;			

		entry = psy_audio_patternnode_entry(p);
		beatspersample = (entry->bpm * psy_audio_sequencer_speed(self)) /
			(self->seqtime.samplerate * 60.0);
		deltaframes = (uintptr_t)(entry->delta / self->beatspersample);
		if (deltaframes >= amount) {
			deltaframes = amount - 1;
		}
		entry->delta = (psy_dsp_big_beat_t)deltaframes;						
	}
	return rv;
}

void psy_audio_sequencer_addinputevent(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* ev, uintptr_t track)
{
	assert(self);

	if (ev) {
		psy_list_append(&self->inputevents,
			psy_audio_patternentry_allocinit_all(ev, 0, 0,
				self->seqtime.bpm, track));
	}
}

void psy_audio_sequencer_recordinputevent(psy_audio_Sequencer* self,
	const psy_audio_PatternEvent* event, uintptr_t track,
	psy_dsp_big_beat_t playposition)
{
	psy_audio_SequenceTrackIterator it;

	assert(self);

	it = psy_audio_sequence_begin(self->sequence, self->sequence->tracks,
		playposition);
	if (it.sequencentrynode) {
		psy_audio_SequenceEntry* entry;
		psy_audio_Pattern* pattern;		
		
		entry = (psy_audio_SequenceEntry*) it.sequencentrynode->entry;
		pattern = psy_audio_patterns_at(self->sequence->patterns,
			entry->patternslot);
		if (pattern) {			
			psy_dsp_big_beat_t quantizedpatternoffset;
			psy_audio_PatternNode* prev;
			psy_audio_PatternNode* node;

			quantizedpatternoffset = ((int)((playposition - entry->offset) *
				self->lpb)) / (psy_dsp_big_beat_t)self->lpb;
			node = psy_audio_pattern_findnode(pattern, 0,
				quantizedpatternoffset, 1.0 / self->lpb, &prev);
			if (node) {					
				psy_audio_pattern_setevent(pattern, node, event);
			} else {
				psy_audio_pattern_insert(pattern, prev, 0,
					quantizedpatternoffset, event);
			}
		}
	}
}

void psy_audio_sequencer_checkiterators(psy_audio_Sequencer* self,
	const psy_audio_PatternNode* node)
{
	psy_List* p;

	assert(self);

	for (p = self->currtracks; p != NULL; psy_list_next(&p)) {
		psy_audio_SequencerTrack* track;
		psy_audio_SequenceTrackIterator* it;

		track = (psy_audio_SequencerTrack*) p->entry;
		it = track->iterator;
		if (it->patternnode == node) {
			psy_audio_sequencetrackiterator_inc_entry(it);
		}
	}
}

uintptr_t psy_audio_sequencer_currframesperline(psy_audio_Sequencer* self)
{
	assert(self);

	return psy_audio_sequencer_frames(self,
		psy_audio_sequencer_currbeatsperline(self));	
}

// sort events by position and pattern track
void psy_audio_sequencer_sortevents(psy_audio_Sequencer* self)
{	
	uintptr_t numevents;

	assert(self);	

	numevents = psy_list_size(self->events);
	if (numevents == 2) {
		if (psy_audio_sequencer_comp_events(self->events,
				psy_list_last(self->events)) > 0) {
			psy_audio_PatternEntry* temp;		
			// swap	entries
			temp = self->events->entry;
			self->events->entry = self->events->tail->entry;
			self->events->tail->entry = temp;
		}
	} else if (numevents > 1) {		
		// sort with qsort
		// convert the event list to an array, sort the array and recreate
		// the sorted event list
		psy_audio_PatternNode** eventarray;		
						
		psy_audio_sequencer_resizeqsortarray(self, numevents);
		eventarray = self->qsortarray;
		if (eventarray) {
			uintptr_t i;
			psy_audio_PatternNode* sorted;
			psy_audio_PatternNode* p;
				
			// copy event node to qsort array
			for (i = 0, p = self->events; p != NULL; psy_list_next(&p), ++i) {
				eventarray[i] = p;
			}
			assert(i != (numevents - 1));
			// sort array
			psy_qsort((void**)eventarray, 0, numevents - 1, (psy_fp_comp)
				psy_audio_sequencer_comp_events);
			// recreate sorted events
			for (sorted = NULL, i = 0; i < numevents; ++i) {
				psy_list_append(&sorted, eventarray[i]->entry);
			}
			psy_list_free(self->events);
			self->events = sorted;
		}		
	}
	// psy_audio_sequencer_assertorder(self);
}

// if qsortarraysize is too small reallocate it according to numevents
void psy_audio_sequencer_resizeqsortarray(psy_audio_Sequencer* self,
	uintptr_t numevents)
{	
	while (numevents >= self->qsortarraysize) {
		psy_audio_PatternNode** qsortarray;

		self->qsortarraysize += QSORTARRAYRESIZE;
		qsortarray = realloc(self->qsortarray,
			sizeof(psy_audio_PatternNode*) * self->qsortarraysize);
		self->qsortarray = qsortarray;
	}
}


int psy_audio_sequencer_comp_events(psy_audio_PatternNode* lhsnode,
	psy_audio_PatternNode* rhsnode)
{
	int rv;
	psy_audio_PatternEntry* lhs;
	psy_audio_PatternEntry* rhs;

	assert(lhsnode && rhsnode);

	lhs = psy_audio_patternnode_entry(lhsnode);
	rhs = psy_audio_patternnode_entry(rhsnode);	
	if (lhs->delta == rhs->delta) {
		if (lhs->track < rhs->track) {
			rv = -1;
		} else {
			rv = 0;
		}
	} else if (lhs->delta < rhs->delta) {
		rv = -1;
	} else {
		rv = 1;
	}
	return rv;		
}	

// void psy_audio_sequencer_assertorder(psy_audio_Sequencer* self)
// {
//		psy_List* p;
//		psy_dsp_big_beat_t last = 0;
// 	
//		p = self->events;
// 		while (p) {
// 			psy_audio_PatternEntry* entry;
// 		
// 			entry = psy_audio_patternnode_entry(p);
// 			assert(entry->delta >= last);
// 			last = entry->delta;			
// 			psy_list_next(&p);
// 		}
// }

void psy_audio_sequencer_jumpto(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	psy_audio_sequencerjump_activate(&self->jump, position);
}

void psy_audio_sequencer_stopat(psy_audio_Sequencer* self,
	psy_dsp_big_beat_t position)
{
	assert(self);

	psy_audio_sequencerjump_activate(&self->jump, position);
	self->jump.dostop = TRUE;
}

psy_dsp_percent_t psy_audio_sequencer_playlist_rowprogress(
	const psy_audio_Sequencer* self)
{
	assert(self);
	
	psy_dsp_big_beat_t seqoffset;
	psy_dsp_big_beat_t length;

	length = 0.0;
	seqoffset = 0.0;
	if (psy_audio_sequencer_playing(self)) {
		const psy_List* p;
		
		for (p = self->currtracks; p != NULL; psy_list_next_const(&p)) {
			psy_audio_SequencerTrack* track;			

			track = (psy_audio_SequencerTrack*)psy_list_entry_const(p);
			if (track->iterator->sequencentrynode) {
				psy_audio_SequenceEntry* seqentry;
				psy_audio_Pattern* pattern;

				seqentry = (psy_audio_SequenceEntry*)
					track->iterator->sequencentrynode->entry;

				if (p == self->currtracks) {
					seqoffset = seqentry->offset;
				} else {
					seqoffset = psy_min(seqoffset, seqentry->offset);
				}
				pattern = psy_audio_sequenceentry_pattern(seqentry,
					self->sequence->patterns);
				if (pattern) {
					length = psy_max(length,
						psy_audio_pattern_length(pattern));
				}
			}
		}
	}
	if (length > 0.0) {
		return (psy_dsp_percent_t)
			((psy_audio_sequencer_position(self) - seqoffset) / length);
	}
	return (psy_dsp_percent_t)0.f;
}
