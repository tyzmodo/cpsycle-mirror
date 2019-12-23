// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uicomponent.h"
#include "uiapp.h"
#include "uimenu.h"
#include "hashtbl.h"
#include <memory.h>
#include <commctrl.h>	// includes the common control header
#include <stdio.h>
#include <shlobj.h>
#include <portable.h>

HINSTANCE appInstance = 0;
HWND appMainComponentHandle = 0;
int iDeltaPerLine = 120;

TCHAR szAppClass[] = TEXT("PsycleApp");
static TCHAR szComponentClass[] = TEXT("PsycleComponent");
static uintptr_t winid = 20000;
ui_font defaultfont = { 0, 0 };
static int defaultbackgroundcolor = 0x00232323;
static int defaultcolor = 0x00D1C5B6;
static HBRUSH defaultbackgroundbrush;
static int mousetracking = 0;

static psy_Table selfmap;
static psy_Table winidmap;
extern psy_Table menumap;

static LRESULT CALLBACK ui_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam);
BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam);
static void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam);
static void handle_scrollparam(SCROLLINFO* si, WPARAM wParam);
static void enableinput(ui_component* self, int enable, int recursive);
static void onpreferredsize(ui_component*, ui_component* sender, 
	ui_size* limit, ui_size* rv);	

void ui_init(uintptr_t hInstance)
{
	WNDCLASS     wndclass ;
	INITCOMMONCONTROLSEX icex;
	ui_fontinfo fontinfo;
	int succ;

	psy_signal_init(&app.signal_dispose);

	psy_table_init(&selfmap);
	psy_table_init(&winidmap);

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
    wndclass.lpfnWndProc   = ui_winproc ;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = (HINSTANCE) hInstance ;
    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
    wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH) ;
    wndclass.lpszMenuName  = NULL ;
    wndclass.lpszClassName = szAppClass ;	
	if (!RegisterClass (&wndclass))
    {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
                      szAppClass, MB_ICONERROR) ;		
    }
	
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = ui_winproc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = sizeof (long); 
	wndclass.hInstance     = (HINSTANCE) hInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject (NULL_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szComponentClass;
     
	RegisterClass (&wndclass) ;
	ui_menu_setup();
	// Ensure that the common control DLL is loaded.     		
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_USEREX_CLASSES;
    succ = InitCommonControlsEx(&icex);        

	appInstance = (HINSTANCE) hInstance;
	
	ui_fontinfo_init(&fontinfo, "Tahoma", 80);	
	ui_font_init(&defaultfont, &fontinfo);	
	defaultbackgroundbrush = CreateSolidBrush(0x00232323);
}

void ui_dispose()
{
	psy_signal_emit(&app.signal_dispose, &app, 0);
	psy_table_dispose(&selfmap);
	psy_table_dispose(&winidmap);
	DeleteObject(defaultfont.hfont);
	DeleteObject(defaultbackgroundbrush);
	psy_signal_dispose(&app.signal_dispose);
}

void ui_replacedefaultfont(ui_component* main, ui_font* font)
{		
	if (font && main) {
		psy_List* p;
		psy_List* q;

		if (main->font.hfont == defaultfont.hfont) {
			main->font.hfont = font->hfont;
			SendMessage((HWND)main->hwnd, WM_SETFONT, (WPARAM) font->hfont, 0);
		}
		for (p = q = ui_component_children(main, 1); p != 0; p = p->next) {
			ui_component* child;

			child = (ui_component*)p->entry;
			if (child->font.hfont == defaultfont.hfont) {
				child->font.hfont = font->hfont;		
				SendMessage((HWND)child->hwnd, WM_SETFONT,
					(WPARAM) font->hfont, 0);
				ui_component_align(child);
			}		
		}		
		psy_list_free(q);
		ui_font_dispose(&defaultfont);
		defaultfont = *font;
		ui_component_align(main);
	}
}

int ui_win32_component_init(ui_component* self, ui_component* parent,
		LPCTSTR classname, 
		int x, int y, int width, int height,
		DWORD dwStyle,
		int usecommand)
{
	int err = 0;
	HINSTANCE hInstance;

	ui_component_init_signals(self);
	if (parent) {
#if defined(_WIN64)		
		hInstance = (HINSTANCE) GetWindowLongPtr(
			(HWND)parent->hwnd, GWLP_HINSTANCE);
#else
		hInstance = (HINSTANCE) GetWindowLong(
			(HWND)parent->hwnd, GWL_HINSTANCE);
#endif
	} else {
		hInstance = appInstance;
	}
	self->hwnd = (uintptr_t) CreateWindow(
		classname,
		NULL,		
		dwStyle,
		x, y, width, height,
		parent ? (HWND) parent->hwnd : NULL,
		usecommand ? (HMENU)winid : NULL,
		hInstance,
		NULL);	
	if ((HWND) self->hwnd == NULL) {
        MessageBox(NULL, "Failed To Create Component", "Error",
			MB_OK | MB_ICONERROR);
		err = 1;
	} else {
		psy_table_insert(&selfmap, (uintptr_t) self->hwnd, self);
	}
	if (err == 0 && usecommand) {
		psy_table_insert(&winidmap, winid, self);
		winid++;		
	}
	ui_component_init_base(self);		
#if defined(_WIN64)		
		self->wndproc = (winproc)GetWindowLongPtr((HWND)self->hwnd, GWLP_WNDPROC);
#else		
		self->wndproc = (winproc)GetWindowLong((HWND)self->hwnd, GWL_WNDPROC);
#endif
	if (classname != szComponentClass && classname != szAppClass) {
#if defined(_WIN64)		
		SetWindowLongPtr((HWND)self->hwnd, GWLP_WNDPROC, (LONG_PTR) ui_com_winproc);
#else	
		SetWindowLong((HWND)self->hwnd, GWL_WNDPROC, (LONG)ui_com_winproc);
#endif
	}
	if (!parent) {
		appMainComponentHandle = (HWND) self->hwnd;
	}
	return err;
}

void ui_component_init(ui_component* component, ui_component* parent)
{		
	ui_win32_component_init(component, parent, szComponentClass,
		0, 0, 90, 90, WS_CHILDWINDOW | WS_VISIBLE, 0);
}

