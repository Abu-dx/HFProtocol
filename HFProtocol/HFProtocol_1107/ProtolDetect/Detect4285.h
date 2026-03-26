#pragma once
#include "ipps.h"

/************************************************************************/
/*   识别 4285
/************************************************************************/

class CPreambleConv;
class CDetect4285
{
public:
	CDetect4285(void);
	~CDetect4285(void);

public:
	void Detect4285(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect);


	// 添加其他参数识别

protected:
	CPreambleConv	*m_CPreambleConv;
	Ipp32fc			*PreambleSym;
	int				PreambleLen;

};