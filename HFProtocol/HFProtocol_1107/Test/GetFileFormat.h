#pragma once
#include "afxwin.h"


// CGetFileFormat 对话框

class CGetFileFormat : public CDialog
{
	DECLARE_DYNAMIC(CGetFileFormat)

public:
	CGetFileFormat(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CGetFileFormat();

// 对话框数据
	enum { IDD = IDD_FORMAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_FileSampleRate;
	int m_FileChannel;
	int m_Filebit;

	CListBox m_ListSampleRate;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	
	
	virtual BOOL OnInitDialog();
	afx_msg void OnLbnSelchangeListsamplerate();
};
