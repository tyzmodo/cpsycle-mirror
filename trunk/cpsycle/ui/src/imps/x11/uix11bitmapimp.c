// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11bitmapimp.h"

#if PSYCLE_USE_TK == PSYCLE_TK_X11

#include "uiapp.h"
#include "uigraphics.h"
#include "uibmpreader.h"
#include "uix11app.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xdbe.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../../detail/portable.h"


/* prototypes */
static void dispose(psy_ui_x11_BitmapImp*);
static int load(psy_ui_x11_BitmapImp*, struct psy_ui_Bitmap* bitmap, const char* path);
static int loadresource(psy_ui_x11_BitmapImp*, int resourceid);
static psy_ui_RealSize dev_size(psy_ui_x11_BitmapImp*);
static int empty(psy_ui_x11_BitmapImp*);
static void dev_settransparency(psy_ui_x11_BitmapImp*, psy_ui_Colour colour);
static void dev_preparemask(psy_ui_x11_BitmapImp*, psy_ui_Colour clrtrans);

/* vtable */
static psy_ui_BitmapImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_x11_BitmapImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable = *self->imp.vtable;
		imp_vtable.dev_dispose =
			(psy_ui_bitmap_imp_fp_dispose)
			dispose;
		imp_vtable.dev_load =
			(psy_ui_bitmap_imp_fp_load)
			load;
		imp_vtable.dev_loadresource =
			(psy_ui_bitmap_imp_fp_loadresource)
			loadresource;
		imp_vtable.dev_size =
			(psy_ui_bitmap_imp_fp_size)
			dev_size;
		imp_vtable.dev_empty =
			(psy_ui_bitmap_imp_fp_empty)
			empty;
		imp_vtable.dev_settransparency =
			(psy_ui_bitmap_imp_fp_settransparency)
			dev_settransparency;
		imp_vtable_initialized = TRUE;
	}
}

/* implementation */
void psy_ui_x11_bitmapimp_init(psy_ui_x11_BitmapImp* self,
	psy_ui_RealSize size)
{
	psy_ui_bitmap_imp_init(&self->imp);
	imp_vtable_init(self);
	self->imp.vtable = &imp_vtable;
	self->mask = 0;	
	if (size.width == 0 && size.height == 0) {
		self->pixmap = 0;
	} else {
		psy_ui_X11App* x11app;
		int screen;
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		screen = DefaultScreen(x11app->dpy);		
		self->pixmap = XCreatePixmap(x11app->dpy,
			DefaultRootWindow(x11app->dpy), (int)size.width, (int)size.height,
			DefaultDepth(x11app->dpy, screen));		
	}
}

void dispose(psy_ui_x11_BitmapImp* self)
{
	if (self->pixmap) {
		psy_ui_X11App* x11app;		
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XFreePixmap(x11app->dpy, self->pixmap);
		self->pixmap = 0;
	}
	if (self->mask) {
		psy_ui_X11App* x11app;		
				
		x11app = (psy_ui_X11App*)psy_ui_app()->imp;
		XFreePixmap(x11app->dpy, self->mask);
		self->mask = 0;
	}
}

int load(psy_ui_x11_BitmapImp* self, struct psy_ui_Bitmap* bitmap, const char* path)
{	
	int rv;
	
	assert(path);

	psy_ui_BmpReader bmpreader;

	psy_ui_bmpreader_init(&bmpreader, bitmap);
	rv = psy_ui_bmpreader_load(&bmpreader, path);
	psy_ui_bmpreader_dispose(&bmpreader);
	return rv;	
}

int loadresource(psy_ui_x11_BitmapImp* self, int resourceid)
{
	/*HBITMAP bitmap;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	bitmap = LoadBitmap(winapp->instance, MAKEINTRESOURCE(resourceid));
	if (bitmap != NULL) {
		dispose(self);
		self->bitmap = bitmap;
	}
	return bitmap == 0;*/
	return 0;
}

psy_ui_RealSize dev_size(psy_ui_x11_BitmapImp* self)
{
	psy_ui_RealSize size;    

	if (self->pixmap) {
        Window root;
        unsigned int temp;
        unsigned int width = 0;
        unsigned int height = 0;
        psy_ui_X11App* xtapp;		

        xtapp = (psy_ui_X11App*)psy_ui_app()->imp;
        XGetGeometry(xtapp->dpy, self->pixmap, &root, &temp, &temp,
            &width, &height, &temp, &temp);
		size.width = width;
		size.height = height;
	} else {
		size.width = 0.0;
		size.height = 0.0;
	}
	return size;
}

int empty(psy_ui_x11_BitmapImp* self)
{
	return self->pixmap == 0;
}

void dev_settransparency(psy_ui_x11_BitmapImp* self, psy_ui_Colour colour)
{
	dev_preparemask(self, colour);
}

void dev_preparemask(psy_ui_x11_BitmapImp* self, psy_ui_Colour clrtrans)
{	
	
}	

#endif