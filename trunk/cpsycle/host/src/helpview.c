/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "helpview.h"
/* host */
#include "resources/resource.h"

/* prototypes */
static void helpview_initbase(HelpView*, psy_ui_Component* parent);
static void helpview_inittabbar(HelpView*, psy_ui_Component* tabbarparent,
	Workspace*);
static void helpview_initsections(HelpView*, Workspace* workspace);
static void helpview_onselectsection(HelpView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void helpview_on_focus(HelpView*, psy_ui_Component* sender);

/* implementation */
void helpview_init(HelpView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	helpview_initbase(self, parent);
	self->workspace = workspace;
	helpview_inittabbar(self, tabbarparent, workspace);
	helpview_initsections(self, workspace);	
	psy_ui_tabbar_select(&self->tabbar, HELPVIEWSECTION_ABOUT);
}

void helpview_initbase(HelpView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(helpview_base(self), parent, NULL);
	psy_ui_component_set_id(helpview_base(self), VIEW_ID_HELPVIEW);
	psy_signal_connect(&helpview_base(self)->signal_focus, self,
		helpview_on_focus);
	psy_signal_connect(&helpview_base(self)->signal_selectsection, self,
		helpview_onselectsection);
}

void helpview_inittabbar(HelpView* self, psy_ui_Component* tabbarparent,
	Workspace* workspace)
{	
	psy_ui_component_init_align(&self->bar, tabbarparent, NULL,
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_init(&self->tabbar, &self->bar);
	psy_ui_component_setalignexpand(&self->bar, psy_ui_HEXPAND);
	psy_ui_component_set_align(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_append_tabs(&self->tabbar, "help.help", "help.about",
		"help.greetings", NULL);	
}

void helpview_initsections(HelpView* self, Workspace* workspace)
{
	psy_ui_notebook_init(&self->notebook, helpview_base(self));
	psy_ui_component_set_align(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	help_init(&self->help, psy_ui_notebook_base(&self->notebook),
		&workspace->config.directories);
	about_init(&self->about, psy_ui_notebook_base(&self->notebook), workspace);
	greet_init(&self->greet, psy_ui_notebook_base(&self->notebook));	
}

void helpview_onselectsection(HelpView* self, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options)
{
	psy_ui_tabbar_select(&self->tabbar, (int)section);
}

void helpview_on_focus(HelpView* self, psy_ui_Component* sender)
{
	psy_ui_Component* view;

	view = psy_ui_notebook_active_page(&self->notebook);
	if (view) {
		/* psy_ui_component_set_focus(view); */
	}
}
