#pragma once


// CDlgDemode 对话框
#include "DlgDemodeView.h"
#include "afxwin.h"
#include "FileWorkThread.h"
#include "HFanalyseDoc.h"
#include "afxcmn.h"
class CDlgDemodeView;
class CDlgDemode : public CDialog
{
	DECLARE_DYNAMIC(CDlgDemode)

public:
	CDlgDemode(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgDemode();

// 对话框数据
	enum { IDD = IDD_DIALOG_DEMODE };


public:

	CDlgDemodeView *m_pParent;
	CHFanalyseDoc* pDemodeDoc;
	HWND wndParent;
	CFileWorkThread m_FileWorkThread;

	BOOL m_bDotNetLook;
	BOOL m_bMarkSortedColumn;
	BOOL m_bMarkChanged;
	BOOL m_bPropListCategorized;
	BOOL m_bShowDragContext;

	CMFCPropertyGridCtrl m_wndPropList;
	CMFCPropertyGridProperty* pPropFre;
	CMFCPropertyGridProperty* pPropBaud;
	CMFCPropertyGridProperty* pPropTH;
	CMFCPropertyGridProperty* pGroupFSK;
	CMFCPropertyGridProperty* pPropFSKOrder;
	CMFCPropertyGridProperty* pPropFSKFre;
	CMFCPropertyGridProperty* pGroupCode ;
	CMFCPropertyGridProperty* pPropDataRate;
	CMFCPropertyGridProperty* pPropIscode;
	CMFCPropertyGridProperty* pPropCodeLeng;

	int m_Insample;
	float m_Fc;
	int dataRate,FECType,InterType;
	float m_Baud;
	float m_DemodeTh;

	CStatic m_wndPropListLocation;
	CComboBox m_ComboSigtype;

	void SetPara(int nType);
	void GetPara();
	void ResetList(int nType);
	LRESULT OnSystenState(WPARAM wParam, LPARAM lParam);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDemode();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeComboSigtype();
	CButton m_btnDemode;
	CProgressCtrl m_FileProcessCtr;
};
