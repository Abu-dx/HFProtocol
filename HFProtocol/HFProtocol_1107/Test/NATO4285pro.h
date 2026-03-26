#pragma once

#if defined(_AFXDLL) || defined(_AFXEXT)
#include <afxwin.h>         // MFC 核心组件和标准组件
#endif
#include "ipps.h"

class NATODecoder;
class CPreambledetect;
class RLS;

struct HeadType4285{
	int position;
	float fre;
	int dataRate;
	CString interType;
};
class AFX_EXT_CLASS CNATO4285pro
{
public:
	CNATO4285pro(void);
	~CNATO4285pro(void);

public:
	CPreambledetect *m_Preambledetect;
	RLS *m_RLS;
	// 变频，匹配滤波
	Ipp32f pfre;
	Ipp64f *pSrcTap;
	int SrcLen;
	IppsFIRState64f_32f *SrcStateReal,*SrcStateImag;
	Ipp32f *pDelayreal,*pDelayimag;
	Ipp32f *pReal,*pImag;

	// 前导符号

	Ipp16s P;
	Ipp32fc *pPreambleSym;
	Ipp32fc *pProbleSym;
	Ipp16s *probe;
	Ipp32f UWEnergy;
	Ipp16s UWleng;


	//  突发检测
	Ipp32fc *proBuf;	//数据块缓存
	int     BufPos;
	int     proBegin;
	Ipp32f  *winBufConv;//相关运算结果缓存
	Ipp32fc *delayBuf;
	int	    delayL;
	Ipp32fc *pConvAB;
	Ipp32f  *pConv;


	Ipp32fc *OneFrame;	
	Ipp16s  OneFrameLen;//  一段帧长
	Ipp32f pEnergy;
	Ipp16s  probeNum;

	int pBuf,pBurst;
	int flag;
	int contin;

	// 定时同步
	float xi0,xi1,xi2,xq0,xq1,xq2;
	Ipp32f muk;
	Ipp32f mtau;
	Ipp32f mfre;
	Ipp32f mphase;	


	// pll
	Ipp32f defrephase;
	Ipp32f frephase;
	Ipp32f pllphase;
	Ipp32f y_f,y_p,eold,mod;
	Ipp32f kp_p,ki_p,ko_p,kd_p,kesai_p,wn_p;

	//  帧计数
	int FrameIdx;
	unsigned char byte_flag, data_byte, bit_num, symbol;

	int dataRate;
	int FECType;
	int InterType;
	int increasment;
	int flushLength;
	NATODecoder* pDecoder;
	Ipp8u *pBufInter;
	Ipp8u *pBufCoded;
	int InterLength;

public:

	void DownFre_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,int FFTLen,Ipp32f roll,Ipp32f Baud,Ipp16s P,int SrctapLen);
	void DownFre_free();
	void DownFre(Ipp16s *pSrc,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,Ipp32fc *pDst,int &DstLen);

	void SRcos_Filter(int N, float RollOff, float fBaudRate, float fSampleRate, float *fMF_Coef);
	float do_fir(float *base, int cur, float *coeff, int len);
	void PSK_map(short style, short M, float I_map[],float Q_map[]); /* 星座映射方式  0——Gray，1——antiGray*/
	int  PSK_MOD(short data[],short data_len,short M,Ipp32fc *out);/* 输入数据向量 *//* 输入输入长度 *//* 调制阶数 2，4，8*/	/* 输出数据 */		
	void Preamble_Gen();

	void CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f *cConv);
	void Burst_detect_ini(int allLen,int convLen,int deciP,int mproBegin);
	void Burst_detect(int proBegin,int proLen,Ipp32fc *UWsig,Ipp32f UWEnergy, Ipp16s convLen,Ipp16s deciP,
		Ipp32f Threshold,Ipp16s *BurstPos,int &BurstNum,int &BufPos,int &BufHavePro);
	void Burst_detect_free();

	void Demode_PSKrealFSE_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,Ipp16s P,int FFTLen,Ipp32f roll,Ipp32f Baud,int SrctapLen,
		int mdataRate,int mFECType,int mInterType);
	void Demode_PSKrealFSE_free();
	void Demode_PSKrealFSE(Ipp16s *pSrcDst,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,
		int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType4285 *headType,int &headNum);

	void Synchronization_PSK(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp16s P,Ipp32fc *pUWsig,Ipp16s UWleng,int delayCma,
		Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng,int bBegin);
	void EqualizationRLS_DFE_PLL_ini(Ipp32f wn, Ipp32f kesai,Ipp16s M_phase,Ipp32f pll_phase,
		int OrderA,int OrderB,Ipp32f mdelta,Ipp32f mlamde);
	void EqualizationRLS_DFE_PLL_free();
	void EqualizationRLS_DFE_PLL(Ipp32fc *pSrc,int nLeng,int P,int delay,Ipp32f mEnergy,Ipp32fc *UWsig,Ipp32fc *pDst,int &outLen,BOOL aid);

	void ParaEstimate(Ipp32fc *pSrc,int nLeng,int P,Ipp32fc *pUWsig,Ipp32f &fre,Ipp32f &phase,Ipp32f &energy);
	void Timing_ini();
	void Timing_estimate(Ipp32fc *pUWsig,Ipp16s UWleng,Ipp32fc *pSrc,Ipp16s P, Ipp32f &tau);
	void Timing_recover(Ipp32fc *pSrc,int nleng,Ipp32f tau,Ipp16s P,Ipp32fc *pDst,int &outleng);
	float sym_interp_i(float xi3, float u,bool ini);
	float sym_interp_q(float xq3, float u,bool ini);
	void Fre_Estimate_LR(Ipp32fc *pUWsig,Ipp32fc *pSrc,Ipp16s nleng,Ipp32f &fre);
	void Remove_fre(Ipp32fc *pSrc,Ipp32s nLeng, Ipp32f rFreq);
	void Phase_VV(Ipp32fc *pSrc,Ipp32fc *pUWsig,Ipp16s len,Ipp32f &phase);


	void DecPSK(Ipp32fc pSrc,Ipp32fc &pDst);
	void DecToBit(Ipp8u pSrc,int M,Ipp8u *pDst,int &outLeng);
	void SaveTobit_ini();
	void SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,Ipp8s modutype);
	void judge(Ipp32fc *pSrc,int nLeng,Ipp8u *pDst,Ipp8s modutype);

	void MapFor4285(Ipp8u *pSrc,int nLeng,Ipp8u *pDst,int &outLeng,Ipp8s modutype);
	void DeHead4285(Ipp16s *BurstPos,int BurstNum,HeadType4285 *headType,int &headNum);

};
