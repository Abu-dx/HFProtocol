
#pragma once
#include "DlgRecognize.h"
class CDlgRecognize;
class CDlgRecognizeView : public CDockablePane
{
	// 构造
public:
	CDlgRecognizeView();
	void AdjustLayout();

	CDlgRecognize *m_DlgRecognize;
	CMFCListCtrl m_ListOut;

	LRESULT OnUpdateOutList(WPARAM wParam, LPARAM lParam);

	// 属性
protected:

	// 实现
public:
	virtual ~CDlgRecognizeView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_MESSAGE_MAP()

public:

};

