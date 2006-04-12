#pragma once
#include <psycle/host/detail/project.hpp>
//#include <boost/multi_array.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		#if 0 ///\todo this class is unfortunatly not used
			template<typename Entry>
			class pattern : public boost::multi_array<Entry, 2>
			{
				public:
					typedef Entry entry;

				public:
					pattern(unsigned int const & lines, unsigned int const & columns)
					:
						boost::multi_array<Entry, 2>(lines, columns)
					{
					}

				public:
					void inline name(std::string const & value) throw() { name_ = value; }
					std::string const inline & name() const throw() { return name_; }
				private:
					std::string name_;

				#if 0 // std::vector
					public:
						unsigned int const inline & lines() const throw() { return size(); }
						void lines(unsigned int const & value)
						{
							resize(lines);
							for(iterator i(begin()) ; i != end() ; ++i) *i.resize(columns);
						}

					public:
						unsigned int const inline & columns() const throw() { return column_; }
						void columns(unsigned int const & value)
						{
							columns_ = value;
							for(iterator i(begin()) ; i != end() ; ++i) *i.resize(columns);
						}
					private:
						unsigned int columns_;
				#endif
			};
		#endif

		UNIVERSALIS__COMPILER__ALIGNED(1) // this normally shouldn't be needed, but it's safer considering the weird way data are loaded/saved
		class PatternEntry
		{
			public:
				inline PatternEntry()
				:
					_note(255),
					_inst(255),
					#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
						#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
					#else
						#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
							_volume(255),
						#endif
					#endif
					_mach(255),
					_cmd(0),
					_parameter(0)
				{
				}
				std::uint8_t _note;
				std::uint8_t _inst;
				#if !defined PSYCLE__CONFIGURATION__VOLUME_COLUMN
					#error PSYCLE__CONFIGURATION__VOLUME_COLUMN isn't defined! Check the code where this error is triggered.
				#else
					#if PSYCLE__CONFIGURATION__VOLUME_COLUMN
						std::uint8_t _volume;
						std::uint8_t _cmd;
						std::uint8_t _parameter;
						std::uint8_t _mach;
					#else
						std::uint8_t _mach;
						std::uint8_t _cmd;
						std::uint8_t _parameter;
					#endif
				#endif
		};

		namespace PatternCmd
		{
			enum
			{
				EXTENDED      = 0xFE, // see below
				SET_TEMPO     = 0xFF,
				NOTE_DELAY    = 0xFD,
				RETRIGGER     = 0xFB,
				RETR_CONT     = 0xFA,
				SET_VOLUME    = 0x0FC,
				SET_PANNING   = 0x0F8,
				BREAK_TO_LINE = 0xF2,
				JUMP_TO_ORDER = 0xF3,
				ARPEGGIO      = 0xF0,

				// Extended Commands from 0xFE
				SET_LINESPERBEAT0  = 0x00, // 
				SET_LINESPERBEAT1  = 0x10, // Range from FE00 to FE1F is reserved for changing lines per beat.
				SET_BYPASS         = 0x20,
				SET_MUTE           = 0x30,
				PATTERN_LOOP       = 0xB0, // Loops the current pattern x times. 0xFEB0 sets the loop start point.
				PATTERN_DELAY      = 0xD0, // causes a "pause" of x rows ( i.e. the current row becomes x rows longer)
				FINE_PATTERN_DELAY = 0xF0  // causes a "pause" of x ticks ( i.e. the current row becomes x ticks longer)
			};
		};
	}
}
