// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "mainframe.h"
#include <ui_app.h>

// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

UIMAIN
{    	
	MainFrame main;			
	int err;
	
	UIINIT;		
	InitMainFrame(&main);	
	ui_component_show_state(&main.component, iCmdShow);		 	 
	err = ui_run();	
	UIDISPOSE;
//  _CrtDumpMemoryLeaks( );   
	return err;
}
