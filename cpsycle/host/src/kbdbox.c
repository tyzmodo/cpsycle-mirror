// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "kbdbox.h"

#include <stdlib.h>
#include <string.h>

static void kbdbox_makekeyproperties(KbdBox*);
static void kbdbox_addkeyproperties(KbdBox*, psy_Properties* section,
	uintptr_t keycode, const char* label, int size, int cr);

void kbdboxkey_init_all(KbdBoxKey* self, int x, int y, int width, int height, const char* label)
{
	psy_ui_setrectangle(&self->position, x, y, width, height);
	self->label = strdup(label);
	self->desc0 = strdup("");
	self->desc1 = strdup("");
	self->desc2 = strdup("");
	self->color = 0x00666666;
}

void kbdboxkey_dispose(KbdBoxKey* self)
{
	free(self->label);
	free(self->desc0);
	free(self->desc1);
	free(self->desc2);
}

KbdBoxKey* kbdboxkey_allocinit_all(int x, int y, int width, int height, const char* label)
{
	KbdBoxKey* rv;

	rv = (KbdBoxKey*)malloc(sizeof(KbdBoxKey));
	if (rv) {
		kbdboxkey_init_all(rv, x, y, width, height, label);
	}
	return rv;
}

static void kbdbox_makekeys(KbdBox*);
static void kbdbox_ondestroy(KbdBox*, psy_ui_Component* sender);
static void kbdbox_ondraw(KbdBox*, psy_ui_Graphics*);
static void kbdbox_drawkey(KbdBox*, psy_ui_Graphics*, KbdBoxKey*);
static void kbdbox_addsmallkey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addmediumkey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addlargerkey(KbdBox*, uintptr_t keycode, const char* label);
static void kbdbox_addlargekey(KbdBox*, uintptr_t keycode, const char* label);

static psy_ui_ComponentVtable kbdbox_vtable;
static int kbdbox_vtable_initialized = 0;

static void kbdbox_vtable_init(KbdBox* self)
{
	if (!kbdbox_vtable_initialized) {
		kbdbox_vtable = *(self->component.vtable);
		kbdbox_vtable.ondraw = (psy_ui_fp_ondraw)kbdbox_ondraw;
		kbdbox_vtable_initialized = 1;
	}
}

void kbdbox_init(KbdBox* self, psy_ui_Component* parent)
{			
	psy_ui_component_init(kbdbox_base(self), parent);
	kbdbox_vtable_init(self);
	self->component.vtable = &kbdbox_vtable;
	psy_ui_component_doublebuffer(kbdbox_base(self));
	psy_signal_connect(&kbdbox_base(self)->signal_destroy, self,
		kbdbox_ondestroy);	
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(80),
			psy_ui_value_makeeh(14)));
	self->corner.width = psy_ui_value_makepx(5);
	self->corner.height = psy_ui_value_makepx(5);
	self->ident = 3;
	self->keyheight = (9 + self->ident) * 3 + 2;
	self->keywidth = 115;
	psy_table_init(&self->keys);
	self->keyset = NULL;
	kbdbox_makekeyproperties(self);
	kbdbox_makekeys(self);
}

void kbdbox_ondestroy(KbdBox* self, psy_ui_Component* sender)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->keys);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		KbdBoxKey* key;

		key = (KbdBoxKey*)psy_tableiterator_value(&it);
		kbdboxkey_dispose(key);
		free(key);
	}
	psy_table_dispose(&self->keys);
	properties_free(self->keyset);
}

void kbdbox_ondraw(KbdBox* self, psy_ui_Graphics* g)
{	
	psy_TableIterator it;

	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);	
	for (it = psy_table_begin(&self->keys);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		KbdBoxKey* key;

		key = (KbdBoxKey*)psy_tableiterator_value(&it);
		kbdbox_drawkey(self, g, key);
	}
}

