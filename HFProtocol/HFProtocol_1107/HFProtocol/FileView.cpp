
#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "HFProtocol.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

CFileView::CFileView()
{
}

CFileView::~CFileView()
{
}

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
//	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_EN_CHANGE(IDC_EDIT_FILE_TREE, &CFileView::OnEnChangeFile)
	ON_NOTIFY(NM_DBLCLK, 1011, &CFileView::OnNMDblclkTree1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 消息处理程序

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();
	// 创建组合:
	DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;

	if (!m_wndObjectEdit.Create(dwViewStyle, rectDummy, this, IDC_EDIT_FILE_TREE))
	{
		TRACE0("未能创建属性组合 \n");
		return -1;      // 未能创建
	}

	m_wndObjectEdit.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));	
	m_wndObjectEdit.EnableFolderBrowseButton();
	//// 创建视图:
	dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	if (!m_wndTreeView.Create(dwViewStyle, rectDummy, this, 1011))
	{
		AfxMessageBox("未能创建文件视图");
		return -1;      // 未能创建
	}

	CString strFileName;	
	strFileName = theApp.m_strAppPathName+"\\filepath.txt";
	CStdioFile myFileRead(strFileName, CFile::modeRead);
	myFileRead.ReadString(m_Pathname);
	myFileRead.Close();

//	m_Pathname = "C:\\WINDOWS";
	m_wndTreeView.Initialize(m_Pathname);
	m_wndObjectEdit.SetWindowText(m_Pathname);

	UpdateData(FALSE);

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}


void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}
	CRect rectClient,rectEdit;
	GetClientRect(rectClient);

	m_wndObjectEdit.GetWindowRect(&rectEdit);

	int cyEdit = rectEdit.Size().cy;
	m_wndObjectEdit.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 20, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndTreeView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top  + cyEdit, rectClient.Width() - 2, rectClient.Height()-cyEdit, SWP_NOACTIVATE | SWP_NOZORDER);

}


void CFileView::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rectTree;
	m_wndTreeView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));

	m_wndObjectEdit.GetWindowRect(rectTree);
	ScreenToClient(rectTree);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndTreeView.SetFocus();
}


void CFileView::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}


void CFileView::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	CString strItem;
	CHFanalyseDoc* m_pHFanalyseDoc;
	CFile m_File;
	
	HTREEITEM hItem=m_wndTreeView.GetSelectedItem();
	if (hItem)
	{
		strItem = m_wndTreeView.GetFullPath(hItem);
		if (strItem.Right(4).MakeUpper() ==".WAV" || strItem.Right(4).MakeUpper() ==".DAT" )
		{
			if (!m_File.Open(strItem, CFile::modeRead)) // 打开文件
			{
				AfxMessageBox("文件打开错误或无此文件!");
				return;
			}
			m_File.Close();	
			m_pHFanalyseDoc = (CHFanalyseDoc*)theApp.pHFanalyseTemplate->OpenDocumentFile(strItem);
		}		
	}
	*pResult = 0;
}
void CFileView::OnEnChangeFile()
{
	UpdateData(TRUE);
	CString m_tempPath;	
	m_wndObjectEdit.GetWindowText(m_tempPath);
	
	if(m_tempPath.Right(1)=="\\")
		m_tempPath = m_tempPath.Left(m_tempPath.GetLength()-1);

	
	if (m_tempPath.Compare(m_Pathname))
	{
		CString strFileName;
		strFileName = theApp.m_strAppPathName+"\\filepath.txt";
		CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
		myFileWrite.WriteString(m_tempPath);
		m_wndTreeView.Initialize(m_tempPath,TRUE);
		m_Pathname = m_tempPath;
		UpdateData(FALSE);
		myFileWrite.Close();
	}
}