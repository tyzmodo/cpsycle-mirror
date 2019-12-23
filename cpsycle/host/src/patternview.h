// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(PATTERNVIEW)
#define PATTERNVIEW

#include <uilabel.h>
#include <uicheckbox.h>
#include <uinotebook.h>
#include "patternproperties.h"
#include "trackerview.h"
#include "pianoroll.h"
#include "StepBox.h"
#include "workspace.h"

typedef struct {
	ui_component component;
	struct PatternView* view;
	Workspace* workspace;
} PatternViewStatus;

void patternviewstatus_init(PatternViewStatus*, ui_component* parent,
	Workspace*);

typedef struct {
	ui_component component;	
	StepBox step;
	ui_checkbox movecursorwhenpaste;
	PatternViewStatus status;	
} PatternViewBar;

void patternviewbar_init(PatternViewBar*, ui_component* parent, Workspace*);

typedef struct PatternView {
	ui_component component;
	ui_notebook notebook;
	ui_notebook editnotebook;
	TrackerView trackerview;
	PatternProperties properties;
	Pianoroll pianoroll;	
	TabBar tabbar;
	Workspace* workspace;
	unsigned int lpb;	
} PatternView;

void patternview_init(PatternView*, ui_component* parent,
		ui_component* tabbarparent,	Workspace*);
void patternview_setpattern(PatternView*, psy_audio_Pattern*);

#endif