// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__SINGLE_PATTERN__INCLUDED
#define PSYCLE__CORE__SINGLE_PATTERN__INCLUDED
#pragma once

#include "patternevent.h"
#include "signalslib.h"
#include "timesignature.h"

namespace psy { namespace core {

	class PSYCLE__CORE__DECL Pattern {
		public:
			typedef std::multimap<double, PatternEvent>::iterator iterator;
			typedef std::multimap<double, PatternEvent>::const_iterator const_iterator;
			typedef std::multimap<double, PatternEvent>::const_reverse_iterator const_reverse_iterator;
			typedef std::multimap<double, PatternEvent>::reverse_iterator reverse_iterator;

			Pattern();
			Pattern(const Pattern& other);

			virtual ~Pattern();

			Pattern& operator=(const Pattern& rhs);
			boost::signal<void (Pattern*)> wasDeleted;
			
			void setID(int id) { id_ = id; }
			int id() const { return id_; }
						
			void setBeatZoom(int zoom);
			int beatZoom() const;
			
			void setEvent(int line, int track, int beat_zoom, const PatternEvent& event);
			PatternEvent event(int line, int track, int beat_zoom);

			void setTweakEvent(int line, int track, const PatternEvent& event);
			PatternEvent tweakEvent(int line, int track);


			void addBar(const TimeSignature& signature);
			void removeBar(float pos);

			float beats() const;

			bool barStart(double pos, TimeSignature & signature) const;
			void clearBars();

			const TimeSignature & playPosTimeSignature(double pos) const;

			void setName(const std::string & name);
			const std::string & name() const;

			void setCategory(const std::string& category);
			const std::string& category() const { return category_; };

			float beatsPerLine() const;

			void Clear();

			void clearTrack( int linenr , int tracknr );
			void clearTweakTrack( int linenr , int tracknr );
			bool lineIsEmpty( int linenr ) const;

			
			iterator find_nearest(int linenr, int beat_zoom);
			const_iterator find_nearest(int linenr, int beat_zoom) const;

			iterator find_lower_nearest( int linenr );
			const_iterator find_lower_nearest( int linenr ) const;
			

			void clearEmptyLines();

			Pattern Clone(double from, double to, int start_track=0, int end_track=32000);
			void erase(double from, double to, int start_track=0, int end_track=32000);

			void Transpose(int delta, double from, double to, int start_track=0, int end_track=32000);

			void scaleBlock(int left, int right, double top, double bottom, float factor);		
			void blockSetInstrument( int left, int right, double top, double bottom, std::uint8_t newInstrument );
			void blockSetMachine( int left, int right, double top, double bottom, std::uint8_t newMachine );

			std::vector<TimeSignature>& timeSignatures();
			const std::vector<TimeSignature>&  timeSignatures() const;

			std::string toXml() const;

			iterator begin() { return lines_.begin(); }
			const_iterator begin() const { return lines_.begin(); }
			iterator end() { return lines_.end(); }
			const_iterator end() const { return lines_.end(); }

			reverse_iterator rbegin() { return lines_.rbegin(); }
			const_reverse_iterator rbegin() const { return lines_.rbegin();}
			reverse_iterator rend() { return lines_.rend(); }
			const_reverse_iterator rend() const { return lines_.rend(); }

			iterator lower_bound(double pos) { return lines_.lower_bound(pos); }
			iterator upper_bound(double pos) { return lines_.upper_bound(pos); }

			iterator insert(double pos, const PatternEvent& ev) {
				return lines_.insert(std::pair<double, PatternEvent>(pos, ev));
			}
			void insert(const Pattern& srcPattern, double to, int to_track=0);			

			iterator erase(iterator pos) {
				iterator temp = pos;
				++temp;
				lines_.erase(pos);
				return temp;
			}

			std::map<double, PatternEvent>::size_type size() const { return lines_.size(); }

			// beatpos with 4 is standard
			double BeatPosWithTPB(int ticks, double beat_pos) {
				if (ticks == 4)
					return beat_pos;
				else
					return 4.0/ticks * beat_pos;
			}

		private:
			int beatZoom_;
			std::string name_;

			std::string category_;
			std::vector<TimeSignature> timeSignatures_;

			TimeSignature zeroTime;

			int id_;						

			std::multimap<double, PatternEvent> lines_;
		};

}}



#endif
