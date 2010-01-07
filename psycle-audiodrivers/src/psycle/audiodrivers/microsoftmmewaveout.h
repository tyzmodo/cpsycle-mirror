/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#pragma once

#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
#include "audiodriver.h"
#include <universalis/stdlib/cstdint.hpp>
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm")
#undef min
#undef max
namespace psycle { namespace core {                                      

using namespace universalis::stdlib;

///\todo work in progress
/// status working, restarting etc not working
///\todo freeing and configure    
class MsWaveOut : public AudioDriver {
	public:
		MsWaveOut();
		~MsWaveOut();

		/*override*/ AudioDriverInfo info() const;

	protected:
		/*override*/ void do_open() {}
		/*override*/ void do_start();
		/*override*/ void do_stop();
		/*override*/ void do_close() {}

	private:
		std::int16_t *buf;
		// mme variables
		HWAVEOUT hWaveOut;   // device handle
		static CRITICAL_SECTION waveCriticalSection;
		static WAVEHDR*         waveBlocks; // array of header structure, 
		// that points to a block buffer
		static volatile /* why volatile ? */ int waveFreeBlockCount;
		static int waveCurrentBlock;

		// mme functions

		// waveOut interface notifies about device is opened, closed, 
		// and what we handle here, when a block finishes.
		static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);      
		WAVEHDR* allocateBlocks();
		static void freeBlocks( WAVEHDR* blockArray );

		// writes a intermediate buffer into a ring buffer to the sound card
		void writeAudio( HWAVEOUT hWaveOut, LPSTR data, int size );

		// thread , the writeAudio loop is in
		// note : waveOutproc is a different, thread, too, but we cant
		// use all winapi calls there we need due to restrictions of the winapi
		HANDLE _hThread;
		static DWORD WINAPI audioOutThread( void *pWaveOut );
		static bool _running; // check, if thread loop should be left
		void fillBuffer();

		bool _dither;
};

}}
#endif