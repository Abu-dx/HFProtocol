#pragma once

#include "JWaveDLLE.h"
#include "PlayPaneView.h"

// CHFanalyseView 窗体视图

class CHFanalyseView : public CFormView
{
	DECLARE_DYNCREATE(CHFanalyseView)

protected:
	CHFanalyseView();           // 动态创建所使用的受保护的构造函数
	virtual ~CHFanalyseView();
	
public:	
	waveRender m_waveRender;


public:
	enum { IDD = IDD_HFANALYSEVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnDraw(CDC* /*pDC*/);
public:
	virtual void OnInitialUpdate();
};


