
// HFanalyseFrame.h : CHFanalyseFrame 类的接口
//

#pragma once
class CHFanalyseFrame : public CMDIChildWndEx
{
	DECLARE_DYNCREATE(CHFanalyseFrame)
public:
	CHFanalyseFrame();

// 属性
public:
	CSplitterWnd m_wndSplitter; // 分割视图控件
// 操作
public:

// 重写
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CHFanalyseFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	virtual void ActivateFrame(int nCmdShow = -1);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
//	afx_msg void OnPlay();
	afx_msg void OnPlay();
	afx_msg void OnUpdatePlay(CCmdUI *pCmdUI);
	afx_msg void OnStop();
	afx_msg void OnUpdateStop(CCmdUI *pCmdUI);
	afx_msg void OnContinue();
	afx_msg void OnUpdateContinue(CCmdUI *pCmdUI);
	afx_msg void OnStopback();
	afx_msg void OnUpdateStopback(CCmdUI *pCmdUI);
};
