#pragma once

#include "ipps.h"
#include "ippsr.h"

#define wMIL141BTLC		10
#define wMIL141BBW0		11
#define wMIL141BBW1		12
#define wMIL141BBW2		13
#define wMIL141BBW3		14
#define wMIL141BBW4		15
#define wMIL141BBW5		16

class CBW0demode;
class CBW1demode;
class CBW3demode;
class CDetect141B;
class CBW2demode;

struct HeadType141B{
	int position;
	float fre;
	int BWtype;
};


class   AFX_EXT_CLASS CMIL141Bpro
{
public:
	CMIL141Bpro(void);
	~CMIL141Bpro(void);

public:
	
	CDetect141B *m_CDetect141B;
	CBW0demode *m_BW0demode;
	CBW1demode *m_BW1demode;
	CBW3demode *m_BW3demode;
	CBW2demode *m_BW2demode;

	Ipp32f frequency;
	int waveType;
	Ipp16s *pBuf;
	Ipp16s *pDelay;
	Ipp16s *pData;
	int dataLen;
	int    DelayLen;

	IppsResamplingPolyphaseFixed_16s *state;
	Ipp16s *inBuf;
	double time;
	int lastread;
	int nHistory;

	int m_Insample;
	int m_Outsample;

public:

	void MIL141Bdemode_ini(int nLeng,int Insample,int Outsample,Ipp16s P,Ipp32f roll,Ipp32f Baud,int SrctapLen);

	void MIL141Bdemode(Ipp16s *pSrc,int nLeng,Ipp16s P,Ipp16s *expout,int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType141B *headType,int &headNum);

	void MIL141Bdemode_free();


	void IppReSampleIni_16s(int nLeng, int history, int nInRate, int nOutRate);
	void IppReSample_16s(Ipp16s *pSrc,int nLeng,Ipp16s *pDst,int &outLen);
	void IppReSamplefree_16s();

};