// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sampleeditor.h"

#include <operations.h>
#include <alignedalloc.h>
#include <exclusivelock.h>

#include <math.h>
#include <string.h>

#include <songio.h>

#include "../../detail/portable.h"

void sampleeditorbar_init(SampleEditorBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_checkbox_init(&self->selecttogether, &self->component);
	psy_ui_checkbox_settext(&self->selecttogether, "Select Channels Together");
	psy_ui_checkbox_check(&self->selecttogether);
	psy_ui_label_init(&self->selstartlabel, &self->component);
	psy_ui_label_settext(&self->selstartlabel, "Selection Start");
	psy_ui_edit_init(&self->selstartedit, &self->component);
	psy_ui_edit_setcharnumber(&self->selstartedit, 10);			
	psy_ui_label_init(&self->selendlabel, &self->component);
	psy_ui_label_settext(&self->selendlabel, "Selection End");
	psy_ui_edit_init(&self->selendedit, &self->component);
	psy_ui_edit_setcharnumber(&self->selendedit, 10);
	sampleeditorbar_clearselection(self);
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT,
		&margin));
}

void sampleeditorbar_setselection(SampleEditorBar* self, uintptr_t selectionstart,
	uintptr_t selectionend)
{
	char text[128];

	psy_ui_edit_enableedit(&self->selstartedit);
	psy_ui_edit_enableedit(&self->selendedit);
	psy_snprintf(text, 128, "%u", (unsigned int)selectionstart);
	psy_ui_edit_settext(&self->selstartedit, text);
	psy_snprintf(text, 128, "%u", (unsigned int)selectionend);
	psy_ui_edit_settext(&self->selendedit, text);
}

void sampleeditorbar_clearselection(SampleEditorBar* self)
{
	psy_ui_edit_preventedit(&self->selstartedit);
	psy_ui_edit_preventedit(&self->selendedit);
	psy_ui_edit_settext(&self->selstartedit, "");
	psy_ui_edit_settext(&self->selendedit, "");
}

static void sampleeditoroperations_updatetext(SampleEditorOperations*,
	Workspace*);
static void sampleeditoroperations_initalign(SampleEditorOperations*);
static void sampleeditorbar_onlanguagechanged(SampleEditorOperations*, Workspace*);

void sampleeditoroperations_init(SampleEditorOperations* self,
	psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_button_init(&self->cut, &self->component);
	psy_ui_button_init(&self->crop, &self->component);
	psy_ui_button_init(&self->copy, &self->component);
	psy_ui_button_init(&self->paste, &self->component);
	psy_ui_button_init(&self->del, &self->component);
	sampleeditoroperations_initalign(self);
	sampleeditoroperations_updatetext(self, workspace);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		sampleeditorbar_onlanguagechanged);
}

void sampleeditoroperations_initalign(SampleEditorOperations* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.5));
	psy_ui_component_enablealign(&self->component);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT, &margin));
}

void sampleeditoroperations_updatetext(SampleEditorOperations* self,
	Workspace* workspace)
{
	psy_ui_button_settext(&self->cut, workspace_translate(workspace,
		"Cut"));
	psy_ui_button_settext(&self->crop, workspace_translate(workspace,
		"Crop"));
	psy_ui_button_settext(&self->copy, workspace_translate(workspace,
		"Copy"));
	psy_ui_button_settext(&self->paste, workspace_translate(workspace,
		"Paste"));
	psy_ui_button_settext(&self->del, workspace_translate(workspace,
		"Delete"));
}

void sampleeditorbar_onlanguagechanged(SampleEditorOperations* self,
	Workspace* workspace)
{
	sampleeditoroperations_updatetext(self, workspace);
	psy_ui_component_align(&self->component);
}

static void sampleeditoramplify_ontweak(SampleEditorAmplify*, psy_ui_Slider*, float value);
static void sampleeditoramplify_onvalue(SampleEditorAmplify*, psy_ui_Slider*, float* value);
static void sampleeditoramplify_ondescribe(SampleEditorAmplify*, psy_ui_Slider*, char* text);

