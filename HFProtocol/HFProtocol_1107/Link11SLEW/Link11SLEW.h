#pragma once

#include "ipps.h"

class Link11Decoder;
class JCRC;
class RLS;
class CPreambledetect;
struct HeadType{
	float frequency;
	int position;
	byte Type;     // 00 主站询问  10 主站报告  11 前哨回复  01 未知错误
	byte address;
	short crcerr;  // 误码
};
class  AFX_EXT_CLASS CLink11SLEW
{
public:
	CLink11SLEW(void);
	~CLink11SLEW(void);

public:

	CPreambledetect *m_Preambledetect;
	RLS *m_RLS;

	Ipp32f m_Outsample;
	Ipp32fc *probe_sym;
	// 变频，匹配滤波
	Ipp32f pfre;
	Ipp64f *pSrcTap;
	int SrcLen;
	IppsFIRState64f_32f *SrcStateReal,*SrcStateImag;
	Ipp32f *pDelayreal,*pDelayimag;
	Ipp32f *pReal,*pImag;

	//  突发检测
	Ipp32fc *proBuf;	//数据块缓存
	int     BufPos;
	int     proBegin;
	Ipp32f  *winBufConv;//相关运算结果缓存
	Ipp32fc *delayBuf;
	int	    delayL;
	Ipp32fc *pConvAB;
	Ipp32f  *pConv;
	Ipp32f pEnergy;

	Ipp16s *probe;
	Ipp32fc *OneFrame;	
	Ipp16s  OneFrameLen;//  一段帧长
	Ipp16s  probeNum;
	int pidx,pBurst;
	int flag;

	// 定时同步
	float xi0,xi1,xi2,xq0,xq1,xq2;
	Ipp32f muk;
	Ipp32f mtau;

	// pll
	Ipp32f mfre;
	Ipp32f mphase;
	Ipp32f defrephase;
	Ipp32f frephase;
	Ipp32f pllphase;
	Ipp32f y_f,y_p,eold,mod;
	Ipp32f kp_p,ki_p,ko_p,kd_p,kesai_p,wn_p;
	

	unsigned char byte_flag, data_byte, bit_num, symbol;

	Link11Decoder *pDecoder;
	JCRC *pCRC;
	//  帧计数
	int FrameIdx;
	int lastframe;
	
public:

	void PSK_map(short style, short M, float I_map[],float Q_map[]); /* 星座映射方式  0——Gray，1——antiGray*/
	int  PSK_MOD(short data[],short data_len,short M,Ipp32fc *out);/* 输入数据向量 *//* 输入输入长度 *//* 调制阶数 2，4，8*/	/* 输出数据 */		

	void CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f *cConv);
	void Burst_detect_ini(int allLen,int convLen,int deciP,int mproBegin);
	void Burst_detect(int proBegin,int proLen,Ipp32fc *UWsig,Ipp32f UWEnergy, Ipp16s convLen,Ipp16s deciP,
		Ipp32f Threshold,Ipp16s *BurstPos,int &BurstNum,int &BufPos,int &BufHavePro);
	void Burst_detect_free();

	void SRcos_Filter(int N, float RollOff, float fBaudRate, float fSampleRate, float *fMF_Coef);
	void DownFre_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,int FFTLen,Ipp32f roll,Ipp32f Baud,Ipp16s P,int SrctapLen);
	void DownFre_free();
	void DownFre(Ipp16s *pSrc,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,Ipp32fc *pDst,int &DstLen);

	void Demode_PSKrealFSE_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,Ipp16s P,int FFTLen,Ipp32f roll,Ipp32f Baud,int SrctapLen);
	void Demode_PSKrealFSE_free();
	void Demode_PSKrealFSE(Ipp16s *pSrcDst,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,
		int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType *headType,int &headNum);
	void Synchronization_RLSCMA_forSLEW(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp16s P,Ipp32fc *pUWsig,Ipp16s UWleng,int delayCma,
		Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng,HeadType *headType,int &headNum,bool bBegin);

	void ParaEstimate(Ipp32fc *pSrc,int nLeng,int P,Ipp32fc *pUWsig,Ipp32f &fre,Ipp32f &phase,Ipp32f &energy);

	void Timing_ini();
	void Timing_estimate(Ipp32fc *pUWsig,Ipp16s UWleng,Ipp32fc *pSrc,Ipp16s P, Ipp32f &tau);
	void Timing_recover(Ipp32fc *pSrc,int nleng,Ipp32f tau,Ipp16s P,Ipp32fc *pDst,int &outleng);
	float sym_interp_i(float xi3, float u,bool ini);
	float sym_interp_q(float xq3, float u,bool ini);

	void Fre_Estimate_LR(Ipp32fc *pUWsig,Ipp32fc *pSrc,Ipp16s nleng,Ipp32f &fre);
	void Remove_fre(Ipp32fc *pSrc,Ipp32s nLeng, Ipp32f rFreq);
	void Phase_VV(Ipp32fc *pSrc,Ipp32fc *pUWsig,Ipp16s len,Ipp32f &phase);
	void dfpll_phase(Ipp32fc *pSrc,Ipp32s nLeng,Ipp32f pll_phase);
	void dfpll_phase_ini(Ipp32f wn, Ipp32f kesai,Ipp16s M_phase);

	void EqualizationRLS_DFE_PLL_ini(Ipp32f wn, Ipp32f kesai,Ipp16s M_phase,Ipp32f pll_phase,
		int OrderA,int OrderB,Ipp32f mdelta,Ipp32f mlamde);
	void EqualizationRLS_DFE_PLL_free();
	void EqualizationRLS_DFE_PLL(Ipp32fc *pSrcDst,int nLeng,int P,int delay,Ipp32f mEnergy,Ipp32fc *UWsig,Ipp32fc *pDst,int &outLen,BOOL aid);

	void SaveTobit_ini();
	void SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,Ipp8s modutype);
	void judge(Ipp32fc *pSrc,Ipp16s nLeng,Ipp8u *pDst,Ipp8s modutype);
	void judge_ini();

	void DeBitPair_PhaseDecode(Ipp8u *pSrc,int nLeng,Ipp8u *pDst,int &outLeng);
	void deinterleaver(Ipp8u *pSrc,Ipp8u *pDst);
	void DecToBit(Ipp8u pSrc,int M,Ipp8u *pDst,int &outLeng);
	void HeadDecode(Ipp8u *pSrc,int crcerr,HeadType &mheadType);
	int FindEnd(Ipp8u *pSrc,int nLeng);
};
