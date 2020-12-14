// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(GENERALCONFIG_H)
#define GENERALCONFIG_H

// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// GeneralConfig

enum {
	PROPERTY_ID_SHOWSTEPSEQUENCER = 500
};

typedef struct GeneralConfig {
	psy_Property* general;
	// references
	psy_Property* parent;	
} GeneralConfig;

void generalconfig_init(GeneralConfig*, psy_Property* parent);

bool generalconfig_showsonginfoonload(const GeneralConfig*);
bool generalconfig_showaboutatstart(const GeneralConfig*);
bool generalconfig_showmaximizedatstart(const GeneralConfig*);
bool generalconfig_saverecentsongs(const GeneralConfig*);
bool generalconfig_playsongafterload(const GeneralConfig*);
bool generalconfig_showingpatternnames(const GeneralConfig*);
bool generalconfig_showplaylisteditor(const GeneralConfig*);
bool generalconfig_showstepsequencer(const GeneralConfig*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEVIEWCONFIG_H */