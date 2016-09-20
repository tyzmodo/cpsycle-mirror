// #include "stdafx.h"

#include "MfcUi.hpp"
#include "Mmsystem.h"
#include "resources/resources.hpp"
// #include "Resource.h"

namespace psycle {
namespace host {
namespace ui {
namespace mfc {


void GraphicsImp::DevFillRegion(const ui::Region& rgn) {    
  check_pen_update();
  check_brush_update();        
  mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn.imp());
  assert(imp);
  ::FillRgn(cr_->m_hDC, imp->crgn(), brush);  
}

void GraphicsImp::DevSetClip(ui::Region* rgn) {
  if (rgn) {
    mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(rgn->imp());
    assert(imp);
    cr_->SelectClipRgn(&imp->crgn());
  } else {
    cr_->SelectClipRgn(0);
  }  
}

RegionImp::RegionImp() { 
	rgn_.CreateRectRgn(0, 0, 0, 0); 
}
  
RegionImp::RegionImp(const CRgn& rgn) {
  assert(rgn.m_hObject);
  rgn_.CreateRectRgn(0, 0, 0, 0);
  rgn_.CopyRgn(&rgn);
}

RegionImp::~RegionImp() {
  rgn_.DeleteObject();
}

RegionImp* RegionImp::DevClone() const {
  return new RegionImp(rgn_);  
}
  
void RegionImp::DevOffset(double dx, double dy) {
  CPoint pt(static_cast<int>(dx), static_cast<int>(dy));
  rgn_.OffsetRgn(pt);
}

int RegionImp::DevCombine(const ui::Region& other, int combinemode) {
  mfc::RegionImp* imp = dynamic_cast<mfc::RegionImp*>(other.imp());
  assert(imp);
  return rgn_.CombineRgn(&rgn_, &imp->crgn(), combinemode);
} 

ui::Rect RegionImp::DevBounds() const { 
  CRect rc;
  rgn_.GetRgnBox(&rc);
  return ui::Rect(ui::Point(rc.left, rc.top), ui::Point(rc.right, rc.bottom));
}

bool RegionImp::DevIntersect(double x, double y) const {
  return rgn_.PtInRegion(static_cast<int>(x), static_cast<int>(y)) != 0;
}

bool RegionImp::DevIntersectRect(const ui::Rect& rect) const {  
  return rgn_.RectInRegion(TypeConverter::rect(rect)) != 0;  
}
  
void RegionImp::DevSetRect(const ui::Rect& rect) {
  rgn_.SetRectRgn(TypeConverter::rect(rect)); 
}

void RegionImp::DevClear() {
  DevSetRect(ui::Rect());  
}

void ImageImp::DevReset(const ui::Dimension& dimension) {
	Dispose();
	bmp_ = new CBitmap();
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	bmp_->CreateCompatibleBitmap(&dc, static_cast<int>(dimension.width()), static_cast<int>(dimension.height()));
	::ReleaseDC(NULL, dc);
}

ui::Graphics* ImageImp::dev_graphics() {
	if (!paint_graphics_.get()) {		
		CDC* memDC = new CDC();
        memDC->CreateCompatibleDC(NULL);
		paint_graphics_.reset(new ui::Graphics(memDC));
		memDC->SelectObject(bmp_);	
	}
	return paint_graphics_.get();
}
  
std::map<HWND, ui::WindowImp*> WindowHook::windows_;
HHOOK WindowHook::_hook = 0;
int WindowID::id_counter = ID_DYNAMIC_CONTROLS_BEGIN;
CWnd DummyWindow::dummy_wnd_;

#define BEGIN_TEMPLATE_MESSAGE_MAP2(theClass, type_name, type_name2, baseClass)			\
	PTM_WARNING_DISABLE														\
	template < typename type_name, typename type_name2 >											\
	const AFX_MSGMAP* theClass< type_name, type_name2 >::GetMessageMap() const			\
		{ return GetThisMessageMap(); }										\
	template < typename type_name, typename type_name2 >											\
	const AFX_MSGMAP* PASCAL theClass< type_name, type_name2 >::GetThisMessageMap()		\
	{																		\
		typedef theClass< type_name, type_name2 > ThisClass;							\
		typedef baseClass TheBaseClass;										\
		static const AFX_MSGMAP_ENTRY _messageEntries[] =					\
		{

// CanvasView
BEGIN_TEMPLATE_MESSAGE_MAP2(WindowTemplateImp, T, I, T)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  //ON_WM_SETFOCUS()
  ON_WM_KILLFOCUS()
	ON_WM_PAINT()
  ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()	
  ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
  ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEHOVER()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
  ON_WM_KEYDOWN()
	ON_WM_KEYUP()
  ON_WM_SETCURSOR()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_SIZE()  
  ON_WM_MOUSEACTIVATE()	
END_MESSAGE_MAP()

template<class T, class I>
BOOL WindowTemplateImp<T, I>::PreTranslateMessage(MSG* pMsg) {  		
  if (pMsg->message==WM_KEYDOWN ) {
    UINT nFlags = 0;
    UINT flags = Win32KeyFlags(nFlags);      
    KeyEvent ev(pMsg->wParam, flags);    
    return WorkEvent(ev, &Window::OnKeyDown, window(), pMsg);
  } else
  if (pMsg->message == WM_KEYUP) {
    UINT nFlags = 0;
    UINT flags = Win32KeyFlags(nFlags);      
    KeyEvent ev(pMsg->wParam, flags);      
    return WorkEvent(ev, &Window::OnKeyUp, window(), pMsg);    
  } else
  if (pMsg->message == WM_MOUSELEAVE) {	
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		CWnd* child = ChildWindowFromPoint(pt, CWP_SKIPINVISIBLE);
		if (!child) {
      mouse_enter_ = true;      
      CRect rc;
      GetWindowRect(&rc);
      MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(this->dev_abs_position().top()), 1, pMsg->wParam);
      return WorkEvent(ev, &Window::OnMouseOut, window(), pMsg);    
		}
  }  else
  if (pMsg->message == WM_LBUTTONDOWN) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(dev_abs_position().top()), 1, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseDown, window(), pMsg);
	} else
  if (pMsg->message == WM_LBUTTONDBLCLK) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(dev_abs_position().top()), 1, pMsg->wParam);
    return WorkEvent(ev, &Window::OnDblclick, window(), pMsg);
  } else
  if (pMsg->message == WM_LBUTTONUP) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(dev_abs_position().top()), 1, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseUp, window(), pMsg);
  } else
  if (pMsg->message == WM_RBUTTONDOWN) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(dev_abs_position().top()), 2, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseDown, window(), pMsg);
  } else    
  if (pMsg->message == WM_RBUTTONDBLCLK) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(dev_abs_position().top()), 2, pMsg->wParam);
    return WorkEvent(ev, &Window::OnDblclick, window(), pMsg);
  } else
  if (pMsg->message == WM_RBUTTONUP) {
    CPoint pt(pMsg->pt);        
    CRect rc;
    GetWindowRect(&rc);
    MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(dev_abs_position().top()), 2, pMsg->wParam);
    return WorkEvent(ev, &Window::OnMouseUp, window(), pMsg);
  } else  
	if (pMsg->message == WM_MOUSEHOVER) {

	} else
  if (pMsg->message == WM_MOUSEMOVE) {
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;    
    m_bTracking = _TrackMouseEvent(&tme);     
    CPoint pt(pMsg->pt);
    CRect rc;
    GetWindowRect(&rc);    
    MouseEvent ev(pt.x - rc.left + static_cast<int>(dev_abs_position().left()), pt.y - rc.top + static_cast<int>(dev_abs_position().top()), 1, pMsg->wParam);
    if (mouse_enter_) {
      mouse_enter_ = false;
      if (window()) {
        try {
          window()->OnMouseEnter(ev);
        } catch (std::exception& e) {
          alert(e.what());      
        }      
      }
    }
    return WorkEvent(ev, &Window::OnMouseMove, window(), pMsg);    
  }
  return CWnd::PreTranslateMessage(pMsg);
}

/*template<class T, class I>
void WindowTemplateImp<T, I>::OnHScroll(UINT a, UINT b, CScrollBar* pScrollBar) {
  ScrollBarImp* sb = (ScrollBarImp*) pScrollBar;
  sb->OnDevScroll(a);
}*/

template<class T, class I>
BOOL WindowTemplateImp<T, I>::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {
  if (!::IsWindow(m_hWnd)) {
    return true;
  }  
  if (!ev.is_default_prevented()) {
     pMsg->hwnd = GetSafeHwnd();
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);        
  }
  return ev.is_propagation_stopped();  
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnKillFocus(CWnd* pNewWnd) {  
}

template<class T, class I>
int WindowTemplateImp<T, I>::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}
  WindowHook::windows_[GetSafeHwnd()] = this;
  // Set the hook
  WindowHook::SetFocusHook();  
	return 0;
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnDestroy() {  
  std::map<HWND, ui::WindowImp*>::iterator it = WindowHook::windows_.find(GetSafeHwnd());
  if (it != WindowHook::windows_.end()) {
    WindowHook::windows_.erase(it);
  }
  bmpDC.DeleteObject();
}

