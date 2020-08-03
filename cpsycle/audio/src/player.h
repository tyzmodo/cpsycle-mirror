// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PLAYER_H
#define psy_audio_PLAYER_H

#include "../../driver/driver.h"
#include "eventdrivers.h"
#include "song.h"
#include "machinefactory.h"
#include "sequencer.h"
#include <signal.h>
#include "library.h"

#include <dither.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	VUMETER_NONE,
	VUMETER_PEAK,
	VUMETER_RMS,	
} VUMeterMode;

typedef struct psy_audio_Player {
	psy_AudioDriver* driver;
	psy_audio_Song* song;
	// empty song for lock minimized
	// song switching
	psy_audio_MachineFactory machinefactory;
	psy_audio_MachineCallback machinecallback;
	psy_audio_Song emptysong;
	psy_audio_Sequencer sequencer;	
	uintptr_t numsongtracks;
	psy_Signal signal_numsongtrackschanged;
	psy_Signal signal_lpbchanged;
	psy_Signal signal_inputevent;
	psy_Signal signal_stop;	
	psy_Library drivermodule;
	EventDrivers eventdrivers;	
	VUMeterMode vumode;
	int recordingnotes;
	int multichannelaudition;	
	psy_Table notestotracks;
	psy_Table trackstonotes;
	psy_Table worked;
	psy_audio_Pattern patterndefaults;
	psy_dsp_Dither dither;
	bool dodither;
} psy_audio_Player;

// init dispose
void psy_audio_player_init(psy_audio_Player*, psy_audio_Song*, void* systemhandle);
void psy_audio_player_dispose(psy_audio_Player*);
// general
void psy_audio_player_setsong(psy_audio_Player*, psy_audio_Song*);
INLINE psy_audio_Song* psy_audio_player_song(psy_audio_Player* self)
{
	return self->song;
}
void psy_audio_player_setnumsongtracks(psy_audio_Player*, uintptr_t numsongtracks);

INLINE uintptr_t psy_audio_player_numsongtracks(psy_audio_Player* self)
{
	return self->numsongtracks;
}

void psy_audio_player_setvumetermode(psy_audio_Player*, VUMeterMode);
VUMeterMode psy_audio_player_vumetermode(psy_audio_Player*);
void psy_audio_player_enabledither(psy_audio_Player*);
void psy_audio_player_disabledither(psy_audio_Player*);
void psy_audio_player_setdither(psy_audio_Player*, uintptr_t depth,
	psy_dsp_DitherPdf, psy_dsp_DitherNoiseShape);

// sequencer
void psy_audio_player_start(psy_audio_Player*);
void psy_audio_player_stop(psy_audio_Player*);

INLINE int psy_audio_player_playing(psy_audio_Player* self)
{
	return psy_audio_sequencer_playing(&self->sequencer);
}

void psy_audio_player_setposition(psy_audio_Player*, psy_dsp_big_beat_t offset);

INLINE psy_dsp_big_beat_t psy_audio_player_position(psy_audio_Player* self)
{
	return psy_audio_sequencer_position(&self->sequencer);
}

INLINE psy_audio_player_setbpm(psy_audio_Player* self, psy_dsp_big_beat_t bpm)
{
	psy_audio_sequencer_setbpm(&self->sequencer, bpm);
}

INLINE psy_dsp_big_beat_t psy_audio_player_bpm(psy_audio_Player* self)
{
	return psy_audio_sequencer_bpm(&self->sequencer);
}

void psy_audio_player_setlpb(psy_audio_Player*, uintptr_t lpb);

INLINE uintptr_t psy_audio_player_lpb(psy_audio_Player* self)
{
	return psy_audio_sequencer_lpb(&self->sequencer);
}

INLINE uintptr_t psy_audio_player_samplerate(psy_audio_Player* self)
{
	return psy_audio_sequencer_samplerate(&self->sequencer);
}

// audio driver
void psy_audio_player_setaudiodriver(psy_audio_Player*, psy_AudioDriver*);
psy_AudioDriver* psy_audio_player_audiodriver(psy_audio_Player*);
void psy_audio_player_loaddriver(psy_audio_Player*, const char* path, psy_Properties* config);
void psy_audio_player_reloaddriver(psy_audio_Player*, const char* path, psy_Properties* config);
void psy_audio_player_restartdriver(psy_audio_Player*, psy_Properties* config);
// event recording
void psy_audio_player_startrecordingnotes(psy_audio_Player*);
void psy_audio_player_stoprecordingnotes(psy_audio_Player*);
int psy_audio_player_recordingnotes(psy_audio_Player*);
// event driver
void psy_audio_player_loadeventdriver(psy_audio_Player*, const char* path);
void psy_audio_player_removeeventdriver(psy_audio_Player*, int id);
void psy_audio_player_restarteventdriver(psy_audio_Player*, int id);
psy_EventDriver* psy_audio_player_kbddriver(psy_audio_Player*);
psy_EventDriver* psy_audio_player_eventdriver(psy_audio_Player*, int id);
unsigned int psy_audio_player_numeventdrivers(psy_audio_Player*);
void psy_audio_player_workmachine(psy_audio_Player*, uintptr_t amount, uintptr_t slot);
void psy_audio_player_setemptysong(psy_audio_Player*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PLAYER_H */