void sampleeditoramplify_init(SampleEditorAmplify* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	char text[128];

	self->workspace = workspace;
	self->gainvalue = (psy_dsp_amp_t) 2/3.f;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_enablealign(&self->component);
	psy_ui_label_init(&self->header, &self->component);
	psy_ui_label_settext(&self->header, "Adjust Volume");
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_slider_init(&self->gain, &self->component);
	psy_ui_slider_showvertical(&self->gain);
	psy_ui_slider_setcharnumber(&self->gain, 4);
	psy_ui_slider_connect(&self->gain, self, sampleeditoramplify_ondescribe,
		sampleeditoramplify_ontweak, sampleeditoramplify_onvalue);
	psy_ui_component_setalign(&self->gain.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->dbdisplay, &self->component);
	sampleeditoramplify_ondescribe(self, 0, text);
	psy_ui_label_settext(&self->dbdisplay, text);
	psy_ui_component_setalign(&self->dbdisplay.component, psy_ui_ALIGN_BOTTOM);
}

void sampleeditoramplify_ontweak(SampleEditorAmplify* self, psy_ui_Slider* slider,
	float value)
{
	char text[128];

	self->gainvalue = (int)(value * 288) / 288.f;

	sampleeditoramplify_ondescribe(self, 0, text);
	psy_ui_label_settext(&self->dbdisplay, text);
}

void sampleeditoramplify_onvalue(SampleEditorAmplify* self, psy_ui_Slider* slider,
	float* value)
{
		*value = self->gainvalue;
}

void sampleeditoramplify_ondescribe(SampleEditorAmplify* self, psy_ui_Slider* slider,
	char* text)
{			
	if (self->gainvalue == 0.f) {
		psy_snprintf(text, 10, "-inf. dB");
	} else {
		float db = (self->gainvalue - 2/3.f) * 144.f;
		psy_snprintf(text, 10, "%.2f dB", db);
	}	
}


static void sampleprocessview_updatetext(SampleEditorProcessView*,
	Workspace*);
static void sampleprocessview_onlanguagechanged(SampleEditorProcessView*,
	Workspace* workspace);
static void sampleprocessview_buildprocessorlist(SampleEditorProcessView*);
static void sampleeditorprocessview_onpreferredsize(SampleEditorProcessView*, psy_ui_Size* limit,
	psy_ui_Size* rv);
static void sampleeditorprocessview_onprocessorselected(SampleEditorProcessView*,
	psy_ui_Component* sender, int index);

static psy_ui_ComponentVtable sampleeditorprocess_vtable;
static int sampleeditorprocess_vtable_initialized = 0;

static void sampleeditorprocess_vtable_init(SampleEditorProcessView* self)
{
	if (!sampleeditorprocess_vtable_initialized) {
		sampleeditorprocess_vtable = *(self->component.vtable);
		sampleeditorprocess_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			sampleeditorprocessview_onpreferredsize;
		sampleeditorprocess_vtable_initialized = 1;
	}
}

void sampleprocessview_init(SampleEditorProcessView* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Margin margin;

	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	sampleeditoroperations_init(&self->copypaste, &self->component, workspace);
	psy_ui_component_setalign(&self->copypaste.component, psy_ui_ALIGN_TOP);
	psy_ui_button_init(&self->process, &self->component);	
	psy_ui_component_setalign(&self->process.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init(&margin, psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.5),
		psy_ui_value_makepx(0));
	sampleeditorprocess_vtable_init(self);
	self->component.vtable = &sampleeditorprocess_vtable;
	psy_ui_component_setmargin(&self->process.component, &margin);
	psy_ui_listbox_init(&self->processors, &self->component);
	psy_ui_component_setalign(&self->processors.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->processors.component, &margin);
	psy_ui_notebook_init(&self->notebook, &self->component);
	psy_ui_component_setalign(&self->notebook.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_enablealign(&self->notebook.component);
	sampleeditoramplify_init(&self->amplify, &self->notebook.component, workspace);	
	sampleprocessview_buildprocessorlist(self);
	psy_ui_component_init(&self->emptypage1,
		psy_ui_notebook_base(&self->notebook));
	psy_ui_component_init(&self->emptypage2,
		psy_ui_notebook_base(&self->notebook));
	psy_ui_component_init(&self->emptypage3,
		psy_ui_notebook_base(&self->notebook));
	psy_ui_component_init(&self->emptypage4,
		psy_ui_notebook_base(&self->notebook));
	psy_ui_component_init(&self->emptypage5,
		psy_ui_notebook_base(&self->notebook));
	psy_ui_component_init(&self->emptypage6,
		psy_ui_notebook_base(&self->notebook));
	psy_ui_listbox_setcursel(&self->processors, 0);
	psy_ui_notebook_setpageindex(&self->notebook, 0);
	sampleprocessview_updatetext(self, workspace);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		sampleprocessview_onlanguagechanged);
	psy_signal_connect(&self->processors.signal_selchanged, self,
		sampleeditorprocessview_onprocessorselected);
}