void ui_component_init_signals(ui_component* component)
{
	psy_signal_init(&component->signal_size);
	psy_signal_init(&component->signal_draw);
	psy_signal_init(&component->signal_timer);
	psy_signal_init(&component->signal_keydown);
	psy_signal_init(&component->signal_keyup);
	psy_signal_init(&component->signal_mousedown);
	psy_signal_init(&component->signal_mouseup);
	psy_signal_init(&component->signal_mousemove);
	psy_signal_init(&component->signal_mousewheel);
	psy_signal_init(&component->signal_mousedoubleclick);
	psy_signal_init(&component->signal_mouseenter);
	psy_signal_init(&component->signal_mousehover);
	psy_signal_init(&component->signal_mouseleave);
	psy_signal_init(&component->signal_scroll);
	psy_signal_init(&component->signal_create);
	psy_signal_init(&component->signal_destroy);
	psy_signal_init(&component->signal_show);
	psy_signal_init(&component->signal_hide);
	psy_signal_init(&component->signal_focus);
	psy_signal_init(&component->signal_focuslost);
	psy_signal_init(&component->signal_align);
	psy_signal_init(&component->signal_preferredsize);
	psy_signal_init(&component->signal_windowproc);
	psy_signal_init(&component->signal_command);
}

void ui_component_init_base(ui_component* self) {
	self->scrollstepx = 100;
	self->scrollstepy = 12;
	self->propagateevent = 0;
	self->preventdefault = 0;
	self->align = UI_ALIGN_NONE;
	self->justify = UI_JUSTIFY_EXPAND;
	self->alignchildren = 0;
	self->alignexpandmode = UI_NOEXPAND;
	memset(&self->margin, 0, sizeof(ui_margin));
	memset(&self->spacing, 0, sizeof(ui_margin));
	self->debugflag = 0;
	self->defaultpropagation = 0;	
	self->visible = 1;
	self->doublebuffered = 0;
	self->wheelscroll = 0;
	self->accumwheeldelta = 0;
	self->handlevscroll = 1;
	self->handlehscroll = 1;
	self->backgroundmode = BACKGROUND_SET;
	self->backgroundcolor = defaultbackgroundcolor;
	self->background = 0;
	self->color = defaultcolor;
	self->cursor = UI_CURSOR_DEFAULT;
	ui_font_init(&self->font, 0);
	ui_component_setfont(self, &defaultfont);
	ui_component_setbackgroundcolor(self, self->backgroundcolor);
	psy_signal_connect(&self->signal_preferredsize, self,
		onpreferredsize);
}

void ui_component_dispose(ui_component* component)
{	
	psy_signal_dispose(&component->signal_size);
	psy_signal_dispose(&component->signal_draw);
	psy_signal_dispose(&component->signal_timer);
	psy_signal_dispose(&component->signal_keydown);
	psy_signal_dispose(&component->signal_keyup);
	psy_signal_dispose(&component->signal_mousedown);
	psy_signal_dispose(&component->signal_mouseup);
	psy_signal_dispose(&component->signal_mousemove);
	psy_signal_dispose(&component->signal_mousewheel);
	psy_signal_dispose(&component->signal_mousedoubleclick);
	psy_signal_dispose(&component->signal_mouseenter);
	psy_signal_dispose(&component->signal_mousehover);
	psy_signal_dispose(&component->signal_mouseleave);
	psy_signal_dispose(&component->signal_scroll);
	psy_signal_dispose(&component->signal_create);
	psy_signal_dispose(&component->signal_destroy);
	psy_signal_dispose(&component->signal_show);
	psy_signal_dispose(&component->signal_hide);
	psy_signal_dispose(&component->signal_focus);
	psy_signal_dispose(&component->signal_focuslost);
	psy_signal_dispose(&component->signal_align);
	psy_signal_dispose(&component->signal_preferredsize);
	psy_signal_dispose(&component->signal_windowproc);
	psy_signal_dispose(&component->signal_command);
	if (component->font.hfont && component->font.hfont != defaultfont.hfont) {
		ui_font_dispose(&component->font);
	}
	if (component->background) {
		DeleteObject(component->background);
	}
}

void ui_component_destroy(ui_component* self)
{
	DestroyWindow((HWND)self->hwnd);
}

