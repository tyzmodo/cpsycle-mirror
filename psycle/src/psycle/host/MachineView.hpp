#pragma once
#include "Psycle.hpp"

#include "MachineGui.hpp" //For SMachineCoords

#if PSYCLE__CONFIGURATION__USE_PSYCORE
namespace psycle {
	namespace core {
		class Song;
		class Machine;
	}
}
using namespace psycle::core;
#endif
#include "Canvas.hpp"

namespace psycle {
	namespace host {

		class CChildView;
		class CMainFrame;
		class WireGui;

#if !PSYCLE__CONFIGURATION__USE_PSYCORE
		class Song;
		class Machine;
#endif

		class MachineView : public PsycleCanvas::Canvas
		{
		public:
			MachineView(class Project* project);
			~MachineView();

			void SetParent(CChildView* parent, CMainFrame* main) {
				parent_ = parent;
				main_ = main;
				PsycleCanvas::Canvas::SetParent((CWnd*)parent_);
			}

			void Rebuild();
			void SetSolo(Machine* mac);
			void ShowNewMachineDlg(double x , double y, Machine* mac, bool from_event);
			// use this, if you want to delete a gui not called from 
			// the machinegui event, that will be deleted
			// (used e.g. in CGearRack)
			void DeleteMachineGui(Machine* mac);
			void SetDeleteMachineGui(Machine* mac, bool in_engine);
			MachineGui* CreateMachineGui(Machine* mac);
			// from_event has to be set to true, if you show the macprop dialog from inside
			// the machinegui OnEvent. In case of replace and delete this is important
			void DoMacPropDialog(Machine* mac, bool from_event);
			void ShowDialog(Machine* mac, double x, double y);
			void UpdatePosition(Machine* mac);
			void UpdateVUs(CDC* devc);			
			void SelectMachine(MachineGui* gui);
			virtual void OnEvent(PsycleCanvas::Event* ev);
			CChildView* child_view() { return parent_; }
			CMainFrame* main();

			void OnNewConnection(MachineGui* sender);
			void OnRewireEnd(WireGui* sender,
							 double x,
							 double y,
							 int picker);
			void OnWireRewire(WireGui* sender, int pick_point);
			void LockVu();
			void UnlockVu();
			// use the next both, if you want to delete a machingui itself from
			// inside a machingui event (like used in macprop dialog)
			void SetDeleteMachineGui(MachineGui* gui, bool in_engine) {
				del_machine_ = gui;
				del_in_engine_ = in_engine;
			}
			void InitSkin();
			void WriteStatusBar(const std::string& text);
			void AddMacViewUndo(); // place holder
			void CenterMaster();
			bool CheckUnsavedSong();
			Song* song();

			SMachineCoords	MachineCoords;			

		private:			
			void BuildWires();
			void WireUp(WireGui* sender,
						MachineGui* connect_to_gui,
						double x,
						double y,
						int picker);
			bool RewireSrc(Machine* tmac, Machine* dmac);
			bool RewireDest(Machine* tmac, Machine* dmac);			
			void RaiseMachinesToTop();
			void UpdateSoloMuteBypass();
			void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
			void LoadMachineBackground();
			void FindMachineSkin(CString findDir, CString findName, BOOL *result);
			
			CChildView* parent_;
			CMainFrame* main_;
			Project* project_;
			std::map<Machine*, MachineGui*> gui_map_;
			WireGui* del_line_;
			WireGui* rewire_line_;
			MachineGui* del_machine_;
			bool is_locked_;
			bool del_in_engine_;
			CBitmap machineskin;
			CBitmap machineskinmask;
			CBitmap machinebkg;
			HBITMAP hbmMachineSkin;
			HBITMAP hbmMachineBkg;	
			HBITMAP hbmMachineDial;	
			int bkgx;
			int bkgy;

		};
	}
}
