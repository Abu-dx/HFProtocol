#pragma once
#include "ipps.h"
class CPreambleConv
{
public:
	CPreambleConv(void);
	~CPreambleConv(void);
public:
	
	//  突发检测	
	Ipp32fc		*pConvAB;
	Ipp32f		*pConv;
	Ipp32fc		*pConvFFT;
	Ipp32f		*pabsConvFFT;
	Ipp32f		*winBufConv;//相关运算结果缓存
	IppsFFTSpec_C_32fc *pConvFFTSpec;

public:
	void PreambleConv_ini(int ConvLen,int deciP,int FFTLen);
	void PreambleConv_free();
	void PreambleConv(Ipp32fc *pSrc,int nLen,Ipp32fc *preamSym,int ConvLen,int deciP,int FFTLen,
		int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect);
	void Preamble_Gen(int type,Ipp32fc *pSym,int &pLeng);
	void PSK_map(short style, short M, float I_map[],float Q_map[]); /* 星座映射方式  0——Gray，1——antiGray*/
	void PSK_MOD(short data[],short data_len,short M,Ipp32fc *out);

	void CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f &decConv,Ipp32f &cConv,Ipp32f &threod);
	void CrossConvFFT(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s ConvLeng,int nFFT,Ipp32f &cConv,int &pIndex);
	int  NumberOfBitsNeeded(short PowerOfTwo);

	
	
};