LRESULT CALLBACK ui_com_winproc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	ui_component*   component;	

	component = psy_table_at(&selfmap, (uintptr_t) hwnd);
	if (component) {		
		switch (message)
		{
			case WM_DESTROY:
				if (component->signal_destroy.slots) {
					psy_signal_emit(&component->signal_destroy, component,
						0);
				}
				ui_component_dispose(component);
				return 0;
			break;
			case WM_TIMER:				
				if (component && component->signal_timer.slots) {
					psy_signal_emit(&component->signal_timer, component, 1,
						(int) wParam);
				}
			case WM_KEYDOWN:				
				if (component->signal_keydown.slots) {
					KeyEvent keyevent;
					
					keyevent_init(&keyevent, (int)wParam, lParam, 
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
						(lParam & 0x40000000) == 0x40000000);
					psy_signal_emit(&component->signal_keydown, component, 1, &keyevent);
				}				
			break;
			case WM_KILLFOCUS:
				if (component->signal_focuslost.slots) {
					psy_signal_emit(&component->signal_focuslost, component, 0);
				}
			break;
		break;		
			default:
			break;
		}
		return CallWindowProc(component->wndproc, hwnd, message, wParam, lParam);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ui_winproc (HWND hwnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{	
    PAINTSTRUCT  ps ;     
	ui_component*   component;
	ui_graphics	 g;
	HMENU		 hMenu;
	ui_menu*	 menu;
	int			 menu_id;		

	component = psy_table_at(&selfmap, (uintptr_t) hwnd);	
	if (component && component->signal_windowproc.slots) {				
		psy_signal_emit(&component->signal_windowproc, component, 3, 
			(LONG)message, (SHORT)LOWORD (lParam), (SHORT)HIWORD (lParam));
		if (component->preventdefault) {					
			return 0;
		} else {
			return DefWindowProc (hwnd, message, wParam, lParam);
		}
	}

	if (component) {
		switch (message)
		{		
			case WM_SHOWWINDOW:							
				if (wParam == TRUE) {
					psy_signal_emit(&component->signal_show, component, 0);
				} else {
					psy_signal_emit(&component->signal_hide, component, 0);
				}
				return 0 ;				
			break;		
			case WM_SIZE:			
				{
					ui_size size;
					if (component->alignchildren == 1) {
						ui_component_align(component);
					}
					size.width = LOWORD(lParam);
					size.height = HIWORD(lParam);
					psy_signal_emit(&component->signal_size, component, 1,
						(void*)&size);
					return 0 ;
				}			
			break;
			case WM_TIMER:			
				if (component->signal_timer.slots) {
					psy_signal_emit(&component->signal_timer, component, 1,
						(int) wParam);				
					return 0 ;
				}
			break;		
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORSTATIC:
			case WM_CTLCOLOREDIT:
				component = psy_table_at(&selfmap, (uintptr_t) lParam);
				if (component) {					
					SetTextColor((HDC) wParam, component->color);
					SetBkColor((HDC) wParam, component->backgroundcolor);
					if ((component->backgroundmode & BACKGROUND_SET) == BACKGROUND_SET) {
						return (intptr_t) component->background;
					} else {
						return (intptr_t) GetStockObject(NULL_BRUSH);
					}
				} else {				
					SetTextColor((HDC) wParam, defaultcolor);
					SetBkColor((HDC) wParam, defaultbackgroundcolor);
					return (intptr_t) defaultbackgroundbrush;
				}
			break;
			case WM_ERASEBKGND:
				return 1;
			break;
			case WM_COMMAND:
			  hMenu = GetMenu (hwnd) ;
			  menu_id = LOWORD (wParam);
			  menu = psy_table_at(&menumap, (uintptr_t) menu_id);
			  if (menu && menu->execute) {	
				menu->execute(menu);
			  }
			  component = psy_table_at(&winidmap, (uintptr_t) LOWORD(wParam));
			  if (component && component->signal_command.slots) {
					psy_signal_emit(&component->signal_command, component, 2, 
						wParam, lParam);
					return 0;
				}
			  return 0 ;  
			break;          
			case WM_CREATE:			
				if (component->signal_create.slots) {	
					psy_signal_emit(&component->signal_create, component, 0);
				}
				return 0 ;
			break;
			case WM_PAINT :			
				if (component->signal_draw.slots ||
						component->backgroundmode != BACKGROUND_NONE) {
					HDC bufferDC;
					HBITMAP bufferBmp;
					HBITMAP oldBmp;
					HDC hdc;				
					RECT rect;
					HFONT hPrevFont = 0;

					hdc = BeginPaint (hwnd, &ps);
					GetClientRect(hwnd, &rect);
					if (component->doublebuffered) {					
						bufferDC = CreateCompatibleDC(hdc);					
						bufferBmp = CreateCompatibleBitmap(hdc, rect.right,
							rect.bottom);
						oldBmp = SelectObject(bufferDC, bufferBmp);					
						ui_graphics_init(&g, bufferDC);
					} else {
						ui_graphics_init(&g, hdc);
					}
					ui_setrectangle(&g.clip,
						ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left,
						ps.rcPaint.bottom - ps.rcPaint.top);				
					if (component->backgroundmode == BACKGROUND_SET) {
						ui_rectangle r;
						ui_setrectangle(&r,
						rect.left, rect.top, rect.right - rect.left,
						rect.bottom - rect.top);				
						ui_drawsolidrectangle(&g, r, component->backgroundcolor);
					}
					if (component->font.hfont) {
						hPrevFont = SelectObject(g.hdc, component->font.hfont);
					} else {
						hPrevFont = SelectObject(g.hdc, defaultfont.hfont);
					}
					psy_signal_emit(&component->signal_draw, component, 1, &g);
					if (hPrevFont) {
						SelectObject(g.hdc, hPrevFont);
					}
					if (component->doublebuffered) {
						g.hdc = hdc;
						BitBlt(hdc, ps.rcPaint.left,ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom,
							bufferDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);				
						SelectObject(bufferDC, oldBmp);
						DeleteObject(bufferBmp);
						DeleteDC(bufferDC);					
					}
					ui_graphics_dispose(&g);
					EndPaint (hwnd, &ps) ;
					return 0 ;
				}
			break;
			case WM_DESTROY:												
				if (component->signal_destroy.slots) {
					psy_signal_emit(&component->signal_destroy, component, 0);
				}
				ui_component_dispose(component);							
				return 0;
			break;
			case WM_SYSKEYDOWN:
				if (wParam >= VK_F10 && wParam <= VK_F12) {
					component->propagateevent = component->defaultpropagation;
					if (component->signal_keydown.slots) {
						KeyEvent keyevent;
						
						keyevent_init(&keyevent, (int)wParam, lParam, 
							GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
							(lParam & 0x40000000) == 0x40000000);
						psy_signal_emit(&component->signal_keydown, component, 1, &keyevent);
					}
					if (component->propagateevent) {					
						SendMessage (GetParent (hwnd), message, wParam, lParam) ;
					}				
					component->propagateevent = component->defaultpropagation;
					return 0;
				}
			break;
			case WM_KEYDOWN:							
				component->propagateevent = component->defaultpropagation;
				if (component->signal_keydown.slots) {
					KeyEvent keyevent;
					
					keyevent_init(&keyevent, (int)wParam, lParam, 
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
						(lParam & 0x40000000) == 0x40000000);
					psy_signal_emit(&component->signal_keydown, component, 1, &keyevent);
				}
				if (component->propagateevent) {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;
				}				
				component->propagateevent = component->defaultpropagation;
				return 0;				
			break;
			case WM_KEYUP:			
				component->propagateevent = component->defaultpropagation;
				if (component->signal_keyup.slots) {
					KeyEvent keyevent;
					
					keyevent_init(&keyevent, (int)wParam, lParam, 
						GetKeyState(VK_SHIFT) < 0, GetKeyState(VK_CONTROL) < 0,
						(lParam & 0x40000000) == 0x40000000);
					psy_signal_emit(&component->signal_keyup, component, 1, &keyevent);
				}
				if (component->propagateevent) {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;
				}				
				component->propagateevent = component->defaultpropagation;
				return 0;
			break;
			case WM_LBUTTONUP:			
				if (component->signal_mouseup.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_LBUTTON, 0);
					psy_signal_emit(&component->signal_mouseup, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_RBUTTONUP:			
				if (component->signal_mouseup.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_RBUTTON, 0);
					psy_signal_emit(&component->signal_mouseup, component, 1,
						&mouseevent);
					return 0 ;
				}			
			break;
			case WM_MBUTTONUP:			
				if (component->signal_mouseup.slots) {			
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_MBUTTON, 0);
					psy_signal_emit(&component->signal_mouseup, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_LBUTTONDOWN:			
				if (component->signal_mousedown.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_LBUTTON, 0);
					psy_signal_emit(&component->signal_mousedown, component, 1,
						&mouseevent);
					return 0 ;
				}			
			break;
			case WM_RBUTTONDOWN:			
				if (component->signal_mousedown.slots) {		
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_RBUTTON, 0);
					psy_signal_emit(&component->signal_mousedown, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_MBUTTONDOWN:			
				if (component->signal_mousedown.slots) {		
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_MBUTTON, 0);
					psy_signal_emit(&component->signal_mousedown, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_LBUTTONDBLCLK:							
				component->propagateevent = component->defaultpropagation;
				if (component->signal_mousedoubleclick.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_LBUTTON, 0);					
					psy_signal_emit(&component->signal_mousedoubleclick, component, 1,
						&mouseevent);
				}
				if (component->propagateevent) {					
					SendMessage (GetParent (hwnd), message, wParam, lParam) ;               
				}				
				component->propagateevent = component->defaultpropagation;
				return 0;
			break;
			case WM_MBUTTONDBLCLK:			
				if (component->signal_mousedoubleclick.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_MBUTTON, 0);
					psy_signal_emit(&component->signal_mousedoubleclick, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;		
			case WM_RBUTTONDBLCLK:			
				if (component->signal_mousedoubleclick.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), MK_RBUTTON, 0);
					psy_signal_emit(&component->signal_mousedoubleclick, component, 1,
						&mouseevent);
					return 0;
				}
			break;
			case WM_MOUSEMOVE:
				if (!mousetracking) {
					TRACKMOUSEEVENT tme;

					if (component && component->signal_mouseenter.slots) {	
						psy_signal_emit(&component->signal_mouseenter, component, 0);
					}
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE | TME_HOVER;
					tme.dwHoverTime = 200;
					tme.hwndTrack = hwnd;
					if (_TrackMouseEvent(&tme)) {
						mousetracking = 1;
					} 
					return 0;
				}								
				if (component->signal_mousemove.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), wParam, 0);
					psy_signal_emit(&component->signal_mousemove, component, 1,
						&mouseevent);
					return 0 ;
				}
			break;
			case WM_SETTINGCHANGE:
			{
				static int ulScrollLines;

				SystemParametersInfo (SPI_GETWHEELSCROLLLINES, 0, &ulScrollLines, 0) ;
      
			   // ulScrollLines usually equals 3 or 0 (for no scrolling)
			   // WHEEL_DELTA equals 120, so iDeltaPerLine will be 40
				if (ulScrollLines)
					iDeltaPerLine = WHEEL_DELTA / ulScrollLines ;
				else
					iDeltaPerLine = 0 ;
			}
			return 0 ;
			break;          
			case WM_MOUSEWHEEL:
				if (component->signal_mousewheel.slots) {
					MouseEvent mouseevent;

					mouseevent_init(&mouseevent, (SHORT)LOWORD (lParam), 
						(SHORT)HIWORD (lParam), LOWORD(wParam), HIWORD(wParam));
					psy_signal_emit(&component->signal_mousewheel, component, 1,
						&mouseevent);
				} else
				if (component->wheelscroll > 0) {
					if (iDeltaPerLine != 0) {
						component->accumwheeldelta += (short) HIWORD (wParam); // 120 or -120
						while (component->accumwheeldelta >= iDeltaPerLine)
						{           
							int iPos;
							int scrollmin;
							int scrollmax;

							ui_component_verticalscrollrange(component, &scrollmin,
								&scrollmax);							
							iPos = ui_component_verticalscrollposition(component) - 
								component->wheelscroll;
							if (iPos < scrollmin) {
								iPos = scrollmin;
							}
							SendMessage((HWND) component->hwnd, 
								WM_VSCROLL,
								MAKELONG(SB_THUMBTRACK, iPos), 0);
							component->accumwheeldelta -= iDeltaPerLine ;							
						}				
						while (component->accumwheeldelta <= -iDeltaPerLine)
						{
							int iPos;
							int scrollmin;
							int scrollmax;

							ui_component_verticalscrollrange(component, &scrollmin,
								&scrollmax);
							iPos = ui_component_verticalscrollposition(component) + 
								component->wheelscroll;
							if (iPos > scrollmax) {
								iPos = scrollmax;
							}
							SendMessage((HWND) component->hwnd, WM_VSCROLL,
								MAKELONG(SB_THUMBTRACK, iPos), 0);							
							component->accumwheeldelta += iDeltaPerLine;							
						}
					}
				}
			break;
			case WM_MOUSEHOVER:			
				if (component->signal_mousehover.slots) {	                    
					psy_signal_emit(&component->signal_mousehover, component, 0);
					return 0;
				}
			break;
			case WM_MOUSELEAVE:	
				mousetracking = 0;
				if (component->signal_mouseleave.slots) {				                    
					psy_signal_emit(&component->signal_mouseleave, component, 0);
					return 0;
				}			
			break;
			case WM_VSCROLL:
				handle_vscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_HSCROLL:
				component = psy_table_at(&selfmap, (uintptr_t) (int) lParam);
				if (component && component->signal_windowproc.slots) {				                    
					psy_signal_emit(&component->signal_windowproc, component, 3, message, wParam, lParam);
					return DefWindowProc (hwnd, message, wParam, lParam);
				}
				handle_hscroll(hwnd, wParam, lParam);
				return 0;
			break;
			case WM_KILLFOCUS:
				if (component->signal_focuslost.slots) {
					psy_signal_emit(&component->signal_focuslost, component, 0);
					return 0;
				}
			break;
			default:			
			break;
		}	
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

void handle_vscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; //, iHorzPos;	
	ui_component*   component;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_VERT, &si) ;	
	// Save the position for comparison later on
	iPos = si.nPos ;

	handle_scrollparam(&si, wParam);	
	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.
	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_VERT, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_VERT, &si) ;
	// If the position has changed, scroll the window and update it
	if (si.nPos != iPos)
	{                    
		component = psy_table_at(&selfmap, (uintptr_t) hwnd);
		if (component && component->signal_scroll.slots) {
			psy_signal_emit(&component->signal_scroll, component, 2, 
				0, (iPos - si.nPos));			
		}
		if (component->handlevscroll) {
			ui_component_scrollstep(component, 0, (iPos - si.nPos));
		}
	}
}

