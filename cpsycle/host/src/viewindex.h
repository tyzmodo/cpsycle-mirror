/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(VIEWINDEX_H)
#define VIEWINDEX_H

/* container */
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	VIEW_ID_MACHINEVIEW		0
#define	VIEW_ID_PATTERNVIEW		1
#define	VIEW_ID_SAMPLESVIEW		2
#define VIEW_ID_INSTRUMENTSVIEW	3
#define	VIEW_ID_SONGPROPERTIES	4
#define	VIEW_ID_SETTINGSVIEW	5
#define	VIEW_ID_HELPVIEW		6
#define	VIEW_ID_RENDERVIEW		7
#define	VIEW_ID_EXPORTVIEW		8
#define	VIEW_ID_CHECKUNSAVED	9
#define	VIEW_ID_CONFIRM			10
#define	VIEW_NUM				11
#define	VIEW_ID_SCRIPTS			12
#define	VIEW_ID_FLOATED			13

#define SECTION_ID_HELPVIEW_HELP			0
#define SECTION_ID_HELPVIEW_ABOUT			1

#define SECTION_ID_MACHINEVIEW_WIRES		0
#define	SECTION_ID_MACHINEVIEW_STACK		1
#define	SECTION_ID_MACHINEVIEW_NEWMACHINE	2

#define SECTION_ID_FILESELECT 0
#define SECTION_ID_FILEVIEW 1

typedef enum {
	CONFIRM_CLOSE,
	CONFIRM_LOAD,
	CONFIRM_NEW,
	CONFIRM_SEQUENCECLEAR
} ConfirmBoxAction;

typedef enum {
	GENERATORS_ENABLED = 1,
	EFFECTS_ENABLED = 2,
	NEWMACHINE_INSERT = 4,
	NEWMACHINE_APPEND = 8,
	NEWMACHINE_ADDEFFECT = 16,
	NEWMACHINE_APPENDSTACK = 32,
	NEWMACHINE_ADDEFFECTSTACK = 64,
} NewMachineBarOptions;

typedef struct ViewIndex {
	uintptr_t id;
	uintptr_t section;
	uintptr_t option;
	uintptr_t seqpos;
} ViewIndex;

INLINE ViewIndex viewindex_make(uintptr_t id, uintptr_t section,
	uintptr_t option, uintptr_t seqpos)
{
	ViewIndex rv;

	rv.id = id;
	rv.section = section;
	rv.seqpos = seqpos;
	rv.option = option;
	return rv;
}

#ifdef __cplusplus
}
#endif

#endif /* VIEWINDEX_H */
