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
#ifndef NPIXMAP_H
#define NPIXMAP_H

#include "nobject.h"
#include "nsystem.h"

#ifdef __unix__
#else
#include "nbitmap.h"
typedef NBitmap NPixmap;
#endif

#ifdef __unix__

class NWindow;

/**
@author Stefan Nattkemper
*/
class NPixmap : public NObject
{
public:

    NPixmap();
    NPixmap(NWindow* win);

    ~NPixmap();

    int width()  const;
    int height() const;


    Pixmap X11Pixmap() const;
    Pixmap X11ShapePixmap() const;

    WinHandle owner() const;

    void loadFromFile( const std::string & filename );
    void createFromXpmData(const char** data);

    bool empty() const;


private:

    int width_, height_;

    Pixmap pixmap_;
    Pixmap shapepixmap_;

    WinHandle owner_;
};
#endif
#endif
