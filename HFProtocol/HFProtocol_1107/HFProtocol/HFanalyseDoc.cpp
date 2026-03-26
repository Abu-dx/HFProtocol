// HFanalyseDoc.cpp : 实现文件
//

#include "stdafx.h"
#include "HFProtocol.h"
#include "HFanalyseDoc.h"
#include "GetFileFormat.h"
#include "MainFrm.h"

// CHFanalyseDoc

IMPLEMENT_DYNCREATE(CHFanalyseDoc, CDocument)

CHFanalyseDoc::CHFanalyseDoc()
{
	m_bFileOpen = FALSE;
	m_FileLength = 0;
}

BOOL CHFanalyseDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CHFanalyseDoc::~CHFanalyseDoc()
{
}


BEGIN_MESSAGE_MAP(CHFanalyseDoc, CDocument)
END_MESSAGE_MAP()


// CHFanalyseDoc 诊断

#ifdef _DEBUG
void CHFanalyseDoc::AssertValid() const
{
	CDocument::AssertValid();
}

#ifndef _WIN32_WCE
void CHFanalyseDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif
#endif //_DEBUG

#ifndef _WIN32_WCE
// CHFanalyseDoc 序列化

void CHFanalyseDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}
#endif


// CHFanalyseDoc 命令

BOOL CHFanalyseDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	CHFProtocol* pApp = (CHFProtocol*)AfxGetApp();
	CFileException e;
	if (m_File.Open(lpszPathName,CFile::modeRead,&e )) // 打开文件
	{
		m_File.Seek(0, CFile::begin);
		m_FileLength = (ULONG)((m_File.GetLength())/sizeof(short)); // 文件长度(字)
		m_bFileOpen = TRUE;
		m_FileName = (CString)lpszPathName;

		m_File.Seek(0, CFile::begin);
		char szTmp[10];
		m_File.Read(szTmp, 4 * sizeof(char)) ;
		if (strncmp(szTmp, _T("RIFF"), 4) != 0) 
		{
			CGetFileFormat m_FileFormat;
			if(m_FileFormat.DoModal()==IDOK)
			{
				WaveOut.m_pcmWaveFormat.nChannels = gl_wChannels;
				WaveOut.m_pcmWaveFormat.nSamplesPerSec = gl_dwSamplesPerSec;
				WaveOut.m_pcmWaveFormat.wBitsPerSample = gl_wBitsPerSample;
			}
			else
			{
				m_File.Close();
				m_bFileOpen = FALSE;
				m_FileLength = 0;
				return FALSE;
			}			
		}
		else
		{
			m_File.Seek(0, CFile::begin);
			WaveOut.Load(&m_File);
		}
		this->SetPathName(lpszPathName);
		return TRUE;

	}
	else
	{
		AfxMessageBox("打开文件失败！");
		return FALSE;
	}

	
}

void CHFanalyseDoc::OnCloseDocument()
{
	// TODO: 在此添加专用代码和/或调用基类
	CMainFrame *pMainFrame = (CMainFrame*)(AfxGetMainWnd());
	if(pMainFrame->m_wndDemodeView.m_DlgDemode->pDemodeDoc == this)
		if(pMainFrame->m_wndDemodeView.m_DlgDemode->m_FileWorkThread.brun)
			return;
	if (m_bFileOpen == TRUE)
	{
		m_File.Close();
		m_bFileOpen = FALSE;
		m_FileLength = 0;
	}
	CDocument::OnCloseDocument();
}
