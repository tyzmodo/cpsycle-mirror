/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community
*   psycle.sf.net
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

#include "configuration.h"
#include "file.h"

// FIXME: these audio drivers don't belong in psycore
#include "../audiodrivers/audiodriver.h"
#if defined PSYCLE__ALSA_AVAILABLE
	#include "../audiodrivers/alsaout.h"
#endif
#if defined PSYCLE__JACK_AVAILABLE
	#include "../audiodrivers/jackout.h"
#endif
#if defined PSYCLE__ESOUND_AVAILABLE
	#include "../audiodrivers/esoundout.h"
#endif
#if defined PSYCLE__GSTREAMER_AVAILABLE
	#include "../audiodrivers/gstreamerout.h"
#endif
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
	#include "../audiodrivers/microsoftdirectsoundout.h"
#endif
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
	#include "../audiodrivers/microsoftmmewaveout.h"
#endif

#include "../audiodrivers/wavefileout.h"
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <exception>
#include <iostream>

// FIXME: Ideally psycore should not depend on qt.
#include <QtCore> // For getting key binding stuff.
#include <QDomDocument> // for reading XML file

namespace psy {

	namespace core {

		UIConfiguration::UIConfiguration()
		{
			//setXmlDefaults();
			setDefaults();
			configureKeyBindings();
			loadConfig();			
			_RecordTweaks = false;
			_RecordUnarmed = true;
		}


		UIConfiguration::~UIConfiguration()
		{			
		}

		const std::string & UIConfiguration::iconPath() const {
			return iconPath_;
		}

		const std::string & UIConfiguration::pluginPath() const {
			return pluginPath_;
		}

		const std::string & UIConfiguration::prsPath() const {
			return prsPath_;
		}

		const std::string & UIConfiguration::hlpPath() const {
			return hlpPath_;
		}

		const std::string & UIConfiguration::ladspaPath() const {
			return ladspaPath_;
		}

		const std::string & UIConfiguration::songPath() const {
			return songPath_;
		}

		bool UIConfiguration::enableSound() const {
			return enableSound_;
		}

		bool UIConfiguration::ft2HomeEndBehaviour() const {
			return ft2HomeEndBehaviour_;
		}

		bool UIConfiguration::shiftArrowForSelect() const {
			return shiftArrowForSelect_;
		}

		bool UIConfiguration::wrapAround() const {
			return wrapAround_;
		}

		bool UIConfiguration::centerCursor() const {
			return centerCursor_;
		}