void sampleprocessview_updatetext(SampleEditorProcessView* self,
	Workspace* workspace)
{
	psy_ui_button_settext(&self->process, workspace_translate(workspace,
		"Process"));
}

void sampleprocessview_onlanguagechanged(SampleEditorProcessView* self,
	Workspace* workspace)
{
	sampleprocessview_updatetext(self, workspace);
	psy_ui_component_align(&self->component);
}

void sampleprocessview_buildprocessorlist(SampleEditorProcessView* self)
{
	psy_ui_listbox_clear(&self->processors);
	psy_ui_listbox_addtext(&self->processors, "Amplify");
	psy_ui_listbox_addtext(&self->processors, "Fade In");
	psy_ui_listbox_addtext(&self->processors, "Fade Out");
	psy_ui_listbox_addtext(&self->processors, "Insert Silence");
	psy_ui_listbox_addtext(&self->processors, "Normalize");
	psy_ui_listbox_addtext(&self->processors, "Remove DC");
	psy_ui_listbox_addtext(&self->processors, "Reverse");
}

void sampleeditorprocessview_onprocessorselected(SampleEditorProcessView* self,
	psy_ui_Component* sender, int index)
{
	psy_ui_notebook_setpageindex(&self->notebook, index);
}

void sampleeditorprocessview_onpreferredsize(SampleEditorProcessView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	if (rv) {
		rv->width = 200;
		rv->height = 60;
	}
}

static void sampleeditorplaybar_initalign(SampleEditorPlayBar*);
static void sampleeditorheader_init(SampleEditorHeader*, psy_ui_Component* parent,
	SampleEditor*);
static void sampleeditorheader_ondraw(SampleEditorHeader*, psy_ui_Graphics*);
static void sampleeditorheader_onpreferredsize(SampleEditorHeader*,
	psy_ui_Size* limit, psy_ui_Size* size);
