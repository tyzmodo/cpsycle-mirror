#include "stdafx.h"
#include "psycle.h"
#include "ConfigDlg.h"
#include "MainFrm.h"
#include "midiinput.h"
#include "NewMachine.h"
#include <operating_system/exception.h>
#include <sstream>
///\file
///\brief implementation file for psycle::host::CPsycleApp.
namespace psycle
{
	namespace host
	{
		BEGIN_MESSAGE_MAP(CPsycleApp, CWinApp)
			//{{AFX_MSG_MAP(CPsycleApp)
			ON_COMMAND(ID_ABOUTPSYCLE, OnAboutpsycle)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		CPsycleApp::CPsycleApp() throw(std::runtime_error)
		{
			operating_system::exceptions::translated::new_thread();
			// Error Logging window
			Global::pLogWindow->Create(IDD_ERRORLOGGER,0);
			Global::pLogWindow->Validate();
			// support for unicode characters on mswin98
			{
				#if 0
					if(!::LoadLibrary("unicows"))
					{
						std::runtime_error e("could not load library unicows: " + operating_system::exceptions::code_description());
						::MessageBox(0, e.what(), "exception", MB_OK | MB_ICONERROR);
						throw e;
					}
				#endif // 0
			}
		}

		CPsycleApp::~CPsycleApp()
		{
		}

		CPsycleApp theApp; /// The one and only CPsycleApp object

		BOOL CPsycleApp::InitInstance()
		{
			// Standard initialization
			// If you are not using these features and wish to reduce the size
			//  of your final executable, you should remove from the following
			//  the specific initialization routines you do not need.

			/* deprecated
				#if defined _AFXDLL
					Enable3dControls();			// Call this when using MFC in a shared DLL
				#else
					Enable3dControlsStatic();	// Call this when linking to MFC statically
				#endif
			*/

			SetRegistryKey(_T("AAS")); // Change the registry key under which our settings are stored.
			
			LoadStdProfileSettings();  // Load standard INI file options (including MRU)
			
			// To create the main window, this code creates a new frame window
			// object and then sets it as the application's main window object.
			
			CMainFrame* pFrame = new CMainFrame;
			m_pMainWnd = pFrame;
			
			if (!Global::pConfig->Read()) // problem reading registry info. missing or damaged
			{
				Global::pConfig->_initialized = false;
				CConfigDlg dlg("Psycle configuration");
				dlg.Init(Global::pConfig);
				if (dlg.DoModal() == IDOK)
				{
					pFrame->m_wndView._outputActive = true;
					Global::pConfig->_initialized = true;
				}
			}
			else
			{
				pFrame->m_wndView._outputActive = true;
			}

			// create and load the frame with its resources
			pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 0, 0);
				// For some reason, there'a First-Chance exception when
				// another pFrame member is called after this LoadFrame
				// (for example, pFrame->ShowWindow(SW_MAXIMIZE);)

			// Sets Icon
			HICON tIcon;
			tIcon=LoadIcon(IDR_MAINFRAME);
			pFrame->SetIcon(tIcon,true);
			pFrame->SetIcon(tIcon,false);
			
			pFrame->m_wndView.LoadMachineSkin();
			pFrame->m_wndView.LoadPatternHeaderSkin();
			pFrame->m_wndView.LoadMachineBackground();
			pFrame->m_wndView.RecalcMetrics();
			pFrame->m_wndView.RecalculateColourGrid();

			// The one and only window has been initialized, so show and update it.

			pFrame->ShowWindow(SW_MAXIMIZE);
			
			// center master machine
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->_x=(pFrame->m_wndView.CW-pFrame->m_wndView.MachineCoords.sMaster.width)/2;
			pFrame->m_wndView._pSong->_pMachine[MASTER_INDEX]->_y=(pFrame->m_wndView.CH-pFrame->m_wndView.MachineCoords.sMaster.width)/2;
			
			pFrame->UpdateWindow();
			
			CNewMachine::LoadPluginInfo();

			// Show splash screen	
			if (Global::pConfig->_showAboutAtStart)
			{
				OnAboutpsycle();
			}
			
			ProcessCmdLine(pFrame); // Process Command Line

			LoadRecent(pFrame); // Import recent files from registry.

			return TRUE;
		}

		/////////////////////////////////////////////////////////////////////////////
		// CPsycleApp message handlers


		int CPsycleApp::ExitInstance() 
		{
			_global.pConfig->Write();
			_global.pConfig->_pOutputDriver->Enable(false);
			Sleep(LOCK_LATENCY);
			_global.pConfig->_pMidiInput->Close();
			CNewMachine::DestroyPluginInfo();
			return CWinApp::ExitInstance();
		}



		/////////////////////////////////////////////////////////////////////////////
		// CAboutDlg dialog used for App About

		class CAboutDlg : public CDialog
		{
		public:
			CAboutDlg();

		// Dialog Data
			//{{AFX_DATA(CAboutDlg)
			enum { IDD = IDD_ABOUTBOX };
			CStatic	m_asio;
			CEdit	m_sourceforge;
			CEdit	m_psycledelics;
			CStatic	m_steincopyright;
			CStatic	m_headerdlg;
			CButton	m_showabout;
			CStatic	m_headercontrib;
			CStatic	m_aboutbmp;
			CEdit	m_contrib;
			CStatic m_versioninfo;
			//}}AFX_DATA

			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CAboutDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

		// Implementation
		protected:
			//{{AFX_MSG(CAboutDlg)
			afx_msg void OnContributors();
			virtual BOOL OnInitDialog();
			afx_msg void OnShowatstartup();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
		{
			//{{AFX_DATA_INIT(CAboutDlg)
			//}}AFX_DATA_INIT
		}

