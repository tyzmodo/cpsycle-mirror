// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UILISTBOX_H)
#define UILISTBOX_H

#include "uicomponent.h"

typedef struct {
   ui_component component;
   Signal signal_selchanged;
} ui_listbox;

void ui_listbox_init(ui_listbox*, ui_component* parent);
void ui_listbox_init_multiselect(ui_listbox*, ui_component* parent);
int ui_listbox_addstring(ui_listbox*, const char* text);
void ui_listbox_setstring(ui_listbox*, const char* text, int index);
void ui_listbox_clear(ui_listbox*);
void ui_listbox_setcursel(ui_listbox*, int index);
int ui_listbox_cursel(ui_listbox*);
void ui_listbox_selitems(ui_listbox*, int* items, int maxitems);
int ui_listbox_selcount(ui_listbox*);

#endif