static void sampleeditorheader_drawruler(SampleEditorHeader*, psy_ui_Graphics*);
static void sampleeditor_initsampler(SampleEditor*);
static void sampleeditor_ondestroy(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_clearwaveboxes(SampleEditor*);
static void sampleeditor_buildwaveboxes(SampleEditor*, psy_audio_Sample*);
static void sampleeditor_onsize(SampleEditor*, psy_ui_Component* sender, psy_ui_Size*);
static void sampleeditor_onzoom(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_setsampleboxzoom(SampleEditor*);
static void sampleeditor_computemetrics(SampleEditor*, SampleEditorMetrics* rv);
static void sampleeditor_onsongchanged(SampleEditor*, Workspace* workspace, int flag, psy_audio_SongFile*);
static void sampleeditor_connectmachinessignals(SampleEditor*, Workspace*);
static void sampleeditor_onplay(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onstop(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onprocess(SampleEditor*, psy_ui_Component* sender);
static void sampleeditor_onmasterworked(SampleEditor*, psy_audio_Machine*,
	uintptr_t slot, psy_audio_BufferContext*);
static void sampleeditor_onselectionchanged(SampleEditor*, WaveBox*);
static void sampleeditor_onscrollzoom_customdraw(SampleEditor*,
	ScrollZoom* sender, psy_ui_Graphics*);
void sampleeditor_onlanguagechanged(SampleEditor*,
	Workspace* workspace);
void sampleeditor_amplify(SampleEditor*, uintptr_t framestart, uintptr_t frameend,
	psy_dsp_amp_t gain);
void sampleeditor_fade(SampleEditor*, uintptr_t framestart, uintptr_t frameend, psy_dsp_amp_t startvol,
	psy_dsp_amp_t endvol);
void sampleeditor_removeDC(SampleEditor*, uintptr_t framestart, uintptr_t frameend);
void sampleeditor_normalize(SampleEditor*, uintptr_t framestart, uintptr_t frameend);
void sampleeditor_reverse(SampleEditor*, uintptr_t framestart, uintptr_t frameend);

enum {
	SAMPLEEDITOR_DRAG_NONE,
	SAMPLEEDITOR_DRAG_LEFT,
	SAMPLEEDITOR_DRAG_RIGHT,
	SAMPLEEDITOR_DRAG_MOVE
};

void sampleeditorplaybar_init(SampleEditorPlayBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	// psy_ui_button_init(&self->loop, &self->component);
	// psy_ui_button_settext(&self->loop, "Loop");	
	// psy_signal_connect(&self->loop.signal_clicked, self, onloopclicked);	
	psy_ui_button_init(&self->play, &self->component);
	psy_ui_button_settext(&self->play, workspace_translate(workspace, "play"));	
	psy_ui_button_init(&self->stop, &self->component);
	psy_ui_button_settext(&self->stop, workspace_translate(workspace, "stop"));
	// psy_signal_connect(&self->stop.signal_clicked, self, onstopclicked);	
	sampleeditorplaybar_initalign(self);	
}

void sampleeditorplaybar_initalign(SampleEditorPlayBar* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT, &margin));
}

// Header
// pianoroll vtable
static psy_ui_ComponentVtable sampleeditorheader_vtable;
static int sampleeditorheader_vtable_initialized = 0;

static void sampleeditorheader_vtable_init(SampleEditorHeader* self)
{
	if (!sampleeditorheader_vtable_initialized) {
		sampleeditorheader_vtable = *(self->component.vtable);
		sampleeditorheader_vtable.ondraw = (psy_ui_fp_ondraw)
			sampleeditorheader_ondraw;
		sampleeditorheader_vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			sampleeditorheader_onpreferredsize;
		sampleeditorheader_vtable_initialized = 1;
	}
}

void sampleeditorheader_init(SampleEditorHeader* self,
	psy_ui_Component* parent, SampleEditor* view)
{
	self->view = view;
	self->scrollpos = 0;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	sampleeditorheader_vtable_init(self);
	self->component.vtable = &sampleeditorheader_vtable;
	psy_ui_component_doublebuffer(&self->component);	
}

void sampleeditorheader_ondraw(SampleEditorHeader* self, psy_ui_Graphics* g)
{	
	sampleeditorheader_drawruler(self, g);	
}

void sampleeditorheader_onpreferredsize(SampleEditorHeader* self,
	psy_ui_Size* limit, psy_ui_Size* rv)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	rv->width = limit->width;
	rv->height = (int)(tm.tmHeight * 1.5);
}

void sampleeditorheader_drawruler(SampleEditorHeader* self, psy_ui_Graphics* g)
{
	psy_ui_Size size;	
	int baseline;		
	psy_dsp_beat_t cpx;	
	int c;
	psy_ui_TextMetric tm;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	baseline = size.height - 1;
	psy_ui_setcolor(g, 0x00CACACA); 
	psy_ui_drawline(g, 0, baseline, size.width, baseline);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x00CACACA);
	for (c = 0, cpx = 0; c <= self->view->metrics.visisteps; 
			cpx += self->view->metrics.stepwidth, ++c) {
		char txt[40];
		int frame;
		
		psy_ui_drawline(g, (int)cpx, baseline, (int)cpx, baseline - tm.tmHeight / 3);
		frame = (int)((c - self->scrollpos) * 
			(self->view->sample
				? (self->view->sample->numframes / self->view->metrics.visisteps)
				: 0));
		psy_snprintf(txt, 40, "%d", frame);		
		psy_ui_textout(g, (int)cpx + (int)(tm.tmAveCharWidth * 0.75), baseline - tm.tmHeight - tm.tmHeight/6, txt, strlen(txt));
	}
}

