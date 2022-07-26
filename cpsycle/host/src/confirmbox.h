/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(CONFIRMBOX_H)
#define CONFIRMBOX_H

/* host */
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CONFIRM_YES,
	CONFIRM_NO,
	CONFIRM_CONTINUE
} ConfirmBoxButton;

typedef struct ConfirmBox {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_execute;
	/* internal */
	psy_ui_Component view;
	psy_ui_Label title;
	psy_ui_Label header;
	psy_ui_Button yes;
	psy_ui_Button no;
	psy_ui_Button cont;	
	ConfirmBoxAction mode;	
} ConfirmBox;

void confirmbox_init(ConfirmBox*, psy_ui_Component* parent);

void confirmbox_set_exit_mode(ConfirmBox*);
void confirmbox_set_labels(ConfirmBox* self, const char* title,
	const char* yesstr, const char* nostr);

INLINE psy_ui_Component* confirmbox_base(ConfirmBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CONFIRMBOX */
