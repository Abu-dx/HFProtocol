#pragma once
#include "ipps.h"

/************************************************************************/
/*   Ê¶±ð141B
/************************************************************************/

class CPreambleConv;
class CDetect141B
{
public:
	CDetect141B(void);
	~CDetect141B(void);

public:
	void Detect141B(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,int &waveType,BOOL &detect);



protected:

	CPreambleConv	*m_CPreambleConv;
	CPreambleConv   *m_CPreambleConvtlc;

	Ipp32fc			*TLCSym;
	int				TLCLen;
	Ipp32fc			*BW3Sym;
	int				BW3Len;
	Ipp32fc			*BW0Sym;
	int				BW0Len;
	Ipp32fc			*BW1Sym;
	int				BW1Len;

};