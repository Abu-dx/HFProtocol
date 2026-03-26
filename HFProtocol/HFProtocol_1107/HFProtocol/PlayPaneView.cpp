// PlayPaneView.cpp : 实现文件
//

#include "stdafx.h"
#include "HFProtocol.h"
#include "PlayPaneView.h"
#include "HFanalyseDoc.h"
#include "HFanalyseView.h"
#include "MainFrm.h"

// CPlayPaneView

IMPLEMENT_DYNCREATE(CPlayPaneView, CFormView)

CPlayPaneView::CPlayPaneView()
	: CFormView(CPlayPaneView::IDD)
	, m_CurTime(_T(""))
{
	alltime = 0;
	curtime = 0;

	startPoints = 0 ;
	endPoints = 0 ;
	selectPointsStart = 0;
	selectPointsEnd = 0;

	m_beginPoints = 0;
	m_endPoints  = 0;
	m_curPoints = 0;

}

CPlayPaneView::~CPlayPaneView()
{
}

void CPlayPaneView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_STOPBACK, m_ButtonStopBack);
	DDX_Control(pDX, IDC_BUTTON_PLAY, m_ButtonPlay);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_ButtonPause);
	DDX_Control(pDX, IDC_BUTTON_BACKWARD, m_ButtonBackward);
	DDX_Control(pDX, IDC_BUTTON_FORWARD, m_ButtonForward);
	DDX_Control(pDX, IDC_BUTTON_GOTOBEGIN, m_ButtonGotobegin);
	DDX_Control(pDX, IDC_BUTTON_GOTOEND, m_ButtonGotoend);
	DDX_Control(pDX, IDC_BUTTON_PLAYTOEND, m_ButtonPlaytoend);
	DDX_Text(pDX, IDC_SHOWTIME, m_CurTime);
	DDX_Control(pDX, IDC_SHOWTIME, m_CtrCurTime);
	DDX_Control(pDX, IDC_EDIT_SEL_BEGIN, m_editSelBegin);
	DDX_Control(pDX, IDC_EDIT_SEL_END, m_editSelEnd);
	DDX_Control(pDX, IDC_EDIT_SEL_LENGTH, m_editSelLength);
	DDX_Control(pDX, IDC_EDIT_SHOW_BEGIN, m_editShowBegin);
	DDX_Control(pDX, IDC_EDIT_SHOW_END, m_editShowEnd);
	DDX_Control(pDX, IDC_EDIT_SHOW_LENGTH, m_editShowLength);
	DDX_Control(pDX, IDC_SAMPLE, m_staticSample);
}

BEGIN_MESSAGE_MAP(CPlayPaneView, CFormView)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CPlayPaneView::OnBnClickedButtonPlay)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CPlayPaneView::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOPBACK, &CPlayPaneView::OnBnClickedButtonStopback)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_GOTOBEGIN, &CPlayPaneView::OnBnClickedButtonGotobegin)
	ON_BN_CLICKED(IDC_BUTTON_BACKWARD, &CPlayPaneView::OnBnClickedButtonBackward)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD, &CPlayPaneView::OnBnClickedButtonForward)
	ON_BN_CLICKED(IDC_BUTTON_GOTOEND, &CPlayPaneView::OnBnClickedButtonGotoend)

	ON_BN_CLICKED(IDC_BUTTON_PLAYTOEND, &CPlayPaneView::OnBnClickedButtonPlaytoend)
	ON_MESSAGE(GRAPH_SEL_START,OnSelStart)
	ON_MESSAGE(GRAPH_SEL_END,OnSelEnd)
END_MESSAGE_MAP()


// CPlayPaneView 诊断

// CPlayPaneView 消息处理程序

