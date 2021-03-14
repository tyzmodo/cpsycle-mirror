// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineviewbar.h"
// host
#include "skingraphics.h"
#include "wireview.h"
#include "machinewireview.h"
// audio
#include <exclusivelock.h>
// std
#include <math.h>
// platform
#include "../../detail/portable.h"
#include "../../detail/trace.h"

// MachineViewBar
static void machineviewbar_onsongchanged(MachineViewBar*, Workspace*,
	int flag, psy_audio_Song*);
void machineviewbar_setmachines(MachineViewBar*, psy_audio_Machines*);
static void machineviewbar_onmixerconnectmodeclick(MachineViewBar*,
	psy_ui_Component* sender);
static void machineviewbar_updateconnectasmixersend(MachineViewBar*);
static void machineviewbar_onmachineinsert(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);
static void machineviewbar_onmachineremoved(MachineViewBar*,
	psy_audio_Machines*, uintptr_t slot);

void machineviewbar_init(MachineViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(machineviewbar_base(self), parent, NULL);
	psy_ui_component_setdefaultalign(machineviewbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_margin_makeem(0.0, 4.0, 0.0, 0.0));
	psy_ui_checkbox_init_text(&self->mixersend, machineviewbar_base(self),
		"machineview.connect-to-mixer-send-return-input");		
	psy_ui_component_hide(psy_ui_checkbox_base(&self->mixersend));
	psy_signal_connect(&self->mixersend.signal_clicked, self,
		machineviewbar_onmixerconnectmodeclick);		
	psy_ui_label_init(&self->status, machineviewbar_base(self));
	psy_ui_label_preventtranslation(&self->status);
	psy_ui_label_setcharnumber(&self->status, 44.0);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		machineviewbar_onsongchanged);
	if (workspace_song(workspace)) {
		machineviewbar_setmachines(self,
			psy_audio_song_machines(workspace_song(workspace)));
	} else {
		machineviewbar_setmachines(self, NULL);
	}
}

void machineviewbar_settext(MachineViewBar* self, const char* text)
{	
	psy_ui_label_settext(&self->status, text);
	psy_ui_label_fadeout(&self->status);
}

void machineviewbar_onmixerconnectmodeclick(MachineViewBar* self,
	psy_ui_Component* sender)
{
	if (psy_ui_checkbox_checked(&self->mixersend)) {
		psy_audio_machines_connectasmixersend(self->machines);		
	} else {
		psy_audio_machines_connectasmixerinput(self->machines);		
	}    
}

void machineviewbar_updateconnectasmixersend(MachineViewBar* self)
{
	if (self->machines &&
		psy_audio_machines_isconnectasmixersend(self->machines)) {
		psy_ui_checkbox_check(&self->mixersend);
	} else {
		psy_ui_checkbox_disablecheck(&self->mixersend);
	}
}

void machineviewbar_onsongchanged(MachineViewBar* self, Workspace* sender,
	int flag, psy_audio_Song* song)
{		
	if (song) {
		machineviewbar_setmachines(self, psy_audio_song_machines(song));
	} else {
		machineviewbar_setmachines(self, NULL);
	}		
}

void machineviewbar_setmachines(MachineViewBar* self,
	psy_audio_Machines* machines)
{
	self->machines = machines;
	if (machines) {		
		psy_signal_connect(&self->machines->signal_insert, self,
			machineviewbar_onmachineinsert);
		psy_signal_connect(&self->machines->signal_removed, self,
			machineviewbar_onmachineremoved);
	}
	machineviewbar_updateconnectasmixersend(self);
}

void machineviewbar_onmachineinsert(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{
	if (psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_show_align(psy_ui_checkbox_base(&self->mixersend));
	}
	if (psy_audio_machines_at(sender, slot)) {
		char text[128];

		psy_snprintf(text, 128, "%s inserted at slot %u",
			psy_audio_machine_editname(psy_audio_machines_at(sender, slot)),
			(unsigned int)slot);
		machineviewbar_settext(self, text);
	}	
}

void machineviewbar_onmachineremoved(MachineViewBar* self,
	psy_audio_Machines* sender, uintptr_t slot)
{	
	char text[128];

	if (!psy_audio_machines_hasmixer(sender)) {		
		psy_ui_component_hide_align(psy_ui_checkbox_base(&self->mixersend));
	}		
	psy_snprintf(text, 128, "Machine removed from slot %u",
		(unsigned int)slot);
	machineviewbar_settext(self, text);	
}