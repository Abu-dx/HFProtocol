
#pragma once
#include "DlgDemode.h"
#include "JConstellationView.h"
class CDlgDemode;
class CDlgDemodeView : public CDockablePane
{
	// 构造
public:
	CDlgDemodeView();
	void AdjustLayout();

	CDlgDemode *m_DlgDemode;
	CMFCListCtrl m_ListOut;
	int m_ListNum;
	JConstellationView m_Constellation;

	LRESULT OnUpdateOutList(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateConstation(WPARAM wParam, LPARAM lParam);
	// 属性
protected:

	// 实现
public:
	virtual ~CDlgDemodeView();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_MESSAGE_MAP()

public:

};

