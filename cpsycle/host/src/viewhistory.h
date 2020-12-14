// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(VIEWHISTORY_H)
#define VIEWHISTORY_H

// container
#include <list.h>

// ViewHistory

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ViewHistoryEntry {
	int id;
	int seqpos;
} ViewHistoryEntry;

typedef struct {
	psy_List* container;
	bool prevented;
	psy_List* currnavigation;
	bool navigating;
} ViewHistory;

void viewhistory_init(ViewHistory*);
void viewhistory_dispose(ViewHistory*);
void viewhistory_clear(ViewHistory*);
void viewhistory_add(ViewHistory*, ViewHistoryEntry view);
void viewhistory_addseqpos(ViewHistory*, int seqpos);
bool viewhistory_back(ViewHistory*);
bool viewhistory_forward(ViewHistory*);
ViewHistoryEntry viewhistory_currview(const ViewHistory*);
bool viewhistory_equal(const ViewHistory*, ViewHistoryEntry);

INLINE bool viewhistory_hascurrview(const ViewHistory* self)
{
	return self->currnavigation != NULL;
}

INLINE void viewhistory_prevent(ViewHistory* self)
{
	self->prevented = TRUE;
}

INLINE void viewhistory_enable(ViewHistory* self)
{
	self->prevented = FALSE;
}

INLINE bool viewhistory_prevented(const ViewHistory* self)
{
	return self->prevented;
}

#ifdef __cplusplus
}
#endif

#endif /* VIEWHISTORY_H */