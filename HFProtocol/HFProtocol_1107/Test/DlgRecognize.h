#pragma once

#include "afxwin.h"
#include "DlgRecognizeView.h"
#include "FileWorkThread.h"
#include "afxcmn.h"

class CDlgRecognizeView;
class CDlgRecognize : public CDialog
{
	DECLARE_DYNAMIC(CDlgRecognize)

public:
	CDlgRecognize(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgRecognize();
// 对话框数据
	enum { IDD = IDD_DIALOG_RECOGNIZE };


public:

	CFileWorkThread m_FileWorkThread;
	CHFanalyseDoc* pRecognizeDoc;
	CButton m_btnRecognize;
	CProgressCtrl m_FileProcessCtr;
	CDlgRecognizeView *m_pParent;

	void GetPara();
	void ResetList(int nType);
	LRESULT OnSystenState(WPARAM wParam, LPARAM lParam);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedProtocal();
	afx_msg void OnBnClickedCheckAll();
	
};
