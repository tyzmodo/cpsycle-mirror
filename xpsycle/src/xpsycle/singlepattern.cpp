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
#include "singlepattern.h"

namespace psycle
{
	namespace host
	{

		SinglePattern::SinglePattern()
		{
			beats_ = 4;
			beatZoom_ = 4;
		}


		SinglePattern::~SinglePattern()
		{
			beforeDelete.emit(this);
		}

		void SinglePattern::setBeatZoom( int zoom )
		{
			beatZoom_ = zoom;
		}

		int SinglePattern::beatZoom( ) const
		{
			return beatZoom_;
		}

		void SinglePattern::setBeats( int beats )
		{
			beats_ = beats;
		}

		int SinglePattern::beats( ) const
		{
			return beats_;
		}

		void SinglePattern::setName( const std::string & name )
		{
			name_ = name;
		}

		const std::string & SinglePattern::name( ) const
		{
			return name_;
		}

	}
}



