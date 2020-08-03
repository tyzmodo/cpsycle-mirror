// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "volslider.h"

#include "convert.h"

#include <songio.h>
#include <uiapp.h>
#include <stdio.h>
#include <math.h>
#include "../../detail/portable.h"

static void volslider_onviewdescribe(VolSlider*, psy_ui_Slider*, char* text);
static void volslider_onviewtweak(VolSlider*, psy_ui_Slider*, float value);
static void volslider_onviewvalue(VolSlider*, psy_ui_Slider*, float* value);

void volslider_init(VolSlider* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_enablealign(&self->component);
	psy_ui_slider_init(&self->slider, &self->component);
	psy_ui_slider_settext(&self->slider, "VU");
	psy_ui_slider_setvaluecharnumber(&self->slider, 10);
	psy_ui_component_setalign(&self->slider.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_connect(&self->slider, self, volslider_onviewdescribe,
		volslider_onviewtweak, volslider_onviewvalue);	
}

void volslider_onviewdescribe(VolSlider* self, psy_ui_Slider* sender, char* text)
{
	if (self->workspace->song && psy_audio_machines_master(
			&self->workspace->song->machines)) {
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(
			psy_audio_machines_master(&self->workspace->song->machines), 0);
		if (param) {
			float normvalue;
			float volume;

			normvalue = psy_audio_machineparam_normvalue(param);
			volume = (normvalue * 2) * (normvalue * 2);			
			psy_snprintf(text, 10, "%.2f dB",
				(float)psy_dsp_convert_amp_to_db(volume));
		}
	}
}

void volslider_onviewtweak(VolSlider* self, psy_ui_Slider* sender, float value)
{
	if (self->workspace->song &&
			psy_audio_machines_master(&self->workspace->song->machines)) {
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(
			psy_audio_machines_master(&self->workspace->song->machines), 0);
		if (param) {
			psy_audio_machineparam_tweak(param, value);
		}
	}
}

void volslider_onviewvalue(VolSlider* self, psy_ui_Slider* sender, float* rv)
{	
	if (self->workspace->song && psy_audio_machines_master(
			&self->workspace->song->machines)) {
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(
			psy_audio_machines_master(&self->workspace->song->machines), 0);
		if (param) {
			*rv = psy_audio_machineparam_normvalue(param);
			return;
		}
	}
	*rv = 0.f;
}