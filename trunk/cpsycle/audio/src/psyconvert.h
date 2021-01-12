// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PSYCONVERT_H
#define psy_audio_PSYCONVERT_H

#include "../../detail/psydef.h"

#include "instrument.h"
#include "pattern.h"
// dsp
#include <envelope.h>
#include <filter.h>

#ifdef __cplusplus
extern "C" {
#endif

// Compatibility methods/structs for song file load/save

// Patterns

typedef struct psy_audio_LegacyPatternEntry {
	uint8_t _note;
	uint8_t _inst;
	uint8_t _mach;
	uint8_t _cmd;
	uint8_t _parameter;
} psy_audio_LegacyPatternEntry;

typedef unsigned char* psy_audio_LegacyPattern;

// legacy pattern
psy_audio_LegacyPattern psy_audio_allocoldpattern(psy_audio_Pattern* pattern, uintptr_t lpb,
	int* rv_patternlines);

const psy_audio_LegacyPatternEntry* psy_audio_ptrackline_const(const
	psy_audio_LegacyPattern, int track, int line);
psy_audio_LegacyPatternEntry* psy_audio_ptrackline(psy_audio_LegacyPattern,
	int track, int line);

// Instruments

typedef struct psy_audio_LegacyInstrument {
	///\name Loop stuff
			///\{
	bool _loop;
	int _lines;
	///\}

	///\verbatim
	/// NNA values overview:
	///
	/// 0 = Note Cut      [Fast Release 'Default']
	/// 1 = Note Release  [Release Stage]
	/// 2 = Note Continue [No NNA]
	///\endverbatim
	unsigned char _NNA;


	int sampler_to_use; // Sampler machine index for lockinst.
	bool _LOCKINST;	// Force this instrument number to change the selected machine to use a specific sampler when editing (i.e. when using the pc or midi keyboards, not the notes already existing in a pattern)

	///\name Amplitude Envelope overview:
	///\{
	/// Attack Time [in Samples at 44.1Khz, independently of the real samplerate]
	int ENV_AT;
	/// Decay Time [in Samples at 44.1Khz, independently of the real samplerate]
	int ENV_DT;
	/// Sustain Level [in %]
	int ENV_SL;
	/// Release Time [in Samples at 44.1Khz, independently of the real samplerate]
	int ENV_RT;
	///\}

	///\name Filter 
	///\{
	/// Attack Time [in Samples at 44.1Khz]
	int ENV_F_AT;
	/// Decay Time [in Samples at 44.1Khz]
	int ENV_F_DT;
	/// Sustain Level [0..128]
	int ENV_F_SL;
	/// Release Time [in Samples at 44.1Khz]
	int ENV_F_RT;

	/// Cutoff Frequency [0-127]
	int ENV_F_CO;
	/// Resonance [0-127]
	int ENV_F_RQ;
	/// EnvAmount [-128,128]
	int ENV_F_EA;
	/// Filter Type. See psycle::helpers::dsp::FilterType. [0..6]
	psy_dsp_FilterType ENV_F_TP;
	///\}

	bool _RPAN;
	bool _RCUT;
	bool _RRES;
} psy_audio_LegacyInstrument;

psy_audio_LegacyInstrument psy_audio_legacyinstrument(const psy_audio_Instrument*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PSYCONVERT_H */