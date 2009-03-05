///\file
///\brief interface file for psycle::host::CSongpDlg.
#pragma once
#include "resources/resources.hpp"
#include "mfc_namespace.hpp"
#include "configuration_options.hpp"

#ifdef use_psycore
		namespace psy {
			namespace core {
				class Song;
			}
		}
#endif


PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

		class Song; // forward declaration

		/// song dialog window.
		class CSongpDlg : public CDialog
		{
		public:
			/// mfc compliant constructor.
#ifdef use_psycore
			CSongpDlg(psy::core::Song *song, CWnd* pParent = 0);
			psy::core::Song* _pSong;
#else
			CSongpDlg(Song *song, CWnd* pParent = 0);
			Song* _pSong;
#endif
		// Dialog Data
			enum { IDD = IDD_SONGPROP };
			CEdit	m_songcomments;
			CEdit	m_songcredits;
			CEdit	m_songtitle;
			bool	readonlystate;
		// Overrides
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		// Implementation
		public:
			void SetReadOnly();
		protected:
			// Generated message map functions
			virtual BOOL OnInitDialog();
			afx_msg void OnOk();
			DECLARE_MESSAGE_MAP()
		};

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
