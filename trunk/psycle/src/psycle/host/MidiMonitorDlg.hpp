///\file
///\brief interface file for psycle::host::CMidiMonitorDlg.
#pragma once
#include "Psycle.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)
		#define	DARK_GREEN	RGB( 0, 128, 0 )
		#define	DARK_RED	RGB( 128, 0, 0 )	

		/// midi monitor window.
		class CMidiMonitorDlg : public CDialog
		{
		public:
			CMidiMonitorDlg(CWnd* pParent = 0);
			/// starts the timer.
			void InitTimer( void );	
			/// updates the values in the dialog.
			void UpdateInfo( void );
		// Dialog Data
			//{{AFX_DATA(CMidiMonitorDlg)
			enum { IDD = IDD_MIDI_MONITOR };
			CStatic	m_tickSync;
			CStatic	m_midiSyncStart;
			CStatic	m_midiSyncStop;
			CStatic	m_midiSyncClock;
			CStatic	m_emulatedSyncStart;
			CStatic	m_emulatedSyncStop;
			CStatic	m_emulatedSyncClock;
			CStatic	m_syncronising;
			CStatic	m_resyncTriggered;
			CStatic	m_receivingMidiData;
			CStatic	m_psycleMidiActive;
			CStatic	m_syncLatency;
			CStatic	m_bufferCapacity;
			CStatic	m_midiVersion;
			CStatic	m_midiHeadroom;
			CStatic	m_syncOffset;
			CStatic	m_syncAdjust;
			CStatic	m_eventsLost;
			CStatic	m_bufferUsed;
			CStatic m_clearEventsLost;
			CListCtrl	m_channelMap;
			CStatic m_ch1;
			CStatic m_ch2;
			CStatic m_ch3;
			CStatic m_ch4;
			CStatic m_ch5;
			CStatic m_ch6;
			CStatic m_ch7;
			CStatic m_ch8;
			CStatic m_ch9;
			CStatic m_ch10;
			CStatic m_ch11;
			CStatic m_ch12;
			CStatic m_ch13;
			CStatic m_ch14;
			CStatic m_ch15;
			CStatic m_ch16;
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CMidiMonitorDlg)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			virtual void OnTimer(UINT nIDEvent);
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CMidiMonitorDlg)
			virtual BOOL OnInitDialog();
			virtual void fnClearEventsLost();
			afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		private:
			void SetStaticFlag( CStatic * pStatic, DWORD flags, DWORD flagMask );	// used for dot control
			void CreateChannelMap( void );	// create the channel map table
			void FillChannelMap( bool overridden = false );	// update the channel map table

			int m_clearCounter;		// use for the 'clear lost events' button
			CFont m_symbolFont;		// custom graphic font
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
