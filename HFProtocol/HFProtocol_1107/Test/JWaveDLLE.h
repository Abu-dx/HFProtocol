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

//#undef AFX_DATA
//#define AFX_DATA AFX_EXT_DATA

// Initialize the DLL, register the classes etc

extern "C" AFX_EXT_API void WINAPI InitJWaveDLL();

/////////////////////////////////////////////////////////////////////////////
#define  XMODE_SAMPLE	1
#define  XMODE_TIME		2

#define  SHOWMODE_BOTH 3
#define  SHOWMODE_TIME 1
#define  SHOWMODE_FRE  2
// Simple Text document
#define  GRAPH_SEL_START  WM_USER+1
#define  GRAPH_SEL_END	  WM_USER+3
#define	 YSCALE_DATAVIEW  WM_USER+4
//UINT64 最大 1844 6744 0737 0955 1615  将每两个点之间化为1024份dJ 每个屏幕显示的是整数份dJ
//那么可以支持的最大文件为1024*1024TB的文件，可以满足绝大多少的要求

// waveRender
class waveImage;
class RenderPicture;

class AFX_EXT_CLASS waveRender : public CWnd
{
	DECLARE_DYNAMIC(waveRender)

	//{数据
	__int64		length_Data;//数据长度
	CArray<RenderPicture*> dataArray;
	int			numberOfData;
	__int64		sampltRate;//采样率
	int			log2fftLength;
	//}

	//{显示
	double		YMoveScale;
	__int64		startPoints;//屏幕显示的起始点
	__int64		endPoints;//屏幕显示的起始点
	__int64		Points_screen;//每一屏幕显示的点数
	long double pointsPerpix;//每个像素占的点数
	int  		AmplitudeMax;//Y轴的缩放
	int			Yoffset;//Y轴的偏移
	__int64		m_selectedPoint_Start;//选中点开始
	__int64		m_selectedPoint_End;//选中点结束
	__int64		playLine;
	CFont		axisFont;
	int			showMode;//显示的类型
	int			axisShowMode;
	__int64		currentSpan;//X坐标间隔系数
	int			currrentIndex;	
	int			currentSpanTime;//X坐标间隔系数(Time)
	bool		isSecend;
	__int64		currentIndexTime;//X坐标间隔系数(Time)
	int			log10currentSpanTime;
	__int64		currentSpanYR;//Y坐标间隔系数
	int			currrentIndexYR;
	__int64		currentSpanYC;//Y坐标间隔系数
	int			currrentIndexYC;
	__int64		currentSpanYFreR;//频率Y坐标间隔系数
	int			currrentIndexYFreR;
	__int64		currentSpanYFreC;//频率Y坐标间隔系数
	int			currrentIndexYFreC;
	//}


	//{状态

	CRect		rect;
	int			Width,WidthOld;
	int			Height;
	int			HeightData;//每一个数据的高度
	int			HeightData_Pic;//每一个数据的子图高度
	int			HeightData_Pic_Component;//每一个数据分量的高度
	CPoint		lastPoint;//上一次move时鼠标所在位置
	int			cursorState;
	HWND		h_MsgWnd;
	//}


	CString JJ(int cuTimeS,int );
	void	axisPointsSpan(CDC*);//坐标轴的点数间隔
	void	axisPointsSpanTime(CDC*);
	void	axisPointsSpanY();//坐标轴的点数间隔
	void	axisPointsSpanYF();//坐标轴的点数间隔
	void	updateScale();
	void	updateSize();//跟新每个图大小的计算

public:
	waveRender();
	virtual ~waveRender();

	//{数据
	bool setData(CFile*pFile,__int64 length_Data,bool complex=false);
	void setFFTLength(int log2fftLength);//fft点数
	//}数据

	//{设置参数
	void setPlayLine(__int64 cuP);
	void setCuPoint(__int64 cuP);//当前的选中位置
	void setCuArea(__int64 startP,__int64 endP);//当前的选中区域
	void SetXRange(__int64 startP,__int64 endP);//设置当前X显示区域
	void SetYRange(__int64 startP,__int64 endP);//设置当前Y显示区域
	void setSampleRate(__int64);
	//}设置参数

	//{获取参数
	__int64 getCuPoint();//当前的选中位置
	void getCuPoint(__int64&cuP);//当前的选中位置
	void getCuArea(__int64 &startP,__int64 &endP);//当前的选中区域
	void getXRange(__int64 &startP,__int64 &endP);//获取当前X显示区域
	void getYRange(__int64 &startP,__int64 &endP);//获取当前Y显示区域
	CRect getXAxisArea();
	//}获取参数

	//{显示
	void setShowMode(int);
	void setXMode(int);
	//}

	void setMsgWnd(HWND);
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
	//三个图层 及其更新函数
	CRgn	rgn;//裁剪区域
	CBitmap Img_background;
	CBitmap Img_waveform,Img_waveformI;
	CBitmap Img_ui;
	bool    update_Background;
	bool	update_Wavefrom;
	bool	update_Ui;

	void UpdateBackground(CDC*);
	void UpdateWavefrom(CDC*);
	void UpdateUi(CDC*);
};

#undef AFX_DATA
#define AFX_DATA

/////////////////////////////////////////////////////////////////////////////
