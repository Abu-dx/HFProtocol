#pragma once
#include "ipps.h"
class CReSample
{
public:
	CReSample(void);
	~CReSample(void);
public:
	Ipp32f **ReSample_taps;
	Ipp32fc *ResampleBuf;
	Ipp64f m_Decimate,faddress;
	Ipp16s m_incimate;
	Ipp32s history,m_taps_onech;
	Ipp64s address;
	Ipp64f m_fstop;
	Ipp64f *pTaps_64f;
	int TapsLen;
	Ipp32fc *pDelay;
	int delayLen;
	IppsHilbertSpec_16s32fc* pSpec;

	Ipp32f m_Insample,m_Outsample;

public:
	void ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen);
	void ReSample(Ipp16s *pSrc,int nLeng,Ipp32fc *pDst,int &outLeng);
	void ReSample_Free();
};
