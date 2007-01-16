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
#ifndef NLAYOUT_H
#define NLAYOUT_H

#include "graphics.h"
#include "region.h"

namespace ngrs {

  const short nAlNone   = 0;
  const short nAlLeft   = 1;
  const short nAlTop    = 2;
  const short nAlRight  = 3;
  const short nAlBottom = 4;
  const short nAlClient = 5;
  const short nAlCenter = 6;

  /**
  @author Stefan
  */

  class Layout{
  public:
    Layout();
    virtual Layout* clone()  const = 0;   // Uses the copy constructor

    virtual ~Layout() = 0;

    virtual void align(class VisualComponent* parent);
    virtual int preferredWidth(const class VisualComponent* target) const = 0;
    virtual int preferredHeight(const class VisualComponent* target) const = 0;

    void setParent(class VisualComponent* parent);
    class VisualComponent* parent() const;

    virtual void drawComponents(class VisualComponent* target, Graphics& g , const ngrs::Region & repaintArea, VisualComponent* sender);

    virtual void add(class VisualComponent* comp);
    virtual void insert(class VisualComponent* comp, int index);
    virtual void remove(class VisualComponent* comp);
    virtual void removeAll();   

  private:

    class VisualComponent* parent_;


  };
}

#endif
