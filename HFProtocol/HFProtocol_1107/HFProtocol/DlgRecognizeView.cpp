
#include "stdafx.h"
#include "HFProtocol.h"
#include "DlgRecognizeView.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgDemodeView

CDlgRecognizeView::CDlgRecognizeView()
{
	m_DlgRecognize = new CDlgRecognize;
}

CDlgRecognizeView::~CDlgRecognizeView()
{
	delete m_DlgRecognize;
	m_DlgRecognize = NULL;
}

BEGIN_MESSAGE_MAP(CDlgRecognizeView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_UPDATE_OUT,OnUpdateOutList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int CDlgRecognizeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;


	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_DlgRecognize->m_pParent = this;
	BOOL bRet = m_DlgRecognize->Create(IDD_DIALOG_RECOGNIZE, this);
	ASSERT( bRet );	
	m_DlgRecognize->ShowWindow(SW_SHOW);
	m_DlgRecognize->ModifyStyle(0,WS_CHILD |WS_VISIBLE |WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	m_DlgRecognize->SetOwner(this);

	DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_VSCROLL | LVS_REPORT |WS_HSCROLL;
	if (!m_ListOut.Create(dwViewStyle, rectDummy, this, 1022))
	{
		AfxMessageBox("未能创建列表");
		return -1;      // 未能创建
	}
	m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
	m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,100);
	m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
	m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,100);
	m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,100);
	m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,100);
	m_ListOut.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ListOut.EnableMarkSortedColumn();

	UpdateData(FALSE);

	return 0;
}

void CDlgRecognizeView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}


void CDlgRecognizeView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}
	CRect rectClient;
	GetClientRect(rectClient);

	m_DlgRecognize->SetWindowPos(NULL, rectClient.left + 1, rectClient.top + 1, 250 - 1, rectClient.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
	m_ListOut.SetWindowPos(NULL, rectClient.left + 1 + 250, rectClient.top + 1, rectClient.Width() - 250 - 2, rectClient.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}


void CDlgRecognizeView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_DlgRecognize->GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));

	m_ListOut.GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));

}

void CDlgRecognizeView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

}

void CDlgRecognizeView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDockablePane::OnShowWindow(bShow, nStatus);
}
LRESULT CDlgRecognizeView::OnUpdateOutList(WPARAM wParam, LPARAM lParam)
{
	UpdateData(TRUE);
	CString text[10];
	int n=m_ListOut.GetItemCount();
	text[0].Format("%d",n+1);
	int nIndex=m_ListOut.InsertItem(n+1,text[0]);

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
		for (int i=1;i<4;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="MIL141B")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3].Format("%f",gl_OutList.frequency);
		text[4] = gl_OutList.frameType;
		for (int i=1;i<5;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	else if (gl_OutList.sigType=="Link11SLEW" || gl_OutList.sigType=="Link11CLEW" || 
		gl_OutList.sigType=="NATO4285" || gl_OutList.sigType=="NATO4529")
	{
		text[1]=gl_OutList.sigType;
		text[2]=gl_OutList.BeginTime;
		text[3].Format("%f",gl_OutList.frequency);
		for (int i=1;i<4;i++)
			m_ListOut.SetItemText(nIndex,i,text[i]);
	}
	m_ListOut.EnsureVisible(nIndex,TRUE);
	return 0;

}

