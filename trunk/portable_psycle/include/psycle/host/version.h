#include <project.h>

#if 0
$Log$
Revision 1.1  2004/12/22 22:43:41  johan-boule
fix closing bug [ 1087782 ] psycle MFC's version number is spread in several places

#endif
		  
///\file
///\brief the version number of the psycle host application.

/// Versions are composed this way:
/// C.M.m.p , where:
/// - C = codebase generation.
/// - M = major version number.
/// - m = minor version number: if 0, release. if different than 0, release candidate for next major version number.
/// - p = patch number.

/// Other files that need to be updated accordingly:
/// - make/doxygen/doxygen.configuration: PROJECT_NUMBER
/// - doc/for-end-users/readme.txt
/// - doc/for-end-users/whatsnew.txt
			
#define PSYCLE__VERSION__CODEBASE 1
#define PSYCLE__VERSION__MAJOR 7
#define PSYCLE__VERSION__MINOR 7
#define PSYCLE__VERSION__PATCH 17 /* $Revision$ $Date$ */
#define PSYCLE__VERSION__QUALITY "alpha"

#define PSYCLE__VERSION \
	PSYCLE__VERSION__QUALITY " " \
	STRINGIZED(PSYCLE__VERSION__CODEBASE) "." \
	STRINGIZED(PSYCLE__VERSION__MAJOR) "." \
	STRINGIZED(PSYCLE__VERSION__MINOR) "." \
	STRINGIZED(PSYCLE__VERSION__PATCH)

#if defined COMPILER__RESOURCE
	#define RC__CompanyName "Psycledelics"
	#define RC__LegalCopyright "(public domain) Copyright (C) 2000-2005 Psycledelics"
	#define RC__License "none, public domain"

	#define RC__ProductName "Psycle Modular Music Creation Studio"
	#define RC__InternalName "Psycle"
	#define RC__ProductVersion PSYCLE__VERSION ", " RC__DEBUG

	#define RC__FileDescription "Psycle Modular Music Creation Studio - Host"
	#define RC__OriginalFilename "psycle.exe"
	#define RC__FileVersion PSYCLE__VERSION ", " RC__DEBUG
	#define RC__SpecialBuild "compiler build tool chain:\r\n" "msvc 7.1"
	#define RC__PrivateBuild "built every sunnyday" //__DATE__ __TIME__

	#if defined NDEBUG
		#define RC__DEBUG "compiled with debugging disabled"
	#else
		#define RC__DEBUG "compiled with debugging enabled"
	#endif

	// [bohan]
	// Actual resource code moved back to psycle.rc ...
	// Dunno why, using msvc's resource compiler, #including this file from it doesn't create the version info ...
	// There's no reason this wouldn't work, it's weird.
	// Anyway, all the version information is set via the above parameters,
	// so that there's no need to change the psycle.rc file.
#endif
