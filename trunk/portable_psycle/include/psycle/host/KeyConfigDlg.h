#if !defined(AFX_KEYCONFIGDLG_H__14EA3837_809C_4274_9DE0_B3FF909E898A__INCLUDED_)
#define AFX_KEYCONFIGDLG_H__14EA3837_809C_4274_9DE0_B3FF909E898A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeyConfigDlg.h : header file
//

#include "InputHandler.h"

/////////////////////////////////////////////////////////////////////////////
// CKeyConfigDlg dialog

class CKeyConfigDlg : public CPropertyPage
{
	DECLARE_DYNCREATE(CKeyConfigDlg)

// Construction
public:
	void FillCmdList();
	void DoCommandList();
	CKeyConfigDlg();   // standard constructor
	char _skinPathBuf[MAX_PATH];

// Dialog Data
	//{{AFX_DATA(CKeyConfigDlg)
	enum { IDD = IDD_KEYCONFIG };
	CSpinButtonCtrl	m_autosave_spin;
	CEdit	m_autosave_mins;
	CButton	m_autosave;
	CButton	m_save_reminders;
	CButton	m_tweak_smooth;
	CButton	m_record_unarmed;
	CButton	m_show_info;
	CButton	m_cmdShiftArrows;
	CButton	m_cmdFT2Del;
	CButton	m_cmdNewHomeBehaviour;
	CButton	m_cmdCtrlPlay;
	CListBox	m_lstCmds;
	CHotKeyCtrl	m_hotkey0;
	CButton	m_wrap;
	CButton	m_centercursor;
	CButton	m_cursordown;
	CEdit	m_numlines;
	CSpinButtonCtrl	m_spinlines;
	CStatic m_textlines;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL bInit;
	long m_prvIdx;
	void UpdateHotKey();	
	void SaveHotKey(long idx,WORD & key,WORD & mods);
	void FindKey(long idx,WORD & key,WORD & mods);
	CmdDef FindCmd(long idx);
	

	// Generated message map functions
	//{{AFX_MSG(CKeyConfigDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCmdlist();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnImportreg();
	afx_msg void OnExportreg();
	afx_msg void OnDefaults();
	afx_msg void OnNone();
	afx_msg void OnUpdateNumLines();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYCONFIGDLG_H__14EA3837_809C_4274_9DE0_B3FF909E898A__INCLUDED_)
