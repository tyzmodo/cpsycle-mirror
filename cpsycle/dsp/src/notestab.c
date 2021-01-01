// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "notestab.h"

static char* notes_tab_a440[256] = {
	"C-m","C#m","D-m","D#m","E-m","F-m","F#m","G-m","G#m","A-m","A#m","B-m", //0
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //1
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //2
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //3
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //4
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //5
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //6
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //7
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //8
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //9
	"off","twk","twf","mcm","tws","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};

static char* notes_tab_a220[256] = {
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //0
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //1
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //2
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //3
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //4
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //5
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //6
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //7
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //8
	"C-9","C#9","D-9","D#9","E-9","F-9","F#9","G-9","G#9","A-9","A#9","B-9", //9
	"off","twk","twf","mcm","tws","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
};

static char* notes_tab_gmpercussion[] = {
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ", // 0..15
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ", // 16..31
	"   ","   ","   ",																				 // 32..34
	"Acoustic Bass Drum","Bass Drum 1","Side Stick","Acoustic Snare",								 // 35..81
	"Hand Clap","Electric Snare","Low Floor Tom","Closed Hi Hat",
	"High Floor Tom","Pedal Hi-Hat","Low Tom","Open Hi-Hat",
	"Low-Mid Tom", "Hi Mid Tom", "Crash Cymbal 1", "High Tom",
	"Ride Cymbal 1", "Chinese Cymbal", "Ride Bell", "Tambourine",
	"Splash Cymbal", "Cowbell", "Crash Cymbal 2", "Vibraslap",
	"Ride Cymbal 2", "Hi Bongo", "Low Bongo", "Mute Hi Conga",
	"Open Hi Conga", "Low Conga", "High Timbale", "Low Timbale",
	"High Agogo", "Low Agogo", "Cabasa", "Maracas",
	"Short Whistle", "Long Whistle", "Short Guiro", "Long Guiro",
	"Claves", "Hi Wood Block", "Low Wood Block", "Mute Cuica",
	"Open Cuica", "Mute Triangle", "Open Triangle"
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ", // 81..96
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ", // 97..112
	"   ","   ","   ","   ", "   ", "   ", "   ",													 // 113..119
	"off","twk","twf","mcm","tws","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
	"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   "
};

char* hex_tab[16] = {
	"0", "1", "2", "3", "4", "5", "6", "7",
	"8", "9", "A", "B", "C", "D", "E", "F"
};

const char* psy_dsp_notetostr(psy_dsp_note_t note, psy_dsp_NotesTabMode mode)
{
	const char* rv;

	switch (mode) {
		case psy_dsp_NOTESTAB_A440:
			rv = notes_tab_a440[note];
		break;
		case psy_dsp_NOTESTAB_A220:
			rv = notes_tab_a220[note];
		break;
		case psy_dsp_NOTESTAB_GMPERCUSSION:
			rv = notes_tab_gmpercussion[note];
			break;
		default:
			rv = notes_tab_a220[255];
		break;
	}
	return rv;
}

const char* const * psy_dsp_notetab(psy_dsp_NotesTabMode mode)
{
	const char* const * rv;

	switch (mode) {
		case psy_dsp_NOTESTAB_A440:
			rv = (const char* const *)notes_tab_a440;
		break;
		case psy_dsp_NOTESTAB_A220:
			rv = (const char* const *)notes_tab_a220;
		break;
		case psy_dsp_NOTESTAB_GMPERCUSSION:
			rv = (const char* const*)notes_tab_gmpercussion;
		break;
		default:
			rv = (const char* const *)notes_tab_a220;
		break;
	}
	return rv;
}
