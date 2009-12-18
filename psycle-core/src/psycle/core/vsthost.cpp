/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
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


#include <psycle/core/vsthost.h>
#if !defined _WIN64 && !defined _WIN32
	#if defined DIVERSALIS__COMPILER__GNU
		#warning ###########################- UNIMPLEMENTED ###################
	#endif
#else

#include <psycle/core/pluginfinder.h>
#include <psycle/core/vstplugin.h>
#include <psycle/core/file.h>
#include <psycle/core/playertimeinfo.h>

#include <iostream>
#include <sstream>
#if defined __unix__ || defined __APPLE__
	#include <dlfcn.h>
#elif defined _WIN32
	#include <windows.h>
#endif
namespace psycle {namespace core { namespace vst {

host::host(MachineCallbacks*calls)
:MachineHost(calls){
	master = new AudioMaster(calls);
}
host::~host()
{
	delete master;
}

host& host::getInstance(MachineCallbacks* callb)
{
	static host instance(callb);
	return instance;
}

Machine* host::CreateMachine(PluginFinder& finder, const MachineKey& key,Machine::id_type id) 
{
	if (key == MachineKey::wrapperVst() ) {
		return static_cast<vst::plugin*>(master->CreateWrapper(0));
	}
	//FIXME: This is a good place where to use exceptions. (task for a later date)
	if (!finder.hasKey(key)) return 0;
	std::string fullPath = finder.lookupDllName(key);
	if (fullPath.empty()) return 0;

	vst::plugin* plug = master->LoadPlugin(fullPath, key, id);
	plug->Init();
	return plug;
}


void host::FillPluginInfo(const std::string& fullName, const std::string& fileName, PluginFinder& finder)
{
	#if defined __unix__ || defined __APPLE__
		if ( fileName.find( ".dll" ) == std::string::npos ) return;
	#else
		if ( fileName.find( ".dll" ) == std::string::npos ) return;
	#endif
	
	vst::plugin *vstPlug=0;
	std::ostringstream sIn;
	MachineKey key( hostCode(), fileName);

	try
	{
		vstPlug = master->LoadPlugin(fullName, key, 0);
	}
	catch(const std::exception & e)
	{
		sIn << typeid(e).name() << std::endl;
		if(e.what()) sIn << e.what(); else sIn << "no message"; sIn << std::endl;
	}
	catch(...)
	{
		sIn << "Type of exception is unknown, cannot display any further information." << std::endl;
	}
	if(!sIn.str().empty())
	{
		PluginInfo pinfo;
		pinfo.setError(sIn.str());

		std::cout << "### ERRONEOUS ###" << std::endl;
		std::cout.flush();
		std::cout << pinfo.error();
		std::cout.flush();
		std::stringstream title; 
		title << "Machine crashed: " << fileName;

		pinfo.setAllow(false);
		pinfo.setName("???");
		pinfo.setAuthor("???");
		pinfo.setDesc("???");
		pinfo.setLibName(fullName);
		pinfo.setVersion("???");
		MachineKey key( hostCode(), fileName, 0);
		finder.AddInfo( key, pinfo);
		if (vstPlug) delete vstPlug;
	}
	else
	{
		if (vstPlug->IsShellMaster())
		{
			char tempName[64] = {0}; 
			VstInt32 plugUniqueID = 0;
			while ((plugUniqueID = vstPlug->GetNextShellPlugin(tempName)) != 0)
			{ 
				if (tempName[0] != 0)
				{
					PluginInfo pinfo;
					pinfo.setLibName(fullName);
					//todo!
					//pinfo.setFileTime();

					pinfo.setAllow(true);
					{
						std::ostringstream s;
						s << vstPlug->GetVendorName() << " " << tempName;
						pinfo.setName(s.str());
					}
					pinfo.setAuthor(vstPlug->GetVendorName());
					pinfo.setRole( vstPlug->IsSynth()?MachineRole::GENERATOR : MachineRole::EFFECT );

					{
						std::ostringstream s;
						s << (vstPlug->IsSynth() ? "VST Shell instrument" : "VST Shell effect") << " by " << vstPlug->GetVendorName();
						pinfo.setDesc(s.str());
					}
					{
						std::ostringstream s;
						s << std::hex << vstPlug->GetVersion();
						pinfo.setVersion(s.str());
					}
					MachineKey keysubPlugin( hostCode(), fileName, plugUniqueID);
					finder.AddInfo( keysubPlugin, pinfo);
				}
			}
		}
		else
		{
			PluginInfo pinfo;
			pinfo.setAllow(true);
			pinfo.setLibName(fullName);
			//todo!
			//pinfo.setFileTime();
			pinfo.setName(vstPlug->GetName());
			pinfo.setAuthor(vstPlug->GetVendorName());
			pinfo.setRole( vstPlug->IsSynth()?MachineRole::GENERATOR : MachineRole::EFFECT );

			{
				std::ostringstream s;
				s << (vstPlug->IsSynth() ? "VST instrument" : "VST effect") << " by " << vstPlug->GetVendorName();
				pinfo.setDesc(s.str());
			}
			{
				std::ostringstream s;
				s << vstPlug->GetVersion();
				pinfo.setVersion(s.str());
			}
			MachineKey key( hostCode(), fileName, 0);
			finder.AddInfo( key, pinfo);
		}
		std::cout << vstPlug->GetName() << " - successfully instanciated";
		std::cout.flush();

		// [bohan] vstPlug is a stack object, so its destructor is called
		// [bohan] at the end of its scope (this cope actually).
		// [bohan] The problem with destructors of any object of any class is that
		// [bohan] they are never allowed to throw any exception.
		// [bohan] So, we catch exceptions here by calling vstPlug.Free(); explicitly.
		try
		{
			delete vstPlug;
			// [bohan] phatmatik crashes here...
			// <magnus> so does PSP Easyverb, in FreeLibrary
		}
		catch(const std::exception & e)
		{
			std::stringstream s; s
				<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
				<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
				<< typeid(e).name() << std::endl;
			if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
			std::cout
				<< std::endl
				<< "### ERRONEOUS ###" << std::endl
				<< s.str().c_str();
			std::cout.flush();
			std::stringstream title; title
				<< "Machine crashed: " << fileName;
		}
		catch(...)
		{
			std::stringstream s; s
				<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
				<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
				<< "Type of exception is unknown, no further information available.";
			std::cout
				<< std::endl
				<< "### ERRONEOUS ###" << std::endl
				<< s.str().c_str();
			std::cout.flush();
			std::stringstream title; title
				<< "Machine crashed: " << fileName;
		}
	}
}
//==============================================================
//==============================================================

vst::plugin* AudioMaster::LoadPlugin(std::string fullName, MachineKey key, Machine::id_type id) {
	currentKey = key;
	currentId = id;
	return dynamic_cast<vst::plugin*>(CVSTHost::LoadPlugin(fullName.c_str(),key.index()));
}
CEffect * AudioMaster::CreateEffect(LoadedAEffect &loadstruct)
{
	return new plugin(pCallbacks,currentKey,currentId,loadstruct);
}

CEffect * AudioMaster::CreateWrapper(AEffect *effect)
{
	return new plugin(pCallbacks,currentId,effect);
}


void AudioMaster::CalcTimeInfo(long lMask)
{
	///\todo: cycleactive and recording to a "Start()" function.
	// automationwriting and automationreading.
	//
	/*
	kVstTransportCycleActive = 1 << 2,
	kVstTransportRecording   = 1 << 3,

	kVstAutomationWriting    = 1 << 6,
	kVstAutomationReading    = 1 << 7,
	*/

	//kVstCyclePosValid = 1 << 12, // start and end
	// cyclestart // locator positions in quarter notes.
	// cycleend   // locator positions in quarter notes.
	const PlayerTimeInfo &info = pCallbacks->timeInfo();

	if(lMask & kVstPpqPosValid)
	{
		vstTimeInfo.flags |= kVstPpqPosValid;
		vstTimeInfo.ppqPos = info.playBeatPos();
		// Disable default handling.
		lMask &= ~kVstPpqPosValid;
	}

	CVSTHost::CalcTimeInfo(lMask);
}


bool AudioMaster::OnCanDo(CEffect &pEffect, const char *ptr) const
{
	using namespace seib::vst::HostCanDos;
	bool value =  CVSTHost::OnCanDo(pEffect,ptr);
	if (value) return value;
	else if (
		//|| (!strcmp(ptr, canDoReceiveVstEvents)) // "receiveVstEvents",
		//|| (!strcmp(ptr, canDoReceiveVstMidiEvent )) // "receiveVstMidiEvent",
		//|| (!strcmp(ptr, "receiveVstTimeInfo" )) // DEPRECATED

		(!strcmp(ptr, canDoReportConnectionChanges )) // "reportConnectionChanges",
		//|| (!strcmp(ptr, canDoAcceptIOChanges )) // "acceptIOChanges",
		||(!strcmp(ptr, canDoSizeWindow )) // "sizeWindow",

		//|| (!strcmp(ptr, canDoAsyncProcessing )) // DEPRECATED
		//|| (!strcmp(ptr, canDoOffline )) // "offline",
		//|| (!strcmp(ptr, "supportShell" )) // DEPRECATED
		//|| (!strcmp(ptr, canDoEditFile )) // "editFile",
		//|| (!strcmp(ptr, canDoSendVstMidiEventFlagIsRealtime ))
		)
		return true;
	return false; // by default, no
}

long AudioMaster::DECLARE_VST_DEPRECATED(OnTempoAt)(CEffect &pEffect, long pos) const
{
	//\todo: return the real tempo in the future, not always the current one
	// pos in Sample frames, return bpm* 10000
	return vstTimeInfo.tempo * 10000;
}
long AudioMaster::OnGetOutputLatency(CEffect &pEffect) const
{
	return pCallbacks->timeInfo().outputLatency();
}
long AudioMaster::OnGetInputLatency(CEffect &pEffect) const
{
	return pCallbacks->timeInfo().inputLatency();
}
void AudioMaster::Log(std::string message)
{
	//todo
}
bool AudioMaster::OnWillProcessReplacing(CEffect &pEffect) const {
	return ((plugin*)&pEffect)->WillProcessReplace();
}


///\todo: Get information about this function
long AudioMaster::OnGetAutomationState(CEffect &pEffect) const { return kVstAutomationUnsupported; }


}}}

#endif