template<class T, class I>
void WindowTemplateImp<T, I>::dev_set_position(const ui::Rect& pos) {
	ui::Point top_left = pos.top_left();
	top_left.Offset(margin_.left(), margin_.top());
  if (window() && window()->parent()) {
	  top_left.Offset(window()->parent()->border_space().left() +
			                window()->parent()->padding().left(),
									  window()->parent()->border_space().top() +
			                window()->parent()->padding().top());
  }	
  SetWindowPos(0, 
		           static_cast<int>(top_left.x()),
		           static_cast<int>(top_left.y()),
		           static_cast<int>(pos.width() + padding_.width() + border_space_.width()),
		           static_cast<int>(pos.height() + padding_.height() + border_space_.height()),
               SWP_NOREDRAW | SWP_NOZORDER | SWP_NOACTIVATE);
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_position() const {
	CRect rc;
	GetWindowRect(&rc);
	if (GetParent()) {
	  ::MapWindowPoints(HWND_DESKTOP, GetParent()->m_hWnd, (LPPOINT)&rc, 2);	
	}	
	return MapPosToBoxModel(rc);
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_abs_position() const {
  CRect rc;
	GetWindowRect(&rc);  
	if (window() && window()->root()) {
    CWnd* root = dynamic_cast<CWnd*>(window()->root()->imp());
		::MapWindowPoints(NULL, root->m_hWnd, (LPPOINT)&rc, 2);		 
		return MapPosToBoxModel(rc);
	}
	return ui::Rect::zero();
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::dev_desktop_position() const {
  CRect rc;
  GetWindowRect(&rc);
	return MapPosToBoxModel(rc);  
}

template<class T, class I>
bool WindowTemplateImp<T, I>::dev_check_position(const ui::Rect& pos) const {
  CRect rc;
	GetWindowRect(&rc);
	if (GetParent()) {
	  ::MapWindowPoints(HWND_DESKTOP, GetParent()->m_hWnd, (LPPOINT)&rc, 2);	
	}
  ui::Point top_left = pos.top_left();
	top_left.Offset(margin_.left(), margin_.top());
  if (window() && window()->parent()) {
	  top_left.Offset(window()->parent()->border_space().left() +
			              window()->parent()->padding().left(),
									  window()->parent()->border_space().top() +
			              window()->parent()->padding().top());
  }	
  ui::Rect pos1 = ui::Rect(top_left, ui::Dimension(pos.width() + padding_.width() + border_space_.width(),
		                                               pos.height() + padding_.height() + border_space_.height()));
  return TypeConverter::rect(pos1).EqualRect(rc);
}

template<class T, class I>
ui::Rect WindowTemplateImp<T, I>::MapPosToBoxModel(const CRect& rc) const {
	 ui::Point top_left(rc.left - margin_.left(), rc.top - margin_.top());	
	   if (window() && window()->parent()) {
		   top_left.Offset(-window()->parent()->border_space().left() - window()->parent()->padding().left(),
			                 -window()->parent()->border_space().top() - window()->parent()->padding().top());
	   }
		 return ui::Rect(top_left,
		              ui::Dimension(rc.Width() - padding_.width() -
										              border_space_.width(),
										            rc.Height() - padding_.height() -
										              border_space_.height()));
}

template<class T, class I>
void WindowTemplateImp<T, I>::dev_set_parent(Window* parent) {  
  if (parent && parent->imp()) {    
    SetParent(dynamic_cast<CWnd*>(parent->imp()));    
    ShowWindow(SW_SHOW | SW_SHOWNOACTIVATE);    
  } else {
    SetParent(DummyWindow::dummy());
  }
}
/*
template<class T, class I>
void WindowTemplateImp<T, I>::OnNcPaint() {
	if (window() && !window()->ornament().expired()) {		
		ui::BoxSpace padding = window()->padding();
		CRect rectWindow;
		GetWindowRect(rectWindow);
		ScreenToClient(rectWindow);
		rectWindow.right = rectWindow.right + static_cast<int>(padding.width());
		rectWindow.bottom = rectWindow.bottom + static_cast<int>(padding.height());
		CRgn rgn;
		rgn.CreateRectRgn(rectWindow.left, rectWindow.top, rectWindow.right, rectWindow.bottom); // 0, 0, 0, 0);		
		if (padding.left() != 0) {
			int fordebugonly(0);
		}
		if (window()->debug_text() == "txt") {
			int fordebugonly(0);
		}
		ui::Region draw_rgn(new ui::mfc::RegionImp(rgn));
		CDC *pDC = GetWindowDC();
		ui::Graphics g(pDC);
		// Just fill everything blue
		//CBrush br(0x00FF0000);
		//rectWindow.right = rectWindow.right + padding.left() + padding.right();
		//rectWindow.bottom = rectWindow.bottom + padding.top() + padding.bottom();
		// pDC->FillRect(rectWindow, &br);
		window()->DrawBackground(&g, draw_rgn);
		g.Dispose();
		rgn.DeleteObject();
	}	
}*/

template<class T, class I>
BOOL WindowTemplateImp<T, I>::OnEraseBkgnd(CDC * pDC) {
  if (window()) {		
	CRect rect;
	GetWindowRect(rect);					
    ::MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT)&rect, 2);		
	ui::Region draw_rgn(ui::Rect(ui::Point(rect.left, rect.top),
						ui::Point(rect.right, rect.bottom)));
	ui::Graphics g(pDC);		
	window()->DrawBackground(&g, draw_rgn);		
  }
  return TRUE;
}


template<class T, class I>
void WindowTemplateImp<T, I>::OnPaint() {	
  CRgn rgn;
  rgn.CreateRectRgn(0, 0, 0, 0);
  int result = GetUpdateRgn(&rgn, FALSE);

  if (!result) {
		return; // If no area to update, exit
	}

  CPaintDC dc(this);

  if (!is_double_buffered_) {    
	  ui::Graphics g(&dc);
	  ui::Region draw_rgn(new ui::mfc::RegionImp(rgn));		
	  g.Translate(padding_.left() + border_space_.left(), padding_.top() + border_space_.top());
	  OnDevDraw(&g, draw_rgn);
	  g.Dispose();
  }
  else {
	if (!bmpDC.m_hObject) { // buffer creation	
		CRect rc;
		GetClientRect(&rc);
		bmpDC.CreateCompatibleBitmap(&dc, rc.right - rc.left, rc.bottom - rc.top);
		char buf[128];
		sprintf(buf, "CanvasView::OnPaint(). Initialized bmpDC to 0x%p\n", (void*)bmpDC);
		TRACE(buf);
	}	
	CDC bufDC;
	bufDC.CreateCompatibleDC(&dc);
	CBitmap* oldbmp = bufDC.SelectObject(&bmpDC);
	ui::Graphics g(&bufDC);
	ui::Region draw_rgn(new ui::mfc::RegionImp(rgn));
	ui::Rect bounds = draw_rgn.bounds();
	OnDevDraw(&g, draw_rgn);
	g.Dispose();
	CRect rc;
	GetClientRect(&rc);
	dc.BitBlt(0, 0, rc.right - rc.left, rc.bottom - rc.top, &bufDC, 0, 0, SRCCOPY);
	bufDC.SelectObject(oldbmp);
	bufDC.DeleteDC();
  }
  rgn.DeleteObject();    
}

template<class T, class I>
void WindowTemplateImp<T, I>::OnSize(UINT nType, int cw, int ch) {  
  if (bmpDC.m_hObject != NULL) { // remove old buffer to force recreating it with new size
	  TRACE("CanvasView::OnResize(). Deleted bmpDC\n");
	  bmpDC.DeleteObject();	  
  }  
  OnDevSize(ui::Dimension(cw, ch));
  CWnd::OnSize(nType, cw, ch);
}

template<class T, class I>
ui::Window* WindowTemplateImp<T, I>::dev_focus_window() {
  Window* result = 0;
  HWND hwnd = ::GetFocus();
  if (hwnd) {
    std::map<HWND, ui::WindowImp*>::iterator it;
    do {
      it = WindowHook::windows_.find(hwnd);
      if (it != WindowHook::windows_.end()) {
        result = it->second->window();
        break;
      }
      hwnd = ::GetParent(hwnd);
    } while (hwnd);
  }
  return result;
}

void WindowImp::DevSetCursor(CursorStyle style) {
  LPTSTR c = 0;
  int ac = 0;
  switch (style) {
    case AUTO        : c = IDC_IBEAM; break;
    case MOVE        : c = IDC_SIZEALL; break;
    case NO_DROP     : ac = AFX_IDC_NODROPCRSR; break;
    case COL_RESIZE  : c = IDC_SIZEWE; break;
    case ALL_SCROLL  : ac = AFX_IDC_TRACK4WAY; break;
    case POINTER     : c = IDC_HAND; break;
    case NOT_ALLOWED : c = IDC_NO; break;
    case ROW_RESIZE  : c = IDC_SIZENS; break;
    case CROSSHAIR   : c = IDC_CROSS; break;
    case PROGRESS    : c = IDC_APPSTARTING; break;
    case E_RESIZE    : c = IDC_SIZEWE; break;
    case NE_RESIZE   : c = IDC_SIZENWSE; break;
    case DEFAULT     : c = IDC_ARROW; break;
    case TEXT        : c = IDC_IBEAM; break;
    case N_RESIZE    : c = IDC_SIZENS; break;
    case S_RESIZE    : c = IDC_SIZENS; break;
    case SE_RESIZE   : c = IDC_SIZENWSE; break;
    case INHERIT     : c = IDC_IBEAM; break;
    case WAIT        : c = IDC_WAIT; break;
    case W_RESIZE    : c = IDC_SIZEWE; break;
    case SW_RESIZE   : c = IDC_SIZENESW; break;
    default          : c = IDC_ARROW; break;
  }
  cursor_ = (c!=0) ? LoadCursor(0, c) : ::LoadCursor(0, MAKEINTRESOURCE(ac));
}

/*
BOOL WindowImp::OnEraseBkgnd(CDC * pDC) {
	if (window() && !window()->ornament().expired()) {
		CRect rc;
		GetClientRect(&rc);
		CRgn rgn;
		rgn.CreateRectRgn(0, 0, rc.Width(), rc.Height());
		
		ui::Region draw_rgn(new ui::mfc::RegionImp(rgn));
		ui::Rect r = draw_rgn.bounds();
		ui::Graphics g(pDC);						
		window()->DrawBackground(&g, draw_rgn);
		g.Dispose();
	}
	return TRUE;
}*/

template class WindowTemplateImp<CWnd, ui::WindowImp>;
template class WindowTemplateImp<CComboBox, ui::ComboBoxImp>;
template class WindowTemplateImp<CScrollBar, ui::ScrollBarImp>;
template class WindowTemplateImp<CButton, ui::ButtonImp>;
template class WindowTemplateImp<CButton, ui::CheckBoxImp>;
template class WindowTemplateImp<CButton, ui::RadioButtonImp>;
template class WindowTemplateImp<CButton, ui::GroupBoxImp>;
template class WindowTemplateImp<CEdit, ui::EditImp>;
template class WindowTemplateImp<CWnd, ui::ScintillaImp>;
template class WindowTemplateImp<CTreeCtrl, ui::TreeViewImp>;
template class WindowTemplateImp<CListCtrl, ui::ListViewImp>;
template class WindowTemplateImp<CFrameWnd, ui::FrameImp>;

BEGIN_MESSAGE_MAP(WindowImp, CWnd)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  //ON_WM_SETFOCUS()
	ON_WM_PAINT()
  ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()	
  ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()	
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
  ON_WM_KEYDOWN()
	ON_WM_KEYUP()
  ON_WM_SETCURSOR()
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_SIZE()  	
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(FrameImp, CFrameWnd)
  ON_WM_SIZE()
	ON_WM_PAINT()
  ON_WM_CLOSE()
  ON_WM_HSCROLL()
  ON_WM_ERASEBKGND()
  ON_WM_KILLFOCUS()
  ON_WM_SETFOCUS()
  ON_WM_NCRBUTTONDOWN()
	ON_WM_LBUTTONDOWN()	
	ON_WM_NCHITTEST()
	ON_COMMAND_RANGE(ID_DYNAMIC_MENUS_START, ID_DYNAMIC_MENUS_END, OnDynamicMenuItems)
END_MESSAGE_MAP()

BOOL FrameImp::PreTranslateMessage(MSG* pMsg) {	
  if (pMsg->message==WM_NCRBUTTONDOWN) {    
    ui::Event ev;
    ui::Point point(pMsg->pt.x, pMsg->pt.y);
    ((Frame*)window())->WorkOnContextPopup(ev, point);    
    return ev.is_propagation_stopped();    
  }
	((Frame*)window())->PreTranslateMessage(pMsg);
  return WindowTemplateImp<CFrameWnd, ui::FrameImp>::PreTranslateMessage(pMsg);
}

void FrameImp::DevShowDecoration() {	
  ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
  ModifyStyle(0, WS_CAPTION, SWP_FRAMECHANGED); 
  ModifyStyle(0, WS_BORDER, SWP_FRAMECHANGED);
  ModifyStyle(0, WS_THICKFRAME, SWP_FRAMECHANGED);
}

void FrameImp::DevHideDecoration() {
  ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);
  ModifyStyle(WS_CAPTION, 0, SWP_FRAMECHANGED); 
  ModifyStyle(WS_BORDER, 0, SWP_FRAMECHANGED);
  ModifyStyle(WS_THICKFRAME, 0, SWP_FRAMECHANGED);
}

void FrameImp::DevPreventResize() {
  ModifyStyle(WS_SIZEBOX, 0, SWP_FRAMECHANGED);
}

void FrameImp::DevAllowResize() {  
  ModifyStyle(0, WS_SIZEBOX, SWP_FRAMECHANGED);	
}

void FrameImp::OnDynamicMenuItems(UINT nID) {
  ui::mfc::MenuContainerImp* mbimp =  ui::mfc::MenuContainerImp::MenuContainerImpById(nID);
  if (mbimp != 0) {
    mbimp->WorkMenuItemEvent(nID);
    return;
  }
}

ui::FrameImp* PopupFrameImp::popup_frame_ = 0;

LRESULT CALLBACK MouseHook(int Code, WPARAM wParam, LPARAM lParam) {
    if (PopupFrameImp::popup_frame_ && 
       (wParam == WM_LBUTTONDOWN || wParam == WM_LBUTTONDBLCLK)) {         
        PopupFrameImp::popup_frame_->DevHide();      
    }    
    return CallNextHookEx(NULL, Code, wParam, lParam);
}

void PopupFrameImp::DevShow() {
  popup_frame_ = 0;  
  mouse_hook_ = SetWindowsHookEx(WH_MOUSE, MouseHook, 0, GetCurrentThreadId());
  ShowWindow(SW_SHOWNOACTIVATE);    
  popup_frame_ = this;
}

void PopupFrameImp::DevHide() {
  PopupFrameImp::popup_frame_ = 0;
  FrameImp::DevHide();  
  if (mouse_hook_) {
    UnhookWindowsHookEx(mouse_hook_);
  }
  mouse_hook_ = 0;  
}

