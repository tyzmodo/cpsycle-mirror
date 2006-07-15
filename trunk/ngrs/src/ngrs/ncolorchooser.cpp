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
#include "ncolorchooser.h"

NColorChooser::NColorChooser()
 : NPanel()
{
  chooseSize = 10;
  cols = 4;
  initColorMap();
}


NColorChooser::~NColorChooser()
{
}

void NColorChooser::initColorMap( )
{
  colorMap.push_back(NColor(0,0,0));
  colorMap.push_back(NColor(255,255,255));
  colorMap.push_back(NColor(0,0,255));
  colorMap.push_back(NColor(0,255,0));
  colorMap.push_back(NColor(255,0,0));

}

void NColorChooser::paint( NGraphics * g )
{
  std::vector<NColor>::iterator it = colorMap.begin();

  int colCount = 0;
  int xp = 0;
  int yp = 0;

  for ( ; it < colorMap.end(); it++) {
    NColor & color = *it;
    g->setForeground(color);

    g->fillRect(xp,yp, chooseSize, chooseSize);

    if (cols == colCount) {
       xp = 0;
       colCount = 0;
       yp+=chooseSize;
    } else {
      xp+=chooseSize;
      colCount++;
    }
  }
}

int NColorChooser::preferredWidth( ) const
{
  return cols*chooseSize;
}

int NColorChooser::preferredHeight( ) const
{
  return d2i((colorMap.size() / (double) cols) * chooseSize);
}

void NColorChooser::onMousePress( int x, int y, int button )
{
  if (button == 1) {
     int col = d2i(x / chooseSize);
     int row = d2i(y / chooseSize);

     int index = col*row + col;
     if (index < colorMap.size()) {
       selectedColor_ = colorMap.at(index);
       colorSelected.emit(selectedColor_);
     }
  }
}

const NColor & NColorChooser::selectedColor( ) const
{
  return selectedColor_;
}


