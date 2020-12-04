// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SONGPROPERTIES_H)
#define SONGPROPERTIES_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>
#include <uiedit.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IntEdit {
	psy_ui_Component component;
	psy_ui_Label desc;
	psy_ui_Edit edit;
	psy_ui_Button less;
	psy_ui_Button more;
	int minval;
	int maxval;
	int restore;
	psy_Signal signal_changed;
} IntEdit;

void intedit_init(IntEdit*, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval);
void intedit_init_connect(IntEdit*, psy_ui_Component* parent,
	const char* desc, int value, int minval, int maxval,
	void* context, void* fp);
int intedit_value(IntEdit*);
void intedit_setvalue(IntEdit*, int value);
void intedit_enableedit(IntEdit*);
void intedit_preventedit(IntEdit*);


INLINE psy_ui_Component* intedit_base(IntEdit* self)
{
	assert(self);

	return &self->component;
}

typedef struct SongPropertiesView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component tabbar;	
	psy_ui_Component title;
	psy_ui_Label label_title;
	psy_ui_Edit edit_title;
	psy_ui_Component credits;
	psy_ui_Label label_credits;
	psy_ui_Edit edit_credits;
	// speed
	psy_ui_Component speed;
	psy_ui_Label label_speed;
	psy_ui_Component speedbar;
	IntEdit tempo;
	IntEdit lpb;
	IntEdit tpb;
	IntEdit etpb;	
	psy_ui_Label realtempo_desc;
	psy_ui_Label realticksperbeat_desc;
	psy_ui_Label realtempo;
	psy_ui_Label realticksperbeat;
	// text
	psy_ui_Component comments;
	psy_ui_Label label_comments;
	psy_ui_Edit edit_comments;
	// references
	psy_audio_Song* song;
	Workspace* workspace;
} SongPropertiesView;

void songpropertiesview_init(SongPropertiesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);
void songpropertiesview_enableedit(SongPropertiesView*);
void songpropertiesview_disableedit(SongPropertiesView*);

#ifdef __cplusplus
}
#endif

#endif /* SONGPROPERTIES_H */
