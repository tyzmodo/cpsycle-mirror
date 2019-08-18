// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "inputmap.h"

void InitInputMap(InputMap* self)
{
	InitIntHashTable(&self->map, 256);
}

void DisposeInputMap(InputMap* self)
{
	DisposeIntHashTable(&self->map);
}

int Cmd(InputMap* self, int input)
{
	return ExistsIntHashTable(&self->map, input) 
		? (int) SearchIntHashTable(&self->map, input)
		: -1;	
}

void DefineInput(InputMap* self, int input, int cmd)
{
	InsertIntHashTable(&self->map, input, (void*)cmd);
}
