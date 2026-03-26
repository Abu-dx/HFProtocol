// HFanalyseView.cpp : 实现文件
//

#include "stdafx.h"
#include "HFProtocol.h"
#include "HFanalyseView.h"


// CHFanalyseView

IMPLEMENT_DYNCREATE(CHFanalyseView, CFormView)

CHFanalyseView::CHFanalyseView()
	: CFormView(CHFanalyseView::IDD)
{

}

CHFanalyseView::~CHFanalyseView()
{
}

void CHFanalyseView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHFanalyseView, CFormView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CHFanalyseView 诊断

#ifdef _DEBUG
void CHFanalyseView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CHFanalyseView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// CHFanalyseView 消息处理程序

int CHFanalyseView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rect;
	GetClientRect(rect);


	if (!::IsWindow(m_waveRender.m_hWnd))
		m_waveRender.Create(NULL,_T(""), WS_CHILD  | WS_VISIBLE, rect, this, 1004);	
	m_waveRender.setShowMode(SHOWMODE_BOTH);
	m_waveRender.setXMode(XMODE_TIME);
	m_waveRender.setFFTLength(8);
	m_waveRender.length_Data = 0;

	return 0;
}

void CHFanalyseView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);
	if (GetSafeHwnd() == NULL)
	{
		return;
	}
	CRect rect;
	GetClientRect(rect);


	if (m_waveRender.m_hWnd != NULL)
	{
		rect.left = rect.left;
		rect.right = rect.right;
		rect.top = rect.top;
		rect.bottom = rect.bottom;
		m_waveRender.SetWindowPos(NULL, rect.left,rect.top ,rect.Width(),rect.Height() , SWP_NOACTIVATE | SWP_NOZORDER);			
		m_waveRender.MoveWindow(&rect);
	}

}

void CHFanalyseView::OnDraw(CDC* /*pDC*/)
{
	// TODO: 在此添加专用代码和/或调用基类
	
	
}

void CHFanalyseView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	pDoc->m_hWndHFanalyseView = m_hWnd;

	__int64 leng = pDoc->m_FileLength;
	if (pDoc->m_bFileOpen)
	{
		m_waveRender.setData(&pDoc->m_File,leng,0);
	}
	
	// TODO: 在此添加专用代码和/或调用基类
}
