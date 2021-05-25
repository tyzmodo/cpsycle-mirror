// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(WORKSPACE_H)
#define WORKSPACE_H

// host
#include "config.h"
#include "undoredo.h"
#include "viewhistory.h"
// audio
#include <machinefactory.h>
#include <player.h>
#include <plugincatcher.h>
#include <song.h>
#include <sequence.h>
#include <signal.h>
// dsp
#include <notestab.h>
// ui
#include <uicomponent.h>
#include <uiapp.h>
// audio
#include <lock.h>
// file
#include <playlist.h>
#include <propertiesio.h>

// Workspace
//
// connects the player with the psycle host ui and configures both
//
// psy_audio_MachineCallback
//         ^
//         |
//       Workspace
//             <>---- PsycleConfig;	              host
//             <>---- ViewHistory
//             <>---- psy_audio_Player;           audio imports
//             <>---- psy_audio_MachineFactory
//             <>---- psy_audio_PluginCatcher
//             <>---- psy_audio_Song

#ifdef __cplusplus
extern "C" {
#endif

// The view id belongs to a component of the client notebook of the mainframe
// view_id = component insert order

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

#define SECTION_ID_MACHINEVIEW_WIRES		0
#define	SECTION_ID_MACHINEVIEW_STACK		1
#define	SECTION_ID_MACHINEVIEW_NEWMACHINE	2

#define SECTION_ID_FILESELECT 0
#define SECTION_ID_FILEVIEW 1

typedef enum {
	GENERATORS_ENABLED = 1,
	EFFECTS_ENABLED = 2,
	NEWMACHINE_INSERT = 4,
	NEWMACHINE_APPEND = 8,
	NEWMACHINE_ADDEFFECT = 16,
	NEWMACHINE_APPENDSTACK = 32,
	NEWMACHINE_ADDEFFECTSTACK = 64,
} NewMachineBarOptions;

// The patternview display modes
typedef enum {
	PATTERN_DISPLAYMODE_TRACKER,					// only tracker visible
	PATTERN_DISPLAYMODE_PIANOROLL,					// only pianoroll visible
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,	// both of them visible
	PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL,
	PATTERN_DISPLAYMODE_NUM
} PatternDisplayMode;

typedef enum {
	CONFIRM_CLOSE,
	CONFIRM_LOAD,
	CONFIRM_NEW,
	CONFIRM_SEQUENCECLEAR
} ConfirmBoxAction;

enum {
	WORKSPACE_NEWSONG,
	WORKSPACE_LOADSONG
};

struct Workspace;

typedef void (*fp_workspace_output)(void* context,
	struct Workspace* sender, const char* text);
typedef void (*fp_workspace_songloadprogress)(void* context,
	struct Workspace* sender, intptr_t progress);

typedef struct Workspace {
	// implements
	psy_audio_MachineCallback machinecallback;
	// Signals
	psy_Signal signal_octavechanged;
	psy_Signal signal_songchanged;
	psy_Signal signal_configchanged;	
	psy_Signal signal_changecontrolskin;
	psy_Signal signal_patterncursorchanged;
	psy_Signal signal_gotocursor;
	// psy_Signal signal_sequenceselectionchanged;
	psy_Signal signal_loadprogress;
	psy_Signal signal_scanprogress;
	psy_Signal signal_scanstart;
	psy_Signal signal_scanend;
	psy_Signal signal_scanfile;
	psy_Signal signal_scantaskstart;
	psy_Signal signal_plugincachechanged;
	psy_Signal signal_beforesavesong;
	psy_Signal signal_showparameters;
	psy_Signal signal_viewselected;
	psy_Signal signal_focusview;
	psy_Signal signal_parametertweak;
	psy_Signal signal_terminal_error;
	psy_Signal signal_terminal_warning;
	psy_Signal signal_terminal_out;		
	psy_Signal signal_status_out;
	psy_Signal signal_followsongchanged;	
	psy_Signal signal_togglegear;
	psy_Signal signal_floatsection;
	psy_Signal signal_docksection;
	psy_Signal signal_machineeditresize;
	psy_Signal signal_buschanged;
	psy_Signal signal_gearselect;
	// audio
	psy_audio_Song* song;
	psy_audio_Player player;
	psy_audio_PluginCatcher plugincatcher;	
	psy_audio_MachineFactory machinefactory;	
	// Psycle settings	
	PsycleConfig config;
	psy_Playlist playlist;	
	psy_ui_Component* mainhandle;	
	ViewHistory viewhistory;
	ViewHistoryEntry restoreview;
	psy_audio_PatternCursor patterneditposition;	
	psy_audio_SequenceSelection sequenceselection;
	int cursorstep;	
	char* filename;
	int followsong;
	int recordtweaks;
	psy_audio_SequenceEntry* lastentry;
	psy_audio_Pattern patternpaste;
	psy_audio_SequencePaste sequencepaste;
	int navigating;
	bool patternsinglemode;
	// ui
	int fontheight;	
	bool hasnewline;
	bool gearvisible;	
	// UndoRedo
	psy_UndoRedo undoredo;
	uintptr_t undosavepoint;
	uintptr_t machines_undosavepoint;
	psy_audio_Lock pluginscanlock;
	psy_audio_Lock outputlock;
	int filescanned;
	char* scanfilename;
	int scanstart;
	int scanend;
	int scantaskstart;
	int plugincachechanged;
	int scanprogress;
	int scanprogresschanged;
	psy_audio_PluginScanTask lastscantask;
	int scanplugintype;
	bool playrow;
	psy_audio_SequencerPlayMode restoreplaymode;
	psy_dsp_big_beat_t restorenumplaybeats;
	bool restoreloop;
	bool startpage;
	psy_List* errorstrs;
	psy_List* statusoutputstrs;
	bool driverconfigloading;
	bool seqviewactive;
} Workspace;

void workspace_init(Workspace*, void* handle);
void workspace_dispose(Workspace*);
void workspace_load_configuration(Workspace*);
void workspace_postload_driverconfigurations(Workspace*);
void workspace_startaudio(Workspace*);
void workspace_save_configuration(Workspace*);
void workspace_clearsequencepaste(Workspace*);
void workspace_save_styleconfiguration(Workspace*);
void workspace_newsong(Workspace*);
void workspace_loadsong_fileselect(Workspace*);
void workspace_loadsong(Workspace*, const char*, bool play);
bool workspace_savesong_fileselect(Workspace*);
void workspace_savesong(Workspace*, const char*);
bool workspace_exportsong(Workspace*);
void workspace_exportmodule(Workspace*, const char* path);
bool workspace_exportmidifile_fileselect(Workspace* self);
void workspace_exportmidifile(Workspace*, const char* path);
void workspace_scanplugins(Workspace*);

INLINE PsycleConfig* workspace_conf(Workspace* self) { return &self->config; }

INLINE void workspace_configure_host(Workspace* self)
{
	psycleconfig_notifyall_changed(&self->config);
	psy_signal_emit(&self->signal_configchanged,
		self, 1, &self->config.config);
}

INLINE psy_audio_Song* workspace_song(Workspace* self) { return self->song; }

INLINE const psy_audio_Song* workspace_song_const(const Workspace* self)
{
	return self->song;
}

INLINE psy_audio_Player* workspace_player(Workspace* self)
{
	return &self->player;
}

psy_Property* workspace_recentsongs(Workspace*);
psy_Playlist* workspace_playlist(Workspace*);
void workspace_load_recentsongs(Workspace*);
void workspace_save_recentsongs(Workspace*);
void workspace_clearrecentsongs(Workspace*);
void workspace_setoctave(Workspace*, int octave);
uintptr_t workspace_octave(Workspace*);
void workspace_configurationchanged(Workspace*, psy_Property*,
	uintptr_t* rebuild_level);
void workspace_onconfigurationchanged(Workspace*, psy_Property*);
void workspace_undo(Workspace*);
void workspace_redo(Workspace*);
void workspace_setpatterncursor(Workspace*, psy_audio_PatternCursor);
psy_audio_PatternCursor workspace_patterncursor(Workspace*);
void workspace_setsequenceeditposition(Workspace*, psy_audio_OrderIndex);
psy_audio_OrderIndex workspace_sequenceeditposition(const Workspace*);
void workspace_setcursorstep(Workspace*, int step);
int workspace_cursorstep(Workspace*);
void workspace_editquantizechange(Workspace*, int diff);
int workspace_hasplugincache(const Workspace*);
psy_audio_PluginCatcher* workspace_plugincatcher(Workspace*);
psy_EventDriver* workspace_kbddriver(Workspace*);
int workspace_followingsong(Workspace*);
void workspace_followsong(Workspace*);
void workspace_stopfollowsong(Workspace*);
void workspace_idle(Workspace*);
void workspace_showparameters(Workspace*, uintptr_t machineslot);
void workspace_selectview(Workspace*, uintptr_t view, uintptr_t section,
	uintptr_t option);
void workspace_focusview(Workspace*);
void workspace_saveview(Workspace*);
void workspace_restoreview(Workspace*);
void workspace_floatsection(Workspace*, int view, uintptr_t section);
void workspace_docksection(Workspace*, int view, uintptr_t section);
void workspace_parametertweak(Workspace*, int slot, uintptr_t tweak, float value);
void workspace_recordtweaks(Workspace*);
void workspace_stoprecordtweaks(Workspace*);
int workspace_recordingtweaks(Workspace*);
void workspace_onviewchanged(Workspace*, ViewHistoryEntry);
void workspace_back(Workspace*);
void workspace_forward(Workspace*);
void workspace_updatecurrview(Workspace*);
ViewHistoryEntry workspace_currview(Workspace*);
void workspace_addhistory(Workspace*);
void workspace_connectasmixersend(Workspace*);
void workspace_connectasmixerinput(Workspace*);
bool workspace_isconnectasmixersend(const Workspace*);
void workspace_togglegear(Workspace*);
bool workspace_gearvisible(const Workspace* self);
bool workspace_songmodified(const Workspace*);
bool workspace_currview_hasundo(Workspace*);
bool workspace_currview_hasredo(Workspace*);
void workspace_clearundo(Workspace*);
psy_dsp_NotesTabMode workspace_notetabmode(Workspace*);
void workspace_playstart(Workspace*);
void workspace_outputwarning(Workspace*, const char* text);
void workspace_outputerror(Workspace*, const char* text);
void workspace_output(Workspace*, const char* text);
void workspace_outputstatus(Workspace*, const char* text);
void workspace_songposdec(Workspace*);
void workspace_songposinc(Workspace*);
void workspace_gotocursor(Workspace*, psy_audio_PatternCursor);
PatternDisplayMode workspace_patterndisplaytype(Workspace*);
void workspace_selectpatterndisplay(Workspace*, PatternDisplayMode);
void workspace_multiselectgear(Workspace*, psy_List* slotlist);
void workspace_connectterminal(Workspace*, void* context,
	fp_workspace_output out,
	fp_workspace_output warning,
	fp_workspace_output error);
void workspace_connectstatus(Workspace*, void* context, fp_workspace_output);
void workspace_connectloadprogress(Workspace*, void* context,
	fp_workspace_songloadprogress);
void workspace_apptitle(Workspace*, char* rv_title, uintptr_t max_len);
const char* workspace_songtitle(const Workspace*);
void workspace_setstartpage(Workspace*);


#ifdef __cplusplus
}
#endif

#endif /* WORKSPACE_H */
