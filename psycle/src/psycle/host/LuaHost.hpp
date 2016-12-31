// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"
#include "LuaInternals.hpp"
#include "LockIF.hpp"

struct lua_State;
struct luaL_Reg;

namespace psycle {
namespace host {

namespace ui { 
  class Commands; 
  class Canvas;  
  class Frame;
  class MenuContainer;
  class Systems;
}
    
class LuaPlugin;

typedef boost::shared_ptr<LuaPlugin> LuaPluginPtr;
extern boost::shared_ptr<LuaPlugin> nullPtr;

class LuaPlugin;

class LuaControl : public LockIF {
 public:
  LuaControl();
  virtual ~LuaControl();
  
  void Load(const std::string& filename);
  virtual void PrepareState();
  void Run();
  void Start();
  virtual void Free();  

  
  lua_State* state() const { return L; }

  // LockIF Implementation
  void lock() const { ::EnterCriticalSection(&cs); }
  void unlock() const { ::LeaveCriticalSection(&cs); }

  std::string install_script() const;
  virtual PluginInfo meta() const;

  void yield();
  void resume();
  
  protected:
   lua_State* L;
   lua_State* LM;
   std::auto_ptr<ui::Commands> invokelater_;   
   PluginInfo parse_info() const;
  private:   
   mutable CRITICAL_SECTION cs;   
};

class LuaStarter : public LuaControl {
 public:
  virtual void PrepareState();  
  static int addmenu(lua_State* L);
  static int replacemenu(lua_State* L);
  static int addextension(lua_State* L);
};

static const int CHILDVIEWPORT = 1;
static const int FRAMEVIEWPORT = 2;

static const int MDI = 3;
static const int SDI = 4;

class LuaProxy : public LuaControl {
 public:
  LuaProxy(LuaPlugin* plug, const std::string& dllname);
  ~LuaProxy();

  // Host accessors
  LuaPlugin& host() { return *host_; }
  LuaPlugin& host() const { return *host_; }  
  LuaMachine* lua_mac() { return lua_mac_; };

  virtual PluginInfo meta() const;
	
  // Script Control	
  void Init();  
  void Reload();
  void PrepareState();
  virtual void Free() {
    LuaControl::Free();
    frame_.reset();
  }

  boost::weak_ptr<ui::Canvas> canvas() {  return lua_mac_->canvas(); }

  // Plugin calls
  void SequencerTick();
  void ParameterTweak(int par, double val);
  void Work(int numsamples, int offset=0);
  void Stop();
  void OnTimer();
  void PutData(unsigned char* data, int size);
  int GetData(unsigned char **ptr, bool all);
	uint32_t GetDataSize();
  void Command(int lastnote, int inst, int cmd, int val);
  void NoteOn(int note, int lastnote, int inst, int cmd, int val);
	void NoteOff(int note, int lastnote, int inst, int cmd, int val);
  bool DescribeValue(int parameter, char * txt);
  void SeqTick(int channel, int note, int ins, int cmd, int val);	
  double Val(int par);
  std::string Id(int par);
	std::string Name(int par);		
	void Range(int par,int &minval, int &maxval);
	int Type(int par);				
  bool OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);  
  bool OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  void call_execute();
	void call_sr_changed(int rate);
	void call_aftertweaked(int idx);	  
	std::string call_help();
	    
  MachineUiType::Value ui_type() const { return lua_mac_->ui_type(); }
  void call_setprogram(int idx);
  int call_numprograms();
  int get_curr_program();
  std::string get_program_name(int bnkidx, int idx);
  MachinePresetType::Value prsmode() const { return lua_mac_->prsmode(); }  
  int num_cols() const { return lua_mac_->numcols(); }
	int num_parameter() const { return lua_mac_->numparams(); }
        
  void OnCanvasChanged();
  void OnActivated(int viewport);
  void OnDeactivated();
  
  boost::weak_ptr<ui::MenuContainer> menu_bar() { return menu_bar_; }

  template<typename T>   
  void InvokeLater(T& f) { 
    lock();
    invokelater_->Add(f);
    unlock();
  }  
  bool IsPsyclePlugin() const;
  bool HasDirectMetaAccess() const;
  void OpenInFrame();
  void ToggleViewPort();
  void set_userinterface(int user_interface) { user_interface_ = user_interface; }
  int userinterface() const { return user_interface_; }
  std::string title() const { return lua_mac_ ? lua_mac_->title() : "noname"; }
  void UpdateWindowsMenu();
  ui::Systems* systems();
  void update_systems_state(lua_State* L);    

private:
  void ExportCFunctions();
  static int invokelater(lua_State* L);
	// script callbacks
  static int set_parameter(lua_State* L);
  static int alert(lua_State* L);
  static int confirm(lua_State* L);  
  static int terminal_output(lua_State* L);  
  static int call_selmachine(lua_State* L);
  static int set_machine(lua_State* L);  
  static int set_menubar(lua_State* L);
  std::string ParDisplay(int par);
  std::string ParLabel(int par);
  const PluginInfo& cache_meta(const PluginInfo& meta) const {
     meta_cache_ = meta;
     is_meta_cache_updated_ = true;
     return meta_cache_;
  }  
  void OnFrameClose(ui::Frame&);
      
