// GetFileFormat.cpp : 实现文件
//

#include "stdafx.h"
#include "HFProtocol.h"
#include "GetFileFormat.h"


// CGetFileFormat 对话框

IMPLEMENT_DYNAMIC(CGetFileFormat, CDialog)

CGetFileFormat::CGetFileFormat(CWnd* pParent /*=NULL*/)
	: CDialog(CGetFileFormat::IDD, pParent)
	, m_FileSampleRate(0)
	, m_FileChannel(0)
	, m_Filebit(0)
{

}

CGetFileFormat::~CGetFileFormat()
{
}

void CGetFileFormat::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SAMPLERATE, m_FileSampleRate);
	DDX_Control(pDX, IDC_LISTSAMPLERATE, m_ListSampleRate);
	DDX_Radio(pDX, IDC_RADIOMONO, m_FileChannel);
}


BEGIN_MESSAGE_MAP(CGetFileFormat, CDialog)
	ON_BN_CLICKED(IDOK, &CGetFileFormat::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CGetFileFormat::OnBnClickedCancel)
	ON_LBN_SELCHANGE(IDC_LISTSAMPLERATE, &CGetFileFormat::OnLbnSelchangeListsamplerate)
END_MESSAGE_MAP()


// CGetFileFormat 消息处理程序


BOOL CGetFileFormat::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_FileChannel=0;
	m_Filebit=0;
	m_FileSampleRate = 9600;

	CheckRadioButton(IDC_RADIOMONO,IDC_RADIOSTEREO,IDC_RADIOMONO);
	CheckRadioButton(IDC_RADIO16BIT,IDC_RADIO16BIT,IDC_RADIO16BIT);

	m_ListSampleRate.AddString("48000");
	m_ListSampleRate.AddString("44100");
	m_ListSampleRate.AddString("32000");
	m_ListSampleRate.AddString("22050");
	m_ListSampleRate.AddString("19200");
	m_ListSampleRate.AddString("16000");
	m_ListSampleRate.AddString("12000");
	m_ListSampleRate.AddString("11052");
	m_ListSampleRate.AddString("10000");
	m_ListSampleRate.AddString("9600");
	m_ListSampleRate.AddString("8000");
	m_ListSampleRate.AddString("6000");
	UpdateData(FALSE);

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CGetFileFormat::OnLbnSelchangeListsamplerate()
{
	// TODO: 在此添加控件通知处理程序代码
	int nselect = m_ListSampleRate.GetCurSel();
	CString rate;
	m_ListSampleRate.GetText(nselect,rate);
	m_FileSampleRate = _ttoi(rate);
	UpdateData(FALSE);

}

void CGetFileFormat::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	gl_dwSamplesPerSec = m_FileSampleRate;

	gl_wChannels = m_FileChannel+1;
	if (m_Filebit==0)
	{
		gl_wBitsPerSample = 16;
	}
	OnOK();
}

void CGetFileFormat::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}
