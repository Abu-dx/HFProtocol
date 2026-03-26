// TESTDLL1.H Public API exported from 'TESTDLL1.DLL'
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

// Note: This sample uses AFX_EXT_CLASS to export an entire class.
// This avoids creating a .DEF file with all the decorated names for
// the class.  Creating a .DEF file is more efficient since the names
// can be exported by ordinal.  To use that method of exporting, much
// like MFC does itself, uncomment the following two lines and add
// the appropriate lines to your .DEF file.
extern "C" AFX_EXT_API void WINAPI InitJEyeViewDLL();
//#undef AFX_DATA
//#define AFX_DATA AFX_EXT_DATA


#define  SHOWMODE_POINT  1
#define  SHOWMODE_VECTOR 0

#define  YSCALE_CONSTELLATION  WM_USER+31

class AFX_EXT_CLASS JConstellationView: public CWnd
{
	DECLARE_DYNAMIC(JConstellationView)


	//{数据
	short* pData;
	int	   length_Data;
	int	   length_Buffer;
	//}数据

	//{绘制参数	
	int	   AmplitudeMax;//最大的幅度
	int    Width;
	int	   Height;
	bool   showMode;
	__int64		currentSpan;//坐标间隔系数
	int			currrentIndex;
	__int64		currentSpanY;//坐标间隔系数
	int			currrentIndexY;
	CFont axisFont;
	//}绘制参数	

	//{状态参数
	int			cursorState;
	HWND     h_MsgWnd;
	//}状态参数

	void	axisPointsSpan(CDC*);//坐标轴的点数间隔
	void	axisPointsSpanY();//坐标轴的点数间隔
public:
	JConstellationView();
	virtual ~JConstellationView();
	//{数据
	void    setData(short*pData,int size);
	//}数据

	//{设置参数
	void	setShowMode(int);
	void	setAmplitude(int);
	//}设置参数

	//{获取参数
	int		getAmplitude();
	int		getShowMode();
	//}
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
private:
	void UpdateBackground(CDC*);
	void UpdateWavefrom(CDC*pDC);
	CBitmap Img_waveform;
	bool	update_Wavefrom;
};

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
