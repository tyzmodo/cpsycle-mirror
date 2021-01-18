// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiwinbitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32

#include "../../uiapp.h"
#include "uiwinapp.h"

// VTable Prototypes
static void dev_dispose(psy_ui_win_BitmapImp*);
static int dev_load(psy_ui_win_BitmapImp*, const char* path);
static int dev_loadresource(psy_ui_win_BitmapImp*, int resourceid);
static psy_ui_Size dev_size(const psy_ui_win_BitmapImp*);
static bool dev_empty(const psy_ui_win_BitmapImp*);

// VTable init
static psy_ui_BitmapImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_win_BitmapImp* self)
{
	assert(self);

	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose = (psy_ui_bitmap_imp_fp_dispose)dev_dispose;
		imp_vtable.dev_load = (psy_ui_bitmap_imp_fp_load)dev_load;
		imp_vtable.dev_loadresource = (psy_ui_bitmap_imp_fp_loadresource)
			dev_loadresource;
		imp_vtable.dev_size = (psy_ui_bitmap_imp_fp_size)dev_size;
		imp_vtable.dev_empty = (psy_ui_bitmap_imp_fp_empty)dev_empty;
		imp_vtable_initialized = TRUE;
	}
}

void psy_ui_win_bitmapimp_init(psy_ui_win_BitmapImp* self, psy_ui_RealSize size)
{
	assert(self);

	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	if (size.width == 0 && size.height == 0) {
		self->bitmap = 0;
	} else {
		HDC hdc;

		hdc = GetDC(NULL);
		SaveDC(hdc);
		self->bitmap = CreateCompatibleBitmap(hdc,
			(int)size.width, (int)size.height);
		RestoreDC(hdc, -1);
		ReleaseDC(NULL, hdc);
	}	
}

void dev_dispose(psy_ui_win_BitmapImp* self)
{
	assert(self);

	if (self->bitmap) {
		DeleteObject(self->bitmap);
		self->bitmap = 0;
	}
}

int dev_load(psy_ui_win_BitmapImp* self, const char* path)
{
	HBITMAP bitmap;

	assert(self);

	bitmap = (HBITMAP)LoadImage(NULL,
		(LPCTSTR)path,
		IMAGE_BITMAP,
		0, 0,
		LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (bitmap != NULL) {
		dev_dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;
}

int dev_loadresource(psy_ui_win_BitmapImp* self, int resourceid)
{
	HBITMAP bitmap;
	psy_ui_WinApp* winapp;

	assert(self);

	winapp = (psy_ui_WinApp*)psy_ui_app()->imp;
	bitmap = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	if (bitmap != NULL) {
		dev_dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;
}

psy_ui_Size dev_size(const psy_ui_win_BitmapImp* self)
{
	assert(self);

	if (self->bitmap) {
		BITMAP bitmap;

		GetObject(self->bitmap, sizeof(BITMAP), &bitmap);
		return psy_ui_size_makepx(bitmap.bmWidth, bitmap.bmHeight);
	}
	return psy_ui_size_zero();	
}

bool dev_empty(const psy_ui_win_BitmapImp* self)
{
	assert(self);

	return self->bitmap == 0;
}

#endif