void ui_component_scrollstep(ui_component* self, intptr_t stepx, intptr_t stepy)
{
	ScrollWindow ((HWND)self->hwnd,
		self->scrollstepx * stepx,
		self->scrollstepy * stepy, 
		NULL, NULL) ;
	UpdateWindow ((HWND)self->hwnd);
}

void handle_hscroll(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO		si;	
    int				iPos; 
	ui_component*   component;
     
	si.cbSize = sizeof (si) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo (hwnd, SB_HORZ, &si) ;	

	// Save the position for comparison later on
	iPos = si.nPos ;
	handle_scrollparam(&si, wParam);
	// Set the position and then retrieve it.  Due to adjustments
	// by Windows it may not be the same as the value set.
	si.fMask = SIF_POS ;
	SetScrollInfo (hwnd, SB_HORZ, &si, TRUE) ;
	GetScrollInfo (hwnd, SB_HORZ, &si) ;

	// If the position has changed, scroll the window and update it

	if (si.nPos != iPos)
	{                    
		component = psy_table_at(&selfmap, (uintptr_t) hwnd);
		if (component && component->signal_scroll.slots) {
			psy_signal_emit(&component->signal_scroll, component, 2, 
				(iPos - si.nPos), 0);			
		}
		if (component->handlehscroll) {
			ui_component_scrollstep(component, (iPos - si.nPos), 0);
		}
	}
}

