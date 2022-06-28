/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "zoombox.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void zoombox_on_destroyed(ZoomBox*);
static void zoombox_on_zoom_in(ZoomBox*, psy_ui_Component* sender);
static void zoombox_on_zoom_out(ZoomBox*, psy_ui_Component* sender);
static void zoombox_on_mouse_wheel(ZoomBox*, psy_ui_MouseEvent*);
static void zoombox_on_edit_accept(ZoomBox*, psy_ui_TextArea* sender);
static void zoombox_on_edit_reject(ZoomBox*, psy_ui_TextArea* sender);
static void zoombox_update(ZoomBox*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ZoomBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			zoombox_on_destroyed;
		vtable.onmousewheel =
			(psy_ui_fp_component_on_mouse_event)
			zoombox_on_mouse_wheel;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(zoombox_base(self), &vtable);
}

/* implementation */
void zoombox_init(ZoomBox* self, psy_ui_Component* parent)
{
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);	
	vtable_init(self);
	psy_signal_init(&self->signal_changed);
	self->zoomrate = 1.0;
	self->zoomstep = 0.1;
	self->minrate = 0.10;
	self->maxrate = 10.0;
	psy_ui_component_set_style_type(&self->component, STYLE_ZOOMBOX);
	psy_ui_component_set_align_expand(&self->component, psy_ui_HEXPAND);
	psy_ui_component_set_default_align(&self->component,
		psy_ui_ALIGN_LEFT, psy_ui_margin_zero());
	psy_ui_button_init_connect(&self->zoomout, zoombox_base(self),
		self, zoombox_on_zoom_out);
	/* zoom out */
	psy_ui_button_prevent_translation(&self->zoomout);
	psy_ui_button_set_text(&self->zoomout, "-");	
	psy_ui_button_set_char_number(&self->zoomout, 2);
	/* zoom */
	psy_ui_textarea_init_single_line(&self->zoom, zoombox_base(self));	
	psy_ui_component_set_style_type(psy_ui_textarea_base(&self->zoom),
		STYLE_ZOOMBOX_EDIT);
	psy_ui_textarea_set_char_number(&self->zoom, 7);
	psy_ui_textarea_enable_input_field(&self->zoom);	
	psy_signal_connect(&self->zoom.signal_accept, self,
		zoombox_on_edit_accept);
	psy_signal_connect(&self->zoom.signal_reject, self,
		zoombox_on_edit_reject);
	zoombox_update(self);
	/* zoom in */
	psy_ui_button_init_connect(&self->zoomin, zoombox_base(self),
		self, zoombox_on_zoom_in);
	psy_ui_button_prevent_translation(&self->zoomin);
	psy_ui_button_set_text(&self->zoomin, "+");	
	psy_ui_button_set_char_number(&self->zoomin, 2);
}

void zoombox_init_connect(ZoomBox* self, psy_ui_Component* parent,
	void* context, void* fp)
{
	assert(self);

	zoombox_init(self, parent);
	psy_signal_connect(&self->signal_changed, context, fp);
}

void zoombox_on_destroyed(ZoomBox* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void zoombox_on_zoom_in(ZoomBox* self, psy_ui_Component* sender)
{
	assert(self);

	zoombox_set_rate(self,  self->zoomrate + self->zoomstep);
}

void zoombox_on_zoom_out(ZoomBox* self, psy_ui_Component* sender)
{
	assert(self);

	zoombox_set_rate(self, self->zoomrate - self->zoomstep);
}

void zoombox_set_rate(ZoomBox* self, double rate)
{	
	assert(self);
	
	rate = psy_max(self->minrate, rate);
	rate = psy_min(self->maxrate, rate);
	if (rate != self->zoomrate) {
		self->zoomrate = rate;
		zoombox_update(self);
		psy_signal_emit(&self->signal_changed, self, 0, self->zoomrate);		
	}	
}

void zoombox_setstep(ZoomBox* self, double step)
{
	assert(self);

	self->zoomstep = step;
}

void zoombox_on_mouse_wheel(ZoomBox* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	if (psy_ui_mouseevent_delta(ev) != 0) {
		zoombox_set_rate(self, self->zoomrate +
			psy_sgn(psy_ui_mouseevent_delta(ev)) * self->zoomstep);
	}
	psy_ui_mouseevent_prevent_default(ev);
	psy_ui_mouseevent_stop_propagation(ev);
}

void zoombox_on_edit_accept(ZoomBox* self, psy_ui_TextArea* sender)
{
	const char* text;		

	assert(self);

	text = psy_ui_textarea_text(&self->zoom);
	if (text) {				
		char temp[64];
		char* p;
		int rate;

		psy_snprintf(temp, 64, "%s", text);
		if (p = strchr(temp, '%')) {
			*p = '\0';
		}
		rate = atoi(temp);
		zoombox_set_rate(self,  psy_min(self->maxrate, psy_max(self->minrate,
			(double)(rate / 100.0))));
		psy_ui_component_set_focus(&self->component);
	}
}

void zoombox_on_edit_reject(ZoomBox* self, psy_ui_TextArea* sender)
{	
	assert(self);

	psy_ui_component_set_focus(&self->component);
}

void zoombox_update(ZoomBox* self)
{
	char text[64];

	assert(self);

	psy_snprintf(text, 64, "%d%%", (int)(self->zoomrate * 100 + 0.5));
	psy_ui_textarea_set_text(&self->zoom, text);
}
