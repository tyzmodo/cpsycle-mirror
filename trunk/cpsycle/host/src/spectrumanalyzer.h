// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SPECTRUMANALYZER_H)
#define SPECTRUMANALYZER_H

#include "uicomponent.h"
#include "workspace.h"
#include <fft.h>

#define SCOPE_SPEC_BANDS 256

typedef struct {	
	psy_ui_Component component;
	psy_audio_Wire wire;
	float invol;
	float mult;
	int bar_heights[SCOPE_SPEC_BANDS];
	int scope_peak_rate;
	int scope_spec_rate;
	int hold;	
	int scope_spec_mode;
	int scope_spec_samples;
	float db_left[SCOPE_SPEC_BANDS];
	float db_right[SCOPE_SPEC_BANDS];
	double phi;
	FFTClass fftSpec;
	Workspace* workspace;
	int lastprocessed;
	int lastwritepos;
} SpectrumAnalyzer;

void spectrumanalyzer_init(SpectrumAnalyzer*, psy_ui_Component* parent, psy_audio_Wire wire, Workspace*);
void spectrumanalyzer_stop(SpectrumAnalyzer*);

#endif /* SPECTRUMANALYZER_H */
