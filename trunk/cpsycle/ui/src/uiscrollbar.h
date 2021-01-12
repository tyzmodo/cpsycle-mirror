// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SCROLLBAR_H
#define psy_ui_SCROLLBAR_H

#include "uibutton.h"

// Scrollbar
//
// displays a scrollbar
//
// psy_ui_Component
//      ^
//      |                         | 
//      |
// psy_ui_Scrollbar

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_ScrollBarPane {
    psy_ui_Component component;
    double pos;
    double screenpos;
    double scrollmax;
    double scrollmin;
    double dragoffset;
    int drag;
    psy_ui_Orientation orientation;
    psy_Signal signal_changed;
    psy_Signal signal_clicked;
} psy_ui_ScrollBarPane;

void psy_ui_scrollbarpane_init(psy_ui_ScrollBarPane*, psy_ui_Component* parent);
void psy_ui_scrollbarpane_setorientation(psy_ui_ScrollBarPane*, psy_ui_Orientation);

typedef struct {
    psy_ui_Component component;
    psy_ui_Button less;
    psy_ui_Button more;
    psy_ui_ScrollBarPane sliderpane;
    psy_Signal signal_changed;
    psy_Signal signal_clicked;    
} psy_ui_ScrollBar;

void psy_ui_scrollbar_init(psy_ui_ScrollBar*, psy_ui_Component* parent);
void psy_ui_scrollbar_setorientation(psy_ui_ScrollBar*, psy_ui_Orientation);
double psy_ui_scrollbar_position(psy_ui_ScrollBar*);
void psy_ui_scrollbar_setscrollrange(psy_ui_ScrollBar*, double scrollmax,
    double scrollmin);
void psy_ui_scrollbar_scrollrange(psy_ui_ScrollBar* self, double* scrollmin,
    double* scrollmax);
void psy_ui_scrollbar_setthumbposition(psy_ui_ScrollBar*, double pos);

INLINE psy_ui_Component* psy_ui_scrollbar_base(psy_ui_ScrollBar* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SCROLLBAR_H */