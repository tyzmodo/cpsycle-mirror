/*      Copyright (C) 2002 Vincenzo Demasi.

        This plugin is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.\n"\

        This plugin is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

        Vincenzo Demasi. E-Mail: <v.demasi@tiscali.it>
*/

#ifndef VDALLPASS_H
#define VDALLPASS_H

#include <psycle/plugin/MachineInterface.h>
#include <psycle/plugin/vd/vdabout.h>
#include <psycle/plugin/vd/allpassfilter.h>

#define LDELAY    0
#define LGAIN     1
#define RDELAY    2
#define RGAIN     3
#define LOCKDELAY 4
#define LOCKGAIN  5
#define PARNUM    6

#define PARCOLS 3

#define VDPLUGINNAME "vd's allpass filter"
#define VDSHORTNAME  "AllPass"
#define VDAUTHOR     "V. Demasi (Built on "__DATE__")"
#define VDCOMMAND    "License"

#define MIN_DELAY  1
#define MAX_DELAY  ALLPASS_FILTER_MAX_DELAY
#define MIN_GAIN   -100
#define MAX_GAIN    100
#define GAIN_NORM   100.0f
#define SET_LDELAY (MAX_DELAY / 2)
#define SET_RDELAY (MAX_DELAY / 2)
#define SET_LGAIN  (MAX_GAIN / 2)
#define SET_RGAIN  (MAX_GAIN / 2)
#define MIN_LOCK 0
#define MAX_LOCK 1
#define SET_LOCK 1



CMachineParameter const parLeftDelay =
{
	"Left Delay", "Left Delay (samples)", MIN_DELAY, MAX_DELAY, MPF_STATE, SET_LDELAY
};

CMachineParameter const parLeftGain =
{
	"Left Gain", "Left Gain (%)", MIN_GAIN, MAX_GAIN, MPF_STATE, SET_LGAIN
};

CMachineParameter const parRightDelay =
{
	"Right Delay", "Right Delay (samples)", MIN_DELAY, MAX_DELAY, MPF_STATE, SET_RDELAY
};

CMachineParameter const parRightGain =
{
	"Right Gain", "Right Gain (%)", MIN_GAIN, MAX_GAIN, MPF_STATE, SET_RGAIN
};

CMachineParameter const parLockDelay =
{
	"Lock Delay", "Lock Delay (on/off)", MIN_LOCK, MAX_LOCK, MPF_STATE, SET_LOCK
};

CMachineParameter const parLockGain =
{
	"Lock Gain", "Lock Gain (on/off)", MIN_LOCK, MAX_LOCK, MPF_STATE, SET_LOCK
};

CMachineParameter const *pParameters[] =
{
	&parLeftDelay,
	&parLeftGain,
	&parRightDelay,
	&parRightGain,
	&parLockDelay,
	&parLockGain
};

CMachineInfo const MacInfo =
{
	MI_VERSION,
	EFFECT,									// flags
	PARNUM,									// numParameters
	pParameters,							// Pointer to parameters
#ifdef _DEBUG
	VDPLUGINNAME " (Debug Build)",			// name
#else
	VDPLUGINNAME,							// name
#endif
	VDSHORTNAME,							// short name
	VDAUTHOR,								// author
	VDCOMMAND,								// A command, that could be use for open an editor, etc...
	PARCOLS
};

class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init();
	virtual void Command();
	virtual void ParameterTweak(int par, int val);
	virtual void SequencerTick();
	virtual bool DescribeValue(char *txt,int const param, int const value);
	virtual void Work(float *pleftsamples, float *prightsamples, int samplesnum, int tracks);
private:
	AllPassFilter leftFilter;
	AllPassFilter rightFilter;
	int lastDelayModified, lastGainModified;
	float lGain, rGain;
};

DLL_EXPORTS

#endif  /* ALLPASS_H */