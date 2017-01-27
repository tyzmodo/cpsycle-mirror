///\file
///\brief interface file for psycle::host::Global.
#pragma once
#include <psycle/host/detail/project.hpp>
#include "version.hpp"
#include "Constants.hpp"
#include "SongStructs.hpp"
#include <Shlwapi.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
//#include <universalis/os/winutf8wrapper.hpp>

// AVRT is the new "multimedia scheduling stuff"
typedef HANDLE (WINAPI *FAvSetMmThreadCharacteristics)   (LPCTSTR,LPDWORD);
typedef BOOL   (WINAPI *FAvRevertMmThreadCharacteristics)(HANDLE);

struct lua_State;

namespace psycle
{
	namespace host
	{
		class Configuration;
		class Song;
		class Player;
		class MachineLoader;
		namespace vst
		{
			class Host;
		}

		extern BOOL Is_Vista_or_Later();

		inline BOOL IsWin64() {
			SYSTEM_INFO si;
			ZeroMemory(&si, sizeof(SYSTEM_INFO));
			GetSystemInfo(&si);
			return (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64);
		}
		inline BOOL IsWow64() {
			return IsOS(OS_WOW6432);
		}
    
    // inherit your class from Timer and override OnTimerViewRefresh to get
    // CChildview ViewRefreshTimer ticks    
    
    class Global
		{
			public:
				Global();
				virtual ~Global() throw();

				static FAvSetMmThreadCharacteristics    pAvSetMmThreadCharacteristics;
				static FAvRevertMmThreadCharacteristics pAvRevertMmThreadCharacteristics;
        
				static inline Song           & song() { return *pSong; }
				static inline Player         & player(){ return *pPlayer; }
				static inline Configuration  & configuration(){ return *pConfig; }
				static inline vst::Host		 & vsthost(){ return *pVstHost; }
				static inline MachineLoader  & machineload() { return *pMacLoad; }        

			protected:
				static Configuration * pConfig;
				static Song * pSong;
				static Player * pPlayer;
				static vst::Host * pVstHost;
				static MachineLoader * pMacLoad;
		};

    class HostViewPort {
     public:
      virtual ~HostViewPort() {}
      virtual void OnHostViewportChange(class LuaPlugin& plugin, int viewport) {}
      virtual void HideExtensionView() {}
      virtual void OnAddViewMenu(class Link& link) {}
      virtual void OnAddHelpMenu(class Link& link) {}
      virtual void OnReplaceHelpMenu(class Link& link, int pos) {}
      virtual void OnAddWindowsMenu(class Link& link) {}
      virtual void OnRemoveWindowsMenu(class LuaPlugin* plugin) {}
      virtual void OnChangeWindowsMenuText(class LuaPlugin* plugin) {}
      virtual void OnPluginCanvasChanged(class LuaPlugin& plugin) {} 
    };
 
	}
}
