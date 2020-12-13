// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SWINGFILLVIEW_H)
#define SWINGFILLVIEW_H

// host
#include "intedit.h"
#include "realedit.h"
#include "workspace.h"
// ui
#include "uibutton.h"
#include "uicheckbox.h"
#include "uilabel.h"

#ifdef __cplusplus
extern "C" {
#endif

// swing fill view.
		
typedef struct SwingFillView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component client;
	IntEdit	tempo;
	IntEdit	width;
	RealEdit variance;
	RealEdit phase;
	psy_ui_Component offsetrow;
	psy_ui_Button actual_bpm;
	psy_ui_Button center_bpm;
	psy_ui_Label offsetdesc;
	// actions	
	psy_ui_Component actions;
	psy_ui_Button apply;
	psy_ui_Button cancel;
	// internal data	
	bool offset;
	// references
	Workspace* workspace;
} SwingFillView;

void swingfillview_init(SwingFillView*, psy_ui_Component* parent, Workspace*);
void swingfillview_reset(SwingFillView* self, int bpm);
void swingfillview_setvalues(SwingFillView*, int tempo, int width, float variance,
	float phase, bool offset);
void swingfillview_values(SwingFillView*, int* rv_tempo, int* rv_width,
	float* rv_variance, float* rv_phase, bool* rv_offset);

INLINE psy_ui_Component* swingfillview_base(SwingFillView* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SWINGFILLVIEW_H */