		/*		void UIConfiguration::setXmlDefaults() {     
		std::string xml_mem_;   

		xml_mem_ =  " <!-- xpsycle configuration file --> ";
		xml_mem_ += " <xpsycle> ";
		xml_mem_ += " <path id='plugindir'";
		xml_mem_ += " src='c:\\Programme\\Psycle\\PsyclePlugins' />";

		//			xml_mem_ += " <audio enable='1' />";
		//			xml_mem_ += " <driver name='mswaveout' />"; 

		/*  xml_mem_ += " <!-- help configuration -->";
		//  xml_mem_ += " <path id='hlpdir' src='~/xpsycle/doc/' />";

		xml_mem_ += " <!-- audio configuration -->";


		xml_mem_ += " <!-- gui configuration -->";

		//  xml_mem_ += " <path id='icondir' src='~/xpsycle/icons/' />";

		//  xml_mem_ += " <path id='prsdir' src='~/.xpsycle/prs/'></path>";

		xml_mem_ += " <!-- keyHandler configuration -->";
		xml_mem_ += " <!-- general keys -->";

		xml_mem_ += " <key id='current_machine-1' mod='ctrl' keychar='NK_Left' />";
		xml_mem_ += " <key id='current_machine+1' mod='ctrl' keychar='NK_Right' />";
		xml_mem_ += " <key id='current_instrument-1' mod='ctrl' keychar='NK_Up' />";
		xml_mem_ += " <key id='current_instrument+1' mod='ctrl' keychar='NK_Down' />";

		xml_mem_ += " <key id='screen_machines' keychar='F2' />";
		xml_mem_ += " <key id='screen_patterns' keychar='F3' />";
		xml_mem_ += " <key id='edit_instrument' keychar='F4' />";
		xml_mem_ += " <key id='screen_sequencer' keychar='F5' />";
		xml_mem_ += " <key id='play_song_start' keychar='F6' />";
		xml_mem_ += " <key id='play_song_normal' keychar='F7' />";
		xml_mem_ += " <key id='stop_playback' keychar='F8' />";
		xml_mem_ += " <key id='add_new_machine' keychar='F9' />";

		xml_mem_ += " <!-- patternview octave 0  -->";

		xml_mem_ += " <key id='oct_C_0' keychar='Y'/>";
		xml_mem_ += " <key id='oct_CS0' keychar='S'/>";
		xml_mem_ += " <key id='oct_D_0' keychar='X'/>";
		xml_mem_ += " <key id='oct_DS0' keychar='D'/>";
		xml_mem_ += " <key id='oct_E_0' keychar='C'/>";
		xml_mem_ += " <key id='oct_F_0' keychar='V'/>";
		xml_mem_ += " <key id='oct_FS0' keychar='G'/>";
		xml_mem_ += " <key id='oct_G_0' keychar='B'/>";
		xml_mem_ += " <key id='oct_GS0' keychar='H'/>";
		xml_mem_ += " <key id='oct_A_0' keychar='N'/>";
		xml_mem_ += " <key id='oct_AS0' keychar='J'/>";
		xml_mem_ += " <key id='oct_B_0' keychar='M'/>";

		xml_mem_ += " <!-- patternview octave 1  -->";

		xml_mem_ += " <key id='oct_C_1' keychar='Q'/>";
		xml_mem_ += " <key id='oct_CS1' keychar='2'/>";
		xml_mem_ += " <key id='oct_D_1' keychar='W'/>";
		xml_mem_ += " <key id='oct_DS1' keychar='3'/>";
		xml_mem_ += " <key id='oct_E_1' keychar='E'/>";
		xml_mem_ += " <key id='oct_F_1' keychar='R'/>";
		xml_mem_ += " <key id='oct_FS1' keychar='5'/>";
		xml_mem_ += " <key id='oct_G_1' keychar='T'/>";
		+1)xml_mem_ += " <key id='oct_GS1' keychar='6'/>";
		xml_mem_ += " <key id='oct_A_1' keychar='Z'/>";
		xml_mem_ += " <key id='oct_AS1' keychar='7'/>";
		xml_mem_ += " <key id='oct_B_1' keychar='U'/>";

		xml_mem_ += " <!-- patternview octave 2  -->";

		xml_mem_ += " <key id='oct_C_2' keychar='I' />";
		xml_mem_ += " <key id='oct_CS2' keychar='9' />";
		xml_mem_ += " <key id='oct_D_2' keychar='O' />";
		xml_mem_ += " <key id='oct_DS2' keychar='0' />";

		xml_mem_ += " <!-- patternview misc key options -->";

		xml_mem_ += " <key id='edit_toggle' mod='ctrl' keychar=' ' />";
		xml_mem_ += " <key id='key_stop' keychar='1' />";
		xml_mem_ += " <key id='current_octave-1' mod='ctrl' keychar='+' />";
		xml_mem_ += " <key id='current_octave+1' mod='ctrl' keychar='#' />";
		xml_mem_ += " <key id='patternstep_dec' keychar='[' />";
		xml_mem_ += " <key id='patternstep_inc' keychar=']' />";

		xml_mem_ += " <!-- patternview navigation key options -->";
		xml_mem_ += " <key id='nav_left' keychar='NK_Left' />";
		xml_mem_ += " <key id='nav_right' keychar='NK_Right' />";
		xml_mem_ += " <key id='nav_up' keychar='NK_Up' />";
		xml_mem_ += " <key id='nav_down' keychar='NK_Down' />";
		xml_mem_ += " <key id='nav_up_16' keychar='NK_Page_Up' />";
		xml_mem_ += " <key id='nav_down_16' keychar='NK_Page_Down' />";
		xml_mem_ += " <key id='nav_top' keychar='NK_Home' />";
		xml_mem_ += " <key id='nav_bottom' keychar='NK_End' />";
		xml_mem_ += " <key id='next_column' keychar='NK_Tab' />";
		xml_mem_ += " <key id='prev_column' keychar='XK_ISO_Left_Tab' />";
		xml_mem_ += " <key id='delete_row' keychar='NK_Delete' />";
		xml_mem_ += " <key id='insert_row' keychar='NK_Insert' />";
		xml_mem_ += " <key id='clear_row' keychar='NK_BackSpace' />";

		xml_mem_ += " <!-- patternview block key options -->";

		xml_mem_ += " <key id='block_copy' mod='ctrl' keychar='C' />";
		xml_mem_ += " <key id='block_cut' mod='ctrl' keychar='X' />";
		xml_mem_ += " <key id='block_delete' mod='ctrl' keychar='Y' />";
		xml_mem_ += " <key id='block_double' mod='ctrl' keychar='D' />";
		xml_mem_ += " <key id='block_halve' mod='ctrl' keychar='H' />";
		xml_mem_ += " <key id='block_interpolate' mod='ctrl' keychar='I' />";
		xml_mem_ += " <key id='block_mix' mod='ctrl' keychar='M' />";
		xml_mem_ += " <key id='block_paste' mod='ctrl' keychar='V' />";
		xml_mem_ += " <key id='block_select_all' mod='ctrl' keychar='A' />";
		xml_mem_ += " <key id='block_select_bar' mod='ctrl' keychar='E' />";
		xml_mem_ += " <key id='block_select_column' mod='ctrl' keychar='R' />";
		xml_mem_ += " <key id='block_select_up' mod='shift' keychar='NK_Up' />";
		xml_mem_ += " <key id='block_select_down' mod='shift' keychar='NK_Down' />";
		xml_mem_ += " <key id='block_select_left' mod='shift' keychar='NK_Left' />";
		xml_mem_ += " <key id='block_select_right' mod='shift' keychar='NK_Right' />";
		xml_mem_ += " <key id='block_select_top' mod='shift' keychar='NK_Home' />";
		xml_mem_ += " <key id='block_select_bottom' mod='shift' keychar='NK_End' />";
		xml_mem_ += " <key id='block_set_instrument' mod='ctrl' keychar='T' />";
		xml_mem_ += " <key id='block_set_machine' mod='ctrl' keychar='G' />";
		xml_mem_ += " <key id='block_start' mod='ctrl' keychar='B' />";
		xml_mem_ += " <key id='block_start' mod='ctrl' keychar='K' />";
		xml_mem_ += " <key id='block_unmark' mod='ctrl' keychar='U' />";

		xml_mem_ += " </xpsycle> ";

		const char* pcLADSPAPath = std::getenv("LADSPA_PATH");
		if ( !pcLADSPAPath) {
		#ifdef __unix__
		pcLADSPAPath = "/usr/lib/ladspa/";
		#else
		pcLADSPAPath = "I:\\Archivos de programa\\Multimedia\\Audacity\\Plug-Ins";
		#endif
		}
		ladspaPath_ = pcLADSPAPath;

		ngrs::XmlParser parser;
		parser.tagParse.connect(this,&UIConfiguration::onConfigTagParse);
		parser.parseString ( xml_mem_ );

		}     */

