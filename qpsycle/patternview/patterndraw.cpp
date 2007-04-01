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

#include "psycore/song.h"

#include "patternview.h"
#include "patterndraw.h"
#include "patterngrid.h"
#include "header.h"

#include <QtGui>

PatternDraw::PatternDraw( PatternView *patView )
{
    patView_ = patView; 
    setAlignment( Qt::AlignLeft | Qt::AlignTop );
    scene_ = new QGraphicsScene(this);
    scene_->setBackgroundBrush( QColor( 30, 30, 30 ) );
    setScene(scene_);

    setupTrackGeometrics( patView_->numberOfTracks() );
    
    lineNumCol_ = new LineNumberColumn( this );
    Header *trackHeader = new Header( this );
    patGrid_ = new PatternGrid( this );

    scene_->addItem( lineNumCol_ );
    scene_->addItem( trackHeader );
    scene_->addItem( patGrid_ );

    trackHeader->setPos( 50, 0 );
    lineNumCol_->setPos( 0, 20 );
    patGrid_->setPos( 50, 20 );
}

void PatternDraw::setupTrackGeometrics( int numberOfTracks ) 
{
    for ( int newTrack = 0; newTrack < numberOfTracks; newTrack++ ) {
        TrackGeometry trackGeometry( *this );
        trackGeometry.setVisibleColumns( 6 );
        trackGeometryMap[ newTrack ] = trackGeometry;
    }

    std::map<int, TrackGeometry>::iterator it;
    it = trackGeometryMap.lower_bound( numberOfTracks );
    while ( it != trackGeometryMap.end() ) {
        trackGeometryMap.erase( it++ );
    }			
}

void PatternDraw::alignTracks() 
{
    std::map<int, TrackGeometry>::iterator it = trackGeometryMap.begin();
    int offset = 0;
    for ( ; it != trackGeometryMap.end(); it++ ) {
        TrackGeometry & geometry = it->second;
        geometry.setLeft( offset );
        std::cout << "tw: " << geometry.width() << std::endl;
        offset+= std::max( 50, geometry.width() );		// 50 is track min width
    }
}

const std::map<int, TrackGeometry> & PatternDraw::trackGeometrics() const {
    return trackGeometryMap;
}

/** 
 * Get the width of the grid up until and including the given track.
 */
int PatternDraw::gridWidthByTrack( int track ) const 
{
    std::map<int, TrackGeometry>::const_iterator it;
    it = trackGeometrics().lower_bound( track );
    int gridWidth = 0;
    if ( it != trackGeometrics().end() ) {
        TrackGeometry trackGeom = it->second;
        gridWidth = trackGeom.left() + trackGeom.width();
    }
    return gridWidth;
}



//
//
// TrackGeometry
//

TrackGeometry::TrackGeometry( ) :
pDraw( 0 ),
    left_(0),
    width_(0),
    visibleColumns_(0),
    visible_(1)
{ }

TrackGeometry::TrackGeometry( PatternDraw & patternDraw ) :
pDraw( &patternDraw ),
    left_(0),
    width_(0),
    visibleColumns_(0),
    visible_(1)
{ }

TrackGeometry::~TrackGeometry() { }

void TrackGeometry::setLeft( int left ) {
    left_ = left;
}

int TrackGeometry::left() const {
    return left_;
}

int TrackGeometry::width() const {
    return pDraw->patternGrid()->visibleColWidth( visibleColumns() );
}

void TrackGeometry::setVisibleColumns( int cols ) {
    visibleColumns_= cols;
}

int TrackGeometry::visibleColumns() const {
    return visibleColumns_;
}

void TrackGeometry::setVisible( bool on) {
    visible_ = on;
}

bool TrackGeometry::visible() const {
    return visible_;
}
