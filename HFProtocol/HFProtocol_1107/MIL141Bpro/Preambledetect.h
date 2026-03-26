#pragma once

#include "ipps.h"

class   CPreambledetect
{
public:
	CPreambledetect(void);
	~CPreambledetect(void);
public:

	Ipp32f **ReSample_taps;
	Ipp32f *ResampleBuf;
	Ipp64f m_Decimate,faddress;
	Ipp16s m_incimate;
	Ipp32s history,m_taps_onech;
	Ipp64s address;
	Ipp64f m_fstop;
	Ipp64f *pTaps_64f;
	int mSrctaplen;
	Ipp32f *pDelay;
	int delayLen;

	Ipp32f m_Insample,m_Outsample;

	//  突发检测
	Ipp32f *pBuf;	//数据块缓存
	Ipp32f *delayBuf;
	int	    delayL;
	int     BufPos;

	Ipp32fc *pConvAB;
	Ipp32f  *pConv;
	Ipp32fc *pConvFFT;
	Ipp32f *pabsConvFFT;
	IppsFFTSpec_C_32fc *pConvFFTSpec;
	Ipp32f  *winBufConv;//相关运算结果缓存
	Ipp16s *ConvIndex;
	Ipp32f pEnergy;


	int *LastPos;
	IppsHilbertSpec_32f32fc* pSpec;

	Ipp32fc *pPreambleSym;
	int PreambleLen;


public:
	void PSK_map(short style, short M, float I_map[],float Q_map[]); /* 星座映射方式  0——Gray，1——antiGray*/
	void PSK_MOD(short data[],short data_len,short M,Ipp32fc *out);
	void Preamble_Gen();
	void Preamble_Gen(int type);

	int  NumberOfBitsNeeded(short PowerOfTwo);

	void Burst_detect_ini(int bufLen);
	void CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f &cConv);
	void CrossConvFFT(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s ConvLeng,int nFFT,Ipp32f &cConv,int &pIndex);

	void Burst_detect_ini(int bufLen,int ConvLen,int deciP,int FFTLen);
	void Burst_detect_free();
	void Burst_detect(int bufLen,Ipp32fc *UWsig,Ipp32f UWEnergy, int convLen,int deciP,int nFFT,
		Ipp32f TH1,Ipp32f TH2,Ipp16s *BurstPos,Ipp32f *fre,int &BurstNum,int &BufPos,int &BufHavePro);

	void ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen);
	void ReSample(Ipp16s *pSrc,int nLeng,Ipp32f *pDst,int &outLeng);
	void ReSample_Free();

	void ProtolDetect_ini(int nLeng,int maxConvLen,int FFTLen,Ipp32f insample,Ipp32f outsample,int deciP);
	void ProtolDetect_free();

};