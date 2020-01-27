// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "greet.h"

 #ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>

static void OnSize(Greet*, psy_ui_Component* sender, psy_ui_Size*);
static void AddString(Greet*, const char* text);
static void Build(Greet* self);
static void BuildOriginal(Greet* self);
static void OnOriginal(Greet*, psy_ui_Component* sender);

void greet_init(Greet* self, psy_ui_Component* parent)
{	
	psy_ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_size, self, OnSize);
	self->current = 1;
	psy_ui_component_settitle(&self->component, "Greetings and info");	
	psy_ui_label_init(&self->header, &self->component);
	psy_ui_label_setstyle(&self->header, WS_CHILD | WS_VISIBLE | SS_CENTER);
	psy_ui_label_settext(&self->header, "Psycledelics, the Community, wants to thank the following people\nfor their contributions in the developement of Psycle");
	ui_listbox_init(&self->greetz, &self->component);	
	psy_ui_label_init(&self->thanks, &self->component);
	psy_ui_label_setstyle(&self->thanks, WS_CHILD | WS_VISIBLE | SS_LEFT);
	psy_ui_label_settext(&self->thanks, "Thanks!");
	psy_ui_button_init(&self->original, &self->component);
	psy_ui_button_settext(&self->original, "Show Original Arguru's Greetings");
	psy_signal_connect(&self->original.signal_clicked, self, OnOriginal);	
/*
	//Original Arguru's Greetings.
	m_greetz.AddString("Hamarr Heylen 'Hymax' [Logo design]");
	m_greetz.AddString("Raul Reales 'DJLaser'");
	m_greetz.AddString("Fco. Portillo 'Titan3_4'");
	m_greetz.AddString("Juliole");
	m_greetz.AddString("Sergio 'Zuprimo'");
	m_greetz.AddString("Oskari Tammelin [buzz creator]");
	m_greetz.AddString("Amir Geva 'Photon'");
	m_greetz.AddString("WhiteNoize");
	m_greetz.AddString("Zephod");
	m_greetz.AddString("Felix Petrescu 'WakaX'");
	m_greetz.AddString("Spiril at #goa [EFNET]");
	m_greetz.AddString("Joselex 'Americano'");
	m_greetz.AddString("Lach-ST2");
	m_greetz.AddString("DrDestral");
	m_greetz.AddString("Ic3man");
	m_greetz.AddString("Osirix");
	m_greetz.AddString("Mulder3");
	m_greetz.AddString("HexDump");
	m_greetz.AddString("Robotico");
	m_greetz.AddString("Krzysztof Foltman [FSM]");

	m_greetz.AddString("All #track at Irc-Hispano");

*/
	Build(self);
}

void Build(Greet* self)
{
	AddString(self, "All the people in the Forums");
	AddString(self, "All at #psycle [EFnet]");

	AddString(self, "Alk [Extreme testing + Coding]");
//	AddString(self, "BigTick [for his excellent VSTs]");
	AddString(self, "bohan");
	AddString(self, "Byte");
	AddString(self, "CyanPhase [for porting VibraSynth]");
	AddString(self, "dazld");
	AddString(self, "dj_d [Beta testing]");
	AddString(self, "DJMirage");
//	AddString(self, "Drax_D [for asking to be here ;D]");
	AddString(self, "Druttis [psy_audio_Machines]");
	AddString(self, "Erodix");
//	AddString(self, "Felix Kaplan / Spirit Of India");
//	AddString(self, "Felix Petrescu 'WakaX'");
//	AddString(self, "Gerwin / FreeH2o");
//	AddString(self, "Imagineer");
	AddString(self, "Arguru/Guru R.I.P. [We follow your steps]");
//	AddString(self, "KooPer");
//	AddString(self, "Krzysztof Foltman / fsm [Coding help]");
//	AddString(self, "krokpitr");
	AddString(self, "ksn [Psycledelics WebMaster]");
	AddString(self, "lastfuture");
	AddString(self, "LegoStar [asio]");
//	AddString(self, "Loby [for being away]");
	AddString(self, "Pikari");
	AddString(self, "pooplog [psy_audio_Machines + Coding]");
	AddString(self, "sampler");
	AddString(self, "[SAS] SOLARiS");
	AddString(self, "hugo Vinagre [Extreme testing]");
	AddString(self, "TAo-AAS");
	AddString(self, "TimEr [Site Graphics and more]");
//	AddString(self, "Vir|us");
}

void BuildOriginal(Greet* self)
{
	//Original Arguru's Greetings.
	AddString(self, "Hamarr Heylen 'Hymax' [Logo design]");
	AddString(self, "Raul Reales 'DJLaser'");
	AddString(self, "Fco. Portillo 'Titan3_4'");
	AddString(self, "Juliole");
	AddString(self, "Sergio 'Zuprimo'");
	AddString(self, "Oskari Tammelin [buzz creator]");
	AddString(self, "Amir Geva 'Photon'");
	AddString(self, "WhiteNoize");
	AddString(self, "Zephod");
	AddString(self, "Felix Petrescu 'WakaX'");
	AddString(self, "Spiril at #goa [EFNET]");
	AddString(self, "Joselex 'Americano'");
	AddString(self, "Lach-ST2");
	AddString(self, "DrDestral");
	AddString(self, "Ic3man");
	AddString(self, "Osirix");
	AddString(self, "Mulder3");
	AddString(self, "HexDump");
	AddString(self, "Robotico");
	AddString(self, "Krzysztof Foltman [FSM]");

	AddString(self, "All #track at Irc-Hispano");
}


void AddString(Greet* self, const char* text)
{
	ui_listbox_addstring(&self->greetz, text);
}

void OnOriginal(Greet* self, psy_ui_Component* sender)
{
	ui_listbox_clear(&self->greetz);
	self->current = self->current == 0;
	if (self->current) {
		Build(self);
		psy_ui_button_settext(&self->original, "Show Original Arguru's Greetings");
	} else {
		BuildOriginal(self);
		psy_ui_button_settext(&self->original, "Show Current Greetings");
	}	
}

void OnSize(Greet* self, psy_ui_Component* sender, psy_ui_Size* size)
{
	psy_ui_component_setposition(&self->header.component, 0, 10, size->width, 40);
	psy_ui_component_setposition(&self->thanks.component, 0, 45, size->width - 10, 15);
	psy_ui_component_setposition(&self->greetz.component, 10, 65, size->width - 30, size->height - 100);
	psy_ui_component_setposition(&self->original.component, 0, size->height - 25, size->width, 20);
}
