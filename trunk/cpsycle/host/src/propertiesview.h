// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(PROPERTIESVIEW)
#define PROPERTIESVIEW

// host
#include "inputdefiner.h"
#include "tabbar.h"
#include "workspace.h"
// ui
#include <uiedit.h>
#include <uilabel.h>
#include <uiscroller.h>
#include <uinotebook.h>
// container
#include <hashtbl.h>
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: Displays psy_Property and allows to edit them.

#define PROPERTIESRENDERER_NUMCOLS 3

typedef struct PropertiesRenderLineState {
	intptr_t cpy;
	intptr_t cpx;
	intptr_t level;
	intptr_t numlines;
	psy_Property* properties;
} PropertiesRenderLineState;

void propertiesrenderlinestate_init(PropertiesRenderLineState*);
void propertiesrenderlinestate_dispose(PropertiesRenderLineState*);

typedef struct PropertiesRenderer {
	// inherits
	psy_ui_Component component;
	// internal data	
	psy_ui_Graphics* g;
	psy_ui_Rectangle selrect;
	int lastlevel;	
	int keyselected;
	int button;	
	intptr_t mx;
	intptr_t my;
	intptr_t cpy;
	intptr_t cpx;
	intptr_t lineheight;
	intptr_t textheight;
	intptr_t centery;
	intptr_t numblocklines;
	intptr_t identwidth;
	bool showkeyselection;
	float col_perc[PROPERTIESRENDERER_NUMCOLS];	
	intptr_t col_width[PROPERTIESRENDERER_NUMCOLS];
	intptr_t col_start[PROPERTIESRENDERER_NUMCOLS];
	psy_ui_Edit edit;
	InputDefiner inputdefiner;
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	psy_ui_Value fixedwidth;
	bool usefixedwidth;
	PropertiesRenderLineState* linestate_clipstart;
	uintptr_t currlinestatecount;
	psy_Table linestates;	
	psy_ui_Colour valuecolour;
	psy_ui_Colour sectioncolour;
	psy_ui_Colour separatorcolour;
	psy_ui_Colour valueselcolour;
	psy_ui_Colour valueselbackgroundcolour;
	bool floated;
	// references
	psy_Property* properties;
	psy_Property* selected;
	psy_Property* search;
	psy_Property* choiceproperty;
	Workspace* workspace;
} PropertiesRenderer;

void propertiesrenderer_init(PropertiesRenderer*, psy_ui_Component* parent,
	psy_Property*, Workspace*);

void propertiesrenderer_setfixedwidth(PropertiesRenderer*, psy_ui_Value width);

INLINE psy_ui_Component* propertiesrenderer_base(PropertiesRenderer* self)
{
	return &self->component;
}

typedef struct PropertiesView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Notebook notebook;
	psy_ui_Component client;
	psy_ui_Component sectionfloated;
	psy_ui_Label floatdesc;
	psy_ui_Component viewtabbar;	
	TabBar tabbar;
	PropertiesRenderer renderer;
	psy_ui_Scroller scroller;
	psy_Signal signal_changed;
	psy_Signal signal_selected;
	// references
	Workspace* workspace;
} PropertiesView;

void propertiesview_init(PropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, psy_Property*, Workspace*);

// float to side bar (see helpview, too)
// todo make it more general
void propertiesview_float(PropertiesView*, uintptr_t section, psy_ui_Component* dest);
void propertiesview_dock(PropertiesView*, uintptr_t section, psy_ui_Component* src);

INLINE void propertiesview_setcolumnwidth(PropertiesView* self,
	float col0_perc, float col1_perc, float col2_perc)
{
	self->renderer.col_perc[0] = col0_perc;
	self->renderer.col_perc[1] = col1_perc;
	self->renderer.col_perc[2] = col2_perc;	
}

INLINE psy_ui_Component* propertiesview_base(PropertiesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* PROPERTIESVIEW */
