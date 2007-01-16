/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "lineshape.h"
#include <cmath>

namespace ngrs {

  LineShape::LineShape()
    : Shape()
  {
    p1_.setXY(0,0);
    p2_.setXY(10,10);
    pickWidth_ = pickHeight_ = 5;
    distance_ = 5;
    calculateRectArea();
  }


  LineShape::~LineShape()
  {
  }

  LineShape * LineShape::clone( ) const
  {
    return new LineShape(*this);
  }

  NPoint LineShape::pickerAt( int i )
  {
    if (i == 0) return p1_; else return p2_;
  }

  int LineShape::pickerSize( )
  {
    return 5;
  }

  void LineShape::setPosition( int left, int top, int width, int height )
  {
    int dx = left - rectArea().left();
    int dy = top  - rectArea().top();
    move(dx,dy);
    resize(width,height);
    calculateRectArea();
  }

  void LineShape::resize( int width, int height )
  {
    int x = rectArea().left();
    int y = rectArea().top();

    int w = rectArea().width();
    int h = rectArea().height();

    p1_.setX( (int) d2i( ((width) / double ( w) ) * (p1_.x() - x) + x));
    p1_.setY( (int) d2i( ((height) / double ( h) ) * (p1_.y() - y) + y));

    p2_.setX( (int) d2i( ((width) / double ( w) ) * (p2_.x() - x) + x));
    p2_.setY( (int) d2i( ((height) / double ( h) ) * (p2_.y() - y) + y));

  }

  void LineShape::calculateRectArea( )
  {
    double  ankathede    = (p1().x() - p2().x());
    double  gegenkathede = (p1().y() - p2().y());
    double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede);

    double cos = ankathede    / hypetenuse;
    double sin = gegenkathede / hypetenuse;

    int dx = (int) (-sin*distance_);
    int dy = (int) (-cos*distance_);


    NPoint  pts[4];
    pts[0].setX ( p1_.x() + dx );
    pts[0].setY ( p1_.y() - dy );
    pts[1].setX ( p2_.x() + dx );
    pts[1].setY ( p2_.y() - dy );
    pts[2].setX ( p2_.x() - dx );
    pts[2].setY ( p2_.y() + dy );
    pts[3].setX ( p1_.x() - dx );
    pts[3].setY ( p1_.y() + dy );

    ngrs::Region region;
    region.setPolygon(pts,4);

    Rect r = region.rectClipBox();
    Shape::setPosition( r.left(), r.top(), r.width(), r.height() );
  }

  void LineShape::move( int dx, int dy )
  {
    p1_.setXY(p1_.x()+dx, p1_.y() +dy);
    p2_.setXY(p2_.x()+dx, p2_.y() +dy);
    calculateRectArea();
  }

  void LineShape::drawPicker( Graphics& g )
  {
    g.setForeground(Color(0,0,0));
    g.fillRect(p1_.x()- pickWidth_/2,p1_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
    g.fillRect(p2_.x()- pickWidth_/2,p2_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
  }

  void LineShape::setLeft( int left )
  {
    int dx = left - rectArea().left();
    move(dx,0);
  }

  void LineShape::setTop( int top )
  {
    int dy = top  - rectArea().top();
    move(0,dy);
  }

  void LineShape::setWidth( int width )
  {
    resize(width,rectArea().height());
    calculateRectArea();
  }

  void LineShape::setHeight( int height )
  {
    resize(rectArea().width(),height);
    calculateRectArea();
  }

  const NPoint & LineShape::p1( )
  {
    return p1_;
  }

  const NPoint & LineShape::p2( )
  {
    return p2_;
  }

  void LineShape::setPoints( NPoint p1, NPoint p2 )
  {
    p1_ = p1;
    p2_ = p2;
    calculateRectArea();
  }

  int LineShape::overPicker( int x, int y )
  {
    if (Rect(p1_.x()-pickWidth_/2,p1_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 0;
    if (Rect(p2_.x()-pickWidth_/2,p2_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 1;
    return -1;
  }

  void LineShape::setPicker( int index, int x, int y )
  {
    if (index == 0) {
      p1_ = NPoint(x,y);
      calculateRectArea();
    } else 
      if (index == 1) {
        p2_ = NPoint(x,y);
        calculateRectArea();
      }
  }

  ngrs::Region LineShape::lineToRegion( )
  {
    double  ankathede    = (p1().x() - p2().x());
    double  gegenkathede = (p1().y() - p2().y());
    double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede);

    double cos = ankathede    / hypetenuse;
    double sin = gegenkathede / hypetenuse;

    int dx = (int) ( -sin*distance_);
    int dy = (int) ( -cos*distance_);


    NPoint  pts[4];
    pts[0].setX ( p1_.x() + dx );
    pts[0].setY ( p1_.y() - dy );
    pts[1].setX ( p2_.x() + dx );
    pts[1].setY ( p2_.y() - dy );
    pts[2].setX ( p2_.x() - dx );
    pts[2].setY ( p2_.y() + dy );
    pts[3].setX ( p1_.x() - dx );
    pts[3].setY ( p1_.y() + dy );

    ngrs::Region region;
    region.setPolygon(pts,4);

    return region;
  }

  void LineShape::setClippingDistance( int d )
  {
    distance_ = d;
  }

  ngrs::Region LineShape::region( )
  {
    return lineToRegion();
  }

  ngrs::Region LineShape::spacingRegion( const Size & spacing )
  {
    return lineToRegion();
  }


  int LineShape::d2i(double d)
  {
    return (int) ( d<0?d-.5:d+.5);
  }

}
