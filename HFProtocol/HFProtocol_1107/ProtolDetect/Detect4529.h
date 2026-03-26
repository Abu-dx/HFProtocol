#pragma once
#include "ipps.h"

/************************************************************************/
/*   识别 4529
/************************************************************************/

class CPreambleConv;
class CDetect4529
{
public:
	CDetect4529(void);
	~CDetect4529(void);

public:
	void Detect4529(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect);


	// 添加其他参数识别

protected:
	CPreambleConv	*m_CPreambleConv;
	Ipp32fc			*PreambleSym;
	int				PreambleLen;

};