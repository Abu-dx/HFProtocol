#pragma once
#include "ipps.h"

/************************************************************************/
/*   识别 SLEW
/************************************************************************/

class CPreambleConv;
class CDetectSLEW
{
public:
	CDetectSLEW(void);
	~CDetectSLEW(void);

public:
	void DetectSLEW(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect);

	// 添加其他参数识别

protected:
	CPreambleConv	*m_CPreambleConv;
	Ipp32fc			*PreambleSym;
	int				PreambleLen;

};