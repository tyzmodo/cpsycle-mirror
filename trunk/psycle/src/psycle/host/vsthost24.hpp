///\file
///\brief interface file for psycle::host::vsthost
#pragma once
#include "machine.hpp"
#include <seib-vsthost/CVSTHost.Seib.hpp>
#include <cstring>
namespace psycle
{
	namespace host
	{
		namespace vst
		{
			// Maximum number of Audio Input/outputs
			// \todo : this shouldn't be a static value. Host should ask the plugin and the array get created dynamically.
			const int max_io = 16;
			// Dialog max ticks for parameters.
			const int quantization = 65535;
			const int MAX_VST_EVENTS = 128;

			// The real VstEvents in the SDK is defined as events[2], so  it cannot be used from a Host point of view.
			typedef struct VstEventsDynamicstruct
			{
				VstInt32 numEvents;		///< number of Events in array
				VstIntPtr reserved;		///< zero (Reserved for future use)
				VstEvent* events[MAX_VST_EVENTS];	///< event pointer array, variable size
			} VstEventsDynamic;

			using namespace seib::vst;


			class host;
			class CVstEffectWnd;

			class plugin : public Machine, public CEffect
			{
			protected:
				class note
				{
				public:
					unsigned char key;
					unsigned char midichan;
				};
				/// midi events queue, is sent to processEvents.
				note trackNote[MAX_TRACKS];
				VstMidiEvent midievent[MAX_VST_EVENTS];
				VstEventsDynamic mevents;
				bool NSActive[16];
				int	NSSamples[16];
				int NSDelta[16];
				int NSDestination[16];
				int NSTargetDistance[16];
				int NSCurrent[16];
				static int pitchWheelCentre;
				int rangeInSemis;
				int currentSemi[16];
				int oldNote[16];
				int	queue_size;
				/// reserves space for a new midi event in the queue.
				/// \return midi event to be filled in, or null if queue is full.
				VstMidiEvent* reserveVstMidiEvent();
				VstMidiEvent* reserveVstMidiEventAtFront(); // ugly hack

				float * inputs[max_io];
				float * outputs[max_io];
				float * _pOutSamplesL;
				float * _pOutSamplesR;
				static float junk[STREAM_SIZE];
				std::string _sDllName;
				std::string _sProductName;
				std::string _sVendorName;

				/// It needs to use Process
				bool requiresProcess;
				/// It needs to use ProcessReplacing
				bool requiresRepl;

			public:
				plugin(LoadedAEffect &loadstruct);
				//this constructor is to be used with the old Song loading routine, in order to create an "empty" plugin.
				plugin(AEffect *effect):CEffect(effect)
				{
					queue_size = 0;
					requiresRepl = 0;
					requiresProcess = 0;
					_nCols=0;
				};
				virtual ~plugin() {};

				// Machine overloaded functions
				//////////////////////////////////////////////////////////////////////////
				// Actions
				virtual void Init(){ Machine::Init();}
				virtual void PreWork(int numSamples,bool clear=true);
				virtual void Work(int numSamples);
				virtual void Tick() { Machine::Tick(); };
				virtual void Tick(int track, PatternEntry * pData);
				virtual void Stop();
				// old fileformat {
				virtual bool PreLoad(RiffFile * pFile, unsigned char &_program, int &_instance);
				virtual bool LoadFromMac(vst::plugin *pMac);
				virtual bool LoadChunk(RiffFile* pFile);
				// }
				virtual bool IsShellMaster() { return (GetPlugCategory() == kPlugCategShell); }
				virtual int GetShellIdx() { return ( IsShellPlugin()) ? uniqueId() : 0;	}
				virtual int GetPluginCategory() { return GetPlugCategory(); }
				virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
				virtual void SaveSpecificChunk(RiffFile * pFile);
				virtual bool Bypass(void) { return Machine::Bypass(); }
				virtual void Bypass(bool e)
				{
					Machine::Bypass(e);
					if (aEffect) 
					{
						if (!bCanBypass) MainsChanged(!e);
						SetBypass(e);
					}
				}
				virtual bool Standby() { return Machine::Standby(); }
				virtual void Standby(bool e)
				{
					Machine::Standby();
					if (aEffect && _mode != MACHMODE_GENERATOR)
					{
						// some plugins ( psp vintage warmer ) might not like to change the state too
						// frequently, or might have a delay which mades fast switching unusable.
						// This is why this is commented out until another solution is found.
//						if (!bCanBypass) MainsChanged(!e);
						SetBypass(e);
					}
				}
//				virtual bool ConnectTo(Machine& dstMac,int dstport=0,int outport=0,float volume=1.0f);
//				virtual bool Disconnect(Machine& dstMac);

				bool AddMIDI(unsigned char data0, unsigned char data1 = 0, unsigned char data2 = 0, unsigned int sampleoffset=0);
				bool AddNoteOn(unsigned char channel, unsigned char key, unsigned char velocity, unsigned char midichannel = 0, unsigned int sampleoffset=0,bool slide=false);
				bool AddNoteOff(unsigned char channel, unsigned char midichannel = 0, bool addatStart = false, unsigned int sampleoffset=0);
				inline void SendMidi();
				int LSB(int val)
				{				
					return val & 0x7f;
				}

