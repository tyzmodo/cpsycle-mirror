
/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "nlabel.h"

using namespace std;

NLabel::NLabel()
 : NVisualComponent()
{
  init();
}

NLabel::NLabel( string text )
{
  init();
  text_ = text;
}

void NLabel::init( )
{
  setGeometry(new NRectShape());
  geometry()->setPosition(0,0,10,10);
  metrics.setFont(font());
  mnemonic_ = '\0';
  halign_=nAlLeft;
  valign_=nAlTop;
  orientation_ = nHorizontal;
  setEvents(false);
}


NLabel::~NLabel()
{
}

void NLabel::paint( NGraphics * g )
{
  int i = 0;
  int start = 0;
  int yp_ = g->textAscent() ;

  do {
    i = text_.find("\n", i);
    string substr;
    if (i != -1) {
       substr = text_.substr(start,i-start);
       start = i+1;
       i+=1;
    } else substr = text_.substr(start);

    int xp_ = 0;

    switch (halign_) {
       case nAlCenter : xp_ = (clientWidth() - g->textWidth(substr)) / 2;
       break;
       default:
       ;
    }

    switch (valign_) {
       case nAlCenter : yp_ = (clientHeight() + g->textHeight() /2 ) / 2;
       break;
       case nAlBottom : yp_ = clientHeight() - g->textDescent();
       break;
       default:
       ;
    }

//  metrics.setFont(font());
//  yp_ = metrics.textAscent();

    g->drawText(xp_, yp_, substr);

    if (mnemonic_!='\0') {
      std::string::size_type pos = substr.find((char) ((int)mnemonic_-32) );
      if (pos==std::string::npos) pos =  substr.find(mnemonic_);
      if (pos!=std::string::npos) {
         int w  = g->textWidth(substr.substr(0,pos));
         int w1 = g->textWidth(substr.substr(0,pos+1));
         g->drawLine(w,yp_+2,w1,yp_+2);
      }
    }
    yp_ = yp_ + g->textHeight();
  } while (i != string::npos);
}

void NLabel::setText( const std::string & text )
{
  text_ = text;
  if (orientation_ == nVertical) {
      // todo not implemented yet
  }
}

const std::string & NLabel::text( ) const
{
  return text_;
}


int NLabel::preferredHeight( ) const
{
  NFontMetrics metrics;
  metrics.setFont(font());
  int i = 0;
  int start = 0;
  int yp_ = metrics.textHeight() ;
  do {
    i = text_.find("\n", i);
    if (i != -1) {
       i+=1;
       yp_ = yp_ + metrics.textHeight();
    }
  } while (i != string::npos);

  return yp_ + spacing().top()+spacing().bottom() +borderTop()+borderBottom();
}

int NLabel::preferredWidth( ) const
{
  NFontMetrics metrics(font());
  int i = 0;
  int start = 0;
  int yp_ = metrics.textHeight() ;
  string substr;
  int xmax = 0;
  do {
    i = text_.find("\n", i);
    if (i != -1) {
       i+=1;
       substr = text_.substr(start,i-start+1);
       start = i;
    } else substr = text_.substr(start);
     if (metrics.textWidth(substr) > xmax) xmax = metrics.textWidth(substr);
  } while (i != string::npos);

  return xmax + spacing().left()+spacing().right()+borderLeft()+borderRight();
}


void NLabel::setMnemonic( char c )
{
  mnemonic_ = c;
}

char NLabel::mnemonic() {
  return mnemonic_;
}


void NLabel::setVAlign( int align )
{
  valign_ = align;
}

void NLabel::setHAlign( int align )
{
  halign_ = align;
}

int NLabel::vAlign( )
{
  return valign_;
}

int NLabel::hAlign( )
{
  return halign_;
}

void NLabel::setTextOrientation( int orientation )
{
  orientation_ = orientation;
}

