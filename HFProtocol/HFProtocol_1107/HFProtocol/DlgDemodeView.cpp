
#include "stdafx.h"
#include "HFProtocol.h"
#include "DlgDemodeView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDemodeView

CDlgDemodeView::CDlgDemodeView()
{
	m_DlgDemode = new CDlgDemode;
}

CDlgDemodeView::~CDlgDemodeView()
{
	delete m_DlgDemode;
	m_DlgDemode = NULL;
}

BEGIN_MESSAGE_MAP(CDlgDemodeView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_UPDATE_OUT,OnUpdateOutList)
	ON_MESSAGE(WM_UPDATE_CONSTR,OnUpdateConstation)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int CDlgDemodeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;


	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_DlgDemode->m_pParent = this;
	BOOL bRet = m_DlgDemode->Create(IDD_DIALOG_DEMODE, this);
	ASSERT( bRet );	
	m_DlgDemode->ShowWindow(SW_SHOW);
	m_DlgDemode->ModifyStyle(0,WS_CHILD |WS_VISIBLE |WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_DlgDemode->SetOwner(this);

	DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_VSCROLL | LVS_REPORT |WS_HSCROLL;
	if (!m_ListOut.Create(dwViewStyle, rectDummy, this, 1021))
	{
		AfxMessageBox(_T("未能创建列表"));
		return -1;      // 未能创建
	}

	m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
	m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
	m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
	m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,50);
	m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,50);
	m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,50);
	m_ListOut.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ListOut.EnableMarkSortedColumn();

	if (!::IsWindow(m_Constellation.m_hWnd))
		m_Constellation.Create(NULL,_T(""), WS_CHILD  | WS_VISIBLE, rectDummy, this, 1021);			
	m_Constellation.setShowMode(SHOWMODE_POINT);
	m_Constellation.setAmplitude(2000);

	UpdateData(FALSE);

	return 0;
}

void CDlgDemodeView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CDlgDemodeView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
		return;
	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectDlg;
	rectDlg = rectClient;
	rectDlg.right = rectDlg.left+250;

	CRect rectList,rectConstellation;
	rectList = rectClient;
	rectConstellation = rectClient;

	rectConstellation.left = rectClient.right - 400;
	rectList.left = rectDlg.right;
	rectList.right = rectConstellation.left;

	m_DlgDemode->SetWindowPos(NULL, rectDlg.left + 1, rectDlg.top + 1, rectDlg.Width() - 2, rectDlg.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
	m_ListOut.SetWindowPos(NULL, rectList.left + 1, rectList.top + 1, rectList.Width() - 2, rectList.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
	m_Constellation.SetWindowPos(NULL, rectConstellation.left + 1, rectConstellation.top + 1, rectConstellation.Width() - 2, rectConstellation.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);

	m_Constellation.Invalidate();
}


void CDlgDemodeView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_DlgDemode->GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));

	m_ListOut.GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));

	m_Constellation.GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
	

}

void CDlgDemodeView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

}
void CDlgDemodeView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDockablePane::OnShowWindow(bShow, nStatus);
}
LRESULT CDlgDemodeView::OnUpdateOutList(WPARAM wParam, LPARAM lParam)
{
	UpdateData(TRUE);
	CString text[10];
	int nIndex;
	int n=m_ListOut.GetItemCount();

	text[0].Format("%d",n+1);
	nIndex=m_ListOut.InsertItem(n+1,text[0]);

	if (gl_OutList.sigType=="MIL110A" || gl_OutList.sigType=="MIL110B")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3].Format("%f",gl_OutList.frequency);
		text[4].Format("%d",gl_OutList.dataRate);
		text[5].Format("%d",gl_OutList.interLeng);
		for (int i=1;i<6;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="MIL141A")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3].Format("%f",gl_OutList.frequency);
		text[4].Format("%s",gl_OutList.message);
		for (int i=1;i<5;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="MIL141B")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3].Format("%f",gl_OutList.frequency);
		text[4].Format("%s",gl_OutList.frameType);
		for (int i=1;i<5;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="Link11SLEW")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3].Format("%f",gl_OutList.frequency);
		text[4].Format("%d",gl_OutList.PU);
		text[5]=gl_OutList.frameType;
		text[6].Format("%d",gl_OutList.CRC);
		for (int i=1;i<7;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="Link11CLEW")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3]=gl_OutList.EndTime;
		text[4].Format("%f",gl_OutList.frequency);
		text[5].Format("%d",gl_OutList.PU);
		text[6]=gl_OutList.frameType;
		for (int i=1;i<7;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="NATO4285" || gl_OutList.sigType=="NATO4529")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3].Format("%f",gl_OutList.frequency);
		text[4].Format("%d",gl_OutList.dataRate);
		text[5].Format("%s",gl_OutList.interType);
		for (int i=1;i<6;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="16CH_110Hz")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		for (int i=1;i<3;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	m_ListOut.EnsureVisible(nIndex,TRUE);
	return 0;

}
LRESULT CDlgDemodeView::OnUpdateConstation(WPARAM wParam, LPARAM lParam)
{
	int* len =(int*)wParam;
	int* pidx = (int*)lParam;
	if(m_DlgDemode->m_FileWorkThread.brun)
		m_Constellation.setData(pDataFiledemode[pidx[0]],len[0]);
	return 0;

}

