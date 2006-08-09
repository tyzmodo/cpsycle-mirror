/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "wavefileout.h"


namespace psycle
{
	namespace host
	{

		WaveFileOut::WaveFileOut()
			: AudioDriver()
		{
			kill_thread = 0;
		}


		WaveFileOut::~WaveFileOut()
		{
		}

		void WaveFileOut::setFileName( const std::string & fileName )
		{
			fileName_ = fileName;
		}

		const std::string & WaveFileOut::fileName( ) const
		{
			return fileName_;
		}

		bool WaveFileOut::Enable( bool e )
		{
			bool _recording = false;
			if (e) {
					kill_thread = 0;
					if(_outputWaveFile.OpenForWrite(fileName().c_str(), _samplesPerSec, _bitDepth, _channelmode ) == DDC_SUCCESS)
							_recording = true;
					else
							_recording = false;
					if (_recording) {
							pthread_create(&threadid, NULL, (void*(*)(void*))audioOutThread, (void*) this);
					}

			} else { // disable fileout
				_outputWaveFile.Close();
				_recording = false;
			}
			return _recording;
		}

		int WaveFileOut::audioOutThread( void * ptr )
		{
			
		}

	}
}






