// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::devices::outputs::direct_sound
#pragma once
#include "../resource.hpp"
#include <universalis/os/exception.hpp>
#include <universalis/compiler/numeric.hpp>

#if !defined DIVERSALIS__OS__MICROSOFT
	#error "this plugin is specific to microsoft's operating system"
#endif

#include <windows.h>

#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "user32") // for ::GetDesktopWindow(), see implementation file
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

#include <mmsystem.h> // winmm lib
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	// unused #pragma comment(lib, "winmm")
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

#include <dsound.h> // dsound lib
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "dsound")
#endif

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__OUTPUTS__DIRECT_SOUND
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins { namespace outputs {

	/// outputs to a soundcard device via microsoft's direct sound output implementation.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK direct_sound : public resource {
		protected: friend class virtual_factory_access;
			direct_sound(engine::plugin_library_reference &, engine::graph &, std::string const & name) throw(universalis::os::exception);
		public:
			engine::ports::inputs::single & in_port() { return *single_input_ports()[0]; }
			bool opened()  const /*override*/;
			bool started() const /*override*/;
		protected:
			void do_open()    throw(universalis::os::exception) /*override*/;
			void do_start()   throw(universalis::os::exception) /*override*/;
			void do_process() throw(universalis::os::exception) /*override*/;
			void do_stop()    throw(universalis::os::exception) /*override*/;
			void do_close()   throw(universalis::os::exception) /*override*/;
			void channel_change_notification_from_port(engine::port const &) throw(engine::exception) /*override*/;
		private:
			::IDirectSound        * direct_sound_;
			::IDirectSound inline & direct_sound_implementation() throw() { assert(direct_sound_); return *direct_sound_; }

			typedef universalis::compiler::numeric</*bits_per_channel_sample*/16>::signed_int output_sample_type;
			std::vector<output_sample_type> last_samples_;

			::IDirectSoundBuffer mutable * buffer_;
			::IDirectSoundBuffer inline  & buffer()       throw() { assert(buffer_); return *buffer_; }
			::IDirectSoundBuffer inline  & buffer() const throw() { assert(buffer_); return *buffer_; }

			bool write_primary_, started_;
			unsigned int buffers_, buffer_size_, total_buffer_size_;
			/// position in byte offset
			unsigned int current_position_;
			unsigned int samples_per_buffer_;
	};
}}}
#include <universalis/compiler/dynamic_link/end.hpp>