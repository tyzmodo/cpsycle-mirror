// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net


#if !defined(PATTERNPROPERTIES)
#define PATTERNPROPERTIES

#include "uilabel.h"
#include "uiedit.h"
#include "uibutton.h"
#include "pattern.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PatternViewSkin;

typedef struct {
	psy_ui_Component component;
	psy_ui_Label namelabel;
	psy_ui_Label lengthlabel;
	psy_ui_Edit nameedit;
	psy_ui_Edit lengthedit;
	psy_ui_Button applybutton;
	// references
	psy_audio_Pattern* pattern;
	Workspace* workspace;
	struct PatternViewSkin* skin;
} PatternProperties;

void patternproperties_init(PatternProperties*, psy_ui_Component* parent, psy_audio_Pattern*,
	struct PatternViewSkin* skin, Workspace*);
void patternproperties_setpattern(PatternProperties*, psy_audio_Pattern*);

#ifdef __cplusplus
}
#endif

#endif