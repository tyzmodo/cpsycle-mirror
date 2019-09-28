// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UNDOREDO_H)
#define UNDOREDO_H

#include <list.h>
#include "command.h"

typedef struct {
	List* undo;
	List* redo;
} UndoRedo;

void undoredo_init(UndoRedo*);
void undoredo_dispose(UndoRedo*);
void undoredo_undo(UndoRedo*);
void undoredo_redo(UndoRedo*);
void undoredo_execute(UndoRedo*, Command*);

#endif