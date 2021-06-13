/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_SEQUENCECMDS_H
#define psy_audio_SEQUENCECMDS_H

#ifdef __cplusplus
extern "C" {
#endif

/* audio */
#include "sequence.h"

/*
** psy_audio_SequenceInsertCommand
*/

typedef struct psy_audio_SequenceInsertCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;
	psy_audio_OrderIndex index;
	uintptr_t patidx;
	psy_audio_SequenceSelection restoreselection;	
} psy_audio_SequenceInsertCommand;

psy_audio_SequenceInsertCommand* psy_audio_sequenceinsertcommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex, uintptr_t patidx);

/*
** psy_audio_SequenceSampleInsertCommand
*/

typedef struct psy_audio_SequenceSampleInsertCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;
	psy_audio_OrderIndex index;
	psy_audio_SampleIndex sampleindex;
	psy_audio_SequenceSelection restoreselection;
} psy_audio_SequenceSampleInsertCommand;

psy_audio_SequenceSampleInsertCommand* psy_audio_sequencesampleinsertcommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex, psy_audio_SampleIndex);


/*
** psy_audio_SequenceMarkerInsertCommand
*/

typedef struct psy_audio_SequenceMarkerInsertCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;
	psy_audio_OrderIndex index;
	char* text;
	psy_audio_SequenceSelection restoreselection;
} psy_audio_SequenceMarkerInsertCommand;

psy_audio_SequenceMarkerInsertCommand* psy_audio_sequencemarkerinsertcommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection,
	psy_audio_OrderIndex, const char* text);

/*
** psy_audio_SequenceRemoveCommand
*/
typedef struct psy_audio_SequenceRemoveCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;		
	psy_audio_SequenceSelection restoreselection;
	psy_audio_Sequence restoresequence;	
} psy_audio_SequenceRemoveCommand;

psy_audio_SequenceRemoveCommand* psy_audio_sequenceremovecommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection);

typedef struct psy_audio_SequenceClearCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;		
	psy_audio_SequenceSelection restoreselection;
	psy_audio_Sequence restoresequence;
} psy_audio_SequenceClearCommand;

psy_audio_SequenceClearCommand* psy_audio_sequenceclearcommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection);

typedef struct psy_audio_SequenceChangePatternCommand {
	/* inherits */
	psy_Command command;
	psy_audio_Sequence* sequence;
	psy_audio_SequenceSelection* selection;
	psy_audio_SequenceSelection restoreselection;
	psy_audio_Sequence restoresequence;
	intptr_t step;
	bool success;
} psy_audio_SequenceChangePatternCommand;

psy_audio_SequenceChangePatternCommand* psy_audio_sequencechangepatterncommand_alloc(
	psy_audio_Sequence*, psy_audio_SequenceSelection* selection,
	intptr_t step);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SEQUENCECMDS_H */
