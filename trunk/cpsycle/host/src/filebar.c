// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filebar.h"

#include <uiopendialog.h>
#include <uisavedialog.h>

static void filebar_updatetext(FileBar*);
static void filebar_initalign(FileBar*);
static void filebar_onnewsong(FileBar*, psy_ui_Component* sender);
static void filebar_onloadsong(FileBar*, psy_ui_Component* sender);
static void filebar_onsavesong(FileBar*, psy_ui_Component* sender);
static void filebar_onlanguagechanged(FileBar*, Workspace* sender);

void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(filebar_base(self), parent);	
	psy_ui_component_enablealign(filebar_base(self));
	psy_ui_component_setalignexpand(filebar_base(self),
		psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->recentbutton, filebar_base(self));
	psy_ui_button_seticon(&self->recentbutton, psy_ui_ICON_MORE);
	psy_ui_label_init(&self->header, filebar_base(self));
	psy_ui_button_init(&self->newbutton, filebar_base(self));
	psy_signal_connect(&self->newbutton.signal_clicked, self,
		filebar_onnewsong);
	psy_ui_button_init(&self->loadbutton, filebar_base(self));
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
	psy_ui_button_init(&self->savebutton, filebar_base(self));
	psy_signal_connect(&self->savebutton.signal_clicked, self,
		filebar_onsavesong);
	psy_ui_button_init(&self->renderbutton, filebar_base(self));	
	filebar_initalign(self);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		filebar_onlanguagechanged);
	filebar_updatetext(self);
}

void filebar_updatetext(FileBar* self)
{
	psy_ui_label_settext(&self->header,
		workspace_translate(self->workspace, "Song"));
	psy_ui_button_settext(&self->newbutton,
		workspace_translate(self->workspace, "New"));	
	psy_ui_button_settext(&self->loadbutton,
		workspace_translate(self->workspace, "Load"));	
	psy_ui_button_settext(&self->savebutton,
		workspace_translate(self->workspace, "Save"));	
	psy_ui_button_settext(&self->renderbutton,
		workspace_translate(self->workspace, "Render"));	
}

void filebar_initalign(FileBar* self)
{	
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(filebar_base(self), psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
}

void filebar_onnewsong(FileBar* self, psy_ui_Component* sender)
{
	if (workspace_songmodified(self->workspace)) {
		workspace_selectview(self->workspace, TABPAGE_CHECKUNSAVED, 0, CHECKUNSAVE_NEW);
	} else {
		workspace_newsong(self->workspace);
	}
}

void filebar_onloadsong(FileBar* self, psy_ui_Component* sender)
{	
	if (workspace_songmodified(self->workspace)) {
		workspace_selectview(self->workspace, TABPAGE_CHECKUNSAVED, 0, CHECKUNSAVE_LOAD);
	} else {
		psy_ui_OpenDialog dialog;
		static char filter[] =
			"All Songs (*.psy *.xm *.it *.s3m *.mod *.wav)" "|*.psy;*.xm;*.it;*.s3m;*.mod;*.wav|"
			"Songs (*.psy)"				        "|*.psy|"
			"FastTracker II Songs (*.xm)"       "|*.xm|"
			"Impulse Tracker Songs (*.it)"      "|*.it|"
			"Scream Tracker Songs (*.s3m)"      "|*.s3m|"
			"Original Mod Format Songs (*.mod)" "|*.mod|"
			"Wav Format Songs (*.wav)"			"|*.wav";

		psy_ui_opendialog_init_all(&dialog, 0, "Load Song", filter, "PSY",
			workspace_songs_directory(self->workspace));
		if (psy_ui_opendialog_execute(&dialog)) {
			workspace_loadsong(self->workspace,
				psy_ui_opendialog_filename(&dialog));
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void filebar_onsavesong(FileBar* self, psy_ui_Component* sender)
{	
	psy_ui_SaveDialog dialog;

	psy_ui_savedialog_init_all(&dialog, 0,
		"Save Song",
		"Songs (*.psy)|*.psy", "PSY",
		workspace_songs_directory(self->workspace));
	if (psy_ui_savedialog_execute(&dialog)) {
		workspace_savesong(self->workspace,
			psy_ui_savedialog_filename(&dialog));
	}
	psy_ui_savedialog_dispose(&dialog);
}

void filebar_onlanguagechanged(FileBar* self, Workspace* sender)
{
	filebar_updatetext(self);
	psy_ui_component_align(filebar_base(self));
}
