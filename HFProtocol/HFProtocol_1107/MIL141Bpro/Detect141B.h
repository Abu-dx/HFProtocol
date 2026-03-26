#pragma once
#include "ipps.h"



/************************************************************************/
/*   识别141B
/************************************************************************/

class CPreambleConv;
class CDetect141B
{
public:
	CDetect141B(void);
	~CDetect141B(void);

public:
	void Detect141B(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &frequency,int &waveType,BOOL &detect);

	void MIL141Bdetect_ini(int nLeng);
	void MIL141Bdetect_free();
	void MIL141Bdetect(Ipp16s *pSrc,int nLeng,int &pIndex,Ipp32f &frequency,int &waveType,BOOL &detect);

	Ipp32fc *pBuf;	//数据块缓存
	Ipp32fc *delayBuf;
	int	    delayL;
	int     BufPos;
	int		maxConvLen;

	IppsHilbertSpec_16s32fc* pSpec;

protected:

	CPreambleConv	*m_CPreambleConvtlc;
	CPreambleConv	*m_CPreambleConv;

	Ipp32fc			*TLCSym;
	int				TLCLen;
	Ipp32fc			*BW3Sym;
	int				BW3Len;
	Ipp32fc			*BW0Sym;
	int				BW0Len;
	Ipp32fc			*BW1Sym;
	int				BW1Len;

};