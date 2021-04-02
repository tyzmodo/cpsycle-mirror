// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiframe.h"
#include "uiapp.h"
#include "uiimpfactory.h"

void psy_ui_frame_init(psy_ui_Frame* self, psy_ui_Component* parent)
{	
	psy_ui_ComponentImp* imp;

	imp = psy_ui_impfactory_allocinit_frameimp(psy_ui_app_impfactory(psy_ui_app()), self, parent);
	psy_ui_component_init_imp(self, parent, imp);
	psy_ui_component_setbackgroundmode(self, psy_ui_NOBACKGROUND);
}

void psy_ui_toolframe_init(psy_ui_Frame* self, psy_ui_Component* parent)
{
	psy_ui_ComponentImp* imp;

	imp = psy_ui_impfactory_allocinit_toolframeimp(psy_ui_app_impfactory(psy_ui_app()), self, parent);
	psy_ui_component_init_imp(self, parent, imp);
	psy_ui_component_setbackgroundmode(self, psy_ui_NOBACKGROUND);
}

void psy_ui_frame_init_main(psy_ui_Frame* self)
{
	psy_ui_frame_init(self, NULL);
	psy_ui_app()->main = self;
}