BEGIN_MESSAGE_MAP(ButtonImp, CButton)  
	ON_WM_PAINT()  
  ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void ButtonImp::OnClick() {
  OnDevClick();
}

BEGIN_MESSAGE_MAP(CheckBoxImp, CButton)  
	ON_WM_PAINT()  
  ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void CheckBoxImp::OnClick() {
  OnDevClick();
}

BEGIN_MESSAGE_MAP(RadioButtonImp, CButton)
	ON_WM_PAINT()
	ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void RadioButtonImp::OnClick() {
	OnDevClick();
}

BEGIN_MESSAGE_MAP(GroupBoxImp, CButton)
	ON_WM_PAINT()
	ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

void GroupBoxImp::OnClick() {
	OnDevClick();
}

BEGIN_MESSAGE_MAP(ComboBoxImp, CComboBox)  
	ON_WM_PAINT()
  ON_CONTROL_REFLECT_EX(CBN_SELENDOK, OnSelect)
END_MESSAGE_MAP()

BOOL ComboBoxImp::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {
  if (!::IsWindow(m_hWnd)) {
    return true;
  }
  if (pMsg->message == WM_LBUTTONDOWN) {
    ev.StopPropagation(); 
  }
  if (!ev.is_default_prevented()) {    
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);        
  }
  return ev.is_propagation_stopped();  
}

BOOL ComboBoxImp::OnSelect() {  
  ui::ComboBox* combo_box = dynamic_cast<ui::ComboBox*>(window());
  assert(combo_box);
  combo_box->select(*combo_box);
  combo_box->OnSelect();
  return FALSE;
}

BEGIN_MESSAGE_MAP(EditImp, CEdit)  
	ON_WM_PAINT()	
  ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

HBRUSH EditImp::CtlColor(CDC* pDC, UINT nCtlColor) {
	pDC->SetTextColor(text_color_);	
	pDC->SetBkColor(background_color_);	
	return background_brush_;
}

void EditImp::dev_set_font(const Font& font) {
   font_ = font;
   mfc::FontImp* imp = dynamic_cast<mfc::FontImp*>(font_.imp());
   assert(imp);   
   ::SendMessage(this->m_hWnd, WM_SETFONT, (WPARAM)(imp->cfont()), TRUE);
}


HTREEITEM TreeNodeImp::DevInsert(TreeViewImp* tree, const ui::Node& node, TreeNodeImp* prev_imp) {  
  TVINSERTSTRUCT tvInsert;
  tvInsert.hParent = hItem;
  tvInsert.hInsertAfter = prev_imp ? prev_imp->hItem : TVI_LAST;
  tvInsert.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;  
  tvInsert.item.iImage = node.image_index();
  tvInsert.item.iSelectedImage =  node.selected_image_index();   
  text_ = Charset::utf8_to_win(node.text());
#ifdef UNICODE
	tvInsert.item.pszText = const_cast<WCHAR *>(text_.c_str());
#else
  tvInsert.item.pszText = const_cast<char *>(text_.c_str());
#endif
  return tree->InsertItem(&tvInsert);
}

BEGIN_MESSAGE_MAP(TreeViewImp, CTreeCtrl)  
  ON_WM_ERASEBKGND()
	ON_WM_PAINT()  
  ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnChange)
  ON_NOTIFY_REFLECT_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT_EX(NM_RCLICK, OnRightClick)
  ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnDblClick)  
END_MESSAGE_MAP()

BOOL TreeViewImp::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {  
  if (!::IsWindow(m_hWnd)) {
    return true;
  }    
  if (!ev.is_default_prevented()) {
    if (pMsg->message == WM_KEYUP || pMsg->message == WM_KEYDOWN) {
      pMsg->hwnd = ::GetFocus();
    }
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);            
  }
  pMsg->hwnd = GetSafeHwnd();
  return ev.is_propagation_stopped();  
}

void TreeViewImp::UpdateNode(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node) {  
  NodeImp* prev_node_imp = prev_node ? prev_node->imp(*this) : 0;
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  node->erase_imp(this);  
  TreeNodeImp* new_imp = new TreeNodeImp();
  node->AddImp(new_imp);
  new_imp->set_node(node);
  new_imp->set_owner(this);
  node->changed.connect(boost::bind(&TreeViewImp::OnNodeChanged, this, _1));
  if (!node->parent().expired()) {
    boost::shared_ptr<ui::Node> parent_node = node->parent().lock();
    TreeNodeImp* parent_imp = dynamic_cast<TreeNodeImp*>(parent_node->imp(*this));
    if (parent_imp) {
      TreeNodeImp* prev_imp = dynamic_cast<TreeNodeImp*>(prev_node_imp);
       new_imp->hItem = parent_imp->DevInsert(this, *node.get(), prev_imp);
       htreeitem_node_map_[new_imp->hItem] = node;      
    }
  }
}

void TreeViewImp::DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node) {  
  recursive_node_iterator end = node->recursive_end();
  recursive_node_iterator it = node->recursive_begin();
  boost::shared_ptr<Node> prev = prev_node;
  UpdateNode(node, prev);
  for ( ; it != end; ++it) {
    UpdateNode((*it), prev);
    prev = *it;
  }
}

void TreeViewImp::DevErase(boost::shared_ptr<Node> node) {  
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {
    DeleteItem(imp->hItem);
  } 
}

void TreeViewImp::DevEditNode(boost::shared_ptr<ui::Node> node) { 
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {
    EditLabel(imp->hItem);    
  }
}

void TreeViewImp::dev_select_node(const boost::shared_ptr<ui::Node>& node) {
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node->imp(*this));
  if (imp) {  
    SelectItem(imp->hItem);    
  }
}

boost::weak_ptr<Node> TreeViewImp::dev_selected() {
  ui::Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<ui::Node>();
}

void TreeViewImp::OnNodeChanged(Node& node) {
  TreeNodeImp* imp = dynamic_cast<TreeNodeImp*>(node.imp(*this));  
  if (imp) {      
    SetItemText(imp->hItem, Charset::utf8_to_win(node.text()).c_str());   
  }
}

BOOL TreeViewImp::OnChange(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
		tree_view()->change(*tree_view(), node->shared_from_this());
    tree_view()->OnChange(node->shared_from_this());
  }
  return FALSE;
}

BOOL TreeViewImp::OnRightClick(NMHDR * pNotifyStruct, LRESULT * result) {
  POINT pt;
  ::GetCursorPos(&pt);
  Node::Ptr node = dev_node_at(ui::Point(pt.x, pt.y));
  ui::Event ev;
  tree_view()->WorkOnContextPopup(ev, ui::Point(pt.x, pt.y), node);
  tree_view()->OnRightClick(node);  
  return FALSE;
}

BOOL TreeViewImp::OnDblClick(NMHDR * pNotifyStruct, LRESULT * result) {
  CPoint pt;
  ::GetCursorPos(&pt);
  ScreenToClient(&pt);  
  MouseEvent ev(pt.x, pt.y, 1, 0);    
  if (window()) {
    window()->OnDblclick(ev);    
  }
  return FALSE;
}

ui::Node::Ptr TreeViewImp::dev_node_at(const ui::Point& pos) const {
	CPoint pt(TypeConverter::point(pos));
  ScreenToClient(&pt);
  HTREEITEM item = HitTest(pt);  
  if (item) {
    std::map<HTREEITEM, boost::weak_ptr<ui::Node> >::const_iterator it;
    it = htreeitem_node_map_.find(item);
    return (it != htreeitem_node_map_.end()) ? it->second.lock() : nullpointer;
  }
  return nullpointer;
}

ui::Node* TreeViewImp::find_selected_node() {  
  std::map<HTREEITEM, boost::weak_ptr<ui::Node> >::iterator it;
  it = htreeitem_node_map_.find(GetSelectedItem());
  return (it != htreeitem_node_map_.end()) ? it->second.lock().get() : 0;
}

BOOL TreeViewImp::OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
		if (edit) {
		  is_editing_ = true;
      CString s;    
      edit->GetWindowText(s);    
      tree_view()->OnEditing(node->shared_from_this(), Charset::win_to_utf8(s.GetString()));
		}
  }
  return FALSE;
}


BOOL TreeViewImp::OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
		if (edit) {
			is_editing_ = false;
			CString s;    
			edit->GetWindowText(s);    
			tree_view()->OnEdited(node->shared_from_this(), Charset::win_to_utf8(s.GetString()));
			tree_view()->edited(*tree_view(), node->shared_from_this(),Charset::win_to_utf8(s.GetString()));
		}
  }
  return FALSE;
}

void TreeViewImp::DevClear() { DeleteAllItems(); }

void TreeViewImp::DevShowLines() {
  ModifyStyle(0, TVS_HASLINES | TVS_LINESATROOT);
}

void TreeViewImp::DevHideLines() {
  ModifyStyle(TVS_HASLINES | TVS_LINESATROOT, 0);
}

void TreeViewImp::DevShowButtons() {
  ModifyStyle(0, TVS_HASBUTTONS);
}

void TreeViewImp::DevHideButtons() {
  ModifyStyle(TVS_HASBUTTONS, 0);
}

HTREEITEM GetNextTreeItem(const CTreeCtrl& treeCtrl, HTREEITEM hItem)
{
      // has this item got any children
      if (treeCtrl.ItemHasChildren(hItem))
      {
            return treeCtrl.GetNextItem(hItem, TVGN_CHILD);
      }
      else if (treeCtrl.GetNextItem(hItem, TVGN_NEXT) != NULL)
      {
            // the next item at this level
            return treeCtrl.GetNextItem(hItem, TVGN_NEXT);
      }
      else
      {
            // return the next item after our parent
            hItem = treeCtrl.GetParentItem(hItem);
            if (hItem == NULL)
            {
                  // no parent
                  return NULL;
            }
            while (hItem && treeCtrl.GetNextItem(hItem, TVGN_NEXT) == NULL)
            {
                  hItem = treeCtrl.GetParentItem(hItem);									
            }
            // next item that follows our parent
            return treeCtrl.GetNextItem(hItem, TVGN_NEXT);
      }
}

// Functions to expands all items in a tree control
void ExpandAll(CTreeCtrl& treeCtrl)
{
     
     HTREEITEM hRootItem = treeCtrl.GetRootItem();
     HTREEITEM hItem = hRootItem;

     while (hItem)
     {
          if (treeCtrl.ItemHasChildren(hItem))
          {
               treeCtrl.Expand(hItem, TVE_EXPAND);
          }
          hItem = GetNextTreeItem(treeCtrl, hItem);
     }
}

void TreeViewImp::DevExpandAll() {
	ExpandAll(*this);  
}


BEGIN_MESSAGE_MAP(ListViewImp, CListCtrl)
  ON_WM_ERASEBKGND()
	ON_WM_PAINT()  
  ON_NOTIFY_REFLECT_EX(LVN_ITEMCHANGED, OnChange)
  ON_NOTIFY_REFLECT_EX(TVN_BEGINLABELEDIT, OnBeginLabelEdit)
  ON_NOTIFY_REFLECT_EX(TVN_ENDLABELEDIT, OnEndLabelEdit)
  ON_NOTIFY_REFLECT_EX(NM_RCLICK, OnRightClick)  
  ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,OnCustomDrawList)
END_MESSAGE_MAP()

BOOL ListViewImp::prevent_propagate_event(ui::Event& ev, MSG* pMsg) {  
  if (!::IsWindow(m_hWnd)) {
    return true;
  }  
  if (!ev.is_default_prevented()) {    
    ::TranslateMessage(pMsg);          
	  ::DispatchMessage(pMsg);        
  }
  return ev.is_propagation_stopped();  
}

void ListNodeImp::DevInsertFirst(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int pos) {  
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | LVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = Charset::utf8_to_win(node.text());
//  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
#ifdef UNICODE
	lvi.pszText = const_cast<WCHAR *>(node_imp->text_.c_str());
#else
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
#endif
  lvi.iImage = node.image_index();
  lvi.iItem = pos;    
  lvi.iSubItem = 0;
  node_imp->lvi = lvi;
  list->InsertItem(&lvi);  
}

