/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCECURSOR_H
#define psy_audio_SEQUENCECURSOR_H

/* local */
#include "sequenceselection.h"
/* dsp */
#include <dsptypes.h>
/* platform */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_SequenceCursor
*/

typedef struct psy_audio_SequenceCursor {	
	psy_audio_OrderIndex orderindex;
	psy_dsp_big_beat_t absoffset;
	psy_dsp_big_beat_t seqoffset;
	uintptr_t track;	
	/* mutable */ uintptr_t linecache;
	uintptr_t lpb;
	uintptr_t column;
	uintptr_t digit;
	uintptr_t patternid;
	uint8_t key;
	uintptr_t noteindex;
} psy_audio_SequenceCursor;

void psy_audio_sequencecursor_init(psy_audio_SequenceCursor*);
void psy_audio_sequencecursor_init_all(psy_audio_SequenceCursor*,
	psy_audio_OrderIndex orderindex);

psy_audio_SequenceCursor psy_audio_sequencecursor_make_all(
	uintptr_t track, psy_dsp_big_beat_t offset, uint8_t key);
psy_audio_SequenceCursor psy_audio_sequencecursor_make(
	uintptr_t track, psy_dsp_big_beat_t offset);

/* compares two pattern edit positions, if they are equal */
bool psy_audio_sequencecursor_equal(psy_audio_SequenceCursor* lhs,
	psy_audio_SequenceCursor* rhs);

void psy_audio_sequencecursor_updatecache(const psy_audio_SequenceCursor*);
void psy_audio_sequencecursor_updateseqoffset(psy_audio_SequenceCursor*,
	const struct psy_audio_Sequence*);
void psy_audio_sequencecursor_update_order(psy_audio_SequenceCursor*,
	const struct psy_audio_Sequence*);
uintptr_t psy_audio_sequencecursor_patternid(const psy_audio_SequenceCursor*,
	const struct psy_audio_Sequence*);
psy_dsp_big_beat_t psy_audio_sequencecursor_seqoffset(
	const struct psy_audio_SequenceCursor*);

INLINE uintptr_t psy_audio_sequencecursor_line(const psy_audio_SequenceCursor* self)
{
	return self->linecache;
}

INLINE uintptr_t psy_audio_sequencecursor_seqline(const psy_audio_SequenceCursor* self)
{
	return (uintptr_t)(psy_audio_sequencecursor_seqoffset(self) * (psy_dsp_big_beat_t)self->lpb);
}

uintptr_t psy_audio_sequencecursor_track(const psy_audio_SequenceCursor*);
uintptr_t psy_audio_sequencecursor_column(const psy_audio_SequenceCursor*);
uintptr_t psy_audio_sequencecursor_digit(const psy_audio_SequenceCursor*);
uintptr_t psy_audio_sequencecursor_noteindex(const psy_audio_SequenceCursor*);

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_offset_abs(const psy_audio_SequenceCursor* self)
{
	return self->absoffset;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_pattern_offset(
	const psy_audio_SequenceCursor* self)
{
	return self->absoffset - self->seqoffset;
}

INLINE uintptr_t psy_audio_sequencecursor_pattern_id(
	const psy_audio_SequenceCursor* self)
{
	return self->patternid;
}

INLINE uintptr_t psy_audio_sequencecursor_line_abs(const psy_audio_SequenceCursor* self)
{
	return cast_decimal(psy_audio_sequencecursor_offset_abs(self) * self->lpb);
}

INLINE uintptr_t psy_audio_sequencecursor_line_pattern(const psy_audio_SequenceCursor* self)
{
	return cast_decimal(psy_audio_sequencecursor_pattern_offset(self) * self->lpb);
}

INLINE void psy_audio_sequencecursor_setabsoffset(psy_audio_SequenceCursor* self,
	psy_dsp_big_beat_t absoffset)
{
	self->absoffset = absoffset;
	psy_audio_sequencecursor_updatecache(self);
}

INLINE void psy_audio_sequencecursor_setseqoffset(psy_audio_SequenceCursor* self,
	psy_dsp_big_beat_t seqoffset)
{
	self->seqoffset = seqoffset;
	psy_audio_sequencecursor_updatecache(self);
}

INLINE void psy_audio_sequencecursor_setorderindex(psy_audio_SequenceCursor* self,
	psy_audio_OrderIndex index)
{
	self->orderindex = index;
}

INLINE psy_audio_OrderIndex psy_audio_sequencecursor_orderindex(
	const psy_audio_SequenceCursor* self)
{
	assert(self);

	return self->orderindex;
}

INLINE uintptr_t psy_audio_sequencecursor_lpb(const psy_audio_SequenceCursor*
	self)
{
	return self->lpb;
}

INLINE psy_dsp_big_beat_t psy_audio_sequencecursor_bpl(
	const psy_audio_SequenceCursor* self)
{
	return (psy_dsp_big_beat_t)1.0 / (psy_dsp_big_beat_t)self->lpb;
}

INLINE void psy_audio_sequencecursor_setlpb(psy_audio_SequenceCursor* self,
	uintptr_t lpb)
{
	self->lpb = lpb;
	psy_audio_sequencecursor_updatecache(self);
}

INLINE bool psy_audio_sequencecursor_intersect_abs(const
	psy_audio_SequenceCursor* self,
	psy_dsp_big_beat_t position_absolute)
{
	return psy_dsp_testrange(position_absolute,
		psy_audio_sequencecursor_offset_abs(self),
		psy_audio_sequencecursor_bpl(self));
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCECURSOR_H */
