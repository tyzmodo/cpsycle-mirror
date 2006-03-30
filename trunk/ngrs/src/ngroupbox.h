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
#ifndef NGROUPBOX_H
#define NGROUPBOX_H

#include <npanel.h>
#include <nborder.h>

/**
@author Stefan
*/
class NGroupBox : public NPanel
{

  class GBorder : public NBorder {
  public:
     GBorder(NGroupBox* groupBox);
     ~GBorder();

     void setHeaderText(std::string text);

     virtual void paint(NGraphics* g, const NShape & geometry);

  private:

     NGroupBox* pBox;
     std::string headerText_;
  };

public:
    NGroupBox();

    ~NGroupBox();

    void setHeaderText(std::string text);

private:

   GBorder* gBorder;
};

#endif
