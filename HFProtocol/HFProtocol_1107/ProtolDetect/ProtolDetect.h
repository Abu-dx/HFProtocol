#pragma once

// 确保 CString 类型定义正确（MFC 扩展 DLL 需要）
#if defined(_AFXDLL) || defined(_AFXEXT)
#include <afxwin.h>         // MFC 核心组件和标准组件
#endif
#include "ipps.h"

#define  wMIL110A		0
#define  wMIL110B		1
#define  wMIL141A		2
#define  wMIL141B		3
#define  wLINK11SLEW	4
#define  wLINK11CLEW	5
#define  wNATO4285		6
#define  wNATO4529		7

#define wMIL141BTLC		10
#define wMIL141BBW0		11
#define wMIL141BBW1		12
#define wMIL141BBW2		13
#define wMIL141BBW3		14
#define wMIL141BBW4		15

class CDetect110A;
class CDetect110B;
class CDetectSLEW;
class CDetect4285;
class CDetect4529;
class CMIL141Apro;
class CDetect141B;
class CDetectCLEW;
class CReSampledetect;


struct ProtocolOut
{
	CStringA ProtocolName;  // 明确使用窄字符版本，避免 MBCS/Unicode 字符集不匹配
	int index;
	float maxCor;
	float frequency;
	int dataRate;
	int InterLen;
	int waveType;
};

class  AFX_EXT_CLASS CProtolDetect
{
public:
	CProtolDetect(void);
	~CProtolDetect(void);
public:

	CMIL141Apro *m_CMIL141Apro;
	CDetect110A *m_Detect110A;
	CDetect110B *m_Detect110B;
	CDetectSLEW *m_CDetectSLEW;
	CDetect4285 *m_CDetect4285;
	CDetect4529 *m_CDetect4529;
	CDetect141B *m_CDetect141B;
	CDetectCLEW *m_CDetectCLEW;
	CReSampledetect   *m_CReSample;

	ProtocolOut m_ProtocolOut[10];

	Ipp32fc *pBuf;	//数据块缓存
	Ipp32fc *delayBuf;
	int	    delayL;
	int     BufPos;

	int		maxConvLen;
	Ipp8u   *outbyte;

public:
	void ProtolDetect_ini(int nLeng,Ipp32f insample,BOOL *selProtol,int ProtolNum);
	void ProtolDetect_free();
	BOOL ProtolDetect(Ipp16s *pSrc,int nLeng,Ipp32f TH,BOOL *selProtol,int ProtolNum,ProtocolOut &m_ProtocolOut);

protected:
	
	
};