// TransformPatternDlg.cpp : implementation file
//
#include <psycle/project.private.hpp>
#include "TransformPatternDlg.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		// CTransformPatternDlg dialog

		IMPLEMENT_DYNAMIC(CTransformPatternDlg, CDialog)

		CTransformPatternDlg::CTransformPatternDlg(CWnd* pParent /*=NULL*/)
			: CDialog(CTransformPatternDlg::IDD, pParent)
		{
			m_filternote.EnableWindow(false);
			m_filtercmd.EnableWindow(false);
			m_replacenote.EnableWindow(false);
			m_replacecmd.EnableWindow(false);
		}

		CTransformPatternDlg::~CTransformPatternDlg()
		{
		}

		void CTransformPatternDlg::DoDataExchange(CDataExchange* pDX)
		{
			CDialog::DoDataExchange(pDX);
			//{{AFX_DATA_MAP(CTransformPatternDlg)
			DDX_Control(pDX, IDC_FILTERNOTE, m_filternote);
			DDX_Control(pDX, IDC_FILTERINS, m_filterins);
			DDX_Control(pDX, IDC_FILTERMAC, m_filtermac);
			DDX_Control(pDX, IDC_FILTERCMD, m_filtercmd);
			DDX_Control(pDX, IDC_REPLACENOTE, m_replacenote);
			DDX_Control(pDX, IDC_REPLACEINS, m_replaceins);
			DDX_Control(pDX, IDC_REPLACEMAC, m_replacemac);
			DDX_Control(pDX, IDC_REPLACECMD, m_replacecmd);
			DDX_Check(pDX, IDC_APPLYTOBLOCK, m_applytoblock);
			//}}AFX_DATA_MAP
		}


		BEGIN_MESSAGE_MAP(CTransformPatternDlg, CDialog)
		//{{AFX_MSG_MAP(CTransformPatternDlg)
			ON_BN_CLICKED(IDAPPLY, &CTransformPatternDlg::OnBnClickedApply)
		//}}AFX_MSG_MAP

		END_MESSAGE_MAP()


		// CTransformPatternDlg message handlers

		void CTransformPatternDlg::OnBnClickedApply()
		{
			char afilternote[32];
			char afilterins[32];
			char afiltermac[32];
			char afiltercmd[32];
			char areplacenote[32];
			char areplaceins[32];
			char areplacemac[32];
			char areplacecmd[32];

			m_filternote.GetWindowText(afilternote,16);
			m_filterins.GetWindowText(afilterins,16);
			m_filtermac.GetWindowText(afiltermac,16);
			m_filtercmd.GetWindowText(afiltercmd,16);
			m_replacenote.GetWindowText(areplacenote,16);
			m_replaceins.GetWindowText(areplaceins,16);
			m_replacemac.GetWindowText(areplacemac,16);
			m_replacecmd.GetWindowText(areplacecmd,16);	

			int filterins = -1;
			int filtermac = -1;
			int replaceins = -1;
			int replacemac = -1;

			if (afilterins[0] !=	'\0')
				filterins = atoi(afilterins);
			else
				TRACE("filterins is blank\n");

			if (afiltermac[0] != '\0')
				filtermac = atoi(afiltermac);
			else
				TRACE("filtermac is blank\n");			

			if (areplaceins[0] != '\0')
				replaceins = atoi(areplaceins);
			else
				TRACE("replaceins is blank\n");

			if (areplacemac[0] != '\0')
				replacemac = atoi(areplacemac);
			else
				TRACE("replacemac is blank\n");

			// now perform the pattern data replacement

			int currentPattern;
			int currentColumn;
			int currentLine;

			int patternCount = 0;
			int columnCount = 0;

			int currentins;
			int currentmac;

			unsigned char * toffset;
			PatternEntry *entry;

			for (currentPattern = 0; currentPattern < patternCount; currentPattern++)
			{
				lineCount = 0;
				for (currentColumn = 0; currentColumn < columnCount; currentColumn++)
				{
					for (currentLine = 0; currentLine < lineCount; currentLine++)
					{
						toffset = _ptrack
						entry = (PatternEntry*) toffset;

						currentins = ;
						currentmac = ;

						if (currentins == filterins)
						{//change entry to replaceins

						}

						if (currentmac == filtermac)
						{//change entry to replacemac

						}
					}
				}
			}
		}
	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
