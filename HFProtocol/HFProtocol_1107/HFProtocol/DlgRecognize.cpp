// DlgRecognize.cpp : 实现文件
//

#include "stdafx.h"
#include "HFProtocol.h"
#include "DlgRecognize.h"
#include "HFanalyseDoc.h"
#include "HFanalyseFrame.h"
#include "MainFrm.h"

// CDlgRecognize 对话框

IMPLEMENT_DYNAMIC(CDlgRecognize, CDialog)

CDlgRecognize::CDlgRecognize(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgRecognize::IDD, pParent)
{
	gl_bRunDetect=FALSE;
}

CDlgRecognize::~CDlgRecognize()
{
}

void CDlgRecognize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROTOCAL, m_btnRecognize);
	DDX_Control(pDX, IDC_FILE_PROGRESS, m_FileProcessCtr);
}


BEGIN_MESSAGE_MAP(CDlgRecognize, CDialog)
	ON_BN_CLICKED(IDC_PROTOCAL, &CDlgRecognize::OnBnClickedProtocal)
	ON_BN_CLICKED(IDC_CHECK_ALL, &CDlgRecognize::OnBnClickedCheckAll)
	ON_MESSAGE(WM_SYSTEM_STATE,OnSystenState)
END_MESSAGE_MAP()


// CDlgRecognize 消息处理程序
void CDlgRecognize::ResetList(int nType)
{
	while(m_pParent->m_ListOut.DeleteColumn(0)) 
		continue;
	m_pParent->m_ListOut.DeleteAllItems();

	switch(nType)
	{
	case 0: // 110A
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,100);
		break;
	case 1: // 110B
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,100);
		break;
	case 2: // 141A
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		break;
	case 3: // 141B
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(4,_T("波形"),LVCFMT_LEFT,100);
		break;
	case 4: // SLEW
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		break;
	case 5: // CLEW
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		break;
	case 6: // 4285
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		break;
	case 7: // 4529
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
		break;
	default:
		break;
	}	
}
void CDlgRecognize::GetPara()
{
	int i;
	CButton *pBtn = (CButton *)GetDlgItem(IDC_CHECK_ALL);
	UINT nCheckIDs[8]={IDC_CHECK_110A,IDC_CHECK_110B,IDC_CHECK_141A,IDC_CHECK_141B,
		IDC_CHECK_LINK11SLEW,IDC_CHECK_LINK11CLEW,IDC_CHECK_4285,IDC_CHECK_4529};
	
	selprotolNum = 8;
	int num=0,idx;
	for (i=0;i<8;i++)
	{
		pBtn=(CButton*)GetDlgItem(nCheckIDs[i]);
		if(pBtn->GetCheck())
		{
			selprotolName[i] = TRUE;
			num++;
			idx = i;
		}
		else
			selprotolName[i] = FALSE;
	}
	if(num==1)
	{
		ResetList(idx);
	}
	else
	{
		ResetList(0);
	}
}

void CDlgRecognize::OnBnClickedProtocal()
{
	CMainFrame *pMainFrame = (CMainFrame*)(AfxGetMainWnd());
	CHFanalyseFrame *pFrame = (CHFanalyseFrame *)pMainFrame->GetActiveFrame();
	pRecognizeDoc = (CHFanalyseDoc*)pFrame->GetActiveDocument();
	

	CString lpString;
	m_btnRecognize.GetWindowText(lpString);

	if (pRecognizeDoc!=NULL && pRecognizeDoc->m_bFileOpen)
	{	
		if(gl_bRunDemode)
		{
			AfxMessageBox("正在解调，请先停止！");
			return;
		}
		if(lpString==_T("协议识别"))
		{
			GetPara();
			m_btnRecognize.SetWindowText(_T("停止"));
			m_FileWorkThread.m_opOpration = CFileWorkThread::opDetect;

			m_FileWorkThread.m_wndCtr = GetSafeHwnd();
			m_FileWorkThread.m_wndOut = pMainFrame->m_wndRecognizeView;
			m_FileWorkThread.m_FileInsample = pRecognizeDoc->WaveOut.m_pcmWaveFormat.nSamplesPerSec;

			m_FileWorkThread.brun = TRUE;
			gl_bRunDetect = TRUE;
			m_FileWorkThread.Start(&pRecognizeDoc->m_File);
		}
		else
		{
			m_btnRecognize.SetWindowText(_T("协议识别"));
			m_FileWorkThread.brun = FALSE; 
			gl_bRunDetect = FALSE;
		}
	}

}

void CDlgRecognize::OnBnClickedCheckAll()
{
	CButton *pBtn = (CButton *)GetDlgItem(IDC_CHECK_ALL);
	UINT nCheckIDs[9]={IDC_CHECK_ALL,IDC_CHECK_110A,IDC_CHECK_110B,IDC_CHECK_141A,IDC_CHECK_141B,
		IDC_CHECK_LINK11SLEW,IDC_CHECK_LINK11CLEW,IDC_CHECK_4285,IDC_CHECK_4529};
	if (pBtn->GetCheck())
	{
		for (int i=0;i<9;i++)
		{
			pBtn=(CButton*)GetDlgItem(nCheckIDs[i]);
			pBtn->SetCheck(1);
		}
	}
	else
	{
		for (int i=0;i<9;i++)
		{
			pBtn=(CButton*)GetDlgItem(nCheckIDs[i]);
			pBtn->SetCheck(0);
		}
	}
}
LRESULT CDlgRecognize::OnSystenState(WPARAM wParam, LPARAM lParam)
{
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pRecognizeDoc->m_hWndHFanalyseView));
	__int64 cuP = wParam;	
	if(m_FileWorkThread.brun)
	{
		pHFanalyseView->m_waveRender.setPlayLine(cuP);
		pHFanalyseView->m_waveRender.Invalidate();
		int nPercent = (int)m_FileWorkThread.nProgressPos;
		m_FileProcessCtr.SetPos(nPercent);
		if (nPercent==100)
		{
			m_btnRecognize.SetWindowText(_T("协议识别"));
			gl_bRunDetect = FALSE;
		}
	}
	return 0;
}
