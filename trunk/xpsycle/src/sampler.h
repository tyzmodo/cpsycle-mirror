/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef SAMPLER_H
#define SAMPLER_H

#include "filter.h"
#include "machine.h"

#define SAMPLER_MAX_POLYPHONY        16
#define SAMPLER_DEFAULT_POLYPHONY     8
#define SAMPLER_CMD_NONE           0x00
#define SAMPLER_CMD_PORTAUP        0x01
#define SAMPLER_CMD_PORTADOWN      0x02
#define SAMPLER_CMD_PORTA2NOTE     0x03
#define SAMPLER_CMD_PANNING        0x08
#define SAMPLER_CMD_OFFSET         0x09
#define SAMPLER_CMD_VOLUME         0x0c
#define SAMPLER_CMD_RETRIG         0x15
#define SAMPLER_CMD_EXTENDED       0x0e
#define SAMPLER_CMD_EXT_NOTEOFF    0xc0
#define SAMPLER_CMD_EXT_NOTEDELAY  0xd0

typedef unsigned long ULONG;
typedef unsigned long long ULONGLONG;
typedef signed long long sint64;

typedef union _ULARGE_INTEGER {
        struct {
                ULONG LowPart;
                ULONG HighPart;
        };
        struct {
                ULONG LowPart;
                ULONG HighPart;
        } u;
        ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;
typedef const union _ULARGE_INTEGER *PCULARGE_INTEGER;


typedef enum
{
  ENV_OFF = 0,
  ENV_ATTACK = 1,
  ENV_DECAY = 2,
  ENV_SUSTAIN = 3,
  ENV_RELEASE = 4,
  ENV_FASTRELEASE = 5
} EnvelopeStage;

class WaveData
{
  public:
    short* _pL;
    short* _pR;
    bool _stereo;
    ULARGE_INTEGER _pos;
    sint64 _speed;
    bool _loop;
    ULONG _loopStart;
    ULONG _loopEnd;
    ULONG _length;
    float _vol;
    float _lVolDest;
    float _rVolDest;
    float _lVolCurr;
    float _rVolCurr;
};

class Envelope
{
  public:
     EnvelopeStage _stage;
     float _value;
     float _step;
     float _attack;
     float _decay;
     float _sustain;
     float _release;
};

class Voice
{
   public:
      Envelope _filterEnv;
      Envelope _envelope;
      int _sampleCounter;
      int _triggerNoteOff;
      int _triggerNoteDelay;
      int _instrument;
      WaveData _wave;
      Filter _filter;
      int _cutoff;
      float _coModify;
      int _channel;
      int effVal;
      //int effPortaNote;
      int effCmd;
      int effretMode;
      int effretTicks;
      float effretVol;
      int effOld;
};

/// sampler.
class Sampler : public Machine
{
  public:


    void Tick();

    Sampler(int index);
    ~Sampler();

    virtual void Init(void);
    virtual void Work(int numSamples);
    virtual void Stop(void);
    virtual void Tick(int channel, PatternEntry* pData);
    virtual char* GetName(void) { return _psName; };
//    virtual bool Load(RiffFile* pFile);
 /*   inline virtual bool LoadSpecificChunk(RiffFile* pFile, int version)
    {
       UINT size;
       pFile->Read(&size,sizeof(size));
       if (size)
       {
          if (version > CURRENT_FILE_VERSION_MACD)
          {
            // data is from a newer format of psycle, it might be unsafe to load.
            pFile->Skip(size);
            return FALSE;
          }
          else
          {
            int temp;
            pFile->Read(&temp, sizeof(temp)); // numSubtracks
            _numVoices=temp;
            pFile->Read(&temp, sizeof(temp)); // quality

            switch (temp)
            {
               case 2:
                 _resampler.SetQuality(dsp::R_SPLINE);
               break;
               case 0:
                 _resampler.SetQuality(dsp::R_NONE);
               break;
               default:
               case 1:
                 _resampler.SetQuality(dsp::R_LINEAR);
               break;
            }
          }
        }
        return true;*/


/*inline virtual void SaveSpecificChunk(RiffFile* pFile) 
			{
				UINT temp;
				UINT size = 2*sizeof(temp);
				pFile->Write(&size,sizeof(size));
				temp = _numVoices;
				pFile->Write(&temp, sizeof(temp)); // numSubtracks
				switch (_resampler.GetQuality())
				{
					case dsp::R_NONE:
						temp = 0;
						break;
					case dsp::R_LINEAR:
						temp = 1;
						break;
					case dsp::R_SPLINE:
						temp = 2;
						break;
				}
				pFile->Write(&temp, sizeof(temp)); // quality
			};*/


void Update(void);

protected:
//   friend CGearTracker;

   static char* _psName;
   int _numVoices;
   Voice _voices[SAMPLER_MAX_POLYPHONY];
   dsp::Cubic _resampler;

   void PerformFx(int voice);
   void VoiceWork(int numsamples, int voice);
   void NoteOff(int voice);
   void NoteOffFast(int voice);
   int VoiceTick(int channel, PatternEntry* pData);
   inline void TickEnvelope(int voice);
   inline void TickFilterEnvelope(int voice);
   unsigned char lastInstrument[MAX_TRACKS];
   static inline int alteRand(int x)
   {
     return (x*rand())/32768;
   };
};

#endif
