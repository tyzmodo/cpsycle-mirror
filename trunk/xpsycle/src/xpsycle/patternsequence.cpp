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
#include "patternsequence.h"
#include <iostream>

namespace psycle
{
	namespace host
	{

		int SequenceLine::idCounter = 0;

		GlobalEvent::GlobalEvent( ) : parameter_(0), target_(-1), target2_(-1), type_(NONE)
		{
		}

		GlobalEvent::GlobalEvent( float parameter ): parameter_(parameter), target_(-1), target2_(-1),type_(NONE)
		{
		}

		GlobalEvent::~ GlobalEvent( )
		{
		}

		void GlobalEvent::setParameter( float parameter )
		{
      parameter_ = parameter;
		}

		float GlobalEvent::parameter( ) const
		{
			return parameter_;
		} // end of psycle namespace

		void GlobalEvent::setTarget( int target )
		{
			target_ = target;
		}

		int GlobalEvent::target( ) const
		{
			return target_;
		}

		void GlobalEvent::setTarget2( int target )
		{
			target2_ = target;
		}

		int GlobalEvent::target2( ) const
		{
			return target2_;
		}

		void GlobalEvent::setType( GlobalType type )
		{
			type_ = type;
		}

		GlobalEvent::GlobalType GlobalEvent::type( ) const
		{
			return type_;
		}


		// pattern Entry contains one ptr to a SinglePattern and the tickPosition for the absolute Sequencer pos

		SequenceEntry::SequenceEntry( )
		{
			line_ = 0;
			pattern_ = 0;
			startPos_ = 0;
			endPos_ = PatternEnd;
		}

		SequenceEntry::SequenceEntry( SequenceLine * line )
		{
			pattern_ = 0;
			line_ = line;
			startPos_ = 0;
			endPos_ = PatternEnd;
		}

		SequenceEntry::~ SequenceEntry( )
		{
		}

		void SequenceEntry::setPattern( SinglePattern * pattern )
		{
			pattern_ = pattern;
		}

		SinglePattern * SequenceEntry::pattern( )
		{
			return pattern_;
		}

		SinglePattern * SequenceEntry::pattern( ) const
		{
			return pattern_;
		}

		float SequenceEntry::patternBeats() const
		{
			return pattern_->beats();
		}

		double SequenceEntry::tickPosition( ) const
		{
			SequenceLine::iterator iter = line_->begin();
			for(; iter!= line_->end(); ++iter)
			{
				if(iter->second==this)
				break;
			}
			if(iter!=line_->end())
			{
				return iter->first;
			}
			return 0;
		}

		// end of PatternEntry

		// represents one track/line in the sequencer
		SequenceLine::SequenceLine( )
		{
			patternSequence_ = 0;
			lineId_ = idCounter;
			idCounter++;
		}

		SequenceLine::SequenceLine( PatternSequence * patSeq )
		{
			patternSequence_ = patSeq;
		}


		SequenceLine::~ SequenceLine( )
		{
			iterator it = begin();
			for ( it; it != end(); it++) delete it->second;
		}

		SequenceEntry* SequenceLine::createEntry( SinglePattern * pattern, double position )
		{
			SequenceEntry* entry = new SequenceEntry(this);
			entry->setPattern(pattern);
			insert(value_type(position, entry));
			return entry;
		}

		void SequenceLine::removeSinglePatternEntries( SinglePattern* pattern )
		{
			iterator it = begin();
			while ( it != end() ) {
				SequenceEntry* entry = it->second;
				if ( entry->pattern() == pattern) {
					delete entry;
					erase(it++);
				} else it++;
			}
		}

		double SequenceLine::tickLength( ) const
		{
			if (size() > 0 ) {
				return rbegin()->first + rbegin()->second->patternBeats();
			} else
  		return 0;
		}

		PatternSequence * SequenceLine::patternSequence( )
		{
			return patternSequence_;
		}


		void SequenceLine::MoveEntry(SequenceEntry* entry, double newpos)
		{
			iterator iter = begin();
			for(; iter!= end(); ++iter)
			{
				if(iter->second==entry)
				break;
			}
			if(iter!=end())
			{
				double oldpos = iter->first;
				erase(iter);
				insert(value_type(newpos, entry));
			}
		}

		void SequenceLine::removeEntry( SequenceEntry * entry )
		{
      iterator iter = begin();
			for(; iter!= end(); ++iter)
			{
				if(iter->second==entry)
				break;
			}
			if(iter!=end())
			{
				SequenceEntry* entry = iter->second;
				erase(iter);
				delete entry;
			}
		}

		const std::string & SequenceLine::name() const
		{
			return name_;
		}

		void SequenceLine::setName(const std::string & newname)
		{
			name_=newname;
		}

		int SequenceLine::id( ) const
		{
			return lineId_;
		}
		//end of sequenceLine;



		// PatternSequence
		PatternSequence::PatternSequence()
		{
		}


		PatternSequence::~PatternSequence()
		{
			for(iterator it = begin(); it != end(); ++it)
				delete *it;
			GlobalIter it = globalEvents_.begin();
			for (; it != globalEvents_.end(); it++) delete it->second;
		}


		SequenceLine * PatternSequence::createNewLine( )
		{
			SequenceLine* line = new SequenceLine(this);
			push_back(line);
			return line;
		}

