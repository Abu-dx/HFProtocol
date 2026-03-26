#pragma once
#include "ipps.h"

/************************************************************************/
/*   Ê¶±ð110A
/************************************************************************/

class CPreambleConv;
class CDetect110A
{
public:
	CDetect110A(void);
	~CDetect110A(void);

public:
	void Detect110A(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,int &dataRate,int &interLeng,BOOL &detect);

protected:
	void IdentifyD1D2(Ipp32fc *pSrc,int typeNum,int &dataRate,int &interLeng);
	CPreambleConv	*m_CPreambleConv;
	Ipp32fc			*PreambleSym;
	int				PreambleLen;

	Ipp32fc			*pD1D2;
	Ipp32fc			**pTypeSym;

};