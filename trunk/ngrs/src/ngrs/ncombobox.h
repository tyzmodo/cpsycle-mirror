/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper                   *
 *   Made in Germany                                                       *
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
#ifndef NCOMBOBOX_H
#define NCOMBOBOX_H

#include "ncustomcombobox.h"

namespace ngrs {

  class NCustomItem;
  class NButton;
  class NEdit;
  class NListBox;
  class NItemEvent;
  class NPopupWindow;

  /**
  @author Stefan
  */

  class NComboBox : public NCustomComboBox
  {
  public:
    NComboBox();

    ~NComboBox();

    signal1<NItemEvent*> itemSelected;

    const std::string & text() const;

    virtual void resize();
    virtual int preferredHeight() const;

    virtual void add(NCustomItem* item);
    virtual void removeChilds();

    void setIndex(int i);
    int selIndex() const;
    int itemCount();
    NCustomItem* itemAt(unsigned int index);

    std::vector<NCustomItem*> & items();

    virtual void onItemClicked(NItemEvent * ev);

  protected:

    NEdit* edit();

  private:

    NBitmap down;

    NEdit*   edit_;
    NButton* downBtn_;

    NListBox* lbox;
    NPopupWindow*  popup;

    void init();

    void onDownBtnClicked(NButtonEvent * ev);

  };

}

#endif
