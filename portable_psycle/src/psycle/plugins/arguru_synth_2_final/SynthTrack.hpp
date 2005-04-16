#pragma once
#include "filter.hpp"
///\file SynthTrack.h
///\brief interface for the CSynthTrack class.

#define FILTER_CALC_TIME	64
#define TWOPI				6.28318530717958647692528676655901f

struct SYNPAR
{
	signed short *pWave;
	signed short *pWave2;
	int osc2detune;
	int osc2finetune;
	int osc2sync;
	int amp_env_attack;
	int amp_env_decay;
	int amp_env_sustain;
	int amp_env_release;
	int vcf_env_attack;
	int vcf_env_decay;
	int vcf_env_sustain;
	int vcf_env_release;
	int vcf_lfo_speed;
	int vcf_lfo_amplitude;
	int vcf_cutoff;
	int vcf_resonance;
	int vcf_type;
	int vcf_envmod;
	int osc_mix;
	int out_vol;
	int arp_mod;
	int arp_bpm;
	int arp_cnt;
	int globaldetune;
	int globalfinetune;
	int synthglide;
	int interpolate;
};

class CSynthTrack  
{
public:

	void InitEffect(int cmd,int val);
	void PerformFx();
	void DoGlide();
	void DisableVibrato();
	void ActiveVibrato(int speed,int depth);
	void Vibrate();
	float Filter(float x);
	void NoteOff(bool stop=false);
	float GetEnvAmp();
	void GetEnvVcf();
	float oscglide;
	float GetSample();
	float GetSampleOsc1();
	float GetSampleOsc2();
	void NoteOn(int note, SYNPAR *tspar,int spd);
	void InitArpeggio();
	
	CSynthTrack();
	virtual ~CSynthTrack();

	int AmpEnvStage;
	int NoteCutTime;
	bool NoteCut;
	filter m_filter;

private:
	float output;
	float lfo_freq;
	float lfo_phase;

	void InitLfo(int freq,int amp);

	short timetocompute;
	void InitEnvelopes(bool force=false);

	float VcfResonance;
	int sp_cmd;
	int sp_val;

	float OSC1Speed;
	float OSC2Speed;
	float ROSC1Speed;
	float ROSC2Speed;
	
	float OSC1Position;
	float OSC2Position;
	
	float OSCvib;
	float VibratoGr;
	float VibratoSpeed;
	float VibratoDepth;

	// Arpeggiator
	int Arp_tickcounter;
	int Arp_samplespertick;
	float Arp_basenote;
	int ArpMode;
	unsigned char ArpCounter;
	unsigned char ArpLimit;

	signed char ArpNote[16][16];

	bool Arp;

	// Envelope [Amplitude]
	float AmpEnvValue;
	float AmpEnvCoef;
	float AmpEnvSustainLevel;
	bool vibrato;
	float OSC1Vol;
	float OSC2Vol;

	// Envelope [Filter]
	float VcfEnvValue;
	float VcfEnvCoef;
	float VcfEnvSustainLevel;
	int VcfEnvStage;
	float VcfEnvMod;
	float VcfCutoff;
	float synthglide;
	
	SYNPAR *syntp;

	inline void ArpTick(void);
	inline void FilterTick(void);
	inline int f2i(double d)
	{
		const double magic = 6755399441055744.0; // 2^51 + 2^52
		double tmp = (d-0.5) + magic;
		return *(int*) &tmp;
	};

};

inline void CSynthTrack::ArpTick()
{
	Arp_tickcounter=0;

	float note=Arp_basenote+(float)ArpNote[syntp->arp_mod-1][ArpCounter];
	OSC1Speed=(float)pow(2.0, note/12.0);

	float note2=note+
	(float)syntp->osc2finetune*0.0038962f+
	(float)syntp->osc2detune;
	OSC2Speed=(float)pow(2.0, note2/12.0);

	if(++ArpCounter>=syntp->arp_cnt)  ArpCounter=0;

	InitEnvelopes();
}