void ListNodeImp::set_text(ui::mfc::ListViewImp* list, const std::string& text) {
  text_ = Charset::utf8_to_win(text);  
#ifdef UNICODE
	lvi.pszText = const_cast<WCHAR *>(text_.c_str());
#else
  lvi.pszText = const_cast<char *>(text_.c_str());
#endif
	
  list->SetItemText(lvi.iItem, lvi.iSubItem, Charset::utf8_to_win(text).c_str());
}

void ListNodeImp::DevSetSub(ui::mfc::ListViewImp* list, const ui::Node& node, ListNodeImp* node_imp, ListNodeImp* prev_imp, int level) {
  LVITEM lvi;
  lvi.mask =  LVIF_TEXT | LVIF_IMAGE;
  lvi.cColumns = 0;
  node_imp->text_ = Charset::utf8_to_win(node.text());  
#ifdef UNICODE
	lvi.pszText = const_cast<WCHAR *>(node_imp->text_.c_str());
#else
  lvi.pszText = const_cast<char *>(node_imp->text_.c_str());
#endif
  lvi.iImage = node.image_index(); 
  lvi.iItem = position();
  lvi.iSubItem = level - 1;
  node_imp->lvi = lvi;
  list->SetItem(&lvi);
  node_imp->set_position(position());
}

ListNodeImp* ListViewImp::UpdateNode(boost::shared_ptr<Node> node, boost::shared_ptr<Node> prev_node, int pos) {
  ListNodeImp* new_imp = new ListNodeImp();  
  NodeImp* prev_node_imp = prev_node ? prev_node->imp(*this) : 0;
  node->erase_imp(this);  
  ImpLookUpIterator it = lookup_table_.find(pos);
  if (it != lookup_table_.end()) {
    lookup_table_.erase(it);
  }  
  node->AddImp(new_imp);
  new_imp->set_node(node);
  new_imp->set_owner(this);  
  node->changed.connect(boost::bind(&ListViewImp::OnNodeChanged, this, _1));
  if (!node->parent().expired()) {
    boost::shared_ptr<ui::Node> parent_node = node->parent().lock();   
    boost::ptr_list<NodeImp>::iterator it = parent_node->imps.begin();
    for ( ; it != parent_node->imps.end(); ++it) {
      ListNodeImp* parent_imp = dynamic_cast<ListNodeImp*>(parent_node->imp(*this));
      if (parent_imp) {             
        ListNodeImp* prev_imp = dynamic_cast<ListNodeImp*>(prev_node_imp);
        int level = node->level();
        if (level == 1) {
          parent_imp->DevInsertFirst(this, *node.get(), new_imp, prev_imp, pos);
          lookup_table_[pos] = new_imp;
        } else {
          parent_imp->DevSetSub(this, *node.get(), new_imp, prev_imp, level);
          lookup_table_[new_imp->position()] = new_imp;
        }
      }
    }
  }
  return new_imp;
}

void ListViewImp::DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node) {  
  recursive_node_iterator end = node->recursive_end();
  recursive_node_iterator it = node->recursive_begin();  
  int pos = 0;
  if (prev_node) {        
    ListNodeImp* prev_imp = dynamic_cast<ListNodeImp*>(prev_node->imp(*this));
    if (prev_imp) {
      pos = prev_imp->position() + 1;
    }
  }
  UpdateNode(node, prev_node, pos);  
  boost::shared_ptr<Node> prev;
  pos = 0;
  for (; it != end; ++it) {    
    UpdateNode((*it), prev, pos);
    if ((*it)->level() == 1) {
      ++pos;
    }
    prev = *it;
  }
}

void ListViewImp::DevErase(ui::Node::Ptr node) {
	if (node) {
		node->erase_imp(this);    
	}
}

void ListViewImp::DevEditNode(ui::Node::Ptr node) {    
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node->imp(*this));
  if (imp) {
   //EditLabel(imp->hItem);    
  }
}

void ListViewImp::dev_select_node(const ui::Node::Ptr& node) {
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node->imp(*this));
  if (imp) {
      //SelectItem(imp->hItem);    
  }
}

boost::weak_ptr<Node> ListViewImp::dev_selected() {
  ui::Node* node = find_selected_node();
  return node ? node->shared_from_this() : boost::weak_ptr<ui::Node>();
}

void ListViewImp::OnNodeChanged(Node& node) {
  ListNodeImp* imp = dynamic_cast<ListNodeImp*>(node.imp(*this));
  if (imp) {      
    imp->set_text(this, node.text());      
  }
}

BOOL ListViewImp::OnChange(NMHDR * pNotifyStruct, LRESULT * result) {   
  Node* node = find_selected_node();
  if (node) {
    list_view()->OnChange(node->shared_from_this());
    list_view()->change(*list_view(), node->shared_from_this());
  }
  return FALSE;
}

BOOL ListViewImp::OnRightClick(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    list_view()->OnRightClick(node->shared_from_this());
  }
  return FALSE;
}

ui::Node* ListViewImp::find_selected_node() {    
  Node* result = 0;
  POSITION pos = GetFirstSelectedItemPosition();
  int selected = -1;
  if (pos != NULL) {
    while (pos) {
      selected = GetNextSelectedItem(pos);      
    }
  } 
  if (selected != -1) {
    ImpLookUpIterator it = lookup_table_.find(selected);
    if (it != lookup_table_.end() && !it->second->node().expired()) {            
      result = it->second->node().lock().get();      
    }
  }  
  return result;
}

std::vector<ui::Node::Ptr> ListViewImp::dev_selected_nodes() {
  std::vector<ui::Node::Ptr> nodes;
  POSITION pos = GetFirstSelectedItemPosition();
  int selected = -1;
  if (pos != NULL) {
    while (pos) {
      selected = GetNextSelectedItem(pos);
      if (selected != -1) {
        ImpLookUpIterator it = lookup_table_.find(selected);
        if (it != lookup_table_.end() && !it->second->node().expired()) {          
          nodes.push_back(it->second->node().lock());                     
        }
      }
    } 
  }
  return nodes;
}
 
BOOL ListViewImp::OnBeginLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
		if (edit) {
      CString s;    
      edit->GetWindowText(s);
      is_editing_ = true;
      list_view()->OnEditing(node->shared_from_this(), Charset::win_to_utf8(s.GetString()));
		}
  }
  return FALSE;
}


BOOL ListViewImp::OnEndLabelEdit(NMHDR * pNotifyStruct, LRESULT * result) {
  Node* node = find_selected_node();
  if (node) {
    CEdit* edit = GetEditControl();
		if (edit) {
		  is_editing_ = false;
      CString s;    
      edit->GetWindowText(s);    
      list_view()->OnEdited(node->shared_from_this(), Charset::win_to_utf8(s.GetString()));
		}
  }
  return FALSE;
}

void ListViewImp::DevClear() { DeleteAllItems(); }

void ListViewImp::OnCustomDrawList(NMHDR *pNMHDR, LRESULT *pResult) {
   NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);   
   *pResult = CDRF_DODEFAULT;
   switch(pLVCD->nmcd.dwDrawStage) {
     case CDDS_PREPAINT:
      *pResult = CDRF_NOTIFYITEMDRAW;
      break;

     case CDDS_ITEMPREPAINT:
      *pResult = *pResult = CDRF_DODEFAULT; // CDRF_NOTIFYSUBITEMDRAW;
      pLVCD->clrTextBk = GetBkColor();
      break;     
   }
}

//end list
BEGIN_MESSAGE_MAP(ScrollBarImp, CScrollBar)  
	ON_WM_PAINT()    
END_MESSAGE_MAP()

BOOL ScrollBarImp::OnChildNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* pResult) {
  if (uMsg == WM_HSCROLL || uMsg == WM_VSCROLL) {
    int nScrollCode = (short)LOWORD(wParam);
	  int nPos = (short)HIWORD(wParam);
    switch(nScrollCode) {
      case SB_THUMBTRACK:     //The user is dragging the scroll box. This message is sent repeatedly until the user releases the mouse button. The nPos parameter indicates the position that the scroll box has been dragged to.         
        dev_set_scroll_position(nPos);
        break;
    }
    
    OnDevScroll(nPos);
  }
  return  WindowTemplateImp<CScrollBar, ui::ScrollBarImp>::OnChildNotify(uMsg, wParam, lParam, pResult);
}

IMPLEMENT_DYNAMIC(ScintillaImp, CWnd)

BEGIN_MESSAGE_MAP(ScintillaImp, CWnd)
ON_WM_ERASEBKGND()
ON_NOTIFY_REFLECT_EX(SCN_CHARADDED, OnModified)
ON_NOTIFY_REFLECT_EX(SCN_MARGINCLICK, OnFolder)         
END_MESSAGE_MAP()

void ScintillaImp::dev_set_lexer(const Lexer& lexer) {
  SetupLexerType();
  SetupHighlighting(lexer);
  SetupFolding(lexer);
}

void ScintillaImp::SetupHighlighting(const Lexer& lexer) {
  f(SCI_SETKEYWORDS, 0, lexer.keywords().c_str());
  f(SCI_STYLESETFORE, SCE_LUA_COMMENT, ToCOLORREF(lexer.comment_color())); 
  f(SCI_STYLESETFORE, SCE_LUA_COMMENTLINE, ToCOLORREF(lexer.comment_line_color()));
  f(SCI_STYLESETFORE, SCE_LUA_COMMENTDOC, ToCOLORREF(lexer.comment_doc_color()));
  f(SCI_STYLESETFORE, SCE_LUA_IDENTIFIER, ToCOLORREF(lexer.identifier_color()));
  f(SCI_STYLESETFORE, SCE_LUA_NUMBER, ToCOLORREF(lexer.number_color()));
  f(SCI_STYLESETFORE, SCE_LUA_STRING, ToCOLORREF(lexer.string_color()));
  f(SCI_STYLESETFORE, SCE_LUA_WORD, ToCOLORREF(lexer.word_color()));
  f(SCI_STYLESETFORE, SCE_LUA_PREPROCESSOR, ToCOLORREF(lexer.identifier_color()));
  f(SCI_STYLESETFORE, SCE_LUA_OPERATOR, ToCOLORREF(lexer.operator_color()));
  f(SCI_STYLESETFORE, SCE_LUA_CHARACTER, ToCOLORREF(lexer.character_code_color()));
}

void ScintillaImp::SetupFolding(const Lexer& lexer) {
  SetFoldingBasics();
  SetFoldingColors(lexer);
  SetFoldingMarkers();
  SetupLexerType();
}

void ScintillaImp::SetFoldingBasics() {
  f(SCI_SETMARGINWIDTHN, 2, 12);
  f(SCI_SETMARGINSENSITIVEN, 2, true);  
  f(SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
  f(SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
  f(SCI_SETPROPERTY, _T("fold"), _T("1"));
  f(SCI_SETPROPERTY, _T("fold.compact"), _T("1"));                
  f(SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CHANGE| SC_AUTOMATICFOLD_CLICK, 0);
}

void ScintillaImp::SetFoldingColors(const Lexer& lexer) {
 for (int i = 25; i <= 31; i++) {    
   f(SCI_MARKERSETBACK, i, ToCOLORREF(lexer.folding_color()));
 }
}

void ScintillaImp::SetFoldingMarkers() {
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);  
  f(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
}

void ScintillaImp::SetupLexerType() {
  f(SCI_SETLEXER, SCLEX_LUA, 0);
}

BOOL ScintillaImp::OnFolder(NMHDR * nhmdr,LRESULT *) { 
  SCNotification *pMsg = (SCNotification*)nhmdr;      
  long lLine = f(SCI_LINEFROMPOSITION, pMsg->position, 0);
  f(SCI_TOGGLEFOLD, lLine, 0);   
  return false;
}

LRESULT __stdcall WindowHook::HookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    CWPSTRUCT* info = (CWPSTRUCT*) lParam;        
    if (info->message == WM_SETFOCUS) {
      FilterHook(info->hwnd);                     
    }
  }
  return CallNextHookEx(_hook , nCode, wParam, lParam);       
}

