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

		// pattern Entry contains one ptr to a SinglePattern and the tickPosition for the absolute Sequencer pos

		struct LessByPointedToValue
			: std::binary_function<SequenceEntry const *, SequenceEntry const *, bool>
		{
			bool operator()(SequenceEntry const * x, SequenceEntry const * y) const
				{ return x->tickPosition() < y->tickPosition(); }
		};

		SequenceEntry::SequenceEntry( )
		{
			pattern_ = 0;
			line_ = 0;
		}

		SequenceEntry::SequenceEntry( SequenceLine * line )
		{
			pattern_ = 0;
			line_ = line;
		}

		SequenceEntry::~ SequenceEntry( )
		{
			beforeDelete.emit(this);
		}

		void SequenceEntry::setPattern( SinglePattern * pattern )
		{
			pattern_ = pattern;
			playIterator_ = pattern_->begin();
		}

		SinglePattern * SequenceEntry::pattern( )
		{
			return pattern_;
		}

		SinglePattern * SequenceEntry::pattern( ) const
		{
			return pattern_;
		}

		void SequenceEntry::setTickPosition( double tick )
		{
			tickPosition_ = tick;
			if (line_) {
				line_->sort(LessByPointedToValue());
				line_->patternSequence()->sort(LessByPointedToValue());
			}
		}

		double SequenceEntry::tickPosition( ) const
		{
			return tickPosition_;
		}

		float SequenceEntry::patternBeats() const
		{
			return pattern_->beats();
		}

		void SequenceEntry::setPlayIteratorToBegin( )
		{
			playIterator_ = pattern_->begin();
		}

		std::list< PatternLine >::iterator & SequenceEntry::playIterator( )
		{
			return playIterator_;
		}

		std::list< PatternLine >::iterator SequenceEntry::begin( )
		{
			pattern_->end();
		}

		std::list< PatternLine >::iterator SequenceEntry::end( )
		{
			return pattern_->end();
		}



		// end of PatternEntry



		// represents one track/line in the sequencer and contains a list of patternEntrys, wich holds a pointer and tickposition to a SinglePattern
		SequenceLine::SequenceLine( )
		{
			patternSequence_ = 0;
		}

		SequenceLine::SequenceLine( PatternSequence * patSeq )
		{
			patternSequence_ = patSeq;
		}


		SequenceLine::~ SequenceLine( )
		{
			std::list<SequenceEntry*>::iterator it = begin();
			for ( it; it != end(); it++) delete *it;
		}

		SequenceEntry* SequenceLine::createEntry( SinglePattern * pattern, double position )
		{
			SequenceEntry* entry = new SequenceEntry(this);
			entry->setPattern(pattern);
			push_back(entry);
			entry->setTickPosition(position);
			entry->pattern()->beforeDelete.connect(this,&SequenceLine::onDeletePattern);
			entry->pattern()->beforeDelete.connect(patternSequence_ ,&PatternSequence::onDeletePattern);
			if (patternSequence_) {
				patternSequence_->push_back(entry);
				patternSequence_->sort(LessByPointedToValue());
  		}

			return entry;
		}

		double SequenceLine::tickLength( ) const
		{
			if (size() > 0 ) {
				return back()->tickPosition() + back()->patternBeats();
			} else
			return 0;
		}

		PatternSequence * SequenceLine::patternSequence( )
		{
			return patternSequence_;
		}

		void SequenceLine::onDeletePattern( SinglePattern * pattern )
		{
			std::list<SequenceEntry*>::iterator it = begin();
			for ( ; it != end(); it++) {
				SequenceEntry* entry = *it;
				if (entry->pattern() == pattern) {
					erase(it);
					break;
				}
			}
		}

		//end of sequenceLine;



		// PatternSequence
		PatternSequence::PatternSequence()
		{
		}


		PatternSequence::~PatternSequence()
		{
			std::vector<SequenceLine*>::iterator it = lines_.begin();
			for ( it; it != lines_.end(); it++) delete *it;
		}



		SequenceLine * PatternSequence::createNewLine( )
		{
			SequenceLine* line = new SequenceLine(this);
			lines_.push_back(line);

			return line;
		}

		const std::vector< SequenceLine * > & PatternSequence::lines( ) const
		{
			return lines_;
		}

		void PatternSequence::onDeletePattern( SinglePattern * pattern )
		{
			std::list<SequenceEntry*>::iterator it = begin();
			for ( ; it != end(); it++) {
				SequenceEntry* entry = *it;
				if (entry->pattern() == pattern) {
					erase(it);
					delete entry;
					break;
				}
			}
		}

	}
}