void CPlayPaneView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	pDoc->m_hWndPlayPaneView = m_hWnd;

	CString sample;
	sample.Format("采样率：%dHz",pDoc->WaveOut.m_pcmWaveFormat.nSamplesPerSec);
	m_staticSample.SetWindowText(sample);

	HICON icon;
	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_STOP),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonStopBack.SetImage(icon);
	m_ButtonStopBack.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonStopBack.SizeToContent();
	m_ButtonStopBack.Invalidate();
	m_ButtonStopBack.SetTooltip("停止");
	m_ButtonStopBack.EnableWindow(FALSE);
	m_bStopBack = FALSE;
	

	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_PLAY),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonPlay.SetImage(icon);
	m_ButtonPlay.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonPlay.SizeToContent();
	m_ButtonPlay.Invalidate();
	m_ButtonPlay.SetTooltip("播放");
	m_bPlay = FALSE;

	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_PAUSE),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonPause.SetImage(icon);
	m_ButtonPause.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonPause.SizeToContent();
	m_ButtonPause.Invalidate();
	m_ButtonPause.SetTooltip("暂停");
	m_ButtonPause.EnableWindow(FALSE);
	m_bPause = FALSE;

	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_GOTOBEGIN),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonGotobegin.SetImage(icon);
	m_ButtonGotobegin.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonGotobegin.SizeToContent();
	m_ButtonGotobegin.Invalidate();
	m_ButtonGotobegin.SetTooltip("到起点");
	m_ButtonGotobegin.EnableWindow(TRUE);

	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_GOTOEND),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonGotoend.SetImage(icon);
	m_ButtonGotoend.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonGotoend.SizeToContent();
	m_ButtonGotoend.Invalidate();
	m_ButtonGotoend.SetTooltip("到终点");
	m_ButtonGotoend.EnableWindow(TRUE);

	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_BACKWARD),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonBackward.SetImage(icon);
	m_ButtonBackward.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonBackward.SizeToContent();
	m_ButtonBackward.Invalidate();
	m_ButtonBackward.SetTooltip("向后");
	m_ButtonBackward.EnableWindow(TRUE);

	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_FORWARD),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonForward.SetImage(icon);
	m_ButtonForward.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonForward.SizeToContent();
	m_ButtonForward.Invalidate();
	m_ButtonForward.SetTooltip("向前");
	m_ButtonForward.EnableWindow(TRUE);

	icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_PLAYTOEND),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_ButtonPlaytoend.SetImage(icon);
	m_ButtonPlaytoend.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
	m_ButtonPlaytoend.SizeToContent();
	m_ButtonPlaytoend.Invalidate();
	m_ButtonPlaytoend.SetTooltip("播放至结束");
	m_ButtonPlaytoend.EnableWindow(TRUE);




	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));
	pHFanalyseView->m_waveRender.setCuPoint(0);
	pHFanalyseView->m_waveRender.setPlayLine(0);
	pHFanalyseView->m_waveRender.getCuArea(startPoints,endPoints);//当前的选中区域
	pHFanalyseView->m_waveRender.getXRange(selectPointsStart,selectPointsEnd);//获取当前X显示区域
	pHFanalyseView->m_waveRender.setSampleRate(pDoc->WaveOut.m_pcmWaveFormat.nSamplesPerSec);	

	pHFanalyseView->m_waveRender.setMsgWnd(m_hWnd);

	UpdateData(FALSE);

	pDoc->m_File.Seek(0, CFile::begin);
	char szTmp[10];
	ZeroMemory(szTmp, 10 * sizeof(char));
	pDoc->m_File.Read(szTmp, 4 * sizeof(char)) ;
	if (strncmp(szTmp, _T("RIFF"), 4) != 0) {
		AfxMessageBox("非wave格式，无法播放！");
	}
	else
	{
		pDoc->m_File.Seek(0, CFile::begin);
		if ( pDoc->WaveOut.IsPlaying() ) {
			if ( !pDoc->WaveOut.Close() ) {
				AfxMessageBox( pDoc->WaveOut.GetError() );
				return;
			}
		}	
		pDoc->WaveOut.Load(&pDoc->m_File);
		if ( !pDoc->WaveOut.IsOutputFormat() ) {
			AfxMessageBox("Format non support?");
			return;
		}
		alltime = pDoc->WaveOut.AllTime();
		curtime = 0;
		//CString time = convertTime(curtime);
		//DisplayTime(time);
	}
	pDoc->m_File.Seek(0, CFile::begin);

	CString time;
	time = convertTime(0);
	m_editShowBegin.SetWindowText(time);
	time = convertTime(pDoc->m_FileLength);
	m_editShowEnd.SetWindowText(time);
	time = convertTime(pDoc->m_FileLength);
	m_editShowLength.SetWindowText(time);
	UpdateData(FALSE);

	
}