void WindowHook::FilterHook(HWND hwnd) {  
  typedef std::map<HWND, ui::WindowImp*> WindowList;
  /*WindowList::iterator it = windows_.find(hwnd);
  if (it != windows_.end()) {
    it->second->window()->
  } else {
    Window::SetFocus(Window::nullpointer);
  }*/
}

void WindowHook::SetFocusHook() {
  if (_hook) {
    return;
  }
  if (!(_hook = SetWindowsHookEx(WH_CALLWNDPROC, 
                                 WindowHook::HookCallback,
                                 AfxGetInstanceHandle(),
                                 GetCurrentThreadId()))) {
    TRACE(_T("ui::MFCView : Failed to install hook!\n"));
  }
}

void WindowHook::ReleaseHook() { 
  UnhookWindowsHookEx(_hook);
}

// MenuContainerImp
std::map<int, MenuContainerImp*> MenuContainerImp::menu_bar_id_map_;

MenuContainerImp::MenuContainerImp() : menu_window_(0), hmenu_(0) {  
}

void MenuContainerImp::set_menu_window(CWnd* menu_window, const Node::Ptr& root_node) {
  menu_window_ = menu_window;  
  if (!menu_window && root_node) {
    hmenu_ = 0;
    root_node->erase_imps(this);    
  } else {
    hmenu_ = menu_window->GetMenu()->m_hMenu;
    DevUpdate(root_node, nullpointer);
  }
}

void MenuContainerImp::DevInvalidate() {
  if (menu_window_) {
    menu_window_->DrawMenuBar();
  }
}

void MenuContainerImp::DevUpdate(const Node::Ptr& node, boost::shared_ptr<Node> prev_node) {
  if (hmenu_) {		
    UpdateNodes(node, hmenu_, ::GetMenuItemCount(hmenu_));
    DevInvalidate();
  }
}

void MenuContainerImp::RegisterMenuEvent(int id, MenuImp* menu_imp) {
  menu_item_id_map_[id] = menu_imp;
  menu_bar_id_map_[id] = this;
}

void MenuContainerImp::UpdateNodes(Node::Ptr parent_node, HMENU parent, int pos_start) {
  if (parent_node) {
    Node::Container::iterator it = parent_node->begin();    
    for (int pos = pos_start; it != parent_node->end(); ++it, ++pos) {
      Node::Ptr node = *it;
      boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
      while (it != node->imps.end()) {
        NodeImp* i = &(*it);
        if (i->owner() == this) {
          it = node->imps.erase(it);            
        } else {
          ++it;
        }
      }
      
      MenuImp* menu_imp = new MenuImp(parent);
      menu_imp->set_owner(this);
      menu_imp->dev_set_position(pos);      
      if (node->size() == 0) {        
        ui::Image* img = !node->image().expired() ? node->image().lock().get() : 0;
        menu_imp->CreateMenuItem(node->text(), img);
        RegisterMenuEvent(menu_imp->id(), menu_imp);
      } else {
        menu_imp->CreateMenu(node->text());        
      }
      node->imps.push_back(menu_imp);      
      if (node->size() > 0) {      
        boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
        for ( ; it != node->imps.end(); ++it) {
          if (it->owner() == this) {
            MenuImp* menu_imp =  dynamic_cast<MenuImp*>(&(*it));    
            if (menu_imp) {
              UpdateNodes(node, menu_imp->hmenu());
            }
            break;
          }
        }        
      }
    }
  }
}

void MenuContainerImp::DevErase(boost::shared_ptr<Node> node) {
  boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
  for (; it != node->imps.end(); ++it) {     
    if (it->owner() == this) {      
      node->imps.erase(it);
      break;
    } 
  }
}

void MenuContainerImp::WorkMenuItemEvent(int id) {
  MenuImp* menu_imp = FindMenuItemById(id);
  assert(menu_imp);
  if (menu_bar()) {
    struct {
     ui::MenuContainer* bar;
     int selectedItemID;
     void operator()(boost::shared_ptr<ui::Node> node, boost::shared_ptr<ui::Node> prev_node) {
       boost::ptr_list<NodeImp>::iterator it = node->imps.begin();
       for ( ; it != node->imps.end(); ++it) {         
         MenuImp* imp = dynamic_cast<MenuImp*>(&(*it));
          if (imp) {
            if (imp->id() == selectedItemID) {
             bar->OnMenuItemClick(node->shared_from_this());
						 bar->menu_item_click(*bar, node->shared_from_this());
            }
          }
        }
      }
    } f;
    f.bar = menu_bar();
    f.selectedItemID = menu_imp->id();
    menu_bar()->root_node().lock()->traverse(f);  
  }
}

MenuImp* MenuContainerImp::FindMenuItemById(int id) {
  std::map<int, MenuImp*>::iterator it = menu_item_id_map_.find(id);
  return (it != menu_item_id_map_.end()) ? it->second : 0;
}

MenuContainerImp* MenuContainerImp::MenuContainerImpById(int id) {
  std::map<int, MenuContainerImp*>::iterator it = menu_bar_id_map_.find(id);
  return (it != menu_bar_id_map_.end()) ? it->second : 0;
}

// PopupMenuImp
PopupMenuImp::PopupMenuImp() : popup_menu_(0) {
  popup_menu_ = ::CreatePopupMenu();
  set_hmenu(popup_menu_);
}

void PopupMenuImp::DevTrack(const ui::Point& pos) {
  ::TrackPopupMenu(popup_menu_,
    TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_HORPOSANIMATION | TPM_VERPOSANIMATION,
    static_cast<int>(pos.x()), static_cast<int>(pos.y()), 0, ::AfxGetMainWnd()->m_hWnd, 0);
}

//MenuImp
MenuImp::~MenuImp()  {  
  if (parent_ && ::IsMenu(parent_)) {      
     RemoveMenu(parent_, pos_, MF_BYPOSITION);    
   } else {     
     ::DestroyMenu(hmenu_);     
   }  
}

void MenuImp::CreateMenu(const std::string& text) {
  hmenu_ = ::CreateMenu();  
  AppendMenu(parent_, MF_POPUP | MF_ENABLED, (UINT_PTR)hmenu_, Charset::utf8_to_win(text).c_str());
  UINT count = ::GetMenuItemCount(parent_);
  pos_ = count - 1;
}

void MenuImp::CreateMenuItem(const std::string& text, ui::Image* image) {
  if (text == "-") {
    ::AppendMenu(parent_, MF_SEPARATOR, 0, NULL);  
  } else {
    id_ = ID_DYNAMIC_MENUS_START + ui::MenuContainer::id_counter++;
    ::AppendMenu(parent_, MF_STRING |  MF_ENABLED, id_, Charset::utf8_to_win(text).c_str());
    if (image) {  
			assert(image->imp());			
			ImageImp* imp = dynamic_cast<ImageImp*>(image->imp());
			assert(imp);
      ::SetMenuItemBitmaps(parent_, id_, MF_BYCOMMAND, (HBITMAP)imp->dev_source()->m_hObject, (HBITMAP)imp->dev_source()->m_hObject); 
    }
  }
}

// GameController
void GameControllersImp::DevScanPluggedControllers(std::vector<int>& plugged_controller_ids) {
  UINT num = joyGetNumDevs();
  UINT game_controller_id = 0;
  for ( ; game_controller_id < num; ++game_controller_id) {
    JOYINFO joyinfo;    
    int err = joyGetPos(game_controller_id, &joyinfo);
    if (err == 0) {      
      plugged_controller_ids.push_back(game_controller_id);      
    }
  }  
}

void GameControllersImp::DevUpdateController(ui::GameController& controller) {  
  JOYINFO joy_info;
  joyGetPos(controller.id(), &joy_info);
  controller.set(joy_info.wXpos, joy_info.wYpos, joy_info.wZpos, static_cast<int>(joy_info.wButtons));
}

// FileInformation.cpp: implementation of the CFileInformation class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileInformation::CFileInformation()
{
	ZeroFileData();
	SetFileDir( CString( _T("default.txt") ) );
}

CFileInformation::CFileInformation( CString path )
{
	Load( path );
}

CFileInformation::CFileInformation( CString dir, CString file )
{
	Load( dir, file );
}

CFileInformation::CFileInformation( WIN32_FIND_DATA fd, CString dir )
{
	SetFileData( fd );
	SetFileDir( dir );
}

CFileInformation::CFileInformation( const CFileInformation& fdi )
{
	SetFileData( fdi.GetFileData() );
	SetFileDir( fdi.GetFileDir() );
}

CFileInformation::CFileInformation( const CFileInformation* fdi )
{
	SetFileData( fdi->GetFileData() );
	SetFileDir( fdi->GetFileDir() );
}

CFileInformation::~CFileInformation()
{
}

void CFileInformation::SetFileData(WIN32_FIND_DATA fd)
{
	memcpy( &m_fd, &fd, sizeof( WIN32_FIND_DATA ) );
}

WIN32_FIND_DATA CFileInformation::GetFileData() const
{
	return m_fd;
}

void CFileInformation::ZeroFileData()
{
	memset( &m_fd, 0, sizeof( WIN32_FIND_DATA ) );
}

CString CFileInformation::GetFileName() const
{
	CString buffer( m_fd.cFileName );
	return buffer;
}

CString CFileInformation::GetFileNameWithoutExt() const
{
	CString buffer = GetFileName();
	int     pos    = buffer.ReverseFind( L'.' );
	return  buffer.Left( pos );
}

CString CFileInformation::GetFileExt() const
{
	CString buffer = GetFileName();
	int     pos    = buffer.ReverseFind( L'.' );
	return  buffer.Right( buffer.GetLength() - pos - 1 );
}

unsigned long CFileInformation::GetFileSize(DWORD &dwFileSizeHigh, DWORD &dwFileSizeLow) const
{
	dwFileSizeHigh = m_fd.nFileSizeHigh;
	dwFileSizeLow  = m_fd.nFileSizeLow;
	return dwFileSizeLow;
}

unsigned long CFileInformation::GetFileSize( EFileSize& fsFormat ) const
{
	DWORD         dwFileSizeHigh = 0;
	DWORD         dwFileSizeLow  = 0;
	unsigned long size           = 0;

	GetFileSize( dwFileSizeHigh,  dwFileSizeLow );

	if( dwFileSizeLow == 0 )
	{
		fsFormat = fsBytes;
		return dwFileSizeLow;
	}

	if( dwFileSizeHigh == 0 )
	{
		size = dwFileSizeLow - ( dwFileSizeLow % 1024 );
	
		if( size == 0 )
		{
			fsFormat = fsBytes;
			return dwFileSizeLow;
		}
		else
		{
			fsFormat = fsKBytes;
			return ( size / 1024 );
		}
	}
	else
	{
		fsFormat = fsKBytes;
		return dwFileSizeHigh * ( MAXDWORD / 1024 ) + dwFileSizeLow / 1024;
	}
}

DWORD CFileInformation::GetFileAttribute() const
{
	return m_fd.dwFileAttributes;
}

BOOL CFileInformation::IsDirectory() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_DIRECTORY ) );
}

BOOL CFileInformation::IsArchive() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_ARCHIVE ) );
}

BOOL CFileInformation::IsNormal() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_NORMAL ) );
}

BOOL CFileInformation::IsHidden() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_HIDDEN ) );
}

BOOL CFileInformation::IsReadOnly() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_READONLY ) );
}

BOOL CFileInformation::FileAttributeReadOnly( BOOL set )
{
	if( GetFileAttribute() == -1 )
		return FALSE;

	if( set )
	{
		return SetFileAttributes( GetFilePath(), GetFileAttribute()|FILE_ATTRIBUTE_READONLY );	
	}
	else
	{
		return SetFileAttributes( GetFilePath(), GetFileAttribute()&~FILE_ATTRIBUTE_READONLY );	
	}
}

BOOL CFileInformation::IsSystem() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_SYSTEM ) );
}

BOOL CFileInformation::IsTemporary() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_TEMPORARY ) );
}

