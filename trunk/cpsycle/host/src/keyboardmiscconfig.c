// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "keyboardmiscconfig.h"

static void keyboardmiscconfig_makekeyboardandmisc(KeyboardMiscConfig*, psy_Property*);

void keyboardmiscconfig_init(KeyboardMiscConfig* self, psy_Property* parent)
{
	assert(self && parent);

	self->parent = parent;
	keyboardmiscconfig_makekeyboardandmisc(self, parent);
	psy_signal_init(&self->signal_changed);
}

void keyboardmiscconfig_dispose(KeyboardMiscConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void keyboardmiscconfig_makekeyboardandmisc(KeyboardMiscConfig* self, psy_Property* parent)
{
	psy_Property* choice;

	assert(self);

	self->keyboard = psy_property_settext(
		psy_property_append_section(parent, "keyboard"),
		"settingsview.keyboard-and-misc");
	psy_property_settext(
		psy_property_append_bool(self->keyboard,
			"playstartwithrctrl", TRUE),
		"Right CTRL = play; Edit Toggle = stop");
	psy_property_settext(
		psy_property_append_bool(self->keyboard,
			"ft2home", TRUE),
		"FT2 Style Home/End Behaviour");
	psy_property_settext(
		psy_property_append_bool(self->keyboard,
			"ft2delete", TRUE),
		"FT2 Style Delete Behaviour");
	psy_property_settext(
		psy_property_append_bool(self->keyboard,
			"effcursoralwayssdown", FALSE),
		"Cursor always moves down in Effect Column");
	psy_property_settext(
		psy_property_append_bool(self->keyboard,
			"recordtweaksastws", 0),
		"settingsview.record-tws");
	psy_property_settext(
		psy_property_append_bool(self->keyboard,
			"advancelineonrecordtweak", 0),
		"settingsview.advance-line-on-record");
	psy_property_settext(
		psy_property_append_bool(self->keyboard,
			"movecursoronestep", 0),
		"Force pattern step 1 when moving with cursors");
	choice = psy_property_settext(
		psy_property_append_choice(self->keyboard,
			"pgupdowntype", 0),
		"Page Up / Page Down step by");
	psy_property_settext(
		psy_property_append_int(choice, "beat",
			0, 0, 0),
		"one beat");
	psy_property_settext(
		psy_property_append_int(choice, "bar",
			0, 0, 0),
		"one bar");
	psy_property_settext(
		psy_property_append_int(choice, "lines",
			0, 0, 0),
		"lines");
	psy_property_settext(
		psy_property_append_int(self->keyboard, "pgupdownstep",
			4, 0, 32),
		"Page Up / Page Down step lines");
	self->keyboard_misc = psy_property_settext(
		psy_property_append_section(self->keyboard, "misc"),
		"Miscellaneous options");
	psy_property_settext(
		psy_property_append_bool(self->keyboard_misc,
			"savereminder", TRUE),
		"\"Save file?\" reminders on Load, New or Exit");
	psy_property_setid(psy_property_settext(
		psy_property_append_int(self->keyboard_misc,
			"numdefaultlines", 64, 1, 1024),
		"Default lines on new pattern"),
		PROPERTY_ID_DEFAULTLINES);
	psy_property_settext(
		psy_property_append_bool(self->keyboard_misc,
			"allowmultiinstances", FALSE),
		"Allow multiple instances of Psycle");
}

bool keyboardmiscconfig_ft2home(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "ft2home", TRUE);
}

bool keyboardmiscconfig_ft2delete(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "ft2delete", TRUE);
}

bool keyboardmiscconfig_effcursoralwaysdown(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "effcursoralwayssdown", FALSE);
}

bool keyboardmiscconfig_playstartwithrctrl(KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "playstartwithrctrl", TRUE);
}

bool keyboardmiscconfig_movecursoronestep(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard, "movecursoronestep", TRUE);
}

bool keyboardmiscconfig_savereminder(const KeyboardMiscConfig* self)
{
	assert(self);

	return  psy_property_at_bool(self->keyboard_misc, "savereminder", TRUE);
}

intptr_t keyboardmiscconfig_patdefaultlines(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_int(self->keyboard_misc, "numdefaultlines", 64);
}

bool keyboardmiscconfig_allowmultipleinstances(const KeyboardMiscConfig* self)
{
	return psy_property_at_bool(self->keyboard_misc, "allowmultiinstances",
		FALSE);
}

bool keyboardmiscconfig_recordtweaksastws(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard,
		"recordtweaksastws", 0);
}

bool keyboardmiscconfig_advancelineonrecordtweak(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_bool(self->keyboard,
		"advancelineonrecordtweak", 0);
}

uintptr_t keyboardmiscconfig_pgupdowntype(const KeyboardMiscConfig* self)
{
	psy_Property* p;

	assert(self);

	p = psy_property_at(self->keyboard, "pgupdowntype", PSY_PROPERTY_TYPE_CHOICE);
	if (p) {
		return (uintptr_t)psy_property_item_int(p);
	}
	return 0;
}

intptr_t keyboardmiscconfig_pgupdownstep(const KeyboardMiscConfig* self)
{
	assert(self);

	return psy_property_at_int(self->keyboard, "pgupdownstep", 4);
}
// events
void keyboardmiscconfig_onchanged(KeyboardMiscConfig* self, psy_Property*
	property)
{
	assert(self);

	psy_signal_emit(&self->signal_changed, self, 1, property);
}

bool keyboardmiscconfig_hasproperty(const KeyboardMiscConfig* self,
	psy_Property* property)
{
	assert(self && self->keyboard);

	return psy_property_insection(property, self->keyboard);
}
