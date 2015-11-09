// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#pragma once
#include <psycle/host/detail/project.hpp>
#include "plugininfo.hpp"
#include "LuaArray.hpp"
#include "LuaInternals.hpp"
#include "LuaGui.hpp"

struct lua_State;
struct luaL_Reg;

namespace universalis { namespace os {
	class terminal;
}}

namespace psycle { namespace host { namespace ui { namespace canvas { struct Event; }}}}

namespace psycle { namespace host {
  //controlling function header
static UINT StartThread (LPVOID param);

//structure for passing to the controlling function
typedef struct THREADSTRUCT
{
    class TDlg*    _this;
        //you can add here other parameters you might be interested on
} THREADSTRUCT;

class TDlg {
public:
  static UINT TDlg::StartThread (LPVOID param)
{
//    THREADSTRUCT*    ts = (THREADSTRUCT*)param;

    //here is the time-consuming process
    //which interacts with your dialog
    AfxMessageBox ("Thread is started!");

        //see the access mode to your dialog controls
    static const char szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";

				CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_DONTADDTORECENT,szFilter);
				if ( dlg.DoModal() == IDOK )
				{
        }

    //you can also call AfxEndThread() here
    return 1;
}

  void TDlg::OnStart()
{
        //call the thread on a button action or menu
    THREADSTRUCT *_param = new THREADSTRUCT;
    _param->_this = this;
    AfxBeginThread (&StartThread, _param);
}
};

class LuaPlugin;

class LuaProxy {
public:
	LuaProxy(LuaPlugin* plug, lua_State* state);
	~LuaProxy();

    const PluginInfo& info() const { return info_; }
	int num_cols() const { return plugimport_->numcols(); }
	int num_parameter() const { return plugimport_->numparams(); }
	double get_parameter_value(int numparam);
	std::string get_parameter_id(int numparam);
	std::string get_parameter_name(int numparam);
	std::string get_parameter_display(int numparam);
	std::string get_parameter_label(int numparam);
	void get_parameter_range(int numparam,int &minval, int &maxval);
	int get_parameter_type(int numparam);
	int call_data(unsigned char **ptr, bool all);
	uint32_t call_data_size();
	void call_putdata(unsigned char* data, int size);
	// calls from proxy to script
	void call_run();
	void call_init();
  LuaCanvas* call_canvas();
	PluginInfo call_info();
	void call_seqtick(int /*channel*/, int /*note*/, int /*ins*/, int /*cmd*/, int /*val*/);
	// calls if noteon modus used
	void call_command(int lastnote, int inst, int cmd, int val);
	void call_noteon(int note, int lastnote, int inst, int cmd, int val);
	void call_noteoff(int note, int lastnote, int inst, int cmd, int val);  
	void call_newline();
	void call_work(int num, int offset=0);
  void call_parameter(int numparameter, double val);
	void call_stop();
  void call_execute();
	void call_sr_changed(int rate);
	void call_aftertweaked(int idx);
	void call_menu(UINT id);
	std::string call_help();
	void free_state();
	void set_state(lua_State* state);
	void reload();
  void update_menu(void* menu);
  ui::MenuBar* get_menu(ui::Menu* menu);
  int gui_type() const { return plugimport_->gui_type(); }
  void call_setprogram(int idx);
  int call_numprograms();
  int get_curr_program();
  std::string get_program_name(int bnkidx, int idx);
  LuaMachine::PRSType prsmode() const { return plugimport_->prsmode(); }
  void lock() const { ::EnterCriticalSection(&cs); }
  void unlock() const { ::LeaveCriticalSection(&cs); }
private:
	void export_c_funcs();
	// script callbacks
	static int set_parameter(lua_State* L);
	static int message(lua_State* L);
	static int terminal_output(lua_State* L);
	static int call_filedialog(lua_State* L);
  static int call_selmachine(lua_State* L);
	static int set_machine(lua_State* L);

	void get_method_strict(lua_State* L, const char* method);
	bool get_method_optional(lua_State* L, const char* method);
	bool get_param(lua_State* L, int index, const char* method);

	std::string GetString();
	PluginInfo info_;
	LuaPlugin *plug_;
	LuaMachine* plugimport_;
	lua_State* L;
	static universalis::os::terminal * terminal;
  static int gui_type_;
  TDlg test;
  mutable CRITICAL_SECTION cs;
};

struct LuaHost {
   static lua_State* load_script(const std::string& dllpath);
   static LuaPlugin* LoadPlugin(const std::string& dllpath, int macIdx);
   static PluginInfo LoadInfo(const std::string& dllpath);
};
}
}