BOOL CFileInformation::IsWinTemporary() const
{
	if( ( GetFileAttribute() != -1 ) &&
		( GetFileAttribute() & FILE_ATTRIBUTE_TEMPORARY ) )
		return TRUE;

	CString fileName = GetFileName();
	fileName.MakeLower();
	if( ( fileName[ 0 ] == L'~' ) ||
		( fileName.Find( _T(".tmp"), 0 ) != -1 ) ||
		( fileName.Find( _T(".bak"), 0 ) != -1 ) )
		return TRUE;

	return FALSE;
}

BOOL CFileInformation::IsNotAvailableNow() const
{
	return ( ( GetFileAttribute() != -1 ) && 
		     ( GetFileAttribute() &  FILE_ATTRIBUTE_OFFLINE ) );
}

BOOL CFileInformation::IsCurrentRoot() const
{
	return ( GetFileName().Compare( _T(".") ) == 0 );
}

BOOL CFileInformation::IsParentDir() const
{
	return ( GetFileName().Compare( _T("..") ) == 0 );
}

BOOL CFileInformation::IsRootFile() const
{
	return IsCurrentRoot() || IsParentDir();
}

BOOL CFileInformation::IsActualFile() const
{
	return !IsDirectory()    &&
		   !IsCurrentRoot()  &&
		   !IsParentDir()    &&
		   !IsWinTemporary() &&
   		   !IsHidden()       &&
		   !IsSystem();
}

CString CFileInformation::GetFileDir() const
{
	return m_dir;
}

void CFileInformation::SetFileDir(const CString &dir)
{
	m_dir = dir;
}

CString CFileInformation::GetFilePath() const
{
	CString path;

	if( m_dir.IsEmpty() || m_dir[ m_dir.GetLength() - 1 ] == L'\\' )
		path = m_dir + GetFileName();
	else
		path = m_dir + _T("\\") + GetFileName();

	return path;
}

FILETIME CFileInformation::GetFileLastWriteTime() const
{
	return m_fd.ftLastWriteTime;
}

BOOL CFileInformation::operator ==( CFileInformation fdi ) const
{
	return IsSomeFilePath( fdi )          &&
		   IsSomeFileSize( fdi )          &&
		   IsSomeFileLastWriteTime( fdi ) &&
		   IsSomeFileAttribute( fdi );
}

BOOL CFileInformation::operator !=( CFileInformation fdi ) const
{
	return !IsSomeFilePath( fdi )          ||
		   !IsSomeFileSize( fdi )          ||
		   !IsSomeFileLastWriteTime( fdi ) ||
		   !IsSomeFileAttribute( fdi );
}

const CFileInformation& CFileInformation::operator =( const CFileInformation& fdi )
{
	SetFileData( fdi.GetFileData() );
	SetFileDir( fdi.GetFileDir() );
	return *this;
}

BOOL CFileInformation::IsSomeFileData( WIN32_FIND_DATA fd ) const
{
	WIN32_FIND_DATA fd2 = GetFileData();
	return ( memcmp( &fd, &fd2, sizeof( WIN32_FIND_DATA ) ) == 0 );
}

BOOL CFileInformation::IsSomeFileData( const CFileInformation& fdi ) const
{
	WIN32_FIND_DATA fd1 = fdi.GetFileData();
	WIN32_FIND_DATA fd2 =     GetFileData();
	return ( memcmp( &fd1, &fd2, sizeof( WIN32_FIND_DATA ) ) == 0 );
}

BOOL CFileInformation::IsSomeFileAttribute( DWORD dwAttribute ) const
{
	DWORD dwAttribute2 = GetFileAttribute();
	return ( dwAttribute2 == dwAttribute );
}

BOOL CFileInformation::IsSomeFileAttribute( const CFileInformation& fdi ) const
{
	DWORD dwAttribute1 = fdi.GetFileAttribute();
	DWORD dwAttribute2 =     GetFileAttribute();
	return ( dwAttribute2 == dwAttribute1 );
}

BOOL CFileInformation::IsSomeFileSize( DWORD dwFileSizeHigh, DWORD dwFileSizeLow ) const
{
	DWORD dwFileSizeHigh2, dwFileSizeLow2;
	GetFileSize(  dwFileSizeHigh2, dwFileSizeLow2 );
	return ( ( dwFileSizeHigh2 == dwFileSizeHigh ) && 
		     ( dwFileSizeLow2  == dwFileSizeLow  ) );
}

BOOL CFileInformation::IsSomeFileSize( const CFileInformation& fdi ) const
{
	DWORD dwFileSizeHigh1, dwFileSizeLow1;
	DWORD dwFileSizeHigh2, dwFileSizeLow2;
	fdi.GetFileSize(  dwFileSizeHigh1, dwFileSizeLow1 );
	    GetFileSize(  dwFileSizeHigh2, dwFileSizeLow2 );
	return ( ( dwFileSizeHigh2 == dwFileSizeHigh1 ) && 
		     ( dwFileSizeLow2  == dwFileSizeLow1  ) );
}

BOOL CFileInformation::IsSomeFileName(CString fileName) const
{
	CString fileName2 = GetFileName();
	fileName.MakeUpper();
	fileName2.MakeUpper();
	return ( fileName2.Compare( fileName ) == 0 );
}

BOOL CFileInformation::IsSomeFileName( const CFileInformation& fdi ) const
{
	CString fileName1 =     GetFileName();
	CString fileName2  = fdi.GetFileName();
	fileName1.MakeUpper();
	fileName2.MakeUpper();
	return ( fileName2.Compare( fileName1 ) == 0 );
}

BOOL CFileInformation::IsSomeFileDir( CString fileDir ) const
{
	CString fileDir2 = GetFileDir();
	fileDir.MakeUpper();
	fileDir2.MakeUpper();
	return ( fileDir2.Compare( fileDir ) == 0 );
}

BOOL CFileInformation::IsSomeFileDir( const CFileInformation& fdi ) const
{
	CString fileDir1 =     GetFileDir();
	CString fileDir2 = fdi.GetFileDir();
	fileDir1.MakeUpper();
	fileDir2.MakeUpper();
	return ( fileDir1.Compare( fileDir1 ) == 0 );
}

BOOL CFileInformation::IsSomeFilePath( CString filePath ) const
{
	CString filePath2 = GetFilePath();
	filePath.MakeUpper();
	filePath2.MakeUpper();
	return ( filePath2.Compare( filePath ) == 0 );
}

BOOL CFileInformation::IsSomeFilePath( const CFileInformation& fdi ) const
{
	CString filePath1 =     GetFilePath();
	CString filePath2 = fdi.GetFilePath();
	filePath1.MakeUpper();
	filePath2.MakeUpper();
	return ( filePath2.Compare( filePath1 ) == 0 );
}

BOOL CFileInformation::IsSomeFileLastWriteTime( FILETIME fileTime ) const
{
	FILETIME fileTime2 = GetFileLastWriteTime();

	return ( fileTime.dwHighDateTime == fileTime2.dwHighDateTime &&
		     fileTime.dwLowDateTime  == fileTime2.dwLowDateTime );
}

BOOL CFileInformation::IsSomeFileLastWriteTime(const CFileInformation &fdi) const
{
	FILETIME fileTime1 =     GetFileLastWriteTime();
	FILETIME fileTime2 = fdi.GetFileLastWriteTime();

	return ( fileTime1.dwHighDateTime == fileTime2.dwHighDateTime &&
		     fileTime1.dwLowDateTime  == fileTime2.dwLowDateTime );
}

CString CFileInformation::ConcPath( const CString& first, const CString& second ) 
{
	CString path;

	if( first.IsEmpty() || first[ first.GetLength() - 1 ] == L'\\' )
		path = first + second;
	else
		path = first + _T("\\") + second;

	return path;
}

int CFileInformation::EnumFiles( CString root, P_FI_List list ) 
{
	WIN32_FIND_DATA ffd;
	CString         path = ConcPath( root, _T("*.*") );
	HANDLE          sh   = FindFirstFile( path, &ffd );

	if( INVALID_HANDLE_VALUE == sh )
		return 0;

	do
	{
		CFileInformation* pFDI = new CFileInformation( ffd, root );

		if( pFDI->IsRootFile() )
		{
			delete pFDI;
		}
		else if( pFDI->IsDirectory() )
		{
			list->AddTail( pFDI );
			EnumFiles( ConcPath( root, pFDI->GetFileName() ), list );
		}
		else if( pFDI->IsActualFile() )
		{
			list->AddTail( pFDI );
		}
		else
		{
			delete pFDI;
		}
	}
	while( FindNextFile( sh, &ffd ) );

	FindClose( sh );

	return list->GetCount();
}

int CFileInformation::EnumDirFiles( CString root, P_FI_List list ) 
{
	WIN32_FIND_DATA ffd;
	CString         path = ConcPath( root, _T("*.*") );
	HANDLE          sh   = FindFirstFile( path, &ffd );

	if( INVALID_HANDLE_VALUE == sh )
		return 0;

	do
	{
		CFileInformation* pFDI = new CFileInformation( ffd, root );

		if( pFDI->IsRootFile() )
		{
			delete pFDI;
		}
		else if( pFDI->IsDirectory() )
		{
			list->AddTail( pFDI );
		}
		else if( pFDI->IsActualFile() )
		{
			list->AddTail( pFDI );
		}
		else
		{
			delete pFDI;
		}
	}
	while( FindNextFile( sh, &ffd ) );

	FindClose( sh );

	return list->GetCount();
}

int CFileInformation::EnumFilesExt( CString root, CString ext, P_FI_List list ) 
{
	WIN32_FIND_DATA ffd;
	CString         path = ConcPath( root, _T("*.*") );
	HANDLE          sh   = FindFirstFile( path, &ffd );

	if( INVALID_HANDLE_VALUE == sh )
		return 0;

	do
	{
		CFileInformation* pFDI = new CFileInformation( ffd, root );

		if( pFDI->IsRootFile() )
		{
			delete pFDI;
		}
		else if( pFDI->IsDirectory() )
		{
			list->AddTail( pFDI );
			EnumFilesExt( ConcPath( root, pFDI->GetFileName() ), ext, list );
		}
		else if( pFDI->IsActualFile() && pFDI->GetFileExt().CompareNoCase( ext ) == 0 )
		{
			list->AddTail( pFDI );
		}
		else
		{
			delete pFDI;
		}
	}
	while( FindNextFile( sh, &ffd ) );

	FindClose( sh );

	return list->GetCount();
}

int CFileInformation::EnumDirFilesExt( CString root, CString ext, P_FI_List list ) 
{
	WIN32_FIND_DATA ffd;
	CString         path = ConcPath( root, _T("*.*") );
	HANDLE          sh   = FindFirstFile( path, &ffd );

	if( INVALID_HANDLE_VALUE == sh )
		return 0;

	do
	{
		CFileInformation* pFDI = new CFileInformation( ffd, root );

		if( pFDI->IsRootFile() )
		{
			delete pFDI;
		}
		else if( pFDI->IsDirectory() )
		{
			list->AddTail( pFDI );
		}
		else if( pFDI->IsActualFile() && pFDI->GetFileExt().CompareNoCase( ext ) == 0 )
		{
			list->AddTail( pFDI );
		}
		else
		{
			delete pFDI;
		}
	}
	while( FindNextFile( sh, &ffd ) );

	FindClose( sh );

	return list->GetCount();
}

void CFileInformation::SortFiles( P_FI_List list )
{
	if( list == NULL || list->GetCount() == 0 )
		return;

	FI_List tempList;
	FI_List fileList;
	FI_List dirList;

	CopyFiles( list, &tempList );
	list->RemoveAll();

	POSITION          listPos = tempList.GetHeadPosition();
	CFileInformation* pFDI    = NULL;

	while( listPos != NULL )
	{
		pFDI = tempList.GetNext( listPos );
		if( pFDI->IsDirectory() )
			dirList.AddTail( pFDI );
		else
			fileList.AddTail( pFDI );
	}

	SortFilesABC( &dirList );
	SortFilesABC( &fileList );

	listPos = dirList.GetHeadPosition();
	while( listPos != NULL )
		list->AddTail( dirList.GetNext( listPos ) );

	listPos = fileList.GetHeadPosition();
	while( listPos != NULL )
		list->AddTail( fileList.GetNext( listPos ) );
	
	tempList.RemoveAll();
	fileList.RemoveAll();
	dirList.RemoveAll();

	return;
}

