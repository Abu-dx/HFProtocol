#pragma once
#include "afxwin.h"
#include "afxcmn.h"

class CPlayPaneView : public CFormView
{
	DECLARE_DYNCREATE(CPlayPaneView)

public:
	CPlayPaneView();           // 动态创建所使用的受保护的构造函数
	virtual ~CPlayPaneView();

public:
	enum { IDD = IDD_DLG_PLAY };


public:

	__int64 alltime;
	__int64 curtime;
	
	__int64 startPoints;
	__int64 endPoints;
	__int64 selectPointsStart;
	__int64 selectPointsEnd;

	__int64 m_beginPoints;
	__int64 m_endPoints;
	__int64 m_curPoints;
	
protected:
	void DisplayTime(CString time);
	CString convertTime(__int64 point);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	CMFCButton m_ButtonStopBack;
	CMFCButton m_ButtonPlay;
	CMFCButton m_ButtonPause;
	CMFCButton m_ButtonGotobegin;
	CMFCButton m_ButtonGotoend;
	CMFCButton m_ButtonBackward;
	CMFCButton m_ButtonForward;
	CMFCButton m_ButtonPlaytoend;

	bool m_bPlay,m_bPause,m_bContinue,m_bStopBack;
	CString m_CurTime;
	CEdit m_CtrCurTime;
	
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonPause();
//	afx_msg void OnBnClickedButtonContinue();
	afx_msg void OnBnClickedButtonStopback();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonGotobegin();
	afx_msg void OnBnClickedButtonBackward();
	afx_msg void OnBnClickedButtonForward();
	afx_msg void OnBnClickedButtonGotoend();
	afx_msg void OnBnClickedButtonPlaytoend();
	CEdit m_editSelBegin;
	CEdit m_editSelEnd;
	CEdit m_editSelLength;
	CEdit m_editShowBegin;
	CEdit m_editShowEnd;
	CEdit m_editShowLength;

	LRESULT OnSelStart(WPARAM wParam, LPARAM lParam);
	LRESULT OnSelEnd(WPARAM wParam, LPARAM lParam);
	CStatic m_staticSample;
};


