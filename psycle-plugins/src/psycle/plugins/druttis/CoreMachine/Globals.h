/* -*- mode:c++, indent-tabs-mode:t -*- */
//////////////////////////////////////////////////////////////////////
//
//				Globals.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
//
//				GLOBALS struct
//
//////////////////////////////////////////////////////////////////////

struct GLOBALS
{
	int								samplingrate;				// Computed with oversampling factor
	int								ticklength;								// Computed with oversampling factor

	/* User fields here */
	float				wtbl[4096];

};