		void UIConfiguration::addAudioDriver(AudioDriver* driver)
		{
			std::cout << "Driver registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		}

		void UIConfiguration::setDefaults()
		{
			AudioDriver* driver = new AudioDriver;
			addAudioDriver(driver);
			_pSilentDriver = driver;
			addAudioDriver(new WaveFileOut);

			setDriverByName("silent");
			enableSound_ = false;

#if defined PSYCLE__ALSA_AVAILABLE
			addAudioDriver(new AlsaOut);
#endif
#if defined PSYCLE__JACK_AVAILABLE
			addAudioDriver(new JackOut);
#endif
#if defined PSYCLE__ESOUND_AVAILABLE
			addAudioDriver(new ESoundOut);
#endif		
#if defined PSYCLE__GSTREAMER_AVAILABLE
			addAudioDriver(new GStreamerOut);
#endif
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
			addAudioDriver(new MsDirectSound);
			// use dsound by default
			setDriverByName("dsound");
			enableSound_ = true;
#endif
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
			addAudioDriver(new MsWaveOut);
#endif
#if defined PSYCLE__NET_AUDIO_AVAILABLE
			addAudioDriver(new NetAudioOut);
#endif
		}

		void UIConfiguration::setDriverByName( const std::string & driverName )
		{
			std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();

			if ( ( it = driverMap_.find( driverName ) ) != driverMap_.end() ) {
				// driver found
				_pOutputDriver = it->second;
			}
			else {
				// driver not found,  set silent default driver
				_pOutputDriver = _pSilentDriver;
			}
			std::cout << "audio driver set as: " << _pOutputDriver->info().name() << std::endl;

		}


		void UIConfiguration::loadConfig()
		{
			std::string path = File::replaceTilde("~" + File::slash() + ".xpsycle.xml");
			if (path.length()!=0) {
				try {
					loadConfig( File::replaceTilde( "~" + File::slash() + ".xpsycle.xml") );
				}
				catch( std::exception const & e ) {
					std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
				}
			} else {
#if 0
#if defined PSYCLE__INSTALL_PATHS__CONFIGURATION
				path = PSYCLE__INSTALL_PATHS__CONFIGURATION "/xpsycle.xml";
#endif
				if (path.length()!=0){
					try {
						loadConfig(path);
					}
					catch(std::exception const & e)
					{
					std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
					}
				}
#endif
			}
		}


		void UIConfiguration::loadConfig( const std::string & path )
		{
			QFile *file = new QFile( QString::fromStdString( path ) );
			if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
				QDomDocument *doc = new QDomDocument();
				doc->setContent( file );
				QDomElement root = doc->firstChildElement();

				// Paths.
				QDomNodeList paths = root.elementsByTagName( "path" );
				for ( int i = 0; i < paths.count(); i++ )
				{
					QDomElement path = paths.item( i ).toElement();
					std::string id = path.attribute("id").toStdString();
					std::string src = path.attribute("src").toStdString();
					if ( id == "icondir" )   iconPath_   = src;
					else  
						if ( id == "plugindir" ) pluginPath_ = src;
						else
							if ( id == "prsdir" )    prsPath_    = src;
							else
								if ( id == "hlpdir" )    hlpPath_    = src; 
								else
									if ( id == "ladspadir" ) ladspaPath_ = src;
									else  
										if ( id == "songdir" )   songPath_   = src;
				}

				// Audio.
				QDomElement audioElm = root.firstChildElement( "audio" );
				std::string enableStr = audioElm.attribute( "enable" ).toStdString();
				int enable = 0;
				if ( enableStr != "" ) enable = QString::fromStdString( enableStr ).toInt();
				enableSound_ = enable;
				if (enable == 0) {
					setDriverByName( "silent" );
					doEnableSound = false;
				} else {
					doEnableSound = true;
				}
				QDomElement driverElm = root.firstChildElement( "driver" );
				if ( doEnableSound ) {		
					setDriverByName( driverElm.attribute("name").toStdString() );
				} 

				// Alsa.  
				if ( _pOutputDriver->info().name() == "alsa" ) {
					// FIXME: what if alsa settings are missing from xml file?
					QDomElement alsaElm = root.firstChildElement( "alsa" ); 
					std::string deviceName = alsaElm.attribute("device").toStdString();
					std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
					if ( ( it = driverMap_.find( "alsa" ) ) != driverMap_.end() ) {
						AudioDriverSettings settings = it->second->settings();
						settings.setDeviceName( deviceName );
						it->second->setSettings( settings );
					}		
				}

				// Options.
				QDomNodeList options = root.elementsByTagName( "option" );
				for ( int i = 0; i < options.count(); i++ )
				{
					QDomElement option = options.item( i ).toElement();
					QString id = option.attribute("id");
					QString value = option.attribute("value");
					if ( id == "ft2-home-end-behaviour" )
						ft2HomeEndBehaviour_ = value.toInt();     
					if ( id == "shift-arrow-for-select" )
						shiftArrowForSelect_ = value.toInt();     
					if ( id == "wrap-around" )
						wrapAround_ = value.toInt();     
					if ( id == "center-cursor" )
						centerCursor_ = value.toInt();     
				}
			}

			doEnableSound = true;
		}


