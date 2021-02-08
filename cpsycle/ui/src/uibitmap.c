// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uibitmap.h"
#include "uiapp.h"
#include "uiimpfactory.h"
#include <stdlib.h>

// VTable Prototypes
static void dispose(psy_ui_Bitmap*);
static int load(psy_ui_Bitmap*, const char* path);
static int loadresource(psy_ui_Bitmap*, int resourceid);
static psy_ui_Size size(const psy_ui_Bitmap*);
static bool empty(const psy_ui_Bitmap*);
static void settransparency(psy_ui_Bitmap*, psy_ui_Colour);

// VTable init
static psy_ui_BitmapVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;		
		vtable.load = load;
		vtable.loadresource = loadresource;
		vtable.size = size;
		vtable.empty = empty;
		vtable.settransparency = settransparency;
		vtable_initialized = TRUE;
	}
}

void psy_ui_bitmap_init(psy_ui_Bitmap* self)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = psy_ui_impfactory_allocinit_bitmapimp(
		psy_ui_app_impfactory(psy_ui_app()),
		psy_ui_realsize_zero());
}

void psy_ui_bitmap_init_size(psy_ui_Bitmap* self, psy_ui_RealSize size)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = psy_ui_impfactory_allocinit_bitmapimp(
		psy_ui_app_impfactory(psy_ui_app()), size);
}

// Delegation Methods to psy_ui_BitmapImp
void dispose(psy_ui_Bitmap* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = NULL;
}

int load(psy_ui_Bitmap* self, const char* path)
{
	return self->imp->vtable->dev_load(self->imp, path);
}

int loadresource(psy_ui_Bitmap* self, int resourceid)
{
	return self->imp->vtable->dev_loadresource(self->imp, resourceid);
}

psy_ui_Size size(const psy_ui_Bitmap* self)
{
	return self->imp->vtable->dev_size(self->imp);
}

bool empty(const psy_ui_Bitmap* self)
{
	return self->imp->vtable->dev_empty(self->imp);
}

void settransparency(psy_ui_Bitmap* self, psy_ui_Colour colour)
{
	self->imp->vtable->dev_settransparency(self->imp, colour);
}

// psy_ui_BitmapImp
static void psy_ui_bitmap_imp_dispose(psy_ui_BitmapImp* self) { }
static int psy_ui_bitmap_imp_load(psy_ui_BitmapImp* self, const char* path)
{
	return 0;
}

static int psy_ui_bitmap_imp_loadresource(psy_ui_BitmapImp* self,
	int resourceid)
{
	return 0;
}

static psy_ui_Size psy_ui_bitmap_imp_size(const psy_ui_BitmapImp* self)
{
	return psy_ui_size_zero();	
}

static bool psy_ui_bitmap_imp_empty(const psy_ui_BitmapImp* self)
{
	return TRUE;
}

static void psy_ui_bitmap_imp_settransparency(psy_ui_BitmapImp* self,
	psy_ui_Colour colour)
{ }

static psy_ui_BitmapImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_bitmap_imp_dispose;
		imp_vtable.dev_load = psy_ui_bitmap_imp_load;
		imp_vtable.dev_loadresource = psy_ui_bitmap_imp_loadresource;
		imp_vtable.dev_size = psy_ui_bitmap_imp_size;
		imp_vtable.dev_empty = psy_ui_bitmap_imp_empty;
		imp_vtable.dev_settransparency = psy_ui_bitmap_imp_settransparency;
		imp_vtable_initialized = TRUE;
	}
}

void psy_ui_bitmap_imp_init(psy_ui_BitmapImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
}
