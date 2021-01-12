// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uievents.h"

void psy_ui_keyevent_init(psy_ui_KeyEvent* self, uint32_t keycode, intptr_t keydata,
	bool shift, bool ctrl, bool alt, int repeat)
{
	self->keycode = keycode;
	self->keydata = keydata;
	self->shift = shift;
	self->ctrl = ctrl;
	self->alt = alt;
	self->repeat = repeat;
	self->bubble = 1;
	self->preventdefault = FALSE;
	self->target = NULL;
}

void psy_ui_keyevent_stoppropagation(psy_ui_KeyEvent* self)
{
	self->bubble = 0;
}

void psy_ui_keyevent_preventdefault(psy_ui_KeyEvent* self)
{
	self->preventdefault = TRUE;
}

void psy_ui_mouseevent_init(psy_ui_MouseEvent* self, double x, double y, intptr_t button,
	intptr_t delta, bool shift, bool ctrl)
{
	self->x = x;
	self->y = y;
	self->button = button;
	self->delta = delta;
	self->bubble = 1;
	self->shift = shift;
	self->ctrl = ctrl;
	self->preventdefault = 0;
	self->target = 0;
}

void psy_ui_mouseevent_stoppropagation(psy_ui_MouseEvent* self)
{
	self->bubble = 0;
}

struct psy_ui_Component* psy_ui_mouseevent_target(psy_ui_MouseEvent* self)
{
	return self->target;
}

void psy_ui_mouseevent_settarget(psy_ui_MouseEvent* self, struct psy_ui_Component* target)
{
	self->target = target;
}