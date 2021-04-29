// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(RECENTVIEW_H)
#define RECENTVIEW_H

#include "propertiesview.h"
#include <uibutton.h>
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// aim: shows recently opened songs

typedef struct RecentBar {
	psy_ui_Component component;
	psy_ui_Component client;
	psy_ui_Button clear;
	psy_ui_Button play;
	psy_ui_Button stop;
	psy_ui_Button del;
	psy_ui_Button up;
	psy_ui_Button down;
} RecentBar;

void recentbar_init(RecentBar*, psy_ui_Component* parent);

typedef struct {	
	psy_ui_Component component;	
	RecentBar bar;	
	PropertiesView view;
	Workspace* workspace;
	bool starting;	
} RecentView;

void recentview_init(RecentView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

INLINE psy_ui_Component* recentview_base(RecentView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* RECENTVIEW_H */
