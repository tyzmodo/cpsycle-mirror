// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "Machine.hpp"
#include "Canvas.hpp"

struct lua_State;
struct luaL_Reg;

namespace psycle { namespace host {

struct menuitem { 	
    menuitem() {
#if !defined WINAMP_PLUGIN
      menu = 0;
#endif
      mid = -1; check = false; }
    std::string id; std::string label; 
#if !defined WINAMP_PLUGIN
    CMenu* menu;
#endif
    int mid; bool check;
  };

  struct LuaMenuBind {  
    static int open(lua_State *L);
    static const char* meta;
  private:
    static int create(lua_State *L);
    static int id(lua_State *L);
    static int label(lua_State *L);
    static int gc(lua_State* L);
    static int check(lua_State* L);
    static int uncheck(lua_State* L);
    static int checked(lua_State* L);
    static int addlistener(lua_State* L);
    static int notify(lua_State* L);
  };

  struct LuaCanvas : public canvas::Canvas {
    LuaCanvas(lua_State* state) : canvas::Canvas(), L(state) {}    
    virtual canvas::Item* OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaCanvasBind {
    static int open(lua_State *L);
    static const char* meta;    
   private:
    static int create(lua_State *L);
    static int gc(lua_State* L);
    static int root(lua_State* L);
    static int setcolor(lua_State* L);
    static int generate(lua_State* L);
  };

  struct LuaGroup : public canvas::Group {
    LuaGroup(lua_State* state) : canvas::Group(), L(state) {}
    LuaGroup(lua_State* state, canvas::Group* parent) :
      canvas::Group(parent, 0, 0), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaGroupBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setxy(lua_State *L);
    static int getxy(lua_State *L);
    static int getfocus(lua_State *L);
    static int getitems(lua_State* L);
    static int remove(lua_State* L);
    static int show(lua_State* L);
    static int hide(lua_State* L);
    static int gc(lua_State* L);    
  };
  
  struct LuaRect : public canvas::Rect {
    LuaRect(lua_State* state) : Rect(), L(state) {}
    LuaRect(lua_State* state, canvas::Group* parent, double x1, double y1, double x2, double y2) :
      canvas::Rect(parent, x1, y1, x2, y2), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaRectBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setpos(lua_State *L);
    static int setcolor(lua_State* L);
    static int setoutlinecolor(lua_State* L);
    static int setxy(lua_State *L);
    static int getxy(lua_State *L);
    static int gc(lua_State* L);
  };

  struct LuaLine : public canvas::Line {
    LuaLine(lua_State* state) : Line(), L(state) {}
    LuaLine(lua_State* state, canvas::Group* parent) :
      canvas::Line(parent), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaLineBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setcolor(lua_State* L);
    static int setpoints(lua_State* L);
    static int gc(lua_State* L);
    static int setxy(lua_State *L);
    static int getxy(lua_State *L);
  };

  struct LuaText : public canvas::Text {
    LuaText(lua_State* state) : canvas::Text(), L(state) {}
    LuaText(lua_State* state, canvas::Group* parent) :
      canvas::Text(parent), L(state) {}
    virtual bool OnEvent(canvas::Event* ev);
   private:
     lua_State* L;
  };

  struct LuaTextBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setxy(lua_State *L);
    static int settext(lua_State* L);
    static int setcolor(lua_State* L);
    static int getxy(lua_State *L);
    static int gc(lua_State* L);
  };

  struct LuaPixBind {
    static int open(lua_State *L);
    static const char* meta;
   private:
    static int create(lua_State *L);
    static int setxy(lua_State *L);    
    static int setsource(lua_State* L);
    static int setsize(lua_State* L);
    static int gc(lua_State* L);    
  };

  struct LuaBitmap {

  };

  } // namespace
} // namespace