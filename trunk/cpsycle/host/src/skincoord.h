// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(SKINCOORD_H)
#define SKINCOORD_H

#include "uidef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {	
	psy_ui_RealRectangle src;
	psy_ui_RealRectangle dest;	
	double range;
} SkinCoord;

void skincoord_init(SkinCoord*);

void skincoord_init_all(SkinCoord*,
	double srcx, double srcy, double srcwidth, double srcheight,
	double destx, double desty, double destwidth, double destheight,
	double range);

void skincoord_setsource(SkinCoord* coord, intptr_t vals[4]);
void skincoord_setdest(SkinCoord* coord, intptr_t vals[4],
	uintptr_t num);

INLINE psy_ui_RealRectangle skincoord_destposition(const SkinCoord* self)
{
	return self->dest;
}

INLINE double skincoord_position(SkinCoord* coord, double value)
{
	return value * coord->range;
}

INLINE bool skincoord_hittest(const SkinCoord* self, double x, double y)
{
	psy_ui_RealRectangle destposition;

	destposition = skincoord_destposition(self);	
	return psy_ui_realrectangle_intersect(&destposition,
		psy_ui_realpoint_make(x, y));
}

#ifdef __cplusplus
}
#endif

#endif