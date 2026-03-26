
// HFProtocol.h : HFProtocol 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"       // 主符号
#include "HFanalyseFrame.h"
#include "HFanalyseDoc.h"
#include "HFanalyseView.h"


// CHFProtocol:
// 有关此类的实现，请参阅 HFProtocol.cpp
//

class CHFProtocol : public CWinAppEx
{
public:
	CHFProtocol();

public:

	CHFanalyseFrame* m_HFanalyseFrm;
	CMultiDocTemplate* pHFanalyseTemplate;
	CHFanalyseDoc* m_pHFanalyseDoc;


	CString m_strAppPathName;


// 重写
public:
	virtual BOOL InitInstance();
	CString GetAppPath();

// 实现
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnOpenhfhist();
	virtual int ExitInstance();
	
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
};

//extern CHFProtocol theApp;