void handle_scrollparam(SCROLLINFO* si, WPARAM wParam)
{
	switch (LOWORD (wParam)) {
		case SB_TOP:
		   si->nPos = si->nMin ;
		break ;
		case SB_BOTTOM:
		   si->nPos = si->nMax ;
		break ;
		case SB_LINEUP:
		   si->nPos -= 1 ;
		break ;
		case SB_LINEDOWN:
		   si->nPos += 1 ;
		break ;
		case SB_PAGEUP:
		   si->nPos -= si->nPage ;
		break ;
		case SB_PAGEDOWN:
		   si->nPos += si->nPage ;
		break ;
		case SB_THUMBTRACK:
		   si->nPos = HIWORD(wParam);
		break ;
		default:
		break ;         
	}
}

ui_size ui_component_size(ui_component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetClientRect((HWND)self->hwnd, &rect);
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

ui_rectangle ui_component_position(ui_component* self)
{   
	ui_rectangle rv;
	RECT rc;
	POINT pt;	
	int width;
	int height;	
	    	
    GetWindowRect((HWND)self->hwnd, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(GetParent((HWND)self->hwnd), &pt);
	rv.left = pt.x;
	rv.top = pt.y;
	rv.right =  pt.x + width;
	rv.bottom = pt.y + height;
	return rv;
}

ui_size ui_component_frame_size(ui_component* self)
{   
	ui_size rv;
	RECT rect ;
	    
    GetWindowRect((HWND)self->hwnd, &rect) ;
	rv.width = rect.right;
	rv.height = rect.bottom;
	return rv;
}

void ui_component_show_state(ui_component* self, int cmd)
{
	ShowWindow((HWND)self->hwnd, cmd);
	UpdateWindow((HWND)self->hwnd) ;
}

void ui_component_show(ui_component* self)
{
	self->visible = 1;
	ShowWindow((HWND)self->hwnd, SW_SHOW);
	UpdateWindow((HWND)self->hwnd) ;
}

void ui_component_hide(ui_component* self)
{
	self->visible = 0;
	ShowWindow((HWND)self->hwnd, SW_HIDE);
	UpdateWindow((HWND)self->hwnd) ;
}

void ui_component_showhorizontalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) | WS_HSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) | WS_HSCROLL);
#endif
}


void ui_component_hidehorizontalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) & ~WS_HSCROLL);
#endif
}

void ui_component_sethorizontalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = min;
	si.nMax = max;
	si.fMask = SIF_RANGE;	
	SetScrollInfo((HWND)self->hwnd, SB_HORZ, &si, TRUE);
}

void ui_component_showverticalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) | WS_VSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) | WS_VSCROLL);
#endif
}

void ui_component_hideverticalscrollbar(ui_component* self)
{
#if defined(_WIN64)
	SetWindowLongPtr((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLongPtr((HWND)self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#else
	SetWindowLong((HWND)self->hwnd, GWL_STYLE, 
		GetWindowLong((HWND)self->hwnd, GWL_STYLE) & ~WS_VSCROLL);
#endif
}

void ui_component_setverticalscrollrange(ui_component* self, int min, int max)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.nMin = max(0, min);
	si.nMax = max(si.nMin, max);
	si.fMask = SIF_RANGE;	
	SetScrollInfo((HWND)self->hwnd, SB_VERT, &si, TRUE);
}

int ui_component_verticalscrollposition(ui_component* self)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;	
	GetScrollInfo((HWND)self->hwnd, SB_VERT, &si);	
	return si.nPos;
}

void ui_component_setverticalscrollposition(ui_component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo((HWND)self->hwnd, SB_VERT, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo((HWND)self->hwnd, SB_VERT, &si, TRUE);
}

int ui_component_horizontalscrollposition(ui_component* self)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;	
	GetScrollInfo((HWND)self->hwnd, SB_HORZ, &si);	
	return si.nPos;
}

void ui_component_sethorizontalscrollposition(ui_component* self, int position)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo((HWND)self->hwnd, SB_HORZ, &si);
	if (position < si.nMax) {			
		si.nPos = (int) position;
	} else {
		si.nPos = si.nMax;
	}
	si.fMask = SIF_POS;	
	SetScrollInfo((HWND)self->hwnd, SB_HORZ, &si, TRUE);
}

void ui_component_verticalscrollrange(ui_component* self, int* scrollmin,
	int* scrollmax)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE;	
	GetScrollInfo((HWND)self->hwnd, SB_VERT, &si);
	*scrollmin = si.nMin;
	*scrollmax = si.nMax;
}

void ui_component_move(ui_component* self, int left, int top)
{
	SetWindowPos((HWND)self->hwnd, NULL, 
	   left, top,
	   0, 0,
	   SWP_NOZORDER | SWP_NOSIZE) ;	
}

void ui_component_resize(ui_component* self, int width, int height)
{	
	SetWindowPos((HWND)self->hwnd, NULL, 
	   0, 0,
	   width, height,
	   SWP_NOZORDER | SWP_NOMOVE);	
}

void ui_component_setposition(ui_component* self, int x, int y, int width, int height)
{	
	SetWindowPos((HWND)self->hwnd, 0, x, y, width, height, SWP_NOZORDER);	
}

void ui_component_setmenu(ui_component* self, ui_menu* menu)
{
	SetMenu((HWND)self->hwnd, menu->hmenu);
}

void ui_component_settitle(ui_component* self, const char* title)
{
	SetWindowText((HWND)self->hwnd, title);
}

void ui_component_enumerate_children(ui_component* self, void* context, 
	int (*childenum)(void*, void*))
{	
	EnumCallback callback;
	
	callback.context = context;
	callback.childenum = childenum;
	EnumChildWindows((HWND)self->hwnd, ChildEnumProc, (LPARAM) &callback);
}