void sampleeditor_onscrollzoom_customdraw(SampleEditor* self, ScrollZoom* sender,
	psy_ui_Graphics* g)
{
	if (self->sample) {
		psy_ui_Rectangle r;
		psy_ui_Size size = psy_ui_component_size(&sender->component);	
		psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
		psy_ui_setcolor(g, 0x00B1C8B0);
		if (!self->sample) {
			psy_ui_TextMetric tm;
			static const char* txt = "No wave loaded";

			tm = psy_ui_component_textmetric(&sender->component);
			psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
			psy_ui_settextcolor(g, 0x00D1C5B6);
			psy_ui_textout(g, (size.width - tm.tmAveCharWidth * strlen(txt)) / 2,
				(size.height - tm.tmHeight) / 2, txt, strlen(txt));
		} else {
			int x;
			int centery = size.height / 2;
			float offsetstep;
			psy_dsp_amp_t scaley;

			scaley = (size.height / 2) / (psy_dsp_amp_t) 32768;
			offsetstep = (float) self->sample->numframes / size.width;
			psy_ui_setcolor(g, 0x00B1C8B0);
			for (x = 0; x < size.width; ++x) {			
				uintptr_t frame = (int)(offsetstep * x);
				float framevalue;
				
				if (frame >= self->sample->numframes) {
					break;
				}
				framevalue = self->sample->channels.samples[0][frame];							
				psy_ui_drawline(g, x, centery, x, centery +
					(int)(framevalue * scaley));
			}
		}
	}
}

void sampleeditor_init(SampleEditor* self, psy_ui_Component* parent,
	Workspace* workspace)
{						
	psy_ui_Margin margin;

	self->sample = 0;
	self->samplerevents = 0;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_signal_connect(&self->component.signal_destroy, self,
		sampleeditor_ondestroy);
	psy_ui_component_enablealign(&self->component);	
	sampleprocessview_init(&self->processview, &self->component, workspace);
	psy_ui_component_setalign(&self->processview.component, psy_ui_ALIGN_RIGHT);
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(0),
		psy_ui_value_makeew(1.5));
	psy_ui_component_setmargin(&self->processview.component, &margin);
	psy_signal_connect(&self->processview.process.signal_clicked, self,
		sampleeditor_onprocess);
	sampleeditorplaybar_init(&self->playbar, &self->component, workspace);
	psy_signal_connect(&self->playbar.play.signal_clicked, self,
		sampleeditor_onplay);
	psy_signal_connect(&self->playbar.stop.signal_clicked, self,
		sampleeditor_onstop);
	psy_ui_component_setalign(&self->playbar.component, psy_ui_ALIGN_TOP);	
	sampleeditorheader_init(&self->header, &self->component, self);
	psy_ui_component_setalign(&self->header.component, psy_ui_ALIGN_TOP);
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.2),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->header.component, &margin);
	psy_table_init(&self->waveboxes);
	psy_ui_component_init(&self->samplebox, &self->component);
	psy_ui_component_enablealign(&self->samplebox);
	psy_ui_component_setbackgroundmode(&self->samplebox, BACKGROUND_NONE);
	psy_ui_component_setalign(&self->samplebox, psy_ui_ALIGN_CLIENT);	
	scrollzoom_init(&self->zoom, &self->component);
	psy_signal_connect(&self->zoom.signal_customdraw, self,
		sampleeditor_onscrollzoom_customdraw);
	psy_ui_component_setalign(&self->zoom.component, psy_ui_ALIGN_BOTTOM);	
	psy_ui_margin_init(&margin, psy_ui_value_makeeh(0.2),
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_setmargin(&self->zoom.component, &margin);
	sampleeditor_computemetrics(self, &self->metrics);
	psy_signal_connect(&self->zoom.signal_zoom, self, sampleeditor_onzoom);
	psy_signal_connect(&workspace->signal_songchanged, self,
		sampleeditor_onsongchanged);	
	sampleeditor_initsampler(self);
	sampleeditor_connectmachinessignals(self, workspace);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		sampleeditor_onlanguagechanged);
}

