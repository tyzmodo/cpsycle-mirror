// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQEDITOR_H)
#define SEQEDITOR_H

#include "workspace.h"

#include <uiscroller.h>

typedef struct SeqEditorTrackState {
	int pxperbeat;
} SeqEditorTrackState;

typedef struct {
	psy_ui_Component component;	
} SeqEditorHeader;

void seqeditorheader_init(SeqEditorHeader*, psy_ui_Component* parent);

INLINE psy_ui_Component* seqeditorheader_base(SeqEditorHeader* self)
{
	return &self->component;
}

struct SeqEditorTrack;
struct SeqEditorTracks;

typedef void (*seqeditortrack_fp_ondraw)(struct SeqEditorTrack*, psy_ui_Graphics*, int x, int y);
typedef void (*seqeditortrack_fp_onpreferredsize)(struct SeqEditorTrack*,
	const psy_ui_Size* limit, psy_ui_Size* rv);

typedef struct SeqEditorTrackVTable {
	seqeditortrack_fp_ondraw ondraw;
	seqeditortrack_fp_onpreferredsize onpreferredsize;
} SeqEditorTrackVTable;

typedef struct SeqEditorTrack {
	SeqEditorTrackVTable* vtable;
	struct SeqEditorTracks* parent;
	Workspace* workspace;
	psy_audio_SequenceTrack* currtrack;
} SeqEditorTrack;

void seqeditortrack_init(SeqEditorTrack*, struct SeqEditorTracks* parent,
	Workspace*);
void seqeditortrack_dispose(SeqEditorTrack*);

SeqEditorTrack* seqeditortrack_alloc(void);
SeqEditorTrack* seqeditortrack_allocinit(struct SeqEditorTracks* parent,
	Workspace*);

void seqeditortrack_updatetrack(SeqEditorTrack*, psy_audio_SequenceTrack*);

INLINE SeqEditorTrack* seqeditortrack_base(SeqEditorTrack* self)
{
	return self;
}

INLINE void seqeditortrack_ondraw(SeqEditorTrack* self, psy_ui_Graphics* g, int x, int y)
{
	self->vtable->ondraw(self, g, x, y);
}

INLINE void seqeditortrack_onpreferredsize(SeqEditorTrack* self,
	const psy_ui_Size* limit, psy_ui_Size* rv)
{
	self->vtable->onpreferredsize(self, limit, rv);
}

typedef struct SeqEditorTracks {
	psy_ui_Component component;
	Workspace* workspace;
	psy_List* tracks;
} SeqEditorTracks;

void seqeditortracks_init(SeqEditorTracks*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* seqeditortracks_base(SeqEditorTracks* self)
{
	return &self->component;
}

typedef struct {
	psy_ui_Component component;
	SeqEditorHeader header;
	psy_ui_Scroller scroller;
	SeqEditorTracks tracks;
	Workspace* workspace;
} SeqEditor;

void seqeditor_init(SeqEditor*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* seqeditor_base(SeqEditor* self)
{
	return &self->component;
}

#endif /* SEQEDITOR_H */