BOOL CALLBACK ChildEnumProc (HWND hwnd, LPARAM lParam)
{
	EnumCallback* callback = (EnumCallback*) lParam;
	ui_component* child = psy_table_at(&selfmap, (uintptr_t) hwnd);
	if (child &&  callback->childenum) {
		return callback->childenum(callback->context, child);		  
	}     
    return FALSE ;
}

psy_List* ui_component_children(ui_component* self, int recursive)
{	
	psy_List* children = 0;	
	if (recursive == 1) {
		EnumChildWindows ((HWND)self->hwnd, AllChildEnumProc, (LPARAM) &children);
	} else {
		uintptr_t hwnd = (uintptr_t)GetWindow((HWND)self->hwnd, GW_CHILD);
		if (hwnd) {
			ui_component* child = psy_table_at(&selfmap, hwnd);
			if (child) {				
				children = psy_list_create(child);				
			}
		}
		while (hwnd) {
			hwnd = (uintptr_t) GetNextWindow((HWND)hwnd, GW_HWNDNEXT);
			if (hwnd) {
				ui_component* child = psy_table_at(&selfmap, hwnd);
				if (child) {					
					psy_list_append(&children, child);							
				}
			}
		}
	}
	return children;
}

BOOL CALLBACK AllChildEnumProc (HWND hwnd, LPARAM lParam)
{
	psy_List** pChildren = (psy_List**) lParam;
	ui_component* child = psy_table_at(&selfmap, (uintptr_t) hwnd);
	if (child) {		
		psy_list_append(pChildren, child);				
	}     
    return TRUE;
}

void ui_component_capture(ui_component* self)
{
	SetCapture((HWND)self->hwnd);
}

void ui_component_releasecapture()
{
	ReleaseCapture();
}

void ui_component_invalidate(ui_component* self)
{
	InvalidateRect((HWND)self->hwnd, NULL, FALSE);
}

void ui_component_invalidaterect(ui_component* self, const ui_rectangle* r)
{
	RECT rc;

	rc.left = r->left;
	rc.top = r->top;
	rc.right = r->right;
	rc.bottom = r->bottom;
	InvalidateRect((HWND)self->hwnd, &rc, FALSE);
}

void ui_component_update(ui_component* self)
{
	UpdateWindow((HWND)self->hwnd);
}

void ui_component_setfocus(ui_component* self)
{
	SetFocus((HWND)self->hwnd);
	psy_signal_emit(&self->signal_focus, self, 0);
}

int ui_component_hasfocus(ui_component* self)
{
	return (HWND) self->hwnd == GetFocus();
}

void ui_component_setfont(ui_component* self, ui_font* source)
{
	ui_font font;

	font.hfont = 0;
	font.stock = 0;
	if (source && source->hfont && source->hfont != defaultfont.hfont) {
		ui_font_init(&font, 0);
		ui_font_copy(&font, source);
	} else {
		font.hfont = defaultfont.hfont;
	}	
	SendMessage((HWND)self->hwnd, WM_SETFONT, (WPARAM) font.hfont, 0);	
	if (self->font.hfont && self->font.hfont != defaultfont.hfont) {
		ui_font_dispose(&self->font);		
	}
	self->font = font;
}

void ui_component_propagateevent(ui_component* self)
{
	self->propagateevent = 1;
}

void ui_component_preventdefault(ui_component* self)
{
	self->preventdefault = 1;
}

int ui_component_visible(ui_component* self)
{
	return IsWindowVisible((HWND) self->hwnd);
}

void ui_component_align(ui_component* self)
{	
	ui_size size;
	ui_textmetric tm;
	ui_point cp_topleft = { 0, 0 };
	ui_point cp_bottomright = { 0, 0 };	
	int cpymax = 0;
	psy_List* p;
	psy_List* q;
	psy_List* wrap = 0;	
	ui_component* client = 0;
		
	size = ui_component_size(self);
	tm = ui_component_textmetric(self);
	cp_bottomright.x = size.width;
	cp_bottomright.y = size.height;
	for (p = q = ui_component_children(self, 0); p != 0; p = p->next) {
		ui_component* component;
			
		component = (ui_component*)p->entry;		
		if (component->visible) {
			ui_size componentsize;			
			componentsize = ui_component_preferredsize(component, &size);

			if (component->align == UI_ALIGN_CLIENT) {
				client = component;
			} 
			if (component->align == UI_ALIGN_FILL) {
				ui_component_setposition(component,
					ui_value_px(&component->margin.left, &tm),
					ui_value_px(&component->margin.top, &tm),				
					size.width - ui_value_px(&component->margin.left, &tm) -
						ui_value_px(&component->margin.right, &tm),
					size.height -
						ui_margin_height_px(&component->margin, &tm));
			} else
			if (component->align == UI_ALIGN_TOP) {
				cp_topleft.y += ui_value_px(&component->margin.top, &tm);
				ui_component_setposition(component, 
					cp_topleft.x + ui_value_px(&component->margin.left, &tm), 
					cp_topleft.y,
					cp_bottomright.x - cp_topleft.x -
						ui_margin_width_px(&component->margin, &tm),
					componentsize.height);
				cp_topleft.y += ui_value_px(&component->margin.bottom, &tm);
				cp_topleft.y += componentsize.height;
			} else
			if (component->align == UI_ALIGN_BOTTOM) {
				cp_bottomright.y -=
					ui_value_px(&component->margin.bottom, &tm);
				ui_component_setposition(component, 
					cp_topleft.x + ui_value_px(&component->margin.left, &tm), 
					cp_bottomright.y - componentsize.height,
					cp_bottomright.x - cp_topleft.x -
						ui_margin_width_px(&component->margin, &tm),						
					componentsize.height);
				cp_bottomright.y -= ui_value_px(&component->margin.top, &tm);
				cp_bottomright.y -= componentsize.height;
			} else
			if (component->align == UI_ALIGN_RIGHT) {
				int requiredcomponentwidth;

				requiredcomponentwidth = componentsize.width +
					ui_margin_width_px(&component->margin, &tm);
				cp_bottomright.x -= requiredcomponentwidth;
				ui_component_setposition(component,
					cp_bottomright.x + ui_value_px(&component->margin.left, &tm),
					cp_topleft.y +
						ui_value_px(&component->margin.top, &tm),
					componentsize.width,										
					size.height -
						ui_margin_height_px(&component->margin, &tm));
			} else
			if (component->align == UI_ALIGN_LEFT) {
				if ((self->alignexpandmode & UI_HORIZONTALEXPAND)
						== UI_HORIZONTALEXPAND) {
				} else {
					int requiredcomponentwidth;

					requiredcomponentwidth = componentsize.width +
						ui_margin_width_px(&component->margin, &tm);
					if (cp_topleft.x + requiredcomponentwidth > size.width) {
						psy_List* w;						
						cp_topleft.x = 0;
						for (w = wrap; w != 0; w = w->next) {
							ui_component* c;
							c = (ui_component*)w->entry;
							ui_component_resize(c, ui_component_size(c).width,
								cpymax - cp_topleft.y -
								ui_margin_height_px(&component->margin, &tm));
						}
						cp_topleft.y = cpymax;
						psy_list_free(wrap);						
						wrap = 0;
					}					
					psy_list_append(&wrap, component);					
				}
				cp_topleft.x += ui_value_px(&component->margin.left, &tm);
				ui_component_setposition(component,
					cp_topleft.x,
					cp_topleft.y + ui_value_px(&component->margin.top, &tm),
					componentsize.width,
					component->justify == UI_JUSTIFY_EXPAND 
					? cp_bottomright.y - cp_topleft.y - 
						ui_margin_height_px(&component->margin, &tm)
					: componentsize.height);
				cp_topleft.x += ui_value_px(&component->margin.right, &tm);
				cp_topleft.x += componentsize.width;				
				if (cpymax < cp_topleft.y + componentsize.height +
						ui_margin_height_px(&component->margin, &tm)) {
					cpymax = cp_topleft.y + componentsize.height +
						ui_margin_height_px(&component->margin, &tm);
				}
			}				
		}
	}
	if (client) {		
		ui_component_setposition(client,
			cp_topleft.x + ui_value_px(&client->margin.left, &tm),
			cp_topleft.y + ui_value_px(&client->margin.top, &tm),
			cp_bottomright.x - cp_topleft.x -
				ui_margin_width_px(&client->margin, &tm),
			cp_bottomright.y - cp_topleft.y -
				ui_margin_height_px(&client->margin, &tm));
	}
	psy_list_free(q);
	psy_list_free(wrap);
	psy_signal_emit(&self->signal_align, self, 0);
}