void sampleeditor_initsampler(SampleEditor* self)
{
	uintptr_t c;

	psy_audio_sampler_init(&self->sampler,
		self->workspace->machinefactory.machinecallback);
	psy_audio_buffer_init(&self->samplerbuffer, 2);
	for (c = 0; c < self->samplerbuffer.numchannels; ++c) {
		self->samplerbuffer.samples[c] = dsp.memory_alloc(MAX_STREAM_SIZE,
			sizeof(float));
	}	
}

void sampleeditor_ondestroy(SampleEditor* self, psy_ui_Component* sender)
{
	uintptr_t c;

	psy_audio_machine_dispose(&self->sampler.custommachine.machine);
	for (c = 0; c < self->samplerbuffer.numchannels; ++c) {
		dsp.memory_dealloc(self->samplerbuffer.samples[c]);
	}
	psy_audio_buffer_dispose(&self->samplerbuffer);
	sampleeditor_clearwaveboxes(self);
	psy_table_dispose(&self->waveboxes);
}

void sampleeditor_clearwaveboxes(SampleEditor* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*) psy_tableiterator_value(&it);
		psy_ui_component_destroy(&wavebox->component);		
		free(wavebox);
	}
	psy_table_clear(&self->waveboxes);
}

void sampleeditor_setsample(SampleEditor* self, psy_audio_Sample* sample)
{
	self->sample = sample;
	sampleeditor_buildwaveboxes(self, sample);
	sampleeditor_computemetrics(self, &self->metrics);
	psy_ui_component_align(&self->samplebox);
	psy_ui_component_invalidate(&self->component);
}

void sampleeditor_buildwaveboxes(SampleEditor* self, psy_audio_Sample* sample)
{
	sampleeditor_clearwaveboxes(self);
	if (self->sample) {
		uintptr_t c;

		for (c = 0; c < psy_audio_buffer_numchannels(&self->sample->channels); ++c) {
			WaveBox* wavebox;

			wavebox = (WaveBox*)malloc(sizeof(WaveBox));
			wavebox_init(wavebox, &self->samplebox);
			wavebox->preventdrawonselect = TRUE;
			wavebox_setsample(wavebox, sample);
			psy_ui_component_setalign(&wavebox->component, psy_ui_ALIGN_CLIENT);
			psy_signal_connect(&wavebox->selectionchanged, self,
				sampleeditor_onselectionchanged);
			psy_table_insert(&self->waveboxes, c, (void*)wavebox);
		}
	}
}

void sampleeditor_onsize(SampleEditor* self, psy_ui_Component* sender,
	psy_ui_Size* size)
{
	sampleeditor_computemetrics(self, &self->metrics);
}

void sampleeditor_onzoom(SampleEditor* self, psy_ui_Component* sender)
{
	sampleeditor_setsampleboxzoom(self);	
}

void sampleeditor_setsampleboxzoom(SampleEditor* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		wavebox_setzoom(wavebox, self->zoom.zoomleft,
			self->zoom.zoomright);
	}	
}

void sampleeditor_computemetrics(SampleEditor* self, SampleEditorMetrics* rv)
{	
	psy_ui_Size sampleboxsize;

	sampleboxsize = psy_ui_component_size(&self->samplebox);
	rv->samplewidth = self->sample
		? (float) sampleboxsize.width / self->sample->numframes
		: 0;	
	rv->visisteps = 10;	
	rv->stepwidth = self->sample
		? rv->samplewidth * (self->sample->numframes / 10)
		: sampleboxsize.width;	
}