void CFileInformation::CopyFiles( const P_FI_List oldList, P_FI_List newList )
{
	if( oldList == NULL || newList == NULL || oldList->GetCount() == 0 )
		return;

	POSITION listPos = oldList->GetHeadPosition();
	while( listPos != NULL )
		newList->AddTail( oldList->GetNext( listPos ) );

	return;
}

void CFileInformation::CopyFilesAndFI( const P_FI_List oldList, P_FI_List newList )
{
	if( oldList == NULL || newList == NULL || oldList->GetCount() == 0 )
		return;

	POSITION listPos = oldList->GetHeadPosition();
	while( listPos != NULL )
		newList->AddTail( new CFileInformation( *oldList->GetNext( listPos ) ) );

	return;
}

void CFileInformation::SortFilesABC( P_FI_List list )
{
	if( list == NULL || list->GetCount() == 0 )
		return;

	CList< CFileInformation, CFileInformation> tempList;
	
	POSITION listPos = list->GetHeadPosition();
	while( listPos != NULL )
	{
		tempList.AddTail( list->GetAt( listPos ) );
		list->GetNext( listPos );
	}
	RemoveFiles( list );
	
	int              i,
		             j,
			         count = tempList.GetCount();
	POSITION         pos1,
		             pos2;
	CFileInformation fdi1,
		             fdi2;
	
	for( j = 0 ; j < count ; j++ )
	{
		pos1 = tempList.GetHeadPosition();

		for( i = 0 ; i < count - 1 ; i++ )
		{
			pos2 = pos1;
			fdi1 = tempList.GetNext( pos1 );
			fdi2 = tempList.GetAt(   pos1 );
			if( fdi1.GetFileName().Compare( fdi2.GetFileName() ) > 0 )
			{
				tempList.SetAt( pos2, fdi2 );
				tempList.SetAt( pos1, fdi1 );
			}
		}
	}

	listPos = tempList.GetHeadPosition();
	while( listPos != NULL )
		list->AddTail( new CFileInformation( tempList.GetNext( listPos ) ) );
	tempList.RemoveAll();
}

BOOL CFileInformation::RemoveFiles( P_FI_List list )
{
	if( list == NULL || list->GetCount() == 0 )
		return FALSE;

	POSITION          listPos = list->GetHeadPosition();
	CFileInformation* pFDI    = NULL;

	while( listPos != NULL )
	{
		pFDI = list->GetNext( listPos );

		if( pFDI == NULL )
			throw "pFDI == NULL";
		
		delete pFDI;
	}

	list->RemoveAll();

	return TRUE;
}

EFileAction CFileInformation::CompareFiles( P_FI_List oldList, P_FI_List newList, CFileInformation& fi )
{
	EFileAction       faType     = faNone;
	POSITION          newListPos = NULL;
	POSITION          oldListPos = NULL;
	CFileInformation* newFI      = NULL;
	CFileInformation* oldFI      = NULL;
	int               nNew       = newList->GetCount();
	int               nOld       = oldList->GetCount();

	if( nOld == nNew )
	{
		newListPos = newList->GetHeadPosition();
		oldListPos = oldList->GetHeadPosition();
		while( oldListPos != NULL && newListPos != NULL )
		{
			oldFI = oldList->GetNext( oldListPos );
			newFI = newList->GetNext( newListPos );

			if( *oldFI != *newFI )
			{
				fi     = *newFI;
				faType = faChange;
				break;
			}
		}
	}
	else if( nOld > nNew )
	{
		BOOL isFind; 

		oldListPos = oldList->GetHeadPosition();
		while( oldListPos != NULL )
		{
			oldFI = oldList->GetNext( oldListPos );
		
			isFind = TRUE;

			newListPos = newList->GetHeadPosition();
			while( newListPos != NULL )
			{
				newFI = newList->GetNext( newListPos );
	
				if( *oldFI == *newFI )
				{
					isFind = FALSE;
					break;
				}
			}

			if( isFind )
			{
				fi     = *oldFI;
				faType = faDelete;
				break;
			}
		}
	}
	else if( nOld < nNew )
	{
		BOOL isFind; 

		newListPos = newList->GetHeadPosition();
		while( newListPos != NULL )
		{
			newFI = newList->GetNext( newListPos );
		
			isFind = TRUE;

			oldListPos = oldList->GetHeadPosition();
			while( oldListPos != NULL )
			{
				oldFI = oldList->GetNext( oldListPos );
	
				if( *oldFI == *newFI )
				{
					isFind = FALSE;
					break;
				}
			}

			if( isFind )
			{
				fi     = *newFI;
				faType = faCreate;
				break;
			}
		}
	}

	return faType;
}

BOOL CFileInformation::FindFilePath( CString root, CString& file )
{
	WIN32_FIND_DATA ffd;
	CString         path   = ConcPath( root, _T("*.*") );
	HANDLE          sh     = FindFirstFile( path, &ffd );
	BOOL            retval = FALSE;

	if( INVALID_HANDLE_VALUE == sh )
		return FALSE;

	do
	{
		CFileInformation fdi( ffd, root );

		if( fdi.IsRootFile() )
			continue;

		if( fdi.IsDirectory() &&
			FindFilePath( ConcPath( root, fdi.GetFileName() ), file ) )
		{
			retval = TRUE;
			break;
		}
		
		if( fdi.IsActualFile() && fdi.IsSomeFileName( file ) )
		{
			file   = fdi.GetFilePath();
			retval = TRUE;
			break;
		}
	}
	while( FindNextFile( sh, &ffd ) );

	FindClose( sh );

	return retval;
}