		void UIConfiguration::configureKeyBindings() // FIXME: Key bindings are host specific, should be moved?
		{
			int modifiers = Qt::NoModifier; // Shouldn't have Qt stuff in psycore in the future.
			inputHandler_.changeKeyCode( cdefKeyC_0, Key( modifiers, Qt::Key_Z ) );
			inputHandler_.changeKeyCode( cdefKeyCS0, Key( modifiers, Qt::Key_S ) );
			inputHandler_.changeKeyCode(cdefKeyD_0,Key(modifiers,Qt::Key_X));
			inputHandler_.changeKeyCode(cdefKeyDS0,Key(modifiers,Qt::Key_D));
			inputHandler_.changeKeyCode(cdefKeyE_0,Key(modifiers,Qt::Key_C));
			inputHandler_.changeKeyCode(cdefKeyF_0,Key(modifiers,Qt::Key_V));
			inputHandler_.changeKeyCode(cdefKeyFS0,Key(modifiers,Qt::Key_G));
			inputHandler_.changeKeyCode(cdefKeyG_0,Key(modifiers,Qt::Key_B));
			inputHandler_.changeKeyCode(cdefKeyGS0,Key(modifiers,Qt::Key_H));
			inputHandler_.changeKeyCode(cdefKeyA_0,Key(modifiers,Qt::Key_N));
			inputHandler_.changeKeyCode(cdefKeyAS0,Key(modifiers,Qt::Key_J));
			inputHandler_.changeKeyCode(cdefKeyB_0,Key(modifiers,Qt::Key_M));
			inputHandler_.changeKeyCode(cdefKeyC_1,Key(modifiers,Qt::Key_Q));
			inputHandler_.changeKeyCode(cdefKeyCS1,Key(modifiers,Qt::Key_2));
			inputHandler_.changeKeyCode(cdefKeyD_1,Key(modifiers,Qt::Key_W));
			inputHandler_.changeKeyCode(cdefKeyDS1,Key(modifiers,Qt::Key_3));
			inputHandler_.changeKeyCode(cdefKeyE_1,Key(modifiers,Qt::Key_E));
			inputHandler_.changeKeyCode(cdefKeyF_1,Key(modifiers,Qt::Key_R));
			inputHandler_.changeKeyCode(cdefKeyFS1,Key(modifiers,Qt::Key_5));
			inputHandler_.changeKeyCode(cdefKeyG_1,Key(modifiers,Qt::Key_T));
			inputHandler_.changeKeyCode(cdefKeyGS1,Key(modifiers,Qt::Key_6));
			inputHandler_.changeKeyCode(cdefKeyA_1,Key(modifiers,Qt::Key_Y));
			inputHandler_.changeKeyCode(cdefKeyAS1,Key(modifiers,Qt::Key_7));
			inputHandler_.changeKeyCode(cdefKeyB_1,Key(modifiers,Qt::Key_U));
			inputHandler_.changeKeyCode(cdefKeyC_2,Key(modifiers,Qt::Key_I));
			inputHandler_.changeKeyCode(cdefKeyCS2,Key(modifiers,Qt::Key_9));
			inputHandler_.changeKeyCode(cdefKeyD_2,Key(modifiers,Qt::Key_O));
			inputHandler_.changeKeyCode(cdefKeyDS2,Key(modifiers,Qt::Key_0));
			inputHandler_.changeKeyCode(cdefKeyE_2,Key(modifiers,Qt::Key_P));
			/*            inputHandler_.changeKeyCode(cdefKeyF_2,Key(modifiers,Qt::Key_X)); // no bindings for now.
			inputHandler_.changeKeyCode(cdefKeyFS2,Key(modifiers,Qt::Key_X));
			inputHandler_.changeKeyCode(cdefKeyG_2,Key(modifiers,Qt::Key_X));
			inputHandler_.changeKeyCode(cdefKeyGS2,Key(modifiers,Qt::Key_X));
			inputHandler_.changeKeyCode(cdefKeyA_2,Key(modifiers,Qt::Key_X));*/

			inputHandler_.changeKeyCode( cdefShowPatternBox, Key( modifiers, Qt::Key_F1 ) );
			inputHandler_.changeKeyCode( cdefShowMachineView, Key( modifiers, Qt::Key_F2 ) );
			inputHandler_.changeKeyCode( cdefShowPatternView, Key( modifiers, Qt::Key_F3 ) );
			inputHandler_.changeKeyCode( cdefShowWaveEditor, Key( modifiers, Qt::Key_F4 ) );
			inputHandler_.changeKeyCode( cdefShowSequencerView, Key( modifiers, Qt::Key_F5 ) );

			/** Play control commands. **/
			inputHandler_.changeKeyCode( cdefPlayFromPos, Key( Qt::NoModifier, Qt::Key_F6 ) );
			inputHandler_.changeKeyCode( cdefPlayStart, Key( Qt::ShiftModifier, Qt::Key_F6 ) );
			inputHandler_.changeKeyCode( cdefLoopEntry, Key( Qt::NoModifier, Qt::Key_F7 ) );
			inputHandler_.changeKeyCode( cdefPlayStop, Key( Qt::NoModifier, Qt::Key_F8 ) );

			inputHandler_.changeKeyCode( cdefInstrDec, Key( Qt::ControlModifier, Qt::Key_Minus ) );
			inputHandler_.changeKeyCode( cdefInstrInc, Key( Qt::ControlModifier, Qt::Key_Equal ) );

			inputHandler_.changeKeyCode( cdefOctaveDn, Key( Qt::KeypadModifier, Qt::Key_Slash ) );
			inputHandler_.changeKeyCode( cdefOctaveUp, Key( Qt::KeypadModifier, Qt::Key_Asterisk ) );

			/** Machine View. **/
			inputHandler_.changeKeyCode( cdefMuteMachine, Key( Qt::ShiftModifier, Qt::Key_M ) );
			inputHandler_.changeKeyCode( cdefSoloMachine, Key( Qt::ShiftModifier, Qt::Key_S ) );
			inputHandler_.changeKeyCode( cdefBypassMachine, Key( Qt::ShiftModifier, Qt::Key_B ) );

			/** Pattern View. **/
			inputHandler_.changeKeyCode( cdefNavUp, Key( Qt::NoModifier, Qt::Key_Up ) );
			inputHandler_.changeKeyCode( cdefNavDown, Key( Qt::NoModifier, Qt::Key_Down ) );
			inputHandler_.changeKeyCode( cdefNavLeft, Key( Qt::NoModifier, Qt::Key_Left ) );
			inputHandler_.changeKeyCode( cdefNavRight, Key( Qt::NoModifier, Qt::Key_Right ) );
			inputHandler_.changeKeyCode( cdefNavPageDn, Key( Qt::NoModifier, Qt::Key_PageDown ) );
			inputHandler_.changeKeyCode( cdefNavPageUp, Key( Qt::NoModifier, Qt::Key_PageUp ) );
			inputHandler_.changeKeyCode( cdefTrackNext, Key( Qt::NoModifier, Qt::Key_Tab ) );
			inputHandler_.changeKeyCode( cdefTrackPrev, Key( Qt::ShiftModifier, Qt::Key_Backtab ) );
			inputHandler_.changeKeyCode( cdefNavTop, Key( Qt::NoModifier, Qt::Key_Home ) );
			inputHandler_.changeKeyCode( cdefNavBottom, Key( Qt::NoModifier, Qt::Key_End ) );

			inputHandler_.changeKeyCode( cdefPatternstepDec, Key( Qt::NoModifier, Qt::Key_BracketLeft ) );
			inputHandler_.changeKeyCode( cdefPatternstepInc, Key( Qt::NoModifier, Qt::Key_BracketRight ) );

			inputHandler_.changeKeyCode( cdefSelectUp, Key( Qt::ShiftModifier, Qt::Key_Up ) );
			inputHandler_.changeKeyCode( cdefSelectDn, Key( Qt::ShiftModifier, Qt::Key_Down ) );
			inputHandler_.changeKeyCode( cdefSelectLeft, Key( Qt::ShiftModifier, Qt::Key_Left ) );
			inputHandler_.changeKeyCode( cdefSelectRight, Key( Qt::ShiftModifier, Qt::Key_Right ) );
			inputHandler_.changeKeyCode( cdefSelectTrack, Key( Qt::ControlModifier, Qt::Key_R ) );
			inputHandler_.changeKeyCode( cdefSelectAll, Key( Qt::ControlModifier, Qt::Key_A ) );
			inputHandler_.changeKeyCode( cdefBlockStart, Key( Qt::ControlModifier, Qt::Key_B ) );
			inputHandler_.changeKeyCode( cdefBlockEnd, Key( Qt::ControlModifier, Qt::Key_E ) );
			inputHandler_.changeKeyCode( cdefBlockUnMark, Key( Qt::ControlModifier, Qt::Key_U ) );

			inputHandler_.changeKeyCode( cdefBlockCopy, Key( Qt::ControlModifier, Qt::Key_C ) );
			inputHandler_.changeKeyCode( cdefBlockCut, Key( Qt::ControlModifier, Qt::Key_X ) );
			inputHandler_.changeKeyCode( cdefBlockPaste, Key( Qt::ControlModifier, Qt::Key_V ) );
			inputHandler_.changeKeyCode( cdefBlockDelete, Key( Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_X ) );

			inputHandler_.changeKeyCode( cdefRowClear, Key(Qt::NoModifier, Qt::Key_Delete) );
		}

		InputHandler & UIConfiguration::inputHandler() {
			return inputHandler_;
		}

	}
}
