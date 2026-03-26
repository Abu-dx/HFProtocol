// DlgDemode.cpp : 实现文件
//

#include "stdafx.h"
#include "HFProtocol.h"
#include "DlgDemode.h"
#include "HFanalyseFrame.h"
#include "HFanalyseView.h"
#include "MainFrm.h"
#include "CustomProperties.h"

// CDlgDemode 对话框

IMPLEMENT_DYNAMIC(CDlgDemode, CDialog)

CDlgDemode::CDlgDemode(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDemode::IDD, pParent)
{
	m_bDotNetLook = TRUE;
	m_bMarkSortedColumn = TRUE;
	m_bPropListCategorized = TRUE;
	m_bMarkChanged = TRUE;
	m_bShowDragContext = TRUE;
}

CDlgDemode::~CDlgDemode()
{
}

void CDlgDemode::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROPLIST_POSITION, m_wndPropListLocation);
	DDX_Control(pDX, IDC_COMBO_SIGTYPE, m_ComboSigtype);
	DDX_Control(pDX, IDC_DEMODE, m_btnDemode);
	DDX_Control(pDX, IDC_FILE_PROGRESS2, m_FileProcessCtr);
}


BEGIN_MESSAGE_MAP(CDlgDemode, CDialog)
	ON_BN_CLICKED(IDC_DEMODE, &CDlgDemode::OnBnClickedDemode)
	ON_CBN_SELCHANGE(IDC_COMBO_SIGTYPE, &CDlgDemode::OnCbnSelchangeComboSigtype)
	ON_MESSAGE(WM_SYSTEM_STATE,OnSystenState)
END_MESSAGE_MAP()


// CDlgDemode 消息处理程序