BOOL CFileInformation::FindFilePathOnDisk( CString& file )
{
    DWORD dwDriveMask = GetLogicalDrives();
	int   i,
		  max_drv     = 26;

#ifdef _UNICODE
	wchar_t  szDir[4];	szDir[1] = TEXT(':'); szDir[2] = TEXT('\\'); szDir[3] = 0;
#endif

#ifndef _UNICODE
	char  szDir[4];	szDir[1] = TEXT(':'); szDir[2] = TEXT('\\'); szDir[3] = 0;
#endif

	// enumerate all logical, fixed drives
	for( i = 0; i < max_drv; dwDriveMask >>= 1, i++ )
	{
		// if logical drive exists
		if( dwDriveMask & 0x01 )
		{
			szDir[0] = TEXT('A') + i;

			if( GetDriveType( szDir ) == DRIVE_FIXED && // if it is a fixed drive
				FindFilePath( szDir, file ) )
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CFileInformation::FindFilePathOnCD( CString& file )
{
    DWORD dwDriveMask = GetLogicalDrives();
	int   i,
		  max_drv     = 26;

#ifdef _UNICODE
	wchar_t  szDir[4];	szDir[1] = TEXT(':'); szDir[2] = TEXT('\\'); szDir[3] = 0;
#endif

#ifndef _UNICODE
	char  szDir[4];	szDir[1] = TEXT(':'); szDir[2] = TEXT('\\'); szDir[3] = 0;
#endif

	// enumerate all logical, fixed drives
	for( i = 0; i < max_drv; dwDriveMask >>= 1, i++ )
	{
		// if logical drive exists
		if( dwDriveMask & 0x01 )
		{
			szDir[0] = TEXT('A') + i;

			if( GetDriveType( szDir ) == DRIVE_CDROM && // if it is a cd drive
				FindFilePath( szDir, file ) )
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CFileInformation::Load(CString file)
{
	if(	!file.IsEmpty() && file[ file.GetLength() - 1 ] == L'\\' )
		file = file.Left( file.GetLength() - 1 );
	
	WIN32_FIND_DATA ffd;
	HANDLE          hFind = FindFirstFile( file, &ffd );

	if( hFind != INVALID_HANDLE_VALUE )
	{
		SetFileData( ffd );
		SetFileDir( GetFileDir( file ) );
		FindClose( hFind );
		return TRUE;
	}

	return FALSE;
}

BOOL CFileInformation::Load(CString dir, CString file)
{
	WIN32_FIND_DATA ffd;
	HANDLE          hFind = FindFirstFile( ConcPath( dir, file ), &ffd );

	if( hFind != INVALID_HANDLE_VALUE )
	{
		SetFileData( ffd );
		SetFileDir( dir );
		FindClose( hFind );
		return TRUE;
	}

	return FALSE;
}

CString CFileInformation::GetFileDir(CString path) const
{
	CString dir;
	dir.Empty();
	int     pos = path.ReverseFind( L'\\' );
	
	if( pos != -1 )
		dir = path.Left( pos );
	
	return dir;
}

BOOL CFileInformation::IsFileExist() const
{
	CString path = GetFilePath();
	
	if( path.IsEmpty() )
		return FALSE;

	CFile cf;
	if( !cf.Open( path, CFile::modeRead|CFile::shareDenyNone ) )
		return FALSE;
	cf.Close();

	return TRUE;
}

void CFileInformation::CopyDir( CString oldRoot, CString newRoot )
{
	if( oldRoot.IsEmpty() || newRoot.IsEmpty() )
		return;

	CFileInformation* pFDI;
	WIN32_FIND_DATA   ffd;
	HANDLE            sh = FindFirstFile( CFileInformation::ConcPath( oldRoot, _T("*.*") ), &ffd );
	
	if( INVALID_HANDLE_VALUE == sh )
		return;

	CFileInformation::CreateDir( newRoot );

	do
	{
		pFDI = new CFileInformation( ffd, oldRoot );

		if( pFDI->IsRootFile() )
		{
			delete pFDI;
			continue;
		}

		if( pFDI->IsDirectory() )
		{
			CopyDir( CFileInformation::ConcPath( oldRoot, pFDI->GetFileName() ), 
				     CFileInformation::ConcPath( newRoot, pFDI->GetFileName() ) );
		}
		
		if( pFDI->IsActualFile() )
		{
			CopyFile( pFDI->GetFilePath(), 
				      CFileInformation::ConcPath( newRoot, pFDI->GetFileName() ),
					  FALSE );
		}

		delete pFDI;
	}
	while( FindNextFile( sh, &ffd ) );

	FindClose( sh );
}

void CFileInformation::MoveDir( CString oldRoot, CString newRoot )
{
	if( oldRoot.IsEmpty() || newRoot.IsEmpty() )
		return;

	CFileInformation* pFDI;
	WIN32_FIND_DATA   ffd;
	HANDLE            sh = FindFirstFile( CFileInformation::ConcPath( oldRoot, _T("*.*") ), &ffd );
	
	if( INVALID_HANDLE_VALUE == sh )
		return;

	CFileInformation::CreateDir( newRoot );
	
	do
	{
		pFDI = new CFileInformation( ffd, oldRoot );

		if( pFDI->IsRootFile() )
		{
			delete pFDI;
			continue;
		}

		if( pFDI->IsDirectory() )
		{
			MoveDir( CFileInformation::ConcPath( oldRoot, pFDI->GetFileName() ), 
				     CFileInformation::ConcPath( newRoot, pFDI->GetFileName() ) );
		}
		
		if( pFDI->IsActualFile() )
		{
			try
			{
				CFile::Rename( pFDI->GetFilePath(),
					           CFileInformation::ConcPath( newRoot, pFDI->GetFileName() ) );
			}
			catch(...)
			{
			}
		}
			
		delete pFDI;
	}
	while( FindNextFile( sh, &ffd ) );

	RemoveDir( oldRoot );

	FindClose( sh );
}

void CFileInformation::RemoveDir( CString dir )
{
    RemoveDirectory( dir );

    WIN32_FIND_DATA find;
	HANDLE          hndle;
    char            *strFindFiles = (char*)malloc( MAX_PATH );
    
	memset( strFindFiles, 0, MAX_PATH );
	strcpy( strFindFiles, dir.GetBuffer( dir.GetLength() ) );
    strcat( strFindFiles, _T("\\*.*") );
    
	hndle = FindFirstFile( strFindFiles, &find );

    while( hndle != INVALID_HANDLE_VALUE ) 
    {    
        char *strFolderItem = (char*)malloc( MAX_PATH );
        
		memset( strFolderItem, 0, MAX_PATH );
		strcpy( strFolderItem, dir.GetBuffer( dir.GetLength() ) );
        strcat( strFolderItem, _T("\\") );
        strcat( strFolderItem, find.cFileName);

        if( ( !strcmp( find.cFileName, _T(".")  ) ) || 
			( !strcmp( find.cFileName, _T("..") ) ) ) 
        {
            RemoveDirectory( strFolderItem );

			free( strFolderItem );

            if( FindNextFile( hndle, &find ) )
			{
                continue;
			}
            else
			{
                RemoveDirectory( dir );
                break;
			}
        }

        if( find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{    
            RemoveDir( strFolderItem );
		}
        else
		{
            SetFileAttributes( strFolderItem, FILE_ATTRIBUTE_NORMAL );
            DeleteFile( strFolderItem );
		}

		free( strFolderItem );

        if( FindNextFile( hndle, &find ) )
		{
			continue;
		}
        else
		{
			break;
		}
    }

	free( strFindFiles );

    FindClose( hndle );
    SetFileAttributes( dir, FILE_ATTRIBUTE_DIRECTORY );
    RemoveDirectory( dir );
}

void CFileInformation::CreateDir( CString dir )
{
	if( dir.IsEmpty() )
		return;

	if( dir[ dir.GetLength() - 1 ] == L'\\' )
		dir = dir.Left( dir.GetLength() - 1 );

	if( dir[ dir.GetLength() - 1 ] == L':' )
		return;

	int pos = dir.ReverseFind( L'\\' );
	CreateDir( dir.Left( pos ) );

	CreateDirectory( dir, NULL );
}

BOOL CFileInformation::IsOk() const
{
	CString dir  = GetFileDir();
	CString name = GetFileName();

	if( IsDirectory() )
		return !dir.IsEmpty();
	else
		return !dir.IsEmpty() && !name.IsEmpty();
}

void CFileInformation::ParseDir( CString root, DirParsCallback action, LPVOID pData )
{
	if( root.IsEmpty() || action == NULL )
		return;

	CFileInformation* pFDI;
	WIN32_FIND_DATA   ffd;
	HANDLE            sh = FindFirstFile( CFileInformation::ConcPath( root, _T("*.*") ), &ffd );
	
	if( INVALID_HANDLE_VALUE == sh )
		return;

	action( root, pData );
	
	do
	{
		pFDI = new CFileInformation( ffd, root );

		if( pFDI->IsRootFile() )
		{
			delete pFDI;
			continue;
		}

		if( pFDI->IsDirectory() )
		{
			ParseDir( CFileInformation::ConcPath( root, pFDI->GetFileName() ), action, pData );
		}
		
		if( pFDI->IsActualFile() )
		{
			action( pFDI->GetFilePath(), pData );
		}

		delete pFDI;
	}
	while( FindNextFile( sh, &ffd ) );

	FindClose( sh );
}

UINT CFileInformation::GetPathLevel( CString path )
{
	UINT n   = 0;
	int  pos = 0;
	
	do
	{
		pos = path.Find( L'\\', pos );
		if( pos++ > 0 )
			n++;
	}
	while( pos > 0 );

	return n;
}

CString CFileInformation::GenerateNewFileName(CString path)
{
	CString newPath = path;
	FILE*   pFile   = fopen( newPath, _T("r") );

	while( pFile != NULL )
	{
		fclose( pFile );

		CFileInformation fi( newPath );
		newPath = fi.GetFileDir() + _T("\\_") + fi.GetFileName();
		
		pFile = fopen( newPath, _T("r") );
	}

	return newPath;
}

CString CFileInformation::GetFileDirectory( CString path )
{
	CString dir(_T(""));
	int     pos = path.ReverseFind( L'\\' );
	
	if( pos != -1 )
		dir = path.Left( pos );

	return dir;
}

CString CFileInformation::GetFileName( CString path )
{
	CString name(_T(""));
	int     pos = path.ReverseFind( L'\\' );
	
	if( pos != -1 )
		name = path.Right( path.GetLength() - pos - 1 );

	return name;
}

CString CFileInformation::GetFileNameWithoutExt( CString path )
{
	CString name = GetFileName( path );
	int     pos = name.ReverseFind( L'.' );
	
	if( pos != -1 )
		name = name.Left( pos );

	return name;
}

CString CFileInformation::GetFileExt( CString path )
{
	CString ext = GetFileName( path );
	int     pos = ext.ReverseFind( L'.' );
	
	if( pos != -1 )
		ext = ext.Right( ext.GetLength() - pos - 1 );

	return ext;
}


// NotifyDirCheck.cpp: implementation of the CNotifyDirCheck class.
//
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Simple Notification Callback
//////////////////////////////////////////////////////////////////////

UINT DefaultNotificationCallback( CFileInformation fiObject, EFileAction faAction, LPVOID lpData )
{
	CString csBuffer;
	CString csFile = fiObject.GetFilePath();

	if( IS_CREATE_FILE( faAction ) )
	{
		csBuffer.Format( "Created %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_DELETE_FILE( faAction ) )
	{
		csBuffer.Format( "Deleted %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_CHANGE_FILE( faAction ) )
	{
		csBuffer.Format( "Changed %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else
		return 1; //error, stop thread

	return 0; //success
}

//////////////////////////////////////////////////////////////////////
// Show Error Message Box
//////////////////////////////////////////////////////////////////////

static void ErrorMessage( CString failedSource )
{
	LPVOID  lpMsgBuf;
	CString csError;
	DWORD   dwLastError = GetLastError();//error number

	//make error comments
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER| 
				   FORMAT_MESSAGE_FROM_SYSTEM| 
				   FORMAT_MESSAGE_IGNORE_INSERTS,
				   NULL,
				   dwLastError,
				   MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),// Default language
				   (LPTSTR)&lpMsgBuf,
				   0,
				   NULL );

	csError.Format( "%s Error N%d\n%s", failedSource, dwLastError, (LPCTSTR)lpMsgBuf );
	AfxMessageBox( csError, MB_OK|MB_ICONSTOP );

	// Free the buffer.
	LocalFree( lpMsgBuf );
}
	
//////////////////////////////////////////////////////////////////////
// Work Thread 
//////////////////////////////////////////////////////////////////////

UINT NotifyDirThread( LPVOID pParam )
{
	BOOL             bStop = FALSE;
	HANDLE           hDir  = NULL; 
	CNotifyDirCheck* pNDC  = (CNotifyDirCheck*)pParam;
	FI_List          newFIL,
		             oldFIL;
	EFileAction      faAction;
	CFileInformation fi;

	if( pNDC == NULL )
		return 0;

	hDir = FindFirstChangeNotification( pNDC->GetDirectory(),
										FALSE,
										FILE_NOTIFY_CHANGE_FILE_NAME  |
										FILE_NOTIFY_CHANGE_DIR_NAME   |
										FILE_NOTIFY_CHANGE_SIZE       |
										FILE_NOTIFY_CHANGE_LAST_WRITE |
										FILE_NOTIFY_CHANGE_ATTRIBUTES );

	if( hDir == INVALID_HANDLE_VALUE )
	{
		ErrorMessage( _T("FindFirstChangeNotification") );
		return 0;
	}
	
 	while( pNDC->IsRun() )
	{
		CFileInformation::RemoveFiles( &oldFIL );
		CFileInformation::EnumFiles( pNDC->GetDirectory(), &oldFIL );

		bStop = FALSE;

		while( WaitForSingleObject( hDir, WAIT_TIMEOUT ) != WAIT_OBJECT_0 )
		{	if( !pNDC->IsRun() )
			{
				bStop = TRUE;//to end
				break;
			}
		}
		if( bStop )
			break;//to end

		CFileInformation::RemoveFiles( &newFIL );
		CFileInformation::EnumFiles( pNDC->GetDirectory(), &newFIL );
		
		Sleep( WAIT_TIMEOUT );

		faAction = CFileInformation::CompareFiles( &oldFIL, &newFIL, fi );

		if( !IS_NOTACT_FILE( faAction ) )
		{
			NOTIFICATION_CALLBACK_PTR ncpAction = pNDC->GetActionCallback();

			if( ncpAction )	//call user's callback
				bStop = ( ncpAction( fi, faAction, pNDC->GetData() ) > 0 );
			else			//call user's virtual function
				bStop = ( pNDC->Action( fi, faAction ) > 0 );

			if( bStop )
				break;//to end
		}
		
		if( FindNextChangeNotification( hDir ) == 0 )
		{
			ErrorMessage( _T("FindNextChangeNotification") );
			return 0;
		}
	}

	//end point of notification thread
	CFileInformation::RemoveFiles( &newFIL );
	CFileInformation::RemoveFiles( &oldFIL );
	
	return FindCloseChangeNotification( hDir );
}

//////////////////////////////////////////////////////////////////////
// Class 
//////////////////////////////////////////////////////////////////////

CNotifyDirCheck::CNotifyDirCheck()
{
	SetDirectory( "" );
	SetActionCallback( NULL );
	SetData( NULL );
	SetStop();
	m_pThread = NULL;
}

CNotifyDirCheck::CNotifyDirCheck(  CString csDir, NOTIFICATION_CALLBACK_PTR ncpAction, LPVOID lpData )
{
	SetDirectory( csDir );
	SetActionCallback( ncpAction );
	SetData( lpData );
	SetStop();
	m_pThread = NULL;
}

CNotifyDirCheck::~CNotifyDirCheck()
{
	Stop();
}

BOOL CNotifyDirCheck::Run()
{
	if( IsRun() || m_pThread != NULL || m_csDir.IsEmpty() )
		return FALSE;

	SetRun();
	m_pThread = AfxBeginThread( NotifyDirThread, this );
	
	if( m_pThread == NULL )
		SetStop();

	return IsRun();
}

void CNotifyDirCheck::Stop()
{
	if( !IsRun() || m_pThread == NULL )
		return;
	
	SetStop();

	WaitForSingleObject( m_pThread->m_hThread, 2 * NOTIFICATION_TIMEOUT );
	m_pThread = NULL;
}

UINT CNotifyDirCheck::Action( CFileInformation fiObject, EFileAction faAction )
{
	CString csBuffer;
	CString csFile = fiObject.GetFilePath();

	if( IS_CREATE_FILE( faAction ) )
	{
		csBuffer.Format( "Created %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_DELETE_FILE( faAction ) )
	{
		csBuffer.Format( "Deleted %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else if( IS_CHANGE_FILE( faAction ) )
	{
		csBuffer.Format( "Changed %s", csFile );
		AfxMessageBox( csBuffer );
	}
	else
		return 1; //error, stop thread

	return 0; //success
}

UINT DirCallback( ui::mfc::CFileInformation fiObject, ui::mfc::EFileAction faAction, LPVOID lpData ) { 
  FileObserver* file_observer = (FileObserver*) lpData;
  CString           csBuffer;
	CString           csFile = fiObject.GetFilePath();	

	if( IS_CREATE_FILE( faAction ) )
	{
    file_observer->OnCreateFile(fiObject.GetFilePath().GetString());
	}
	else if( IS_DELETE_FILE( faAction ) )
	{
    file_observer->OnDeleteFile(fiObject.GetFilePath().GetString());
	}
	else if( IS_CHANGE_FILE( faAction ) )
	{
    file_observer->OnChangeFile(fiObject.GetFilePath().GetString());    
	}
	else
	{
		return 1; //error, stop thread
	}
  return 0;
}

FileObserverImp::FileObserverImp(FileObserver* file_observer) : 
     ui::FileObserverImp(file_observer) {  
	notify_dir_change_.SetData(file_observer);	
	notify_dir_change_.SetActionCallback(DirCallback);	
}


} // namespace mfc
} // namespace ui
} // namespace host
} // namespace psycle