void onpreferredsize(ui_component* self, ui_component* sender, ui_size* limit,
	ui_size* rv)
{			
	if (rv) {
		ui_size size;		

		size = ui_component_size(self);		
		if (self->alignchildren) {			
			ui_textmetric tm;			
			psy_List* p;
			psy_List* q;
			ui_point cp = { 0, 0 };
			ui_size maxsize = { 0, 0 };

			tm = ui_component_textmetric(self);			
			size.width = (self->alignexpandmode & UI_HORIZONTALEXPAND) ==
				UI_HORIZONTALEXPAND 
				? 0
				: limit->width;			
			for (p = q = ui_component_children(self, 0); p != 0; p = p->next) {
				ui_component* component;
					
				component = (ui_component*)p->entry;		
				if (component->visible) {
					ui_size componentsize;			
					
					componentsize = ui_component_preferredsize(component, &size);
					if (component->align == UI_ALIGN_TOP ||
							component->align == UI_ALIGN_BOTTOM) {
						cp.y += componentsize.height +
							ui_margin_height_px(&component->margin, &tm);
						if (maxsize.height < cp.y) {
							maxsize.height = cp.y;
						}
						if (maxsize.width < componentsize.width +
								ui_margin_width_px(&component->margin, &tm)) {
							maxsize.width = componentsize.width +
								ui_margin_width_px(&component->margin, &tm);
						}
					} else					
					if (component->align == UI_ALIGN_LEFT) {					
						if (size.width != 0) {
							int requiredcomponentwidth;

							requiredcomponentwidth = componentsize.width +
								ui_margin_width_px(&component->margin, &tm);
							if (cp.x + requiredcomponentwidth > size.width) {
								cp.y = maxsize.height;
								cp.x = 0;							
							}						
						}
						cp.x += componentsize.width +
							ui_margin_width_px(&component->margin, &tm);						
						if (maxsize.width < cp.x) {
							maxsize.width = cp.x;
						}
						if (maxsize.height < cp.y + componentsize.height +
								ui_margin_height_px(&component->margin, &tm)) {
							maxsize.height = cp.y + componentsize.height +
								ui_margin_height_px(&component->margin, &tm);
						}
					}				
				}
			}
			psy_list_free(q);
			*rv = maxsize;
		} else {
			*rv = size;
		}
	}	
}

void ui_component_setmargin(ui_component* self, const ui_margin* margin)
{	
	if (margin) {
		self->margin = *margin;		
	} else {
		memset(&self->margin, 0, sizeof(ui_margin));
	}
}

void ui_component_setspacing(ui_component* self, const ui_margin* spacing)
{	
	if (spacing) {
		self->spacing = *spacing;
	} else {
		memset(&self->spacing, 0, sizeof(ui_margin));
	}
}

void ui_component_setalign(ui_component* self, UiAlignType align)
{
	self->align = align;
}

void ui_component_enablealign(ui_component* self)
{
	self->alignchildren = 1;	
}

void ui_component_setalignexpand(ui_component* self, UiExpandMode mode)
{
	self->alignexpandmode = mode;
}

void ui_component_preventalign(ui_component* self)
{
	self->alignchildren = 0;
}

void ui_component_enableinput(ui_component* self, int recursive)
{
	enableinput(self, TRUE, recursive);
}

void ui_component_preventinput(ui_component* self, int recursive)
{
	enableinput(self, FALSE, recursive);
}

void enableinput(ui_component* self, int enable, int recursive)
{	
	EnableWindow((HWND) self->hwnd, enable);
	if (recursive) {
		psy_List* p;
		psy_List* q;
		
		for (p = q = ui_component_children(self, recursive); p != 0; p = p->next) {
			EnableWindow((HWND)((ui_component*)(p->entry))->hwnd, enable);
		}
		psy_list_free(q);
	}
}

int ui_openfile(ui_component* self, char* szTitle, char* szFilter,
	char* szDefExtension, const char* szInitialDir, char* szOpenName)
{
	int rv;
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	*szOpenName = '\0'; 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner= (HWND) self->hwnd; 
	ofn.lpstrFilter= szFilter;	
	ofn.lpstrCustomFilter= (LPSTR)NULL; 
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= szOpenName; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle= szTitle; 
	ofn.nMaxFileTitle= MAX_PATH; 
	ofn.lpstrTitle= (LPSTR)NULL; 
	ofn.lpstrInitialDir= (LPSTR) szInitialDir; 
	ofn.Flags= OFN_HIDEREADONLY|OFN_FILEMUSTEXIST; 
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= szDefExtension;
	rv = GetOpenFileName(&ofn);
	InvalidateRect(appMainComponentHandle, 0, FALSE);
	UpdateWindow(appMainComponentHandle);
	return rv;
}