void sampleeditor_onsongchanged(SampleEditor* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{
	sampleeditor_connectmachinessignals(self, workspace);
	sampleeditorbar_clearselection(&self->sampleeditortbar);
}

void sampleeditor_connectmachinessignals(SampleEditor* self,
	Workspace* workspace)
{
	if (workspace && workspace->song &&
			machines_master(&workspace->song->machines)) {
		psy_signal_connect(
			&machines_master(&workspace->song->machines)->signal_worked, self,
			sampleeditor_onmasterworked);
	}
}

void sampleeditor_onplay(SampleEditor* self, psy_ui_Component* sender)
{	
	if (self->workspace->song && self->sample) {
		psy_audio_PatternEvent event;
		psy_audio_exclusivelock_enter();
		psy_list_free(self->samplerevents);
		patternevent_init_all(&event,
			(unsigned char) 48,
			(unsigned char) instruments_slot(&self->workspace->song->instruments),
			NOTECOMMANDS_MACH_EMPTY,
			NOTECOMMANDS_VOL_EMPTY,
			0, 0);	
		patternentry_init_all(&self->samplerentry, &event, 0, 0, 120.f, 0);
		self->samplerevents = psy_list_create(&self->samplerentry);
		psy_audio_exclusivelock_leave();
	}
}

void sampleeditor_onstop(SampleEditor* self, psy_ui_Component* sender)
{	
	psy_audio_PatternEvent event;

	psy_audio_exclusivelock_enter();
	psy_list_free(self->samplerevents);
	patternevent_init_all(&event,
		NOTECOMMANDS_RELEASE,
		0,		
		NOTECOMMANDS_MACH_EMPTY,
		NOTECOMMANDS_VOL_EMPTY,
		0, 0);	
	self->samplerevents = psy_list_create(&self->samplerentry);
	patternentry_init_all(&self->samplerentry, &event, 0, 0, 120.f, 0);
	psy_audio_exclusivelock_leave();
}

void sampleeditor_onmasterworked(SampleEditor* self, psy_audio_Machine* machine,
	uintptr_t slot, psy_audio_BufferContext* bc)
{
	psy_audio_BufferContext samplerbc;
		
	psy_audio_buffercontext_init(&samplerbc, self->samplerevents, 0,
		&self->samplerbuffer, bc->numsamples, 16);
	psy_audio_buffer_clearsamples(&self->samplerbuffer, bc->numsamples);
	psy_audio_machine_work(&self->sampler.custommachine.machine, &samplerbc);
	psy_audio_buffer_addsamples(bc->output, &self->samplerbuffer, bc->numsamples, 
		(psy_dsp_amp_t) 1.f);
	if (self->samplerevents) {
		patternentry_dispose(&self->samplerentry);
	}
	psy_list_free(self->samplerevents);
	self->samplerevents = 0;
	
}

void sampleeditor_onprocess(SampleEditor* self, psy_ui_Component* sender)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->waveboxes);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		WaveBox* wavebox;

		wavebox = (WaveBox*)psy_tableiterator_value(&it);
		if (wavebox_hasselection(wavebox)) {
			int selected;

			selected = psy_ui_listbox_cursel(&self->processview.processors);
			switch (selected) {
			case 0: // Amplify
			{
				double ratio;

				ratio = pow(10.0, (double)((double)(self->processview.amplify.gainvalue) - 2 / (double)3) / (1 / (double)7));
				sampleeditor_amplify(self, wavebox->selectionstart, wavebox->selectionend,
					(psy_dsp_amp_t)ratio);				
			}
			break;
			case 1: // FadeIn
				sampleeditor_fade(self, wavebox->selectionstart, wavebox->selectionend,
					0.f, 1.f);
				break;
			case 2: // FadeOut
				sampleeditor_fade(self, wavebox->selectionstart, wavebox->selectionend,
					1.f, 0.f);
				break;
			case 4:
				sampleeditor_normalize(self, wavebox->selectionstart, wavebox->selectionend);
				break;
			case 5:
				sampleeditor_removeDC(self, wavebox->selectionstart, wavebox->selectionend);				
				break;
			case 6:
				sampleeditor_reverse(self, wavebox->selectionstart, wavebox->selectionend);
				break;
			default:
				break;
			}
		} else {

		}
	}
	psy_ui_component_invalidate(&self->samplebox);
}

void sampleeditor_amplify(SampleEditor* self, uintptr_t framestart, uintptr_t frameend, psy_dsp_amp_t gain)
{	
	dsp.mul(self->sample->channels.samples[0] + framestart, frameend - framestart + 1, gain);
}

