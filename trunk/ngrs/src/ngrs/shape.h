/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef SHAPE_H
#define SHAPE_H

#include "rect.h"
#include "graphics.h"
#include "size.h"
#include "point.h"
#include "region.h"

/**
@author  Stefan
*/

namespace ngrs {

  class Shape{
  public:
    Shape();

    virtual ~Shape();

    virtual void setPosition(int left, int top, int width, int height);
    virtual void setLeft(int left);
    virtual void setTop(int top);
    virtual void setWidth(int width);
    virtual void setHeight(int height);
    const Rect & rectArea() const;

    virtual void fill(Graphics& g, const ngrs::Region & repaintArea);
    virtual void drawRectPicker(Graphics& g);
    virtual void drawPicker(Graphics& g);
    virtual int  overPicker(int x, int y);

    int top() const;
    int left() const;
    int width() const;
    int height() const;

    virtual NPoint pickerAt(int i);
    virtual int pickerSize();
    virtual void setPicker(int index, int x, int y);

    virtual ngrs::Region region();
    virtual ngrs::Region spacingRegion(const Size & spacing);

    virtual Shape* clone()  const = 0;   // Uses the copy constructor

  private:

    Rect rectArea_;

  };

}

#endif
