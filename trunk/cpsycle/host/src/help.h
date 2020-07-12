// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(HELP_H)
#define HELP_H

#include "about.h"
#include "greet.h"
#include "tabbar.h"
#include "workspace.h"

#include <uieditor.h>

// aim: displays the help files with the Scintilla component, an open source
//      source editor component, first used with psycle 1.12 and avoids the
//      opening of an external text editor viewer like notepad.

typedef struct {
	psy_ui_Component component;
	psy_ui_Editor editor;
	TabBar tabbar;
	Workspace* workspace;
	psy_Table filenames;
} Help;

void help_init(Help*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* help_base(Help* self)
{
	return &self->component;
}

#endif