inline void CSynthTrack::FilterTick()
{
	lfo_phase+=lfo_freq;

	if (lfo_phase>TWOPI) lfo_phase-=TWOPI;
	
	float const VcfLfoVal=sin(lfo_phase)*(float)syntp->vcf_lfo_amplitude;
	int realcutoff=VcfCutoff+VcfLfoVal+VcfEnvMod*VcfEnvValue;

	if (realcutoff<1) realcutoff=1;
	if (realcutoff>240) realcutoff=240;

	m_filter.setfilter(syntp->vcf_type%10,realcutoff,syntp->vcf_resonance);
	timetocompute=FILTER_CALC_TIME;

}

inline float CSynthTrack::GetSample()
{

	if(AmpEnvStage)
	{
		if ((ArpMode>0) && (++Arp_tickcounter>Arp_samplespertick)) ArpTick();
	
		if ( syntp->interpolate )  // Quite Pronounced CPU usage increase...
		{
			int iOsc=f2i(OSC1Position);
			float fractpart=OSC1Position-iOsc;
			float d0=syntp->pWave[iOsc-1];
			float d1=syntp->pWave[iOsc];
			float d2=syntp->pWave[iOsc+1];
			float d3=syntp->pWave[iOsc+2];
			output=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC1Vol;
			
			iOsc=f2i(OSC2Position);
			fractpart=OSC2Position-iOsc;
			d0=syntp->pWave2[iOsc-1];
			d1=syntp->pWave2[iOsc];
			d2=syntp->pWave2[iOsc+1];
			d3=syntp->pWave2[iOsc+2];
			output+=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC2Vol;

		}
		else
		{
			output=syntp->pWave[f2i(OSC1Position)]*OSC1Vol+
				   syntp->pWave2[f2i(OSC2Position)]*OSC2Vol;
		}

		if(vibrato)
		{
			OSC1Position+=ROSC1Speed+OSCvib;
			OSC2Position+=ROSC2Speed+OSCvib;
		}
		else
		{
			OSC1Position+=ROSC1Speed;
			OSC2Position+=ROSC2Speed;
		}
		
		if(OSC1Position>=2048.0f)
		{
			OSC1Position-=2048.0f;
		
			if(syntp->osc2sync)	OSC2Position=OSC1Position;
		}

		if(OSC2Position>=2048.0f) OSC2Position-=2048.0f;

		GetEnvVcf();

		if(!timetocompute--) FilterTick();
		
		if ( syntp->vcf_type > 9 ) return m_filter.res2(output)*GetEnvAmp();
		else return m_filter.res(output)*GetEnvAmp();
	}
	else return 0;
}

inline float CSynthTrack::GetSampleOsc1()
{

	if(AmpEnvStage)
	{
		if ((ArpMode>0) && (++Arp_tickcounter>Arp_samplespertick)) ArpTick();
	
		if ( syntp->interpolate )  // Quite Pronounced CPU usage increase...
		{
			int iOsc=f2i(OSC1Position);
			float fractpart=OSC1Position-iOsc;
			float d0=syntp->pWave[iOsc-1];
			float d1=syntp->pWave[iOsc];
			float d2=syntp->pWave[iOsc+1];
			float d3=syntp->pWave[iOsc+2];
			output=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC1Vol;
		}
		else
		{
			output=syntp->pWave[f2i(OSC1Position)]*OSC1Vol;
		}

		if(vibrato)	OSC1Position+=ROSC1Speed+OSCvib;
		else		OSC1Position+=ROSC1Speed;
		
		if(OSC1Position>=2048.0f)  OSC1Position-=2048.0f;
		
		GetEnvVcf();

		if(!timetocompute--) FilterTick();

		if ( syntp->vcf_type > 9 ) return m_filter.res2(output)*GetEnvAmp();
		else return m_filter.res(output)*GetEnvAmp();
	}
	else return 0;
}
inline float CSynthTrack::GetSampleOsc2()
{
	if(AmpEnvStage)
	{
		if ((ArpMode>0) && (++Arp_tickcounter>Arp_samplespertick)) ArpTick();
	
		if ( syntp->interpolate )  // Quite Pronounced CPU usage increase...
		{
			int iOsc=f2i(OSC2Position);
			float fractpart=OSC2Position-iOsc;
			float d0=syntp->pWave2[iOsc-1];
			float d1=syntp->pWave2[iOsc];
			float d2=syntp->pWave2[iOsc+1];
			float d3=syntp->pWave2[iOsc+2];
			output=((((((((3*(d1-d2))-d0)+d3)*0.5f*fractpart)+((2*d2)+d0)-(((5*d1)+d3)*0.5f))*fractpart)+((d2-d0)*0.5f))*fractpart+d1)*OSC2Vol;
		}
		else
		{
			output=syntp->pWave2[f2i(OSC2Position)]*OSC2Vol;
		}

		if(vibrato)	OSC2Position+=ROSC2Speed+OSCvib;
		else		OSC2Position+=ROSC2Speed;
		
		if(OSC2Position>=2048.0f) OSC2Position-=2048.0f;
		
		GetEnvVcf();

		if(!timetocompute--) FilterTick();

		if ( syntp->vcf_type > 9 ) return m_filter.res2(output)*GetEnvAmp();
		else return m_filter.res(output)*GetEnvAmp();
	}
	else  return 0;
}

