/* -*- mode:c++, indent-tabs-mode:t -*- */
/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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
#ifndef TRACKHEADER_H
#define TRACKHEADER_H

class QGraphicsRectItem;
class QPaintEvent;

#include <QWidget>

namespace qpsycle {

class PatternDraw;

class TrackHeader: public QWidget {
public:
	TrackHeader( PatternDraw* pPatternDraw );
	~TrackHeader();

	void paintEvent( QPaintEvent *event );

	static const int height();
private:
	PatternDraw* pDraw;
	static const int height_;
};

} // namespace qpsycle

#endif
