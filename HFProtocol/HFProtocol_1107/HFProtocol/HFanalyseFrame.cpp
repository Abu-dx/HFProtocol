
// HFanalyseFrame.cpp : CHFanalyseFrame 类的实现
//

#include "stdafx.h"
#include "HFProtocol.h"

#include "HFanalyseFrame.h"
#include "PlayPaneView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CHFanalyseFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CHFanalyseFrame, CMDIChildWndEx)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_COMMAND(ID_PLAY, &CHFanalyseFrame::OnPlay)
	ON_UPDATE_COMMAND_UI(ID_PLAY, &CHFanalyseFrame::OnUpdatePlay)
	ON_COMMAND(ID_STOP, &CHFanalyseFrame::OnStop)
	ON_UPDATE_COMMAND_UI(ID_STOP, &CHFanalyseFrame::OnUpdateStop)
	ON_COMMAND(ID_CONTINUE, &CHFanalyseFrame::OnContinue)
	ON_UPDATE_COMMAND_UI(ID_CONTINUE, &CHFanalyseFrame::OnUpdateContinue)
	ON_COMMAND(ID_STOPBACK, &CHFanalyseFrame::OnStopback)
	ON_UPDATE_COMMAND_UI(ID_STOPBACK, &CHFanalyseFrame::OnUpdateStopback)
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CHFanalyseFrame::CHFanalyseFrame()
{
	// TODO: 在此添加成员初始化代码
}

CHFanalyseFrame::~CHFanalyseFrame()
{
}


BOOL CHFanalyseFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CHFanalyseFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CHFanalyseFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame 消息处理程序

void CHFanalyseFrame::OnDestroy()
{
	//CHFProtocol* pApp = (CHFProtocol*)AfxGetApp(); 
	//pApp->m_HFsysFrm = NULL;

	CMDIChildWndEx::OnDestroy();
	// TODO: 在此处添加消息处理程序代码
}


void CHFanalyseFrame::ActivateFrame(int nCmdShow)
{
	// TODO: 在此添加专用代码和/或调用基类
	nCmdShow = SW_SHOWMAXIMIZED; // 使子帧窗口最大化
	CMDIChildWndEx::ActivateFrame(nCmdShow);
}

BOOL CHFanalyseFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (!m_wndSplitter.CreateStatic(this, 2, 1))  // 创建2行1列分割
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	// add the first splitter pane - the default view in column 0
	// 创建第一个格子，在0列中使用默认的视图（由文档模板决定, CADHistDigitView）
	if (!m_wndSplitter.CreateView(0, 0, 
		pContext->m_pNewViewClass, CSize(0, 0), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	} 

	if (!m_wndSplitter.CreateView(1, 0,  
		RUNTIME_CLASS(CPlayPaneView), CSize(0, 0), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	} 
	CRect rect;
	GetWindowRect(&rect); 
	m_wndSplitter.SetRowInfo(0,rect.Height(),10);
	m_wndSplitter.SetRowInfo(1,10,10);
	m_wndSplitter.RecalcLayout();
	
	return TRUE;
//	return CMDIChildWndEx::OnCreateClient(lpcs, pContext);
}
int CHFanalyseFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIChildWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

void CHFanalyseFrame::OnPlay()
{
	// TODO: 在此添加命令处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));
	pPlayPaneView->OnBnClickedButtonPlay();
}

void CHFanalyseFrame::OnUpdatePlay(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));
	pCmdUI->Enable(pPlayPaneView->m_bPlay?1:0);	
}

void CHFanalyseFrame::OnStop()
{
	// TODO: 在此添加命令处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));
	pPlayPaneView->OnBnClickedButtonPause();
}

void CHFanalyseFrame::OnUpdateStop(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));
		
	pCmdUI->Enable(pPlayPaneView->m_bPause?1:0);	

}

void CHFanalyseFrame::OnContinue()
{
	// TODO: 在此添加命令处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));

}

void CHFanalyseFrame::OnUpdateContinue(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));
	pCmdUI->Enable(pPlayPaneView->m_bContinue?1:0);	
	
}

void CHFanalyseFrame::OnStopback()
{
	// TODO: 在此添加命令处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));
	pPlayPaneView->OnBnClickedButtonStopback();
}

void CHFanalyseFrame::OnUpdateStopback(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetActiveDocument();  // 在Frame中取得当前文档指针
	CPlayPaneView* pPlayPaneView = (CPlayPaneView*)(CWnd::FromHandle(pDoc->m_hWndPlayPaneView));
	pCmdUI->Enable(pPlayPaneView->m_bStopBack?1:0);	
}