		void PatternSequence::removeLine( SequenceLine * line )
		{
			iterator it = find(begin(), end(), line);
			if ( it != end() ) {
				erase(it);
				delete line;
			}
		}

		PatternData* PatternSequence::patternData() {
			return &patternData_;
		}

		////////////////////////////////////////////////////////////////////////////
		// GetLinesInRange
		// Objective: return the PatternLines that are active in the range [start, start+length).
	    // Parameters: 
		//		start : start time in beats since playback begin.
		// 	length: length of the range. start+length is the last position (non inclusive)
		//		(return) events : A multimap of lines (multimap of beatposition and PatternLine )
		void PatternSequence::GetLinesInRange( double start, double length, std::multimap<double, PatternLine>& events ) 
		{
			int seqlineidx = 1; // index zero reserved for live events (midi in, or pc keyb)
		    // Iterate over each timeline of the sequence,
			for( iterator seqIt = this->begin(); seqIt != this->end(); ++seqIt )
			{
				SequenceLine *pSLine = *seqIt;
				// locate the "sequenceEntry"s which starts nearer to "start+length"
				SequenceLine::reverse_iterator sLineIt( pSLine->lower_bound(start+length) );
				// and iterate backwards to include any other that is inside the range [start,start+length)
				// (The UI won't allow more than one pattern for the same range in the same timeline, but 
				// this was left open in the player code)
				for(; sLineIt != pSLine->rend() && sLineIt->first + sLineIt->second->patternBeats() >= start; ++sLineIt )
				{
					// take the pattern,
					SinglePattern* pPat = sLineIt->second->pattern();
					SinglePattern::iterator patIt   = pPat->lower_bound( start-sLineIt->first ),
					patEnd = pPat->lower_bound( start+length-sLineIt->first );
					// and iterate through the lines that are inside the range
					for( ; patIt != patEnd; ++patIt)
						{
						PatternLine *thisline= &(patIt->second);
						PatternLine tmpline;
						PatternLine::iterator lineIt = thisline->begin();
						// Since the player needs to differentiate between tracks of different SequenceEntrys, 
						// we generate a temporary PatternLine with a special column value.
						for( ;lineIt != thisline->end() ;lineIt++)
						{
							tmpline[seqlineidx*1024+lineIt->first]=lineIt->second;
						}
						
						// finally add the PatternLine to the event map.
						events.insert( SinglePattern::value_type( patIt->first + sLineIt->first, tmpline ) );
						}
				}
			    seqlineidx++;
			}
		}
	
		double PatternSequence::GetNextGlobalEvents(double start, double length, std::vector<GlobalEvent*>& globals, bool bInclusive)
		{
			GlobalIter firstIt = ( bInclusive ?
				  	globalEvents_.lower_bound(start) :
				  	globalEvents_.upper_bound(start) );
			if(firstIt != globalEvents_.end() && firstIt->first < start+length)
			{
				double pos = firstIt->first;
				GlobalIter lastIt = globalEvents_.upper_bound(pos);
				for(; firstIt != lastIt; ++firstIt)
					globals.push_back(firstIt->second);
				return pos;
			}
			return start+length;
		}

		GlobalEvent * PatternSequence::createBpmChangeEntry( double position , float bpm)
		{
			GlobalEvent* bpmEvent = new GlobalEvent(bpm);
			bpmEvent->setType(GlobalEvent::BPM_CHANGE);
			globalEvents_.insert(GlobalMap::value_type(position, bpmEvent));

			return bpmEvent;
		}

		void PatternSequence::moveGlobalEvent( GlobalEvent * entry, double newpos )
		{
			GlobalIter iter = globalEvents_.begin();
			for(; iter!= globalEvents_.end(); ++iter)
			{
				if(iter->second==entry)
				break;
			}
			if(iter!=globalEvents_.end())
			{
				double oldpos = iter->first;
				globalEvents_.erase(iter);
				globalEvents_.insert(GlobalMap::value_type(newpos, entry) );
			}
		}

		double PatternSequence::globalTickPosition( GlobalEvent * event ) const
		{
			// O(n) runtime here (multiplies from gui resize to O(n^2)!!!!
			// this is worse and could be avoided if Global Events have a tickPosition and we avoid instead that map
			GlobalMap::const_iterator iter = globalEvents_.begin();
			for(; iter!= globalEvents_.end(); ++iter)
			{
				if(iter->second==event)
				break;
			}
			if(iter!=globalEvents_.end())
			{
				return iter->first;
			}
			return 0;
		}

		const PatternSequence::GlobalMap & PatternSequence::globalEvents( )
		{
			return globalEvents_;
		}

		void PatternSequence::removeSinglePattern( SinglePattern * pattern )
		{
			for(iterator it = begin(); it != end(); ++it) {
				(*it)->removeSinglePatternEntries(pattern);
			}
			patternData_.removeSinglePattern(pattern);
		}

		void PatternSequence::removeAll( )
		{
			for(iterator it = begin(); it != end(); ++it) {
				delete *it;
			}
			clear();
			patternData_.removeAll();
		}

		double PatternSequence::tickLength( ) const
		{
			double max = 0;
			for(const_iterator it = begin(); it != end(); ++it) {
				SequenceLine* line = *it;
				max = std::max(line->tickLength(), max);
			}
			return max;
		}

	} // end of host namespace


}





