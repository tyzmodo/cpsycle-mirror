#pragma once
#pragma unmanaged
/** @file
 *  @brief 
 *  $Date$
 *  $Revision$
 */

#pragma pack(1)
namespace psycle{
	namespace host{
		static const char * XM_HEADER = "extended module: ";

		struct XMCMD {
			//	(*) = If the command byte is zero, the last nonzero byte for the command should be used.
			//  (0) = It is executed (also) on tick 0, else, it does nothing on tick 0.
			//  (0!) = It is executed *only* on tick 0.
			enum{
			  ARPEGGIO		=	0x00,// Arpeggio. If arpeggio value is zero, then command= NONE.
			  PORTAUP		=	0x01,// Portamento Up		 (*)
			  PORTADOWN		=	0x02,// Portamento Down	 (*)
			  PORTA2NOTE	=	0x03,//  Tone Portamento	 (*)
			  VIBRATO		=	0x04,// Do Vibrato		 (*)
			  TONEPORTAVOL	=	0x05,// Tone Portament & Volume Slide (*)
			  VIBRATOVOL	=	0x06,// Vibrato & Volume Slide (*)
			  TREMOLO		=	0x07,// Tremolo			 (*)
			  PANNING		=	0x08,// Set Panning Position
			  OFFSET		=	0x09,// Set Sample Offset
			  VOLUMESLIDE	=	0x0a,// Volume Slide		 (*)
			  POSITION_JUMP	=	0x0b,// Position Jump
			  VOLUME		=	0x0c,// Set Volume
			  PATTERN_BREAK	=	0x0d,// Pattern Break
			  EXTENDED		=	0x0e,// Extend Command
			  SETSPEED		=	0x0f,// Set Speed or BPM
			  SET_GLOBAL_VOLUME	=	0x10,// Set Global Volume
			  GLOBAL_VOLUME_SLIDE =	0x11,// Global Volume Slide (*)
			  NOTE_OFF		=	0x14,// Note Off
			  SET_ENV_POSITION =	0x15,// Set Envelope Position
			  PANNINGSLIDE	=	0x19,// PANNING SLIDE		 (*)
			  RETRIG			=	0x1B,// Retrigger Note	 (*)
			  TREMOR			=	0x1D,// Tremor
			  EXTEND_XM_EFFECTS =	0x21,// Extend XM Effects	
			  PANBRELLO		=	0x22,// Panbrello
			  MIDI_MACRO		=	0x23// Panbrello
			};
		};
		struct XMCMD_E
		{
			enum{
			  E_FINE_PORTA_UP	=	0x10, //E1 (*) Fine porta up
			  E_FINE_PORTA_DOWN	=	0x20, //E2 (*) Fine porta down
			  E_GLISSANDO_WAVE	=	0x30, //E3     Set gliss control 
			  E_VIBRATO_WAVE	=	0x40, //E4     Set vibrato control
			  E_FINETUNE		=	0x50, //E5     Set finetune
			  E_PATTERN_LOOP	=	0x60, //E6     Set loop begin/loop
			  E_TREMOLO_WAVE	=	0x70, //E7     Set tremolo control
			  E_MOD_RETRIG		=	0x90, //E9     Retrig note
			  E_FINE_VOLUME_UP	=	0xA0, //EA (*) Fine volume slide up
			  E_FINE_VOLUME_DOWN=	0xB0, //EB (*) Fine volume slide down
			  E_DELAYED_NOTECUT	=	0xC0, //EC     Note cut
			  E_NOTE_DELAY		=	0xD0, //ED     Note delay
			  E_PATTERN_DELAY	=	0xE0, //EE     Pattern delay
			  E_SET_MIDI_MACRO	=	0xF0
			};
		};

		struct XMCMD_X
		{
			enum{
			  X_EXTRA_FINE_PORTA_UP	=	0x10, //X1 (*) Extra fine porta up
			  X_EXTRA_FINE_PORTA_DOWN =	0x20, //X2 (*) Extra fine porta downp
			  X9					=	0x90, //X9 (*) Modplug's Extension
			  X_HIGH_OFFSET			=	0xA0
			};
		};
		struct XMCMD_X9
		{
			enum{
			  X9_SURROUND_OFF	=	0x0,
			  X9_SURROUND_ON		=	0x1,
			  X9_REVERB_OFF		=	0x8,
			  X9_REVERB_FORCE	=	0x9,
			  X9_STANDARD_SURROUND =	0xA,
			  X9_QUAD_SURROUND	=	0xB,// Select quad surround mode: this allows you to pan in the rear channels, especially useful for 4-speakers playback. Note that S9A and S9B do not activate the surround for the current channel, it is a global setting that will affect the behavior of the surround for all channels. You can enable or disable the surround for individual channels by using the S90 and S91 effects. In quad surround mode, the channel surround will stay active until explicitely disabled by a S90 effect
			  X9_GLOBAL_FILTER	=	0xC,// Select global filter mode (IT compatibility). This is the default, when resonant filters are enabled with a Zxx effect, they will stay active until explicitely disabled by setting the cutoff frequency to the maximum (Z7F), and the resonance to the minimum (Z80).
			  X9_LOCAL_FILTER	=	0xD,// Select local filter mode (MPT beta compatibility): when this mode is selected, the resonant filter will only affect the current note. It will be deactivated when a new note is being played.
			  X9_PLAY_FORWARD	=	0xE,// Play forward. You may use this to temporarily force the direction of a bidirectional loop to go forward.
			  X9_PLAY_BACKWARD	=	0xF // Play backward. The current instrument will be played backwards, or it will temporarily set the direction of a loop to go backward. 
			};
		};


		static const UINT XMVOLCMD_EXG_TABLE[16] = 
		{
			0,
				XMCMD::VOLUME,
				XMCMD::VOLUME,
				XMCMD::VOLUME,
				XMCMD::VOLUME,
				XMCMD::VOLUME,
				XMCMD::VOLUMESLIDE, //down
				XMCMD::VOLUMESLIDE, //up
				XMCMD::VOLUMESLIDE, //down
				XMCMD::VOLUMESLIDE, //up
				XMCMD::VIBRATO, // speed
				XMCMD::VIBRATO, // depth  ( vibrato trigger )
				XMCMD::PANNING,
				XMCMD::PANNINGSLIDE,
				XMCMD::PANNINGSLIDE,
				XMCMD::PORTA2NOTE
		};

	struct XMFILEHEADER
	{
		DWORD size;
		WORD norder;
		WORD restartpos;
		WORD channels;
		WORD patterns;
		WORD instruments;
		WORD flags;
		WORD speed;
		WORD tempo;
		BYTE order[256];
	};


	struct XMINSTRUMENTHEADER
	{
		DWORD size;
		CHAR name[22];
		BYTE type;
		BYTE samples;
		BYTE samplesh;
	};

	struct XMSAMPLEHEADER
	{
		DWORD shsize;
		BYTE snum[96];
		WORD venv[24];
		WORD penv[24];
		BYTE vnum, pnum;
		BYTE vsustain, vloops, vloope, psustain, ploops, ploope;
		BYTE vtype, ptype;
		BYTE vibtype, vibsweep, vibdepth, vibrate;
		WORD volfade;
		WORD res;
		BYTE reserved1[20];
	};

	struct XMSAMPLESTRUCT
	{
		DWORD samplen;
		DWORD loopstart;
		DWORD looplen;
		BYTE vol;
		signed char finetune;
		BYTE type;
		BYTE pan;
		signed char relnote;
		BYTE res;
		char name[22];
	};
}
}
#pragma pack()