int ui_savefile(ui_component* self, char* szTitle, char* szFilter,
	char* szDefExtension, const char* szInitialDir, char* szFileName)
{
	int rv;
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	*szFileName = '\0'; 
	ofn.lStructSize= sizeof(OPENFILENAME); 
	ofn.hwndOwner= (HWND) self->hwnd; 
	ofn.lpstrFilter= szFilter;	
	ofn.lpstrCustomFilter= (LPSTR)NULL; 
	ofn.nMaxCustFilter= 0L; 
	ofn.nFilterIndex= 1L; 
	ofn.lpstrFile= szFileName; 
	ofn.nMaxFile= MAX_PATH; 
	ofn.lpstrFileTitle= szTitle; 
	ofn.nMaxFileTitle= _MAX_PATH; 
	ofn.lpstrTitle= (LPSTR)NULL; 
	ofn.lpstrInitialDir= (LPSTR) szInitialDir;
	ofn.Flags= OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset= 0; 
	ofn.nFileExtension= 0; 
	ofn.lpstrDefExt= szDefExtension;
	InvalidateRect(appMainComponentHandle, 0, FALSE);
	UpdateWindow(appMainComponentHandle);
	rv = GetSaveFileName(&ofn);
	return rv;
}
	
void ui_component_setbackgroundmode(ui_component* self, BackgroundMode mode)
{
	self->backgroundmode = mode;	
}

void ui_component_setbackgroundcolor(ui_component* self, unsigned int color)
{
	self->backgroundcolor = color;
	if (self->background) {
		DeleteObject(self->background);
	}
	self->background = CreateSolidBrush(color);
}

void ui_component_setcolor(ui_component* self, unsigned int color)
{
	self->color = color;
}

ui_size ui_component_textsize(ui_component* self, const char* text)
{
	ui_size rv;
	ui_graphics g;
	HFONT hPrevFont = 0;
	HDC hdc;
	
	hdc = GetDC((HWND)self->hwnd);	
    SaveDC (hdc) ;          
	ui_graphics_init(&g, hdc);
	if (self->font.hfont) {
		hPrevFont = SelectObject(hdc, self->font.hfont);
	}
	rv = ui_textsize(&g, text);
	if (hPrevFont) {
		SelectObject(hdc, hPrevFont);
	}
	ui_graphics_dispose(&g);
	RestoreDC (hdc, -1);	
	ReleaseDC((HWND)self->hwnd, hdc);
	return rv;
}

ui_component* ui_component_parent(ui_component* self)
{			
	return (ui_component*) psy_table_at(&selfmap, 
		(uintptr_t) GetParent((HWND)self->hwnd));
}

psy_List* ui_components_setalign(psy_List* list, UiAlignType align, const ui_margin* margin)
{
	psy_List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setalign((ui_component*)p->entry, align);
		if (margin) {
			ui_component_setmargin((ui_component*)p->entry, margin);
		}
	}
	return list;
}

psy_List* ui_components_setmargin(psy_List* list, const ui_margin* margin)
{
	psy_List* p;

	for (p = list; p != 0; p = p->next) {
		ui_component_setmargin((ui_component*)p->entry, margin);		
	}
	return list;
}

ui_size ui_component_preferredsize(ui_component* self, ui_size* limit)
{
	ui_size rv;	
	psy_signal_emit(&self->signal_preferredsize, self, 2, limit, &rv);	
	return rv;	
}

ui_textmetric ui_component_textmetric(ui_component* self)
{			
	TEXTMETRIC tm;
	HDC hdc;		
	HFONT hPrevFont = 0;	
	
	hdc = GetDC((HWND)self->hwnd);	
    SaveDC(hdc) ;          	
	if (self->font.hfont) {
		hPrevFont = SelectObject(hdc, self->font.hfont);
	}
	GetTextMetrics (hdc, &tm);
	if (hPrevFont) {
		SelectObject(hdc, hPrevFont);
	}	
	RestoreDC(hdc, -1);	
	ReleaseDC((HWND)self->hwnd, hdc);
	return tm;
}

void ui_component_seticonressource(ui_component* self, int ressourceid)
{
#if defined(_WIN64)	
	SetClassLongPtr((HWND)self->hwnd, GCLP_HICON, 
		(intptr_t)LoadIcon(appInstance, MAKEINTRESOURCE(ressourceid)));
#else	
	SetClassLong((HWND)self->hwnd, GCL_HICON, 
		(intptr_t)LoadIcon(appInstance, MAKEINTRESOURCE(ressourceid)));
#endif
}

ui_component* ui_maincomponent(void)
{
	return psy_table_at(&selfmap, (uintptr_t) appMainComponentHandle);
}

void ui_component_starttimer(ui_component* self, unsigned int id,
	unsigned int interval)
{
	SetTimer((HWND)self->hwnd, id, interval, 0);
}

void ui_component_stoptimer(ui_component* self, unsigned int id)
{
	KillTimer((HWND)self->hwnd, id);
}

int ui_browsefolder(ui_component* self, const char* title, char* path)
{

	///\todo: alternate browser window for Vista/7: http://msdn.microsoft.com/en-us/library/bb775966%28v=VS.85%29.aspx
	// SHCreateItemFromParsingName(
	int val= 0;
	
	LPMALLOC pMalloc;
	// Gets the Shell's default allocator
	//
	path[0] = '\0';
	if (SHGetMalloc(&pMalloc) == NOERROR)
	{
		char pszBuffer[MAX_PATH];		
		BROWSEINFO bi;
		LPITEMIDLIST pidl;

		pszBuffer[0]='\0';
		// Get help on BROWSEINFO struct - it's got all the bit settings.
		//
		bi.hwndOwner = (HWND) self->hwnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = pszBuffer;
		bi.lpszTitle = title;
#if defined _MSC_VER > 1200
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
#else
		bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
#endif
		bi.lpfn = NULL;
		bi.lParam = 0;
		// This next call issues the dialog box.
		//
		if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
			if (SHGetPathFromIDList(pidl, pszBuffer)) {
				// At this point pszBuffer contains the selected path
				//
				val = 1;
				psy_snprintf(path, MAX_PATH, "%s", pszBuffer);
				path[MAX_PATH - 1] = '\0';				
			}
			// Free the PIDL allocated by SHBrowseForFolder.
			//
			pMalloc->lpVtbl->Free(pMalloc, pidl);
		}
		// Release the shell's allocator.
		//
		pMalloc->lpVtbl->Release(pMalloc);
	}
	return val;
}