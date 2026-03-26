#pragma once
#include "ipps.h"

/************************************************************************/
/*   识别 CLEW
/************************************************************************/

class CDetectCLEW
{
public:
	CDetectCLEW(void);
	~CDetectCLEW(void);

public:
	void DetectCLEW_ini(int nLen,Ipp32f insample);
	void DetectCLEW_free();
	void DetectCLEW(Ipp32fc *pSrc,int nLen,Ipp32f TH,int &pIndex,Ipp32f &decsum,Ipp32f &fdopule);

protected:

	Ipp32fc *pdataBuf;
	Ipp32fc *pdataDelay;
	int		datadelayLen;
	int		pdataPos;
	int		maxDelayLen;

	//para OFDM参数
	int OFDM_symL,OFDM_fft,OFDM_cpxL;
	// detect
	Ipp32fc *bufDetectFFT; 
	IppsFFTSpec_C_32fc* pDetectFFTSpec;
	int detect_fft;
	int idx_dopule,idx_16,idx_data1,idx_data14,delt;

	void FFTdetect(Ipp32fc *pSrc,int nLen,Ipp32f &fdopule,Ipp32f &decsum,Ipp32f &sum16);

protected:
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

	void ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen);
	void ReSample(Ipp32fc *pSrc,int nLeng,Ipp32fc *pDst,int &outLeng);
	void ReSample_Free();

};