				int MSB(int val)
				{				
					return (val & 0x3F80) >> 7;
				}
				// Properties
				//////////////////////////////////////////////////////////////////////////
				virtual void SetSampleRate(int sr) { CEffect::SetSampleRate((float)sr); }
				//\todo:
				virtual const char * const GetDllName() const throw() { return _sDllName.c_str(); }
				virtual char * GetName() throw() { return (char*)_sProductName.c_str(); }
				inline const char * const GetVendorName() const throw() { return _sVendorName.c_str(); }
				virtual const std::uint32_t GetAPIVersion() { return GetVstVersion(); }
				virtual const std::uint32_t GetVersion() { return GetVendorVersion(); }
				//
				virtual void GetParamRange(int numparam,int &minval, int &maxval) {	minval = 0; maxval = quantization; }
				virtual int GetNumParams() { return numParams(); }
				virtual void GetParamName(int numparam, char * parval) { if (numparam<numParams()) CEffect::GetParamName(numparam,parval); }
				virtual void GetParamValue(int numparam, char * parval);
				virtual int GetParamValue(int numparam)
				{
					if(numparam < numParams())
						return f2i(GetParameter(numparam) * quantization);
					return 0;
				}
				virtual bool SetParameter(int numparam, int value)
				{
					if(numparam < numParams())
					{
						CEffect::SetParameter(numparam,float(value)/float(quantization));
						return true;
					}
					return false;
				}
				virtual void SetParameter(int numparam, float value) { if(numparam < numParams())CEffect::SetParameter(numparam,value); }
				virtual bool DescribeValue(int parameter, char * psTxt);

				virtual void InsertOutputWireIndex(int wireIndex,int dstmac)
				{ MainsChanged(false); ConnectOutput(0,true); ConnectOutput(1,true); Machine::InsertOutputWireIndex(wireIndex,dstmac);  MainsChanged(true); }
				virtual void InsertInputWireIndex(int wireIndex,int srcmac,float wiremultiplier,float initialvol=1.0f)
				{ MainsChanged(false); ConnectInput(0,true); ConnectInput(1,true); Machine::InsertInputWireIndex(wireIndex,srcmac,wiremultiplier,initialvol);  MainsChanged(true); }
				virtual void DeleteOutputWireIndex(int wireIndex)
				{ MainsChanged(false); ConnectOutput(0,false); ConnectOutput(1,false); Machine::DeleteOutputWireIndex(wireIndex);  MainsChanged(true); }
				virtual void DeleteInputWireIndex(int wireIndex)
				{ MainsChanged(false); ConnectInput(0,false); ConnectInput(1,false); Machine::DeleteInputWireIndex(wireIndex);  MainsChanged(true); }
				virtual float GetAudioRange(){ return 1.0f; }



				// CEffect overloaded functions
				//////////////////////////////////////////////////////////////////////////
				virtual void EnterCritical() {;}
				virtual void LeaveCritical() {;}
				virtual bool WillProcessReplace() { return !requiresProcess && (CanProcessReplace() || requiresRepl); }
				/// IsIn/OutputConnected are called when the machine receives a mainschanged(on), so the correct way to work is
				/// doing an "off/on" when a connection changes.
				virtual bool DECLARE_VST_DEPRECATED(IsInputConnected)(int input) { return ((input < 2)&& (_numInputs!=0)); } 
				virtual bool DECLARE_VST_DEPRECATED(IsOutputConnected)(int output) { return ((output < 2) && (_numOutputs!=0)); }
				// AEffect asks host about its input/outputspeakers.
				virtual VstSpeakerArrangement* OnHostInputSpeakerArrangement() { return 0; }
				virtual VstSpeakerArrangement* OnHostOutputSpeakerArrangement() { return 0; }
				// AEffect informs of changed IO. verify numins/outs, speakerarrangement and the likes.
				virtual bool OnIOChanged() { return false; }
/*
				virtual void SetEditWnd(CEffectWnd* wnd)
				{
					CEffect::SetEditWnd(wnd);
					editorWindow = reinterpret_cast<CVstEffectWnd*>(wnd);
				}
*/
			};

			class host : public CVSTHost
			{
			public:
				host(){	quantization = 0xFFFF; SetBlockSize(STREAM_SIZE); SetTimeSignature(4,4); vstTimeInfo.smpteFrameRate = kVstSmpte25fps; };
				virtual ~host(){;}

				///< Helper class for Machine Creation.
				//static Machine* CreateFromType(int _id, std::string _dllname);
				virtual CEffect * CreateEffect(LoadedAEffect &loadstruct) { return new plugin(loadstruct); }
				virtual CEffect * CreateWrapper(AEffect *effect) { return new plugin(effect); }

				///> Plugin gets Info from the host
				virtual bool OnGetProductString(char *text) { strcpy(text, "Psycle"); return true; }
				virtual long OnGetHostVendorVersion() { return 1850; }
				virtual bool OnCanDo(CEffect &pEffect,const char *ptr);
				virtual long OnGetHostLanguage() { return kVstLangEnglish; }
				virtual void CalcTimeInfo(long lMask = -1);
				virtual long DECLARE_VST_DEPRECATED(OnTempoAt)(CEffect &pEffect, long pos);
				virtual long DECLARE_VST_DEPRECATED(OnGetNumAutomatableParameters)(CEffect &pEffect) { return 0xFF; } // the size of the aux column.
				virtual long OnGetAutomationState(CEffect &pEffect);
				virtual long OnGetInputLatency(CEffect &pEffect);
				virtual long OnGetOutputLatency(CEffect &pEffect);
				//\todo : how can this function be implemented? :o
				virtual long OnGetCurrentProcessLevel(CEffect &pEffect) { return 0; }
				virtual bool OnWillProcessReplacing(CEffect &pEffect) { return ((plugin*)&pEffect)->WillProcessReplace(); }

				///> Plugin sends actions to the host
				virtual bool OnProcessEvents(CEffect &pEffect, VstEvents* events) { return false; }
			};
		}
	}
}