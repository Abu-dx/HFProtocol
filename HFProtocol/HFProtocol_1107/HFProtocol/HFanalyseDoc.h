#pragma once

// CHFanalyseDoc 文档
#include "xWave.h"
class CHFanalyseDoc : public CDocument
{
	DECLARE_DYNCREATE(CHFanalyseDoc)

public:
	CHFanalyseDoc();
	virtual ~CHFanalyseDoc();
#ifndef _WIN32_WCE
	virtual void Serialize(CArchive& ar);   // 为文档 I/O 重写
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	CFile m_File;          // 历史文件
	CString m_FileName;
	DWORD m_FileLength;    // 文件长度
	BOOL m_bFileOpen;       // 文件是否被打开
	HWND m_hWndHFanalyseView;
	HWND m_hWndPlayPaneView;

	CxWave WaveOut;

protected:
	virtual BOOL OnNewDocument();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
};
