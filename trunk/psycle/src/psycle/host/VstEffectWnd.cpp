//\file
///\brief implementation file for psycle::host::CVstEditorDlg.
#include <psycle/host/detail/project.private.hpp>
#include "VstEffectWnd.hpp"
#include "VstHost24.hpp"
#include "Configuration.hpp"
namespace psycle { namespace host {

	using namespace seib::vst;
	
		CVstGui::CVstGui(CFrameMachine* frame,vst::plugin* effect)
		:CBaseParamView(frame)
		,pEffect(effect)
		{}

		void CVstGui::Open()
		{ 
			pEffect->EditOpen(WindowPtr());
		}
		void CVstGui::WindowIdle() { pEffect->EditIdle(); }
		bool CVstGui::GetViewSize(CRect& rect)
		{
			ERect* pRect(0);
			pEffect->EditGetRect(&pRect);
			if (!pRect)
				return false;

			rect.left = rect.top = 0;
			rect.right = pRect->right - pRect->left;
			rect.bottom = pRect->bottom - pRect->top;
			return true;
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		IMPLEMENT_DYNAMIC(CVstEffectWnd, CFrameMachine)

		BEGIN_MESSAGE_MAP(CVstEffectWnd, CFrameMachine)
			ON_WM_CREATE()
			ON_WM_CLOSE()
			ON_COMMAND(ID_PROGRAMS_OPENPRESET, OnProgramsOpenpreset)
			ON_COMMAND(ID_PROGRAMS_SAVEPRESET, OnProgramsSavepreset)
		END_MESSAGE_MAP()

		CVstEffectWnd::CVstEffectWnd(vst::plugin* effect, CChildView* wndView_, CFrameMachine** windowVar_)
		: CEffectWnd(effect)
		, CFrameMachine(effect, wndView_, windowVar_)
		{
		}
		int CVstEffectWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CFrameMachine::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}
			vstmachine().SetEditWnd(this);
			return 0;
		}

		void CVstEffectWnd::OnClose()
		{
//			vstmachine().EnterCritical();             // make sure we're not processing 
			vstmachine().EditClose();              // tell effect edit window's closed 
//			vstmachine().LeaveCritical();             // re-enable processing          
			std::list<HWND>::iterator it = secwinlist.begin();
			while ( it != secwinlist.end() )
			{
				::SendMessage(*it, WM_CLOSE, 0, 0);
				++it;
			}
			CFrameMachine::OnClose();
		}