void sampleeditor_fade(SampleEditor* self, uintptr_t framestart, uintptr_t frameend, psy_dsp_amp_t startvol,
	psy_dsp_amp_t endvol)
{
	uintptr_t frame;
	uintptr_t j;
	double slope;
	
	slope = (endvol - startvol) / (double)(frameend - framestart + 1);
	for (frame = framestart, j = 0; frame <= frameend; ++frame, ++j) {
		self->sample->channels.samples[0][frame] *= (psy_dsp_amp_t)(startvol + j * slope);
	}
}

void sampleeditor_reverse(SampleEditor* self, uintptr_t framestart, uintptr_t frameend)
{
	uintptr_t j;
	uintptr_t halved;

	halved = (uintptr_t)floor((frameend - framestart + 1) / 2.0f);

	for (j = 0; j < halved; ++j) {
		psy_dsp_amp_t temp;

		temp = self->sample->channels.samples[0][framestart + j];
		self->sample->channels.samples[0][framestart + j] = self->sample->channels.samples[0][frameend - j];
		self->sample->channels.samples[0][frameend - j] = temp;
	}
}

// (Fideloop's)
void sampleeditor_normalize(SampleEditor* self, uintptr_t framestart, uintptr_t frameend) 
{
	psy_dsp_amp_t maxL = 0;
	double ratio = 0;
	uintptr_t numframes;

	numframes = frameend - framestart + 1;	
	maxL = dsp.maxvol(self->sample->channels.samples[0] + framestart, numframes);
	if (maxL > 0.0) {
		ratio = (double)32767 / maxL;
	}
	if (ratio != 1) {
		dsp.mul(self->sample->channels.samples[0] + framestart, numframes,
			(psy_dsp_amp_t)ratio);	
	}
}

void sampleeditor_removeDC(SampleEditor* self, uintptr_t framestart, uintptr_t frameend)
{
	uintptr_t c;
	uintptr_t numframes;
	double meanL = 0.0;
	double meanR = 0.0;
	psy_dsp_amp_t* wdLeft;
	psy_dsp_amp_t* wdRight;
	psy_dsp_amp_t buf;

	wdLeft = self->sample->channels.samples[0];
	wdRight = NULL;

	numframes = frameend - framestart + 1;
	for (c = framestart; c < framestart + numframes; c++) {
		meanL = meanL + ((double)*(wdLeft + c) / numframes);

		//if (wdStereo) meanR = (double)meanR + ((double)*(wdRight + c) / length);
	}

	for (c = framestart; c < framestart + numframes; c++)
	{
		buf = *(wdLeft + c);
		if (meanL > 0)
		{
			if ((double)(buf - meanL) < (-32768))	*(wdLeft + c) = (psy_dsp_amp_t) -32768;
			else	*(wdLeft + c) = (psy_dsp_amp_t)(buf - meanL);
		} else if (meanL < 0)
		{
			if ((double)(buf - meanL) > 32767) *(wdLeft + c) = (psy_dsp_amp_t) 32767;
			else *(wdLeft + c) = (psy_dsp_amp_t)(buf - meanL);
		}
	}
}

void sampleeditor_onselectionchanged(SampleEditor* self, WaveBox* sender)
{
	if (wavebox_hasselection(sender)) {
		sampleeditorbar_setselection(&self->sampleeditortbar, sender->selectionstart,
			sender->selectionend);
	} else {
		sampleeditorbar_clearselection(&self->sampleeditortbar);
	}
	if (psy_ui_checkbox_checked(&self->sampleeditortbar.selecttogether)) {
		psy_TableIterator it;
		for (it = psy_table_begin(&self->waveboxes);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			WaveBox* wavebox;

			wavebox = (WaveBox*)psy_tableiterator_value(&it);
			if (wavebox != sender) {
				if (wavebox_hasselection(sender)) {
					wavebox_setselection(wavebox, sender->selectionstart,
						sender->selectionend);
				} else {
					wavebox_clearselection(wavebox);
				}				
			}
		}
		psy_ui_component_invalidate(&self->samplebox);
		psy_ui_component_update(&self->samplebox);
	} else {
		psy_ui_component_invalidate(&sender->component);
		psy_ui_component_update(&sender->component);
	}	
}

void sampleeditor_onlanguagechanged(SampleEditor* self,
	Workspace* workspace)
{
	psy_ui_component_align(&self->component);
}