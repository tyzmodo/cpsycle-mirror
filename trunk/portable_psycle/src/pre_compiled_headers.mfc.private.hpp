///\file
///\brief inclusions of mfc headers which must be pre-compiled.
#if defined PRE_COMPILED_HEADERS__MFC
	#error pre-compiled headers for mfc already included
#else
	#define PRE_COMPILED_HEADERS__MFC
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#pragma message("parsing " __FILE__)



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MFC core and "standard" components.
#include <afxwin.h>
#include <afxext.h> // MFC extensions.
#include <afxdtctl.h> // MFC support for Internet Explorer 4 Common Controls.
#if !defined _AFX_NO_AFXCMN_SUPPORT
	#include <afxcmn.h> // MFC support for Windows Common Controls.
#endif
#include <afxmt.h>
///////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GDI+.
#include <gdiplus.h>				
#pragma comment(lib, "gdiplus")



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// \todo bad stuff that shouldn't be here

#include "psycle/host/global.hpp" // missing #includes in psycle's source code.



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#pragma message("done parsing " __FILE__)



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end
