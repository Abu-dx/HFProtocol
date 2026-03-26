
#pragma once

#include "ViewTree.h"
#include "DirTreeCtrl.h"

class CFileView : public CDockablePane
{
// 构造
public:
	CFileView();
	void AdjustLayout();

	CDirTreeCtrl	m_wndTreeView;
	CMFCEditBrowseCtrl m_wndObjectEdit;
	CString m_Pathname;


// 属性
protected:

// 实现
public:
	virtual ~CFileView();
	int GetIconIndex(LPTSTR FilePath,UINT uFlags);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEnChangeFile();
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:

	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

};