void kbdbox_drawkey(KbdBox* self, psy_ui_Graphics* g, KbdBoxKey* key)
{
	int centerx;
	psy_ui_Size textsize;
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);	
	textsize = psy_ui_component_textsize(kbdbox_base(self), key->label);
	centerx = (key->position.right - key->position.left -
		psy_ui_value_px(&textsize.width, &tm)) / 2;	
	psy_ui_setcolor(g, key->color);
	psy_ui_drawroundrectangle(g, key->position, self->corner);
	psy_ui_settextcolor(g, 0x00666666);
	psy_ui_textout(g, key->position.left + centerx, key->position.top + 1,
		key->label, strlen(key->label));
	psy_ui_settextcolor(g, key->color);
	psy_ui_textout(g, key->position.left + 4, key->position.top -3 +
		psy_ui_value_px(&textsize.height, &tm),
		key->desc0, strlen(key->desc0));
	psy_ui_textout(g, key->position.left + 4 + 40, key->position.top - 3 +
		psy_ui_value_px(&textsize.height, &tm),
		key->desc1, strlen(key->desc1));
	psy_ui_textout(g, key->position.left + 4 + 80, key->position.top - 3 +
		psy_ui_value_px(&textsize.height, &tm),
		key->desc2, strlen(key->desc2));
}

void kbdbox_makekeys(KbdBox* self)
{
	if (self->keyset) {
		psy_Properties* p;

		self->cpx = 0;
		self->cpy = 0;

		p = psy_properties_find(self->keyset, "main");
		if (p && p->children) {
			int currrow = 0;
			for (p = p->children; p != NULL; p = p->next) {				
				int keycode;

				keycode = psy_properties_int(p, "keycode", -1);
				if (keycode != -1) {
					const char* label;
					int size;
					int cr;

					label = psy_properties_readstring(p, "label", "");
					size = psy_properties_int(p, "size", 0);
					cr = psy_properties_int(p, "cr", 0);

					if (cr) {
						self->cpy += self->keyheight + self->ident;
						self->cpx = 0;
					}
					if (size == 0) {
						kbdbox_addsmallkey(self, keycode, label);
					} else
					if (size == 1) {
						kbdbox_addmediumkey(self, keycode, label);
					} else
					if (size == 2) {
						kbdbox_addlargerkey(self, keycode, label);
					} else
					if (size == 3) {
						kbdbox_addlargekey(self, keycode, label);
					}
				}
				
			}
		}
	}
}
	
void kbdbox_makekeyproperties(KbdBox* self)
{
	psy_Properties* main;
	int kc;
	int col = 0;

	self->keyset = psy_properties_create();
	main = psy_properties_create_section(self->keyset, "main");
		
	kc = 0x1000;
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_ESCAPE, "ESC", 1, 0);
	kbdbox_addkeyproperties(self, main, kc++, "", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F1, "F1", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F2, "F2", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F3, "F3", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F4, "F4", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F5, "F5", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F6, "F6", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F7, "F7", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F8, "F8", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F9, "F9", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F10, "F10", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F11, "F11", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F12, "F12", 0, 0);
	
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACKQUOTE, "`", 0, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT1, "1", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT2, "2", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT3, "3", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT4, "4", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT5, "5", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT6, "6", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT7, "7", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT8, "8", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT9, "9", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_DIGIT0, "0", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_MINUS, "-", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_EQUAL, "=", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACK, "BACK", 0, 0);
	
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_TAB, "TAB", 1, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_Q, "Q", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_W, "W", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_E, "E", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_R, "R", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_T, "T", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_Y, "Y", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_U, "U", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_I, "I", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_O, "O", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_P, "P", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BRACKETLEFT, "[", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BRACKETRIGHT, "]", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACKSLASH, "\\", 0, 0);
	
	kbdbox_addkeyproperties(self, main, kc++, "CAPS", 2, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_A, "A", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_S, "S", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_D, "D", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_F, "F", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_G, "G", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_H, "H", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_J, "J", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_K, "K", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_L, "L", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SEMICOLON, ";", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_BACKSLASH, "\"", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_QUOTE, "'", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_RETURN, "ENTER", 2, 0);
	
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SHIFT, "SHIFT", 1, 1);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_Z, "Z", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_X, "X", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_C, "C", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_V, "V", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_B, "B", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_N, "N", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_M, "M", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_COMMA, ",", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_PERIOD, ".", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SLASH, "/", 0, 0);
	kbdbox_addkeyproperties(self, main, kc++, "SHIFT", 1, 0);
	
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_CONTROL, "CTRL", 2, 1);
	kbdbox_addkeyproperties(self, main, kc++, "", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_MENU, "ALT", 0, 0);
	kbdbox_addkeyproperties(self, main, psy_ui_KEY_SPACE, "Psycle", 3, 0);
	kbdbox_addkeyproperties(self, main, kc++, "ALT", 1, 0);
	kbdbox_addkeyproperties(self, main, kc++, "", 0, 0);
	kbdbox_addkeyproperties(self, main, kc++, "CTRL", 0, 0);
}