  mutable bool is_meta_cache_updated_;
  mutable PluginInfo meta_cache_;
  LuaPlugin *host_;
  LuaMachine* lua_mac_;	
  mutable CRITICAL_SECTION cs;  
  boost::weak_ptr<ui::MenuContainer> menu_bar_;
  boost::shared_ptr<ui::Frame> frame_;
  int user_interface_;
  std::auto_ptr<ui::Systems> systems_;  
};

class LuaPluginBind {
 public:
  static int open(lua_State *L);
  static const char* meta;  
  static int create(lua_State* L);
  static int gc(lua_State* L);
};

class Link {
  public:
   Link() : viewport_(CHILDVIEWPORT), user_interface_(MDI) {}
   Link(const std::string& dll_name, const std::string& label, int viewport, int user_interface) : 
      dll_name_(dll_name),
      label_(label),
      viewport_(viewport),
      user_interface_(user_interface) {
   }

   const std::string& dll_name() const { return dll_name_; }
   const std::string& label() const { return label_; }
   const int viewport() const { return viewport_; }
   const int user_interface() const { return user_interface_; }

   boost::weak_ptr<LuaPlugin> plugin;
   
 private:
   std::string label_, dll_name_;   
   int viewport_, user_interface_;
};

// Container for HostExtensions
class HostExtensions {     
 public:
  typedef std::list<LuaPluginPtr> List;  
  typedef std::map<std::uint16_t, Link> MenuMap;
  //typedef std::map<std::uint16_t, LuaPlugin*> MenuMap;

  HostExtensions(class CChildView* child_view) : child_view_(child_view), active_lua_(0), menu_pos_(8) {}
  ~HostExtensions() {}  

  void Load(CMenu* view_menu);
  void StartScript();
  void Free();  
  typedef HostExtensions::List::iterator iterator;
  virtual iterator begin() { return extensions_.begin(); }
  virtual iterator end() { return extensions_.end(); }
  virtual bool empty() const { return true; }
  void Add(const LuaPluginPtr& ptr) { extensions_.push_back(ptr); }
  void Remove(const LuaPluginPtr& ptr) {         
    RemoveFromWindowsMenu(ptr.get());
    extensions_.remove(ptr);        
  }
  HostExtensions::List Get(const std::string& name);
  LuaPluginPtr Get(int idx);  
  void ReplaceHelpMenu(Link& link, int pos);
  void AddViewMenu(Link& link);
  void AddHelpMenu(Link& link);
  CMenu* FindSubMenu(CMenu* parent, const std::string& text);
  MenuMap& menuItemIdMap() { return menuItemIdMap_; }
  void OnDynamicMenuItems(UINT nID);
  void OnPluginCanvasChanged(LuaPlugin& plugin);
  void OnPluginViewPortChanged(LuaPlugin& plugin, int viewport);
  void HideActiveLua();
  void HideActiveLuaMenu();
  void InitWindowsMenu();
  void RemoveFromWindowsMenu(LuaPlugin* plugin);
  LuaPluginPtr Execute(Link& link);
  void ChangeWindowsMenuText(LuaPlugin* plugin);
  void AddToWindowsMenu(Link& link);

 private:   
  lua_State*  load_script(const std::string& dllpath);
  void AutoInstall();
  std::vector<std::string> search_auto_install();
  std::string menu_label(const Link& link) const;

  HostExtensions::List extensions_;  
  MenuMap menuItemIdMap_;  
  class CChildView* child_view_;
  LuaPlugin* active_lua_;
  int menu_pos_;
  HMENU windows_menu_;
};

struct LuaGlobal {   
   static std::map<lua_State*, LuaProxy*> proxy_map;
   static LuaProxy* proxy(lua_State* L) {
     std::map<lua_State*, LuaProxy*>::iterator it = proxy_map.find(L);
     return it != proxy_map.end() ? it->second : 0;
   }   
   static lua_State* load_script(const std::string& dllpath);   
   static PluginInfo LoadInfo(const std::string& dllpath);         
   static Machine* GetMachine(int idx);
   static class LuaPlugin* GetLuaPlugin(int idx) {
      Machine* mac = GetMachine(idx);
      assert(mac);
      if (mac->_type == MACH_LUA) {
        return (LuaPlugin*) mac;
      }
      return 0;
   }
   static std::vector<LuaPlugin*> GetAllLuas();
   template<typename T>   
   static void InvokeLater(lua_State* L, T& f) {
     LuaGlobal::proxy(L)->InvokeLater(f); 
   }   
};

 
}  // namespace host
}  // namespace psycle