		void CAboutDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CAboutDlg)
			DDX_Control(pDX, IDC_ASIO, m_asio);
			DDX_Control(pDX, IDC_EDIT5, m_sourceforge);
			DDX_Control(pDX, IDC_EDIT2, m_psycledelics);
			DDX_Control(pDX, IDC_STEINBERGCOPY, m_steincopyright);
			DDX_Control(pDX, IDC_HEADERDLG, m_headerdlg);
			DDX_Control(pDX, IDC_SHOWATSTARTUP, m_showabout);
			DDX_Control(pDX, IDC_HEADER, m_headercontrib);
			DDX_Control(pDX, IDC_ABOUTBMP, m_aboutbmp);
			DDX_Control(pDX, IDC_EDIT1, m_contrib);
			DDX_Control(pDX, IDC_VERSION_INFO, m_versioninfo);
			//}}AFX_DATA_MAP
		}

		BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
			//{{AFX_MSG_MAP(CAboutDlg)
			ON_BN_CLICKED(IDC_BUTTON1, OnContributors)
			ON_BN_CLICKED(IDC_SHOWATSTARTUP, OnShowatstartup)
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		// App command to run the dialog

		/////////////////////////////////////////////////////////////////////////////
		// CPsycleApp message handlers

		void CPsycleApp::OnAboutpsycle() /// Shows About Box
		{
			CAboutDlg dlg;
			dlg.DoModal();
		}

		void CAboutDlg::OnContributors() 
		{
			if ( m_aboutbmp.IsWindowVisible() )
			{
				m_aboutbmp.ShowWindow(SW_HIDE);
				m_contrib.ShowWindow(SW_SHOW);
				m_headercontrib.ShowWindow(SW_SHOW);
				m_headerdlg.ShowWindow(SW_SHOW);
				m_psycledelics.ShowWindow(SW_SHOW);
				m_sourceforge.ShowWindow(SW_SHOW);
				m_asio.ShowWindow(SW_SHOW);
				m_steincopyright.ShowWindow(SW_SHOW);
			}
			else 
			{
				m_aboutbmp.ShowWindow(SW_SHOW);
				m_contrib.ShowWindow(SW_HIDE);
				m_headercontrib.ShowWindow(SW_HIDE);
				m_headerdlg.ShowWindow(SW_HIDE);
				m_psycledelics.ShowWindow(SW_HIDE);
				m_sourceforge.ShowWindow(SW_HIDE);
				m_asio.ShowWindow(SW_HIDE);
				m_steincopyright.ShowWindow(SW_HIDE);
			}
		}

		BOOL CAboutDlg::OnInitDialog() 
		{
			CDialog::OnInitDialog();

			m_contrib.SetWindowText
					(
						"Josep M� Antol�n. [JAZ]\t\tDeveloper since release 1.5" "\r\n"
						"Jeremy Evers [pooplog]\t\tDeveloper since release 1.7" "\r\n"
						"James Redfern [alk]\t\tDeveloper since release 1.7.6" "\r\n"
						"Johan Boul� [bohan]\t\tDeveloper since release 1.7.3" "\r\n"
						"Daniel Arena\t\t\tDeveloper in release 1.5 & 1.6" "\r\n"
						"Marcin Kowalski [FideLoop]\t\tDeveloper in release 1.6" "\r\n"
						"Mark McCormack\t\t\tMIDI (in) Support" "\r\n"
						"Mats H�jlund\t\t\tMain Developer until release 1.5" "\r\n"
						"Juan Antonio Arguelles [Arguru]\tCreator and Main Developer until release 1.0" "\r\n" // (Internal Recoding) .. doesn't fit in the small box :/
						"Hamarr Heylen\t\t\tInitial Graphics" "\r\n"
						"David Buist\t\t\tAdditional Graphics" "\r\n"
						"frown\t\t\t\tAdditional Graphics" "\r\n"
						"/\\/\\ark\t\t\t\tAdditional Graphics" "\r\n"
						"Haralabos Michael\t\tInstaller and Debugging help"
					);
			m_showabout.SetCheck(Global::pConfig->_showAboutAtStart);

			m_psycledelics.SetWindowText("http://psycle.pastnotecut.org");
			m_sourceforge.SetWindowText("http://psycle.sourceforge.net");

			std::ostringstream s;
			s << "Version " VERSION_NUMBER
				//" $Date$"
				//VERSION_NUMBER
				" [built on " __DATE__ ", " __TIME__ "]";
			m_versioninfo.SetWindowText(s.str().c_str());

			// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
			return TRUE;
		}

		void CAboutDlg::OnShowatstartup() 
		{
			if ( m_showabout.GetCheck() )  Global::pConfig->_showAboutAtStart = true;
			else Global::pConfig->_showAboutAtStart=false;
		}

		void CPsycleApp::ProcessCmdLine(CMainFrame* pFrame)
		{
			if(*(m_lpCmdLine) != 0)
			{
				char * tmpName =new char[257];
				strncpy(tmpName, m_lpCmdLine+1, strlen(m_lpCmdLine+1) -1 );
				tmpName[strlen(m_lpCmdLine+1) -1 ] = 0;
				pFrame->m_wndView.OnFileLoadsongNamed(tmpName, 1);
				delete tmpName;
			}
		}

		void CPsycleApp::LoadRecent(CMainFrame* pFrame)
		{
			//This one should be into Configuration class. It isn't, coz I'm not much
			//into Psycle internal configuration loading routines.
			//If YOU are, go and put it where it sould be put.
			//
			//I know there's a class "Registry" in psycle, but... I don't like using it.
			//I think it's a little bit nonsense to use class that does not nuch more
			//than API itself. The only one for plus is variable encapsulation.
			//
			//Fideloop.
			//
			HKEY RegKey;
			HMENU hRootMenuBar, hFileMenu;
			MENUITEMINFO hNewItemInfo;
			UINT ids[] = {ID_FILE_RECENT_01,
						ID_FILE_RECENT_02,
						ID_FILE_RECENT_03,
						ID_FILE_RECENT_04};

			int iCount =0;
			char nameBuff[256];
			char cntBuff[3];
			DWORD cntSize = sizeof(cntBuff);
			DWORD nameSize = sizeof(nameBuff);
			DWORD nValues = 0;

			hRootMenuBar = ::GetMenu(pFrame->m_hWnd);
			hFileMenu = GetSubMenu(hRootMenuBar, 0);
			pFrame->m_wndView.hRecentMenu = GetSubMenu(hFileMenu, 10);

			char key[72]=CONFIG_ROOT_KEY;
			strcat(key,"\\RecentFiles");
			if (RegOpenKeyEx(HKEY_CURRENT_USER , key, 0, KEY_READ, &RegKey) == ERROR_SUCCESS)
			{
				RegQueryInfoKey(RegKey, 0, 0, 0, 0, 0, 0, &nValues, 0, 0, 0, 0);
				if (nValues)
				{
					DeleteMenu(pFrame->m_wndView.hRecentMenu, 0, MF_BYPOSITION);
					while (RegEnumValue(RegKey,
									iCount,
									cntBuff,
									&cntSize,
									NULL,
									NULL,
									(unsigned char*)nameBuff,
									&nameSize) == ERROR_SUCCESS)
					{
								
						hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
						hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
						hNewItemInfo.fType		= MFT_STRING;
						hNewItemInfo.wID		= ids[iCount];
						hNewItemInfo.cch		= strlen(nameBuff);
						hNewItemInfo.dwTypeData = nameBuff;

						InsertMenuItem(pFrame->m_wndView.hRecentMenu, iCount, TRUE, &hNewItemInfo);
						cntSize = sizeof(cntBuff);
						nameSize = sizeof(nameBuff);
						iCount++;
					}
					RegCloseKey(RegKey);
				}
			}
		}

		void CPsycleApp::SaveRecent(CMainFrame* pFrame)
		{
			HKEY RegKey;
			HMENU hRootMenuBar, hFileMenu;
			DWORD Effect;
			
			int iCount;
			char nameBuff[256];
			char cntBuff[3];
			UINT nameSize;
			
			hRootMenuBar = ::GetMenu(pFrame->m_hWnd);
			hFileMenu = GetSubMenu(hRootMenuBar, 0);
			pFrame->m_wndView.hRecentMenu = GetSubMenu(hFileMenu, 10);
			
			if (RegOpenKeyEx(HKEY_CURRENT_USER, CONFIG_ROOT_KEY, 0, KEY_WRITE, &RegKey) == ERROR_SUCCESS)
			{
				RegDeleteKey(RegKey, "RecentFiles");
			}
			RegCloseKey(RegKey);
			
			char key[72]=CONFIG_ROOT_KEY;
			strcat(key,"\\RecentFiles");	
			if (RegCreateKeyEx(HKEY_CURRENT_USER,
								key,
								0,
								0,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&RegKey,
								&Effect) == ERROR_SUCCESS)
			{
				for (iCount = 0; iCount<GetMenuItemCount(pFrame->m_wndView.hRecentMenu);iCount++)
				{
					nameSize = GetMenuString(pFrame->m_wndView.hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
					GetMenuString(pFrame->m_wndView.hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
					if (strcmp(nameBuff, "No recent files"))
					{
						itoa(iCount, cntBuff, 10);
						RegSetValueEx(RegKey, cntBuff, 0, REG_SZ, (const unsigned char*)nameBuff, nameSize);
					}

				}
				RegCloseKey(RegKey);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// new gui ... just a test, anyone can freely throw this code away
#if 0

/*
	in main class constructor add:

			#if defined OPERATING_SYSTEM__CROSSPLATFORM
			{
				#if !defined NDEBUG
					operating_system::console console;
				#endif
				(*new GUI).start(); // starts the new gui in parallel with the mfc one, each in their own thread
			}
			#endif
*/

#undef OPERATING_SYSTEM__CROSSPLATFORM
#if defined OPERATING_SYSTEM__CROSSPLATFORM
	#include <operating_system/logger.h>
	#include <softsynth/gui/gui.h>
	#include <boost/thread/thread.hpp>
	#include <boost/thread/mutex.hpp>
	class GUI
	{
	public:
		GUI();
		void start() throw(boost::thread_resource_error);
		void operator()() throw();
	private:
		boost::thread * thread_;
	};

	GUI::GUI() : thread_(0) {}

	void GUI::operator()() throw()
	{
		softsynth::gui::main();
	}

	void GUI::start() throw(boost::thread_resource_error)
	{
		if(thread_) return;
		try
		{
			template<typename Functor> class Thread
			{
			public:
				inline Thread(Functor & functor) : functor_(functor) {}
				inline void operator()() throw() { functor_(); }
			private:
				Functor & functor_;
			};
			thread_ = new boost::thread(Thread<GUI>(*this));
		}
		catch(const boost::thread_resource_error & e)
		{
			std::cerr << typeid(e).name() << ": " << e.what() << std::endl;;
			throw;
		}
	}
#endif
#endif // 0
