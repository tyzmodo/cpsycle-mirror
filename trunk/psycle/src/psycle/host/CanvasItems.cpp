// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "CanvasItems.hpp"
#include "Psycle.hpp"
#include "PsycleConfig.hpp"
#include "Ui.hpp"
#include "CScintilla.hpp"

namespace psycle {
namespace host  {
namespace ui {
namespace canvas {

void Rect::Draw(Graphics* g, Region& draw_region) {
  // double z = acczoom();  
  g->SetColor(fillcolor_);
  g->FillRect(0, 0, width_, height_);  
/*    CRect rect(x1_*z, y1_*z, x2_*z, y2_*z);
  CPen pen;
  pen.CreatePen(PS_SOLID, 1, ToCOLORREF(strokecolor_));
  CBrush brush(ToCOLORREF(fillcolor_));
  CBrush* pOldBrush = devc->SelectObject(&brush);
  CPen* pOldPen = devc->SelectObject(&pen);
  double alpha = 1; // (GetAlpha(fillcolor_)) / 255.0;
  if (alpha == 1) {
    if (bx_!=0 || by_!=0) {
      CPoint pt(bx_*z, by_*z);
      devc->RoundRect(rect, pt);
    } else {
      devc->Rectangle(rect);
    }
  } else {
    this->paintRect(*devc, rect, ToCOLORREF(strokecolor_),
      ToCOLORREF(fillcolor_), (alpha*127));
  }
  devc->SelectObject(pOldPen);
  devc->SelectObject(pOldBrush);*/
}

/*bool Rect::paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity) {
  XFORM rXform;
  hdc.GetWorldTransform(&rXform);
  XFORM rXform_new = rXform;
  rXform_new.eDx = zoomabsx();
  rXform_new.eDy = zoomabsy();
  hdc.SetGraphicsMode(GM_ADVANCED);
  hdc.SetWorldTransform(&rXform_new);
  dim.right -= dim.left;
  dim.bottom -= dim.top;
  dim.left = dim.top = 0;
  HDC tempHdc         = CreateCompatibleDC(hdc);
  BLENDFUNCTION blend = {AC_SRC_OVER, 0, opacity, 0};
  HBITMAP hbitmap;       // bitmap handle
  BITMAPINFO bmi;        // bitmap header
  // zero the memory for the bitmap info
  ZeroMemory(&bmi, sizeof(BITMAPINFO));
  // setup bitmap info
  // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas.
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = dim.right-dim.left;
  bmi.bmiHeader.biHeight = dim.bottom-dim.top;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = (dim.right-dim.left) * (dim.bottom-dim.top) * 4;
  // create our DIB section and select the bitmap into the dc
  hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0x0);
  SelectObject(tempHdc, hbitmap);
  SetDCPenColor(tempHdc, penCol);
  SetDCBrushColor(tempHdc, brushCol);
  HBRUSH br = CreateSolidBrush(brushCol);
  FillRect(tempHdc, &dim, br);
  AlphaBlend(hdc, dim.left, dim.top, dim.right, dim.bottom, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend);
  DeleteObject(hbitmap);
  DeleteObject(br);
  DeleteObject(tempHdc);
  hdc.SetGraphicsMode(GM_ADVANCED);
  hdc.SetWorldTransform(&rXform);
  return 0;
}*/

bool Rect::onupdateregion() {
  rgn_->SetRect(0, 0, width_, height_);  
  return true;
}

void Line::Draw(Graphics* g, Region& draw_region) {  
  g->SetColor(color());
  double mx, my;
  mx = my = 0;
  for (Points::iterator it = pts_.begin(); it != pts_.end(); ++it) {
    Point& pt = (*it);
    if (it != pts_.begin()) {
      g->DrawLine(mx, my, pt.first, pt.second);
    }
    mx = pt.first;
    my = pt.second;
  }  
}

Item::Ptr Line::Intersect(double x, double y, Event* ev, bool &worked) {
  /*double distance_ = 5;
  Point  p1 = PointAt(0);
  Point  p2 = PointAt(1);
  double  ankathede    = p1.first - p2.first;
  double  gegenkathede = p1.second - p2.second;
  double  hypetenuse   = sqrt(ankathede*ankathede + gegenkathede*gegenkathede);
  if (hypetenuse == 0) return 0;
  double cos = ankathede    / hypetenuse;
  double sin = gegenkathede / hypetenuse;
  int dx = static_cast<int> (-sin*distance_);
  int dy = static_cast<int> (-cos*distance_);
  std::vector<CPoint> pts;
  CPoint p;
  p.x = p1.first + dx; p.y = p1.second - dy;
  pts.push_back(p);
  p.x = p2.first + dx; p.y = p2.second - dy;
  pts.push_back(p);
  p.x = p2.first - dx; p.y = p2.second + dy;
  pts.push_back(p);
  p.x = p1.first - dx; p.y = p1.second + dy;
  pts.push_back(p);
  CRgn rgn;
  rgn.CreatePolygonRgn(&pts[0],pts.size(), WINDING);
  Item::Ptr item = rgn.PtInRegion(x-this->x(),y-this->y()) ? this : 0;
  rgn.DeleteObject();*/
  return Item::Ptr();
}

bool Line::onupdateregion() {  
  double x1, y1, x2, y2;
  double dist = 5;
  BoundRect(x1, y1, x2, y2);
  double zoom = 1.0; // parent() ? parent()->zoom() : 1.0;
  rgn_->SetRect((x1-dist)*zoom, (y1-dist)*zoom, (x2+2*dist+1)*zoom, (y2+2*dist+1)*zoom);    
  return true;
}

void Text::Init(double zoom) {
  color_ = 0;
  LOGFONT lfLogFont;
  memset(&lfLogFont, 0, sizeof(lfLogFont));
  lfLogFont.lfHeight = 12*zoom;
  strcpy(lfLogFont.lfFaceName, "Arial");
  font_.CreateFontIndirect(&lfLogFont);
}

bool Text::onupdateregion() {  
  Canvas* c =  const_cast<Text*>(this)->root();
  if (c) {
    HDC dc = GetDC(0);
    SIZE extents = {0};
    HFONT old_font =
      reinterpret_cast<HFONT>(SelectObject(dc, font_));
    GetTextExtentPoint32(dc, text_.c_str(), text_.length(),
      &extents);
    SelectObject(dc, old_font);
    ReleaseDC(0, dc);
    text_w = extents.cx;
    text_h = extents.cy;
    rgn_->SetRect(0, 0, text_w, text_h);
    return true;
  }    
  return false;
}

void Text::Draw(Graphics* g, Region& draw_region) { 
    g->SetColor(color_);
    g->DrawString(text_, 0, 0);    
}

// Pic
inline void PremultiplyBitmapAlpha(HDC hDC, HBITMAP hBmp)
{
  BITMAP bm = { 0 };
  GetObject(hBmp, sizeof(bm), &bm);
  BITMAPINFO* bmi = (BITMAPINFO*) _alloca(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
  ::ZeroMemory(bmi, sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  BOOL bRes = ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, NULL, bmi, DIB_RGB_COLORS);
  if( !bRes || bmi->bmiHeader.biBitCount != 32 ) return;
  LPBYTE pBitData = (LPBYTE) ::LocalAlloc(LPTR, bm.bmWidth * bm.bmHeight * sizeof(DWORD));
  if( pBitData == NULL ) return;
  LPBYTE pData = pBitData;
  ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, pData, bmi, DIB_RGB_COLORS);
  for( int y = 0; y < bm.bmHeight; y++ ) {
    for( int x = 0; x < bm.bmWidth; x++ ) {
      pData[0] = (BYTE)((DWORD)pData[0] * pData[3] / 255);
      pData[1] = (BYTE)((DWORD)pData[1] * pData[3] / 255);
      pData[2] = (BYTE)((DWORD)pData[2] * pData[3] / 255);
      pData += 4;
    }
  }
  ::SetDIBits(hDC, hBmp, 0, bm.bmHeight, pBitData, bmi, DIB_RGB_COLORS);
  ::LocalFree(pBitData);
}

void Pic::Draw(Graphics* g, Region& draw_region) {
  g->DrawImage(image_, 0, 0, width_, height_, xsrc_, ysrc_);
  // todo zoom  
}

void Pic::SetImage(Image* image) {
  STR();
  image_ = image;
  image_->size(width_, height_);  
  FLS();
}

bool Pic::onupdateregion() {
  rgn_->SetRect(0, 0, width_, height_);
  return true;
}

IMPLEMENT_DYNAMIC(CTree, CTreeCtrl) 

BEGIN_MESSAGE_MAP(CTree, CTreeCtrl)
ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnClick)
END_MESSAGE_MAP()

std::list<boost::shared_ptr<TreeItem> > Tree::SubChildren() {
    std::list<boost::shared_ptr<TreeItem> > allitems;
    std::list<boost::shared_ptr<TreeItem> >::iterator it = children_.begin();
    for (; it != children_.end(); ++it) {
      boost::shared_ptr<TreeItem> item = *it;
      allitems.push_back(item);
      std::list<boost::shared_ptr<TreeItem> > subs = item->SubChildren();
      std::list<boost::shared_ptr<TreeItem> >::iterator itsub = subs.begin();
      for (; itsub != subs.end(); ++itsub) {
        allitems.push_back(*it);
      }
    }
    return allitems;
  }

void Tree::onclick(HTREEITEM hItem) {
  TreeItem::TreeItemList subitems = SubChildren(); 
  TreeItem::TreeItemList::iterator it = subitems.begin();
  for ( ; it != subitems.end(); ++it) {
    TreeItem::Ptr subitem = *it;
    if (subitem->hItem == hItem) {
      subitem->OnClick();
      break;
    }
   }  
}


} // namespace canvas
} // namespace ui
} // namespace host
} // namespace psycle