BOOL CDlgDemode::OnInitDialog()
{
	CDialog::OnInitDialog();


	int i;
	CString sigtype[12]={"MIL_110A","MIL_110B","MIL_141A","MIL_141B","Link11_SLEW","Link11_CLEW","NATO_4285","NATO_4529"};
	for(i=0;i<12;i++){
		m_ComboSigtype.AddString(sigtype[i]);
		m_ComboSigtype.SetCurSel(0);
	}

	CRect rectPropList;
	m_wndPropListLocation.GetClientRect(&rectPropList);
	m_wndPropListLocation.MapWindowPoints(this, &rectPropList);
	m_wndPropList.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rectPropList, this, (UINT)-1);
	m_wndPropList.EnableHeaderCtrl();
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook(m_bDotNetLook);
	m_wndPropList.MarkModifiedProperties(m_bMarkChanged);
	m_wndPropList.SetAlphabeticMode(!m_bPropListCategorized);
	m_wndPropList.SetShowDragContext(m_bShowDragContext);


	pPropFre = new CMFCPropertyGridProperty(_T("中心载波"),(COleVariant) _T("1800"), _T("信号中心载波，单位Hz"));
	pPropFre->AllowEdit(TRUE);
	m_wndPropList.AddProperty(pPropFre);

	pPropBaud = new CMFCPropertyGridProperty(_T("符号速率"),(COleVariant) _T("2400"), _T("信号符号速率，单位Baud"));
	pPropBaud->AllowEdit(TRUE);
	m_wndPropList.AddProperty(pPropBaud);

	pPropTH = new CBoundedNumberSubProp( _T("门限"), (COleVariant) 60l, 1, 10, _T("信号解调门限，范围1-10"));
	pPropTH->EnableSpinControl(TRUE,1,10);
	m_wndPropList.AddProperty(pPropTH);
	m_DemodeTh = 0.6;

	pGroupFSK = new CMFCPropertyGridProperty(_T("FSK参数"));
	pPropFSKOrder = new CMFCPropertyGridProperty(_T("阶数"),(COleVariant) _T("2FSK"), _T("MFSK阶数"));
	pPropFSKOrder->AddOption(_T("2FSK"));
	pPropFSKOrder->AddOption(_T("4FSK"));
	pPropFSKOrder->AddOption(_T("8FSK"));
	pPropFSKOrder->AllowEdit(FALSE);
	pGroupFSK->AddSubItem(pPropFSKOrder);
	pPropFSKFre = new CMFCPropertyGridProperty(_T("频间"),(COleVariant) _T("250"), _T("频率间隔"));
	pPropFSKFre->AllowEdit(TRUE);
	pGroupFSK->AddSubItem(pPropFSKFre);
	m_wndPropList.AddProperty(pGroupFSK);

	pGroupCode = new CMFCPropertyGridProperty(_T("信道编码"));
	pPropDataRate = new CMFCPropertyGridProperty(_T("数据率"),(COleVariant) _T("2400"), _T("信息速率，单位bit/s"));
	pPropDataRate->AddOption(_T("3600"));
	pPropDataRate->AddOption(_T("2400"));
	pPropDataRate->AddOption(_T("1200"));
	pPropDataRate->AddOption(_T("600"));
	pPropDataRate->AddOption(_T("300"));
	pPropDataRate->AddOption(_T("150"));
	pPropDataRate->AddOption(_T("75"));
	pPropDataRate->AllowEdit(FALSE);
	pGroupCode->AddSubItem(pPropDataRate);
	pPropIscode = new CMFCPropertyGridProperty(_T("编码"),(COleVariant) _T("是"), _T("是否编码"));
	pPropIscode->AddOption(_T("是"));
	pPropIscode->AddOption(_T("否"));
	pPropIscode->AllowEdit(FALSE);
	pGroupCode->AddSubItem(pPropIscode);
	pPropCodeLeng = new CMFCPropertyGridProperty(_T("交织长度"),(COleVariant) _T("Long"), _T("交织长度"));
	pPropCodeLeng->AddOption(_T("No"));
	pPropCodeLeng->AddOption(_T("Short"));
	pPropCodeLeng->AddOption(_T("Long"));
	pPropCodeLeng->AllowEdit(FALSE);
	pGroupCode->AddSubItem(pPropCodeLeng);
	m_wndPropList.AddProperty(pGroupCode);

	gl_bRunDemode =FALSE;
	SetPara(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CDlgDemode::OnCbnSelchangeComboSigtype()
{
	int nStype;
	nStype = m_ComboSigtype.GetCurSel();
	SetPara(nStype);
	ResetList(nStype);
}

void CDlgDemode::SetPara(int nType)
{
	switch(nType)
	{
	case 0: // 110A
		pPropFre->Show(TRUE);
		pPropBaud->Show(TRUE);
		pPropFre->SetValue("1800");
		pPropBaud->SetValue("2400");
		pGroupFSK->Show(FALSE);
		pGroupCode->Show(FALSE);
		pPropTH->Show(TRUE);
		pPropTH->EnableSpinControl(TRUE,1,10);
		pPropTH->SetValue((COleVariant) 7l);
		pPropTH->SetDescription(_T("信号解调门限，范围1-10"));
		break;
	case 1: // 110B
		pPropFre->Show(TRUE);
		pPropBaud->Show(TRUE);
		pPropFre->SetValue("1800");
		pPropBaud->SetValue("2400");
		pGroupFSK->Show(FALSE);
		pGroupCode->Show(FALSE);
		pPropTH->Show(TRUE);
		pPropTH->EnableSpinControl(TRUE,1,10);
		pPropTH->SetValue((COleVariant) 6l);
		pPropTH->SetDescription(_T("信号解调门限，范围1-10"));
		break;
	case 2: // 141A
		pPropFre->Show(TRUE);
		pPropFre->SetValue("1625");
		pPropBaud->Show(TRUE);
		pPropBaud->SetValue("125");
		pGroupFSK->Show(TRUE);
		pPropFSKOrder->SetValue(_T("8FSK"));
		pPropFSKOrder->AllowEdit(FALSE);
		pPropFSKFre->SetValue(_T("250"));
		pPropFSKFre->AllowEdit(FALSE);
		pGroupCode->Show(FALSE);
		pPropTH->Show(FALSE);
		break;
	case 3: // 141B
		pPropFre->Show(TRUE);
		pPropBaud->Show(TRUE);
		pPropFre->SetValue("1800");
		pPropBaud->SetValue("2400");
		pGroupFSK->Show(FALSE);
		pGroupCode->Show(FALSE);
		pPropTH->Show(TRUE);
		pPropTH->EnableSpinControl(TRUE,1,10);
		pPropTH->SetValue((COleVariant) 6l);
		pPropTH->SetDescription(_T("信号解调门限，范围1-10"));
		break;
	case 4: // SLEW
		pPropFre->Show(TRUE);
		pPropBaud->Show(TRUE);
		pPropFre->SetValue("1800");
		pPropBaud->SetValue("2400");
		pGroupFSK->Show(FALSE);
		pGroupCode->Show(FALSE);
		pPropTH->Show(TRUE);
		pPropTH->EnableSpinControl(TRUE,1,10);
		pPropTH->SetValue((COleVariant) 6l);
		pPropTH->SetDescription(_T("信号解调门限，范围1-10"));
		break;
	case 5: // CLEW
		pPropFre->Show(TRUE);
		pPropFre->SetValue("605");
		pPropBaud->Show(FALSE);
		pGroupFSK->Show(FALSE);
		pGroupCode->Show(FALSE);
		pPropTH->Show(TRUE);
		pPropTH->EnableSpinControl(TRUE,1,10);
		pPropTH->SetValue((COleVariant) 1l);
		pPropTH->SetDescription(_T("信号解调门限，范围1-10"));
		break;
	case 6: // 4285
		pPropFre->Show(TRUE);
		pPropFre->SetValue("1800");
		pPropBaud->Show(TRUE);
		pPropBaud->SetValue("2400");
		pGroupFSK->Show(FALSE);
		pGroupCode->Show(TRUE);
		pPropTH->Show(TRUE);
		pPropTH->EnableSpinControl(TRUE,1,10);
		pPropTH->SetValue((COleVariant) 8l);
		pPropTH->SetDescription(_T("信号解调门限，范围1-10"));
		pPropDataRate->RemoveAllOptions();
		pPropDataRate->AddOption("3600");pPropDataRate->AddOption("2400");pPropDataRate->AddOption("1200");
		pPropDataRate->AddOption("600");pPropDataRate->AddOption("300");pPropDataRate->AddOption("150");
		pPropDataRate->AddOption("75");
		pPropDataRate->SetValue(_T("3600"));
		break;
	case 7: // 4529
		pPropFre->Show(TRUE);
		pPropFre->SetValue("2400");
		pPropBaud->Show(TRUE);
		pPropBaud->SetValue("1200");
		pGroupFSK->Show(FALSE);
		pGroupCode->Show(TRUE);
		pPropTH->Show(TRUE);
		pPropTH->EnableSpinControl(TRUE,1,10);
		pPropTH->SetValue((COleVariant) 8l);
		pPropTH->SetDescription(_T("信号解调门限，范围1-10"));
		pPropDataRate->RemoveAllOptions();
		pPropDataRate->AddOption("1800");pPropDataRate->AddOption("1200");pPropDataRate->AddOption("600");
		pPropDataRate->AddOption("300");pPropDataRate->AddOption("150");pPropDataRate->AddOption("75");
		pPropDataRate->SetValue(_T("1800"));
		break;
	default:
		break;
	}
}
void CDlgDemode::GetPara()
{
	CString text;
	text = (CString)pPropFre->GetValue();
	m_Fc = atof(text);
	text = (CString)pPropBaud->GetValue();
	m_Baud = atof(text);
	text = (CString)pPropDataRate->GetValue();
	dataRate = atoi(text);

	int nStype = m_ComboSigtype.GetCurSel();
	if(nStype==0 || nStype==1 || nStype==3 || nStype==4 || nStype==6 || nStype==7)
		m_DemodeTh = atof((CString)pPropTH->GetValue())/10.0;
	else if(nStype==5)
		m_DemodeTh = atof((CString)pPropTH->GetValue());
	else if(nStype==2) // 141A
		m_DemodeTh = atof((CString)pPropTH->GetValue())/10.0;


	text = (CString)pPropIscode->GetValue();
	if(text=="是")
		FECType = 1;
	else
		FECType = 0;
	text = (CString)pPropCodeLeng->GetValue();
	if(text=="No")
		InterType = 0;
	else if(text=="Short")
		InterType = 1;
	else if(text=="Long")
		InterType = 2;
}

void CDlgDemode::OnBnClickedDemode()
{
	CMainFrame *pMainFrame = (CMainFrame*)(AfxGetMainWnd());
	CHFanalyseFrame *pFrame = (CHFanalyseFrame *)pMainFrame->GetActiveFrame();
	pDemodeDoc = (CHFanalyseDoc*)pFrame->GetActiveDocument();
	GetPara();

	CString lpString;
	m_btnDemode.GetWindowText(lpString);

	

	if (pDemodeDoc!=NULL && pDemodeDoc->m_bFileOpen)
	{	
		if(gl_bRunDetect)
		{
			AfxMessageBox("正在识别，请先停止！");
			return;
		}
		if(lpString==_T("解调"))
		{
			m_btnDemode.SetWindowText(_T("停止"));
			m_FileWorkThread.m_opOpration = CFileWorkThread::opDemode;
			
			m_FileWorkThread.m_wndCtr = GetSafeHwnd();
			m_FileWorkThread.m_wndOut = pMainFrame->m_wndDemodeView;

			m_pParent->m_ListOut.DeleteAllItems();
			m_FileWorkThread.m_fileStytle = m_ComboSigtype.GetCurSel();
			m_FileWorkThread.m_FileFc = m_Fc;
			m_FileWorkThread.m_FileInsample = pDemodeDoc->WaveOut.m_pcmWaveFormat.nSamplesPerSec;
			m_FileWorkThread.m_FileBaud = m_Baud;
			m_FileWorkThread.dataRate = dataRate;
			m_FileWorkThread.FECType = FECType;
			m_FileWorkThread.InterType = InterType;
			m_FileWorkThread.m_DemodeTh = m_DemodeTh;
			m_FileWorkThread.brun = TRUE;
			gl_bRunDemode =TRUE;
			m_FileWorkThread.Start(&pDemodeDoc->m_File);
		}
		else if (lpString==_T("停止"))
		{
			m_btnDemode.SetWindowText(_T("解调"));
			m_FileWorkThread.brun = FALSE; 
			gl_bRunDemode =FALSE;
		}
	}
}

void CDlgDemode::ResetList(int nType)
{
	while(m_pParent->m_ListOut.DeleteColumn(0)) 
		continue;
	m_pParent->m_ListOut.DeleteAllItems();

	switch(nType)
	{
	case 0: // 110A
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,50);
		m_pParent->m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,50);
		break;
	case 1: // 110B
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,50);
		m_pParent->m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,50);
		break;
	case 2: // 141A
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(4,_T("信息"),LVCFMT_LEFT,100);
		break;
	case 3: // 141B
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(4,_T("波形"),LVCFMT_LEFT,50);
		break;
	case 4: // SLEW
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(4,_T("地址码"),LVCFMT_LEFT,50);
		m_pParent->m_ListOut.InsertColumn(5,_T("帧类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(6,_T("CRC"),LVCFMT_LEFT,50);
		break;
	case 5: // CLEW
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("结束时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(4,_T("频率"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(5,_T("地址码"),LVCFMT_LEFT,50);
		m_pParent->m_ListOut.InsertColumn(6,_T("帧类型"),LVCFMT_LEFT,80);
		break;
	case 6: // 4285
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,50);
		m_pParent->m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,50);
		break;
	case 7: // 4529
		m_pParent->m_ListOut.InsertColumn(0,_T("序号"),LVCFMT_LEFT,40);
		m_pParent->m_ListOut.InsertColumn(1,_T("信号类型"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(2,_T("起始时刻"),LVCFMT_LEFT,100);
		m_pParent->m_ListOut.InsertColumn(3,_T("频率"),LVCFMT_LEFT,80);
		m_pParent->m_ListOut.InsertColumn(4,_T("数据率"),LVCFMT_LEFT,50);
		m_pParent->m_ListOut.InsertColumn(5,_T("交织长度"),LVCFMT_LEFT,50);
		break;
	default:
		break;
	}	
}
LRESULT CDlgDemode::OnSystenState(WPARAM wParam, LPARAM lParam)
{
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDemodeDoc->m_hWndHFanalyseView));
	__int64 cuP = wParam;	
	if(m_FileWorkThread.brun)
	{
		pHFanalyseView->m_waveRender.setPlayLine(cuP);
		pHFanalyseView->m_waveRender.Invalidate();
		int nPercent = (int)m_FileWorkThread.nProgressPos;
		m_FileProcessCtr.SetPos(nPercent);
		if (nPercent==100)
		{
			m_btnDemode.SetWindowText(_T("解调"));
			gl_bRunDemode =FALSE;
		}
	}
	return 0;
}