		void CVstEffectWnd::OnProgramsOpenpreset()
		{
			char tmp[2048];
			if (!vstmachine().OnGetChunkFile(tmp))
			{
				std::strncpy(tmp,reinterpret_cast<char*>(vstmachine().OnGetDirectory()),1024);
				std::strcat(tmp,"\\");
			}
			CFileDialog dlg(TRUE,
				"fxb",
				tmp,
				OFN_ENABLESIZING | OFN_NOCHANGEDIR,
				"Effect Bank Files (.fxb)|*.fxb|Effect Program Files (.fxp)|*.fxp|All Files|*.*||");

			if (dlg.DoModal() != IDOK)
				return;

			if ( dlg.GetFileExt() == "fxb" )
			{
				CFxBank b(dlg.GetPathName());
				if ( b.Initialized() ) vstmachine().LoadBank(b);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);
			}
			else if ( dlg.GetFileExt() == "fxp" )
			{
				CFxProgram p(dlg.GetPathName());
				if ( p.Initialized() ) vstmachine().LoadProgram(p);
				else
					MessageBox("Error Loading file", NULL, MB_ICONERROR);

			}
			FillProgramCombobox();
		}

		void CVstEffectWnd::OnProgramsSavepreset()
		{
			char tmp[2048];
			if (!vstmachine().OnGetChunkFile(tmp))
			{
				std::strncpy(tmp,reinterpret_cast<char*>(vstmachine().OnGetDirectory()),1024);
				std::strcat(tmp,"\\");
			}
			CFileDialog dlg(FALSE,
				"fxb",
				tmp,
				OFN_CREATEPROMPT | OFN_ENABLESIZING |
				OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN |
				OFN_OVERWRITEPROMPT,
				"Effect Bank Files (.fxb)|*.fxb|Effect Program Files (.fxp)|*.fxp|All Files|*.*||");
			if (dlg.DoModal() == IDOK)
			{
				if ( dlg.GetFileExt() == "fxb")
					SaveBank((char *)(LPCSTR)dlg.GetPathName());
				else if ( dlg.GetFileExt() == "fxp")
					SaveProgram((char *)(LPCSTR)dlg.GetPathName());
			}
		}


		//////////////////////////////////////////////////////////////////////////
		void CVstEffectWnd::RefreshUI()
		{
			FillProgramCombobox();
		}
		CBaseParamView* CVstEffectWnd::CreateView()
		{
			if ( pEffect->HasEditor())
			{
				CVstGui* gui = new CVstGui(this,&vstmachine());
				gui->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
					CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL);
				return gui;
			}
			else
			{
				return CFrameMachine::CreateView();
			}
		}

		void CVstEffectWnd::ResizeWindow(int w,int h)
		{
			CRect rcW;
			GetWindowRect(&rcW);

			CRect rc;
			rc.left = (short)rcW.left;	rc.top = (short)rcW.top;
			rc.right = rc.left + w;	rc.bottom = rc.top + h;
			ResizeWindow(&rc);
			pView->WindowIdle();
		}
		void CVstEffectWnd::ResizeWindow(CRect* pRect)
		{
			CFrameMachine::ResizeWindow(pRect);
		}


		//////////////////////////////////////////////////////////////////////////
		// OnOpenFileSelector : called when effect needs a file selector        //
		//																		//
		//		This function is based on the VSTGUI3.0 source					//
		//////////////////////////////////////////////////////////////////////////
		bool CVstEffectWnd::OpenFileSelector (VstFileSelect *ptr)
		{
			if (!ptr)
				return false;

			char fileName[_MAX_PATH];
			fileName[0]='\0';
			char *filePath;

			if	((ptr->command == kVstFileLoad) 
				||	(ptr->command == kVstFileSave)
				||	(ptr->command == kVstMultipleFilesLoad))
			{
				OPENFILENAME ofn; // common dialog box structure
				// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = GetParent()->m_hWnd;
				ofn.lpstrTitle = ptr->title;
				ofn.lpstrFileTitle = fileName;
				ofn.nMaxFileTitle = sizeof (fileName) - 1;
				if (ptr->command == kVstMultipleFilesLoad) {
					filePath = new char[_MAX_PATH * 100];
					ofn.lpstrFile = filePath;
					ofn.nMaxFile = _MAX_PATH * 100 - 1;
				} else {
					filePath = new char[_MAX_PATH];
					ofn.lpstrFile = filePath;
					ofn.nMaxFile = _MAX_PATH - 1;
				}
				filePath[0] = '\0';

				std::string filefilter;
				for (int i=0;i<ptr->nbFileTypes;i++)
				{
					filefilter += ptr->fileTypes[i].name; filefilter.push_back('\0');
					filefilter += "*."; filefilter += ptr->fileTypes[i].dosType; filefilter.push_back('\0');
				}
				filefilter += "All (*.*)"; filefilter.push_back('\0');
				filefilter += "*.*"; filefilter.push_back('\0');

				ofn.lpstrFilter =filefilter.c_str();
				if (ptr->nbFileTypes >= 1) {
					ofn.lpstrDefExt = ptr->fileTypes[0].dosType;
				}
				ofn.nFilterIndex = 1;
				if ( ptr->initialPath != 0) {
					ofn.lpstrInitialDir = ptr->initialPath;
				} else {
					ofn.lpstrInitialDir =  (char*)vstmachine().OnGetDirectory();
				}
				if (ptr->command == kVstFileSave) {
					ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
				} else if (ptr->command == kVstMultipleFilesLoad) {
					ofn.Flags = OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
				} else {
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_ENABLESIZING;
				}
				// Display the Open dialog box.
				int asdf = ::GetOpenFileName(&ofn);
				if(asdf==TRUE)
				{
					if (ptr->command == kVstMultipleFilesLoad)
					{
						char string[_MAX_PATH], directory[_MAX_PATH];
						string[0] = '\0'; directory[0] = '\0';
						char *previous = ofn.lpstrFile;
						unsigned long len;
						bool dirFound = false;
						ptr->returnMultiplePaths = new char*[100];
						long i = 0;
						while (*previous != 0)
						{
							if (!dirFound) 
							{
								dirFound = true;
								strcpy (directory, previous);
								len = (unsigned long)strlen (previous) + 1;  // including 0
								previous += len;

								if (*previous == 0)
								{  // 1 selected file only		
									ptr->returnMultiplePaths[i] = new char [strlen (directory) + 1];
									strcpy (ptr->returnMultiplePaths[i++], directory);
								}
								else
								{
									if (directory[strlen (directory) - 1] != '\\')
										strcat (directory, "\\");
								}
							}
							else 
							{
								sprintf (string, "%s%s", directory, previous);
								len = (unsigned long)strlen (previous) + 1;  // including 0
								previous += len;

								ptr->returnMultiplePaths[i] = new char [strlen (string) + 1];
								strcpy (ptr->returnMultiplePaths[i++], string);
							}
						}
						ptr->nbReturnPath = i;
						delete[] filePath;
					}
					else if ( ptr->returnPath == NULL || ptr->sizeReturnPath == 0)
					{
						ptr->reserved = 1;
						ptr->returnPath = filePath;
						ptr->sizeReturnPath = (VstInt32)strlen(filePath);
						ptr->nbReturnPath = 1;
					}
					else 
					{
						strncpy(ptr->returnPath,filePath,ptr->sizeReturnPath);
						ptr->nbReturnPath = 1;
						delete[] filePath;
					}
					return true;
				}
				else delete[] filePath;
			}
			else if (ptr->command == kVstDirectorySelect)
			{
				std::string result;
				if ( ptr->returnPath == 0)
				{
					ptr->reserved = 1;
					ptr->returnPath = new char[_MAX_PATH];
					ptr->sizeReturnPath = _MAX_PATH -1;
					ptr->nbReturnPath = 1;
				}
				if(ptr->initialPath)
					result = ptr->initialPath;
				else
					result = "";
				
				if(CPsycleApp::BrowseForFolder(GetParent()->m_hWnd, ptr->title, result))
				{
					strncpy(ptr->returnPath, result.c_str(), _MAX_PATH-1);
					ptr->nbReturnPath=1;
				}
			}
			return ptr->returnPath>0;
		}

		//////////////////////////////////////////////////////////////////////////
		// OnCloseFileSelector : called when effect needs a file selector       //
		//																		//
		//		This function is based on the VSTGUI3.0 source					//
		//////////////////////////////////////////////////////////////////////////
		bool CVstEffectWnd::CloseFileSelector (VstFileSelect *ptr)
		{
			if ( ptr->command == kVstMultipleFilesLoad)
			{
				for (int i=0; i < ptr->nbReturnPath;i++)
				{
					delete[] ptr->returnMultiplePaths[i];
				}
				delete[] ptr->returnMultiplePaths;
				return true;
			}
			else if ( ptr->reserved == 1) 
			{
				delete[] ptr->returnPath;
				return true;
			}
			return false;
		}

		//////////////////////////////////////////////////////////////////////////
		// OnOpenWindow : called to open yet another window                     //
		//////////////////////////////////////////////////////////////////////////

		void * CVstEffectWnd::OpenSecondaryWnd(VstWindow& window)
		{
			CFrameWnd *pWnd = new CFrameWnd;
			if (pWnd)
			{
				HWND hWnd = pWnd->GetSafeHwnd();
				// ignored at the moment: style, position
				ERect rc = {0};
				rc.right = window.width;
				rc.bottom = window.height;
				pWnd->SetWindowText(window.title);
				pWnd->ShowWindow(SW_SHOWNORMAL);
				pWnd->MoveWindow(0,0,window.width,window.height,true);
				window.winHandle = hWnd;
				secwinlist.push_back(hWnd);
				return hWnd;
			}
			return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		// OnCloseWindow : called to close a window opened in OnOpenWindow      //
		//////////////////////////////////////////////////////////////////////////

		bool CVstEffectWnd::CloseSecondaryWnd(VstWindow& window)
		{
			if (!::IsWindow((HWND)window.winHandle))
				return false;

			secwinlist.remove((HWND)window.winHandle);
			::SendMessage((HWND)window.winHandle, WM_CLOSE, 0, 0);
			window.winHandle = 0;
			return true;
		}

		bool CVstEffectWnd::SetParameterAutomated(long index, float value)
		{
			///\todo: This should go away. Find a way to do the Mouse Tweakings. Maybe via sending commands to player? Inputhandler?
			int newval= helpers::math::lround<int,float>(value * vst::quantization);
			if(index>= 0 || index < vstmachine().GetNumParams())
			{
				Automate(index, newval, false);
				return true;
			}
			return false;
		}
	}   // namespace
}   // namespace
