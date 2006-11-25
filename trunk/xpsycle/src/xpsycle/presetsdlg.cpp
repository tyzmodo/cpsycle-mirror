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
#include "presetsdlg.h"
#include "machine.h"
#include "plugin.h"
#include "global.h"
#include "framemachine.h"
#include "fileio.h"
#include "configuration.h"
#include <ngrs/nbutton.h>
#include <ngrs/ncheckbox.h>
#include <ngrs/ngridlayout.h>
#include <ngrs/nlistbox.h>
#include <ngrs/nborderlayout.h>
#include <ngrs/nitem.h>

namespace psycle {
	namespace host {

using namespace std;


Preset::Preset( ) : numpars_(0), dataSize_(0)
{
}

Preset::Preset( int numpars, int dataSize ) : numpars_(numpars), dataSize_(dataSize)
{
}


bool Preset::loadFromFile( RiffFile* f )
{
    char cbuf[32];
    if (!f->ReadChunk(cbuf,sizeof(cbuf))) return 0;
    
    name_ = string(cbuf);  // read the preset name

    for (int i = 0; i < numpars_; i++) {
      int temp;
      f->Read(temp);
      params_.push_back(temp);
    }
    for (int i = 0; i < dataSize_; i++) {
      byte temp;
      f->Read(temp);
      data_.push_back(temp);
    }
    return 1;
}

const std::string & Preset::name( ) const
{
  return name_;
}


void Preset::tweakMachine( Machine * mac )
{
  if(mac->_type == MACH_PLUGIN) {
      int i = 0;
      for (std::vector<int>::iterator it = params_.begin(); it < params_.end(); it++) {
        try {
          reinterpret_cast<Plugin *>(mac)->proxy().ParameterTweak(i, *it);
        }
        catch(const std::exception &) {
        }
        catch(...) {
        }
        i++;
      }

      try {
        reinterpret_cast<Plugin *>(mac)->proxy().PutData(&data_[0]); // Internal save
      }
      catch(const std::exception &) {
      }
      catch(...) {
      }
  }
}

//
// start of presetDlg
//

PresetsDlg::PresetsDlg(FrameMachine* mac)
  : NDialog(), fMac(mac)
{
  setTitle("Machine Presets");

  pane()->setSpacing(5,5,5,5);

  NPanel* btnBar = new NPanel();
    NGridLayout grid;
      grid.setVgap(5);
      grid.setHgap(5);
    btnBar->setLayout(grid);
    NButton* btn;
    btnBar->add(new NButton("Save",false));
    btnBar->add(new NButton("Delete",false));
    btnBar->add(new NButton("Import",false));
    btnBar->add(new NButton("Export",false));
    btnBar->add(new NCheckBox("Preview"));
    btn = new NButton("Use");
      btn->setFlat(false);
      btn->clicked.connect(this,&PresetsDlg::onUseClicked);
    btnBar->add(btn);
    btnBar->add(new NButton("Close",false));
  pane()->add(btnBar,nAlRight);

  lBox = new NListBox();
  pane()->add(lBox,nAlClient);

  loadPresets();

  setPosition(100,100,300,350);
}


PresetsDlg::~PresetsDlg()
{
}

void PresetsDlg::loadPresets( )
{
  string filename(fMac->pMac()->GetDllName());

  string::size_type pos = filename.find('.')  ;
  if ( pos == string::npos ) {
    filename  = filename + '.' + "prs";
  } else {
    filename = filename.substr(0,pos)+".prs";
  }

  try {
      std::cout << Global::pConfig()->prsPath+filename << std::endl;
      RiffFile f;
      if (!f.Open(Global::pConfig()->prsPath+filename)) {
        throw "couldn`t open file";
      }
      int numpresets;
      f.Read(numpresets);
      int filenumpars;
      f.Read(filenumpars);
      if (numpresets >= 0) {
        // old file format .. do not support so far ..
      } else {
        // new file format
        if (filenumpars == 1) {
          int filepresetsize;
          // new preset format version 1
          // new preset format version 1

          int numParameters = ((Plugin*) fMac->pMac())->GetInfo()->numParameters;
          int sizeDataStruct = ((Plugin *) fMac->pMac())->proxy().GetDataSize();

          int numpresets;
          f.Read(numpresets);
          int filenumpars;
          f.Read(filenumpars);
          f.Read(filepresetsize);

          if (( filenumpars != numParameters )  || (filepresetsize != sizeDataStruct)) return;

          while (!f.Eof() ) {
            Preset newPreset(numParameters, sizeDataStruct);
            newPreset.loadFromFile(&f);
            presetMap[newPreset.name()] = newPreset;
            lBox->add(new NItem(newPreset.name()));
          }
        }
      }
  } catch (const char * e) {
      // couldn`t open presets
     std::cerr << e << std::endl;
  }
}

void PresetsDlg::onUseClicked( NButtonEvent * ev )
{
  if (lBox->selIndex() != -1) {
    NCustomItem* item = lBox->itemAt(lBox->selIndex());
    std::map<std::string,Preset>::iterator itr;
    if ( (itr = presetMap.find(item->text())) != presetMap.end() ) {
        itr->second.tweakMachine(fMac->pMac() );
    }
    fMac->updateValues();
  };
}

}
}

