/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMPONENTSTYLE_H
#define psy_ui_COMPONENTSTYLE_H

#include "uistyle.h"

/*
** psy_ui_ComponentStyle
**
** Assigns styles to special states
**
** todo: add specificity for mixing several state styles
**       the states flag stores combinations of states but only one style can
**       be selected at the moment
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psy_ui_StyleState {
	psy_ui_STYLESTATE_NONE     = 0,
	psy_ui_STYLESTATE_HOVER    = 1,
	psy_ui_STYLESTATE_SELECT   = 2,
	psy_ui_STYLESTATE_FOCUS    = 4,
	psy_ui_STYLESTATE_DISABLED = 8,
	psy_ui_STYLESTATE_ACTIVE   = 16,
	psy_ui_STYLESTATE_NUM      = 32
} psy_ui_StyleState;

typedef struct psy_ui_ComponentStyle {	
	psy_Table* styles;			/* StyleState X StyleId */	
	psy_ui_Style* currstyle;	/* 
								** Style for the current style state 
								** - if no state available points to the style
								**   for STYLESTATE_NONE
								** - if no style at all available points to
								**   overridestyle
								*/
	psy_ui_Style overridestyle;	/* overrides currstyle for component setter */
	psy_ui_StyleState states;	/* bit flag holding the activated states */
	intptr_t debugflag;
} psy_ui_ComponentStyle;

void psy_ui_componentstyle_init(psy_ui_ComponentStyle*);
void psy_ui_componentstyle_dispose(psy_ui_ComponentStyle*);

/* checks if a state has a style */
bool psy_ui_componentstyle_hasstyle(const psy_ui_ComponentStyle*,
	psy_ui_StyleState);
/* returns the style belonging to a state (see todo) */
psy_ui_Style* psy_ui_componentstyle_style(psy_ui_ComponentStyle*,
	psy_ui_StyleState);
/* assigns a style to a state */
void psy_ui_componentstyle_setstyle(psy_ui_ComponentStyle*,
	psy_ui_StyleState, uintptr_t style_id);
/* checks if a state is activated */
bool psy_ui_componentstyle_hasstate(const psy_ui_ComponentStyle*,
	psy_ui_StyleState);
/* activeates a state */
bool psy_ui_componentstyle_addstate(psy_ui_ComponentStyle*, psy_ui_StyleState);
/* deactivates a state */
bool psy_ui_componentstyle_removestate(psy_ui_ComponentStyle*, psy_ui_StyleState);
/* selects a state from the states flag (see todo) */
bool psy_ui_componentstyle_updatestate(psy_ui_ComponentStyle*);
/* sets the currstate according to the style state */
bool psy_ui_componentstyle_setcurrstate(psy_ui_ComponentStyle*,
	psy_ui_StyleState);
/* sets the currstate according to current style state */
void psy_ui_componentstyle_updatecurrstate(psy_ui_ComponentStyle*);
/* returns state flags */
INLINE psy_ui_StyleState psy_ui_componentstyle_state(const psy_ui_ComponentStyle* self)
{
	return self->states;
}
/* returns the style id belonging to the current state (see todo) */
uintptr_t psy_ui_componentstyle_currstyleid(const psy_ui_ComponentStyle*);

/*
** Properties
*/
INLINE void psy_ui_componentstyle_setmargin(psy_ui_ComponentStyle* self,
		psy_ui_Margin margin)
{
	psy_ui_style_setmargin(&self->overridestyle, margin);	
}

INLINE psy_ui_Margin psy_ui_componentstyle_margin(const psy_ui_ComponentStyle* self)
{	
	if (self->overridestyle.marginset) {
		return self->overridestyle.margin;
	} 
	return self->currstyle->margin;
}

INLINE void psy_ui_componentstyle_setspacing(psy_ui_ComponentStyle* self, psy_ui_Margin spacing)
{
	psy_ui_style_setspacing(&self->overridestyle, spacing);
}

INLINE psy_ui_Margin psy_ui_componentstyle_spacing(const psy_ui_ComponentStyle * self)
{
	if (self->overridestyle.spacingset) {
		return self->overridestyle.spacing;
	}
	return self->currstyle->spacing;
}

INLINE psy_ui_Colour psy_ui_componentstyle_colour(const psy_ui_ComponentStyle* self)
{
	if (self->overridestyle.colour.mode.set) {
		return self->currstyle->colour;
	}
	return self->currstyle->colour;
}

INLINE void psy_ui_componentstyle_setcolour(psy_ui_ComponentStyle* self, psy_ui_Colour colour)
{
	self->overridestyle.colour = colour;
}

INLINE psy_ui_Colour psy_ui_componentstyle_backgroundcolour(const psy_ui_ComponentStyle* self)
{
	if (self->overridestyle.backgroundcolour.mode.set) {
		return self->overridestyle.backgroundcolour;
	}
	return self->currstyle->backgroundcolour;
}

INLINE void psy_ui_componentstyle_setbackgroundcolour(psy_ui_ComponentStyle* self,
	psy_ui_Colour colour)
{
	self->overridestyle.backgroundcolour = colour;
}

INLINE const psy_ui_Border* psy_ui_componentstyle_border(const psy_ui_ComponentStyle* self)
{
	if (self->overridestyle.border.mode.set) {
		return &self->overridestyle.border;
	}
	return &self->currstyle->border;
}

INLINE void psy_ui_componentstyle_setborder(psy_ui_ComponentStyle* self, const psy_ui_Border* border)
{
	if (!border) {
		self->overridestyle.border.mode.set = 0;
		return;
	}
	self->overridestyle.border = *border;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTSTYLE_H */