void CPlayPaneView::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));

	pHFanalyseView->m_waveRender.getCuArea(selectPointsStart,selectPointsEnd);//当前的选中区域
	pHFanalyseView->m_waveRender.getXRange(startPoints,endPoints);//获取当前X显示区域
	pHFanalyseView->m_waveRender.getCuPoint(m_curPoints);

	if(!m_bPause)
	{
		if ( pDoc->WaveOut.IsPlaying() ) 
			return;
		if ( !pDoc->WaveOut.Open() ) {
			AfxMessageBox( pDoc->WaveOut.GetError() );
			return;
		}
		if (selectPointsEnd==-1) // 不选择时从当前点播放
		{
			if ( !pDoc->WaveOut.Play(m_curPoints,endPoints) ) {
				AfxMessageBox( pDoc->WaveOut.GetError() );
				return;
			}
			m_beginPoints = m_curPoints;
			m_endPoints = endPoints;
		}
		else //选择播放
		{	
			if ( !pDoc->WaveOut.Play(selectPointsStart,selectPointsEnd) ) {
				AfxMessageBox( pDoc->WaveOut.GetError() );
				return;
			}
			m_beginPoints = selectPointsStart;
			m_endPoints = selectPointsEnd;
		}
		m_ButtonPause.EnableWindow(TRUE);
		m_ButtonStopBack.EnableWindow(TRUE);		
		m_bPause = FALSE;
		m_bStopBack = TRUE;
		m_bPlay = TRUE;
		SetTimer(1,100,NULL);
	}
	else
	{
		HICON icon;
		icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_PAUSE),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		m_ButtonPause.SetImage(icon);
		m_ButtonPause.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
		m_ButtonPause.SizeToContent();
		m_ButtonPause.Invalidate();
		m_ButtonPause.SetTooltip("继续");
		m_bPause = FALSE;
		m_bPlay = TRUE;
		if ( !pDoc->WaveOut.Continue() ) {
			AfxMessageBox( pDoc->WaveOut.GetError() );
		}
		SetTimer(1,100,NULL);
	}	
	
}

void CPlayPaneView::OnBnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	HICON icon;
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	if(!m_bPause){
		icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_PAUSE),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		m_ButtonPause.SetImage(icon);
		m_ButtonPause.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
		m_ButtonPause.SizeToContent();
		m_ButtonPause.Invalidate();
		m_ButtonPause.SetTooltip("暂停");

		if ( !pDoc->WaveOut.Pause() ) {
			AfxMessageBox( pDoc->WaveOut.GetError() );
		}
		m_bPause = TRUE;
		m_bPlay = FALSE;
		KillTimer(1);
	}
	else
	{
		icon = (HICON)::LoadImage(::AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_PAUSE),IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		m_ButtonPause.SetImage(icon);
		m_ButtonPause.m_nFlatStyle = CMFCButton::BUTTONSTYLE_FLAT;
		m_ButtonPause.SizeToContent();
		m_ButtonPause.Invalidate();
		m_ButtonPause.SetTooltip("继续");
		m_bPause = FALSE;
		m_bPlay = TRUE;
		if ( !pDoc->WaveOut.Continue() ) {
			AfxMessageBox( pDoc->WaveOut.GetError() );
		}
		SetTimer(1,100,NULL);
	}

}

void CPlayPaneView::OnBnClickedButtonGotobegin()
{
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));
	if ( !pDoc->WaveOut.Close() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
	}
	pHFanalyseView->m_waveRender.setCuPoint(0);
	pHFanalyseView->m_waveRender.setPlayLine(0);
	pHFanalyseView->m_waveRender.Invalidate();
	KillTimer(1);
	m_bPause = FALSE;
	OnBnClickedButtonPlay();

}

void CPlayPaneView::OnBnClickedButtonBackward()
{
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));

	if ( !pDoc->WaveOut.IsPlaying() ) 
		return;
	__int64 pos = m_beginPoints + pDoc->WaveOut.GetPosition();
	pos = pos - 10000;
	if(pos<0)
		pos =0;
	if ( !pDoc->WaveOut.Close() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
	}
	KillTimer(1);

	if ( !pDoc->WaveOut.Open() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
		return;
	}
	if(pos>=m_endPoints)
		pos = m_endPoints;
	if ( !pDoc->WaveOut.Play(pos,m_endPoints) ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
		return;
	}
	m_beginPoints = pos;
	SetTimer(1,100,NULL);

}

void CPlayPaneView::OnBnClickedButtonForward()
{
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));

	if ( !pDoc->WaveOut.IsPlaying() ) 
		return;
	__int64 pos = m_beginPoints + pDoc->WaveOut.GetPosition();
	pos = pos + 10000;
	if(pos>=m_endPoints)
		pos = m_endPoints;
	if ( !pDoc->WaveOut.Close() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
	}
	KillTimer(1);

	if ( !pDoc->WaveOut.Open() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
		return;
	}
	if ( !pDoc->WaveOut.Play(pos,m_endPoints) ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
		return;
	}
	m_beginPoints = pos;
	SetTimer(1,100,NULL);
}

void CPlayPaneView::OnBnClickedButtonGotoend()
{
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));
	if ( !pDoc->WaveOut.Close() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
	}
	pHFanalyseView->m_waveRender.setPlayLine(endPoints);
	pHFanalyseView->m_waveRender.Invalidate();
	KillTimer(1);
	m_bPause = FALSE;
}

void CPlayPaneView::OnBnClickedButtonPlaytoend()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));
	pHFanalyseView->m_waveRender.getXRange(startPoints,endPoints);//获取当前X显示区域
	pHFanalyseView->m_waveRender.getCuPoint(m_curPoints);

	if ( pDoc->WaveOut.IsPlaying() ) 
	{
		pDoc->WaveOut.Close();
		KillTimer(1);
	}

	if ( !pDoc->WaveOut.Open() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
		return;
	}

	if ( !pDoc->WaveOut.Play(m_curPoints,endPoints) ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
		return;
	}
	m_beginPoints = m_curPoints;
	m_endPoints = endPoints;

	m_ButtonPause.EnableWindow(TRUE);
	m_ButtonStopBack.EnableWindow(TRUE);		
	m_bPause = FALSE;
	m_bPlay = TRUE;
	SetTimer(1,100,NULL);

}

void CPlayPaneView::OnBnClickedButtonStopback()
{
	// TODO: 在此添加控件通知处理程序代码
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));
	if ( !pDoc->WaveOut.Close() ) {
		AfxMessageBox( pDoc->WaveOut.GetError() );
	}
	
	pHFanalyseView->m_waveRender.setCuPoint(0);
	pHFanalyseView->m_waveRender.setPlayLine(0);
	pHFanalyseView->m_waveRender.Invalidate();

	CString time = convertTime(0);
	DisplayTime(time);
	m_ButtonPause.EnableWindow(FALSE);
	m_ButtonStopBack.EnableWindow(FALSE);
	m_ButtonPlay.EnableWindow(TRUE);
	m_bPause = FALSE;
	m_bPlay = FALSE;
	KillTimer(1);
}

void CPlayPaneView::OnTimer(UINT_PTR nIDEvent)
{

	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString time;
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));
	
	pHFanalyseView->m_waveRender.getCuPoint(m_curPoints);
	pHFanalyseView->m_waveRender.getXRange(startPoints,endPoints);//获取当前X显示区域
	__int64 showLen = endPoints-startPoints;

	__int64 pos = m_beginPoints + pDoc->WaveOut.GetPosition();
	
	time = convertTime(pos);
	DisplayTime(time);
	pHFanalyseView->m_waveRender.setPlayLine(pos);
	
	if (pos>=m_endPoints)
	{

		time = convertTime(m_beginPoints);
		DisplayTime(time);

		pHFanalyseView->m_waveRender.setPlayLine(0);
		if ( !pDoc->WaveOut.Close() ) {
			AfxMessageBox( pDoc->WaveOut.GetError() );
		}
		pHFanalyseView->m_waveRender.Invalidate();
		m_ButtonPlay.EnableWindow(TRUE);
		time = convertTime(0);
		DisplayTime(time);
		m_ButtonPause.EnableWindow(FALSE);
		m_ButtonStopBack.EnableWindow(FALSE);
		m_bPause = FALSE;
		m_bStopBack = TRUE;
		m_bPlay = FALSE;
		KillTimer(1);
		return;
	}
	if (pos>=endPoints)
	{
		pHFanalyseView->m_waveRender.SetXRange(pos-showLen,pos);
	}
	pHFanalyseView->m_waveRender.Invalidate();
	
	CFormView::OnTimer(nIDEvent);
}
void CPlayPaneView::DisplayTime(CString time)
{
	CFont font;
	font.CreateFont(30, 0, 0, 0, 60, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));
	m_CtrCurTime.SetFont(&font,TRUE);
	m_CurTime = time;
	UpdateData(FALSE);
}
CString CPlayPaneView::convertTime(__int64 point)
{
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();

	__int64 t = 1000*point/pDoc->WaveOut.m_pcmWaveFormat.nSamplesPerSec;  //pDoc->WaveOut.GetAve();
	__int32 minute = t/60000;
	__int64 milsecond = t%60000;
	__int32 second = milsecond/1000;
	__int32 milsec = milsecond%1000;

	__int32 hour = minute/60;
	minute = minute%60;

	CString m_Time;
	m_Time.Format("%02d:%02d:%02d.%03d",hour,minute,second,milsec);

	return m_Time;
	
}

LRESULT CPlayPaneView::OnSelStart(WPARAM wParam, LPARAM lParam)
{
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));

	__int64 curStart,curEnd,xStart,xEnd;
	pHFanalyseView->m_waveRender.getCuArea(curStart,curEnd);//当前的选中区域
	pHFanalyseView->m_waveRender.getXRange(xStart,xEnd);//获取当前X显示区域

	CString time;
	time = convertTime(curStart);
	m_editSelBegin.SetWindowText(time);
	
	DisplayTime(time);

	m_editSelEnd.SetWindowText("");
	m_editSelLength.SetWindowText("");

	time = convertTime(xStart);
	m_editShowBegin.SetWindowText(time);
	time = convertTime(xEnd);
	m_editShowEnd.SetWindowText(time);
	time = convertTime(xEnd-xStart);
	m_editShowLength.SetWindowText(time);

	UpdateData(FALSE);

	return 0;
}
LRESULT CPlayPaneView::OnSelEnd(WPARAM wParam, LPARAM lParam)
{
	
	CHFanalyseDoc* pDoc = (CHFanalyseDoc*)GetDocument();
	CHFanalyseView* pHFanalyseView = (CHFanalyseView*)(CWnd::FromHandle(pDoc->m_hWndHFanalyseView));

	__int64 curStart,curEnd,xStart,xEnd,temp;
	pHFanalyseView->m_waveRender.getCuArea(curStart,curEnd);//当前的选中区域
	pHFanalyseView->m_waveRender.getXRange(xStart,xEnd);//获取当前X显示区域

	if(curStart>curEnd)
	{
		temp = curStart;
		curStart = curEnd;
		curEnd = temp;
	}

	CString time;
	time = convertTime(curStart);
	m_editSelBegin.SetWindowText(time);
	time = convertTime(curEnd);
	m_editSelEnd.SetWindowText(time);
	time = convertTime(curEnd-curStart);
	m_editSelLength.SetWindowText(time);

	return 0;
}
