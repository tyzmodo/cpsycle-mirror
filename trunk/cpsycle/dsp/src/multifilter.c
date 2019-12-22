// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "multifilter.h"

static const char* names[] = {
	"LowPass 12E",
	0
};

static void init(psy_dsp_MultiFilter* self) { }
static void dispose(psy_dsp_MultiFilter* self) { }
static void reset(psy_dsp_MultiFilter*);
static float work(psy_dsp_MultiFilter*, float sample);
static void setcutoff(psy_dsp_MultiFilter*, float cutoff);
static float cutoff(psy_dsp_MultiFilter*);
static void setressonance(psy_dsp_MultiFilter*, float ressonance);
static float ressonance(psy_dsp_MultiFilter*);
static void setsamplerate(psy_dsp_MultiFilter*, float samplerate);
static float samplerate(psy_dsp_MultiFilter*);
static void update(psy_dsp_MultiFilter*, int full);

static filter_vtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_dsp_Filter* filter)
{
	if (!vtable_initialized) {
		vtable = *filter->vtable;
		vtable.reset = (psy_dsp_fp_filter_reset) reset;
		vtable.work = (psy_dsp_fp_filter_work) work;
		vtable.update = (psy_dsp_fp_filter_update) update;
		vtable.setcutoff = (psy_dsp_fp_filter_setcutoff) setcutoff;
		vtable.cutoff = (psy_dsp_fp_filter_cutoff) cutoff;
		vtable.setressonance = (psy_dsp_fp_filter_setressonance) setressonance;
		vtable.ressonance = (psy_dsp_fp_filter_ressonance) ressonance;
		vtable.setsamplerate = (psy_dsp_fp_filter_setsamplerate) setsamplerate;
		vtable.samplerate = (psy_dsp_fp_filter_samplerate) samplerate;
		vtable.work = (psy_dsp_fp_filter_work) work;
		vtable_initialized = 1;
	}	
}

void psy_dsp_multifilter_init(psy_dsp_MultiFilter* self)
{
	psy_dsp_filter_init(&self->filter);
	vtable_init(&self->filter);
	self->filter.vtable = &vtable;
	self->samplerate = 44100.f;
	self->cutoff = 1.f;
	self->q = 0.f;	
	psy_dsp_lowpass12e_init(&self->lowpass12E);
	self->selected = F_NONE;
	psy_dsp_multifilter_settype(self, self->selected);
}

void psy_dsp_multifilter_settype(psy_dsp_MultiFilter* self, FilterType type)
{
	psy_dsp_Filter* filter;

	self->selected = type;	
	switch (self->selected) {
		case F_LOWPASS12E:
			filter = (psy_dsp_Filter*)(&self->lowpass12E);
			filter->vtable->setcutoff(filter, self->cutoff);
			filter->vtable->setressonance(filter, self->q);
		break;
		default:
			filter = 0;			
		break;
	}	
	self->selectedfilter = filter;
}

FilterType psy_dsp_multifilter_type(psy_dsp_MultiFilter* self)
{
	return self->selected;
}

const char* psy_dsp_multifilter_name(psy_dsp_MultiFilter* self, FilterType type)
{
	return names[(int)type];
}

unsigned int psy_dsp_multifilter_numfilters(psy_dsp_MultiFilter* self)
{
	return F_NUMFILTERS;
}

void psy_dsp_multifilter_inittables(unsigned int samplerate)
{
	psy_dsp_LowPass12E lp12e;
	
	psy_dsp_lowpass12e_init(&lp12e); // forces static stable to initialize
	lp12e.customfilter.filter.vtable->setsamplerate(
		&lp12e.customfilter.filter, (float)samplerate);
}

void reset(psy_dsp_MultiFilter* self)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->reset(self->selectedfilter);
	}
}

psy_dsp_amp_t work(psy_dsp_MultiFilter* self, psy_dsp_amp_t sample)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->work(self->selectedfilter, sample)
		: sample;
}

static void setcutoff(psy_dsp_MultiFilter* self, float cutoff)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->setcutoff(self->selectedfilter, cutoff);
	}
	self->cutoff = cutoff;
}

static float cutoff(psy_dsp_MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->cutoff(self->selectedfilter)
		: self->cutoff;
}

void setressonance(psy_dsp_MultiFilter* self, float ressonance)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->setressonance(self->selectedfilter,
			ressonance);
	}
	self->q = ressonance;
}

float ressonance(psy_dsp_MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->ressonance(self->selectedfilter)
		: self->q;
}

void setsamplerate(psy_dsp_MultiFilter* self, float samplerate)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->setsamplerate(self->selectedfilter,
			samplerate);
	}
	self->samplerate = samplerate;
}

float samplerate(psy_dsp_MultiFilter* self)
{
	return self->selectedfilter 
		? self->selectedfilter->vtable->samplerate(self->selectedfilter)
		: self->samplerate;
}

void update(psy_dsp_MultiFilter* self, int full)
{ 
	if (self->selectedfilter) {
		self->selectedfilter->vtable->update(self->selectedfilter, full);
	}
}