void kbdbox_addkeyproperties(KbdBox* self, psy_Properties* section,
	uintptr_t keycode, const char* label, int size, int cr)
{
	psy_Properties* key;

	key = psy_properties_create_section(section, "key");
	psy_properties_append_int(key, "keycode", keycode, 0, 0);	
	psy_properties_append_string(key, "label", label);
	psy_properties_append_int(key, "size", size, 0, 0);
	psy_properties_append_int(key, "cr", cr, 0, 0);
}

void kbdbox_addsmallkey(KbdBox* self, uintptr_t keycode, const char* label)
{
	KbdBoxKey* key;

	key = kbdboxkey_allocinit_all(self->cpx, self->cpy, self->keywidth, self->keyheight, label);
	psy_table_insert(&self->keys, keycode, key);
	self->cpx += self->keywidth + self->ident;
}

void kbdbox_addmediumkey(KbdBox* self, uintptr_t keycode, const char* label)
{
	KbdBoxKey* key;

	key = kbdboxkey_allocinit_all(self->cpx, self->cpy, 
		(int)(self->keywidth * 1.3), self->keyheight, label);
	psy_table_insert(&self->keys, keycode, key);
	self->cpx += (int)(self->keywidth * 1.3) + self->ident;
}

void kbdbox_addlargerkey(KbdBox* self, uintptr_t keycode, const char* label)
{
	KbdBoxKey* key;

	key = kbdboxkey_allocinit_all(self->cpx, self->cpy,
		(int)(self->keywidth * 1.5), self->keyheight, label);
	psy_table_insert(&self->keys, keycode, key);
	self->cpx += (int)(self->keywidth * 1.5 + self->ident);
}

void kbdbox_addlargekey(KbdBox* self, uintptr_t keycode, const char* label)
{
	KbdBoxKey* key;

	key = kbdboxkey_allocinit_all(self->cpx, self->cpy,
		self->keywidth * 6, self->keyheight, label);
	psy_table_insert(&self->keys, keycode, key);
	self->cpx += self->keywidth * 6 + self->ident;
}

void kbdbox_setcolor(KbdBox* self, uintptr_t keycode, psy_ui_Color color)
{
	KbdBoxKey* key;

	key = psy_table_at(&self->keys, keycode);
	if (key) {
		key->color = color;
		psy_ui_component_invalidate(kbdbox_base(self));
	}
}

void kbdbox_setdescription(KbdBox* self, uintptr_t keycode, int shift, int ctrl,
	const char* text)
{
	KbdBoxKey* key;

	key = psy_table_at(&self->keys, keycode);
	if (key) {
		uintptr_t maxchars;
		uintptr_t numchars;
		char** desc;

		if (shift) {
			desc = &key->desc1;
		} else 
		if (ctrl) {
			desc = &key->desc2;
		} else {
			desc = &key->desc0;
		}		
		free(*desc);
		maxchars = 12;
		*desc = strdup(text);
		numchars = min(strlen(*desc), maxchars);
		if (numchars < strlen(*desc)) {
			*desc = strdup(*desc + (strlen(*desc) - maxchars));
		} else {
			*desc = strdup(*desc);
		}
		psy_ui_component_invalidate(kbdbox_base(self));
	}
}
