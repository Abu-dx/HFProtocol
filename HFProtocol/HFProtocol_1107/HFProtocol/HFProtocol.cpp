
// HFProtocol.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "MainFrm.h"
#include "HFanalyseFrame.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHFProtocol

BEGIN_MESSAGE_MAP(CHFProtocol, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CHFProtocol::OnAppAbout)
	ON_COMMAND(ID_OPENHFHIST, &CHFProtocol::OnOpenhfhist)
	ON_COMMAND(ID_FILE_OPEN, &CHFProtocol::OnOpenhfhist)
END_MESSAGE_MAP()


// CHFProtocol 构造

CHFProtocol::CHFProtocol()
{

	m_bHiColorIcons = TRUE;
	m_HFanalyseFrm = NULL;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CHFProtocol 对象

CHFProtocol theApp;


// CHFProtocol 初始化

BOOL CHFProtocol::InitInstance()
{
	m_strAppPathName = GetAppPath();

	CString strFileName;
	strFileName = m_strAppPathName+"\\filepath.txt";
	TRY
	{
		CStdioFile myFileRead(strFileName, CFile::modeRead);
	}
	CATCH( CFileException, e )
	{
		CStdioFile myFileWrite(strFileName, CFile::modeWrite | CFile::modeCreate);
		myFileWrite.WriteString(m_strAppPathName);
	}
	END_CATCH


	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);


	CWinAppEx::InitInstance();


	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)

	InitContextMenuManager();
	InitShellManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 注册应用程序的文档模板。文档模板
	// 将用作文档、框架窗口和视图之间的连接
//	CMultiDocTemplate* pDocTemplate;

	pHFanalyseTemplate = new CMultiDocTemplate(IDR_HFANALYSES,
		RUNTIME_CLASS(CHFanalyseDoc),
		RUNTIME_CLASS(CHFanalyseFrame), // 自定义 MDI 子框架
		RUNTIME_CLASS(CHFanalyseView));
	if (!pHFanalyseTemplate)
		return FALSE;
	AddDocTemplate(pHFanalyseTemplate);

	// 创建主 MDI 框架窗口
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// 仅当具有后缀时才调用 DragAcceptFiles
	//  在 MDI 应用程序中，这应在设置 m_pMainWnd 之后立即发生


	// 分析标准外壳命令、DDE、打开文件操作的命令行
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	
	// 调度在命令行中指定的命令。如果
	// 用 /RegServer、/Register、/Unregserver 或 /Unregister 启动应用程序，则返回 FALSE。
	//if (!ProcessShellCommand(cmdInfo))
	//	return FALSE;
	// 主窗口已初始化，因此显示它并对其进行更新
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	
	return TRUE;
}



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CHFProtocol::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CHFProtocol 自定义加载/保存方法

void CHFProtocol::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CHFProtocol::LoadCustomState()
{
}

void CHFProtocol::SaveCustomState()
{
}

CString CHFProtocol::GetAppPath()
{
	TCHAR AppPathName[MAX_PATH];
	GetModuleFileName(NULL,AppPathName,MAX_PATH);
	CString strFilePath,strFilePath1;
	strFilePath = (CString)AppPathName;
	CHAR chccc = '\\';
	int nLeft = 0;
	int nn = strFilePath.ReverseFind(chccc);
	strFilePath1 = strFilePath.Left(nn);

	return strFilePath1;
}


void CHFProtocol::OnOpenhfhist()
{

	BeginWaitCursor();  // 开始漏斗鼠标

	CString strFileName=NULL;
	CFile m_File;
	
	
	CFileDialog FileDlg(TRUE,NULL,NULL,OFN_READONLY,"All Files (*.*) |*.*|数据文件(*.dat) |*.dat|Wave文件(*.wav) |*.wav||",NULL);
	if(FileDlg.DoModal()==IDOK)
	{
		
		strFileName = FileDlg.GetPathName();
		char* pFileName = strFileName.GetBuffer(sizeof(strFileName)); // 类型转换
		if (!m_File.Open(pFileName, CFile::modeRead)) // 打开文件
		{
			AfxMessageBox("文件打开错误或无此文件!");
			return;
		}
		m_File.Close();	
		m_pHFanalyseDoc = (CHFanalyseDoc*)pHFanalyseTemplate->OpenDocumentFile(strFileName);
	}
	else
	{
		return;
	}
	
	EndWaitCursor(); // 停止漏斗鼠标
}
CDocument* CHFProtocol::OpenDocumentFile(LPCTSTR lpszFileName)
{
	// TODO: 在此添加专用代码和/或调用基类
	BeginWaitCursor();  // 开始漏斗鼠标
	CFile File;

   	if (File.Open(lpszFileName, CFile::modeRead)) // 如果文件打开成功
	{
		File.Close();
		m_pHFanalyseDoc = (CHFanalyseDoc*)pHFanalyseTemplate->OpenDocumentFile(lpszFileName);
		return m_pHFanalyseDoc; // 返回文档指针
	}
	else
	{
        AfxMessageBox("文件打开错误或无此文件!");
		return NULL;
	}

	EndWaitCursor(); // 停止漏斗鼠标
//	return CWinAppEx::OpenDocumentFile(lpszFileName);
}
int CHFProtocol::ExitInstance()
{
	return CWinAppEx::ExitInstance();
}