inline float CSynthTrack::GetEnvAmp()
{
	switch(AmpEnvStage)
	{
	case 1: // Attack
		AmpEnvValue+=AmpEnvCoef;
		
		if(AmpEnvValue>1.0f)
		{
			AmpEnvCoef=(1.0f-AmpEnvSustainLevel)/(float)syntp->amp_env_decay;
			AmpEnvStage=2;
		}

		return AmpEnvValue;
	break;

	case 2: // Decay
		AmpEnvValue-=AmpEnvCoef;
		
		if(AmpEnvValue<AmpEnvSustainLevel)
		{
			AmpEnvValue=AmpEnvSustainLevel;
			AmpEnvStage=3;

			if(!syntp->amp_env_sustain)
			AmpEnvStage=0;
		}

		return AmpEnvValue;
	break;

	case 3:
		return AmpEnvValue;
	break;

	case 4: // Release
		AmpEnvValue-=AmpEnvCoef;

		if(AmpEnvValue<0.0f)
		{
			AmpEnvValue=0.0f;
			AmpEnvStage=0;
		}

		return AmpEnvValue;
	break;
	
	case 5: // FastRelease
		AmpEnvValue-=AmpEnvCoef;

		if(AmpEnvValue<0.0f)
		{
			AmpEnvValue=0.0f;
			AmpEnvStage=1;
			AmpEnvCoef=1.0f/(float)syntp->amp_env_attack;
		}

		return AmpEnvValue;
	break;
	
	}

	return 0;
}

inline void CSynthTrack::GetEnvVcf()
{
	switch(VcfEnvStage)
	{
	case 1: // Attack
		VcfEnvValue+=VcfEnvCoef;
		
		if(VcfEnvValue>1.0f)
		{
			VcfEnvCoef=(1.0f-VcfEnvSustainLevel)/(float)syntp->vcf_env_decay;
			VcfEnvStage=2;
		}
	break;

	case 2: // Decay
		VcfEnvValue-=VcfEnvCoef;
		
		if(VcfEnvValue<VcfEnvSustainLevel)
		{
			VcfEnvValue=VcfEnvSustainLevel;
			VcfEnvStage=3;
		}
	break;

	case 4: // Release
		VcfEnvValue-=VcfEnvCoef;

		if(VcfEnvValue<0.0f)
		{
			VcfEnvValue=0.0f;
			VcfEnvStage=0;
		}
	break;

	case 5: // FastRelease
		VcfEnvValue-=VcfEnvCoef;

		if(VcfEnvValue<0.0f)
		{
			VcfEnvValue=0.0f;
			VcfEnvStage=1;
			VcfEnvCoef=1.0f/(float)syntp->vcf_env_attack;
		}

	break;
	}
}
