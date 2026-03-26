#pragma once
#include "ipps.h"
#include "ippsr.h"
class CReSampleFSK
{
public:
	CReSampleFSK(void);
	~CReSampleFSK(void);
public:
	/*Ipp32f **ReSample_taps;
	Ipp32f *ResampleBuf;
	Ipp64f m_Decimate,faddress;
	Ipp16s m_incimate;
	Ipp32s history,m_taps_onech;
	Ipp64s address;
	Ipp64f m_fstop;
	Ipp64f *pTaps_64f;
	int TapsLen;
	Ipp32f *pDelay;
	int delayLen;

	Ipp32f m_insample;
	Ipp32f m_outsample;*/
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
	/*void ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen);
	void ReSample(Ipp16s *pSrc,int nLeng,Ipp32f *pDst,int &outLeng);
	void ReSample_Free();*/


	void IppReSampleIni_16s(int nLeng, int history, int nInRate, int nOutRate);
	void IppReSample_16s(Ipp16s *pSrc,int nLeng,Ipp16s *pDst,int &outLen);
	void IppReSamplefree_16s();
};
