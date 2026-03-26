#pragma once
#include "ipps.h"

struct burstpos
{
	DWORD   m_begin;
	DWORD	m_end;
	DWORD	m_address;
	DWORD	m_type;
	float   m_fduopule;
	short   m_complete;
} ;
class  CLink11CLEW
{
public:
	CLink11CLEW(void);
	~CLink11CLEW(void);
public:
	// 下变频 重采样
	Ipp32s m_nLeng;	
	Ipp32f m_rFreq;
	Ipp32f *pSin,*pCos;
	Ipp64f *taps;
	Ipp64f m_fstop;
	IppsFIRState64f_32f *ppStateR, *ppStateI;
	Ipp32f *pDlyLineR, *pDlyLineI,*xr, *xi;
	Ipp32f m_Phase,m_Phase2;

	Ipp32f **ReSample_taps;
	Ipp64f m_Decimate,faddress;
	Ipp16s m_incimate;
	Ipp32s history,m_taps_onech,m_tapsLen;
	Ipp64s address;
	IppWinType m_Wintype;
	//para OFDM参数
	float OFDM_fs;
	float OFDM_fc;
	Ipp16s OFDM_symL,OFDM_fft,OFDM_cpxL;
	Ipp16s OFDM_fftbit;

	Ipp16s *OFDM_useIdx;
	Ipp32f *OFDM_usef;
	Ipp16s OFDM_useNum;

	Ipp32fc *bufOFDMFFT; 
	IppsFFTSpec_C_32fc* pOFDMFFTSpec;

	// detect
	Ipp32fc *bufDetectFFT; 
	IppsFFTSpec_C_32fc* pDetectFFTSpec;
	Ipp16s detect_fft;
	Ipp16s idx_dopule,idx_16,idx_data1,idx_data14,delt;

	// 多普勒估计
	Ipp16s duopulefft;
	Ipp32fc *bufduopuleFFT; 
	IppsFFTSpec_C_32fc* pduopuleFFTSpec;
	Ipp32f fduopule1,fduopule2;
	Ipp32f frephase,frephase2;

	//  流程控制
	Ipp32fc *signalbuf; // 处理缓存
	Ipp16s signalp;     // 处理指针
	Ipp16s pilotp;
	Ipp16s flag;
	Ipp16s pData;
	Ipp32f energy;
	// 定时同步
	Ipp16s timeidx;
	Ipp64f *timefir;
	Ipp32f *phaseref;
	// FIR
	Ipp16s pfirlen;
	float *sambufI,*sambufQ;
	short pidx;

	//
	DWORD beginpos,endpos,countpos;
	burstpos *m_burstpos; 
	short m_burstnum;

	unsigned char byte_flag, data_byte, bit_num, symbol;

public:
	void ReSample_ini(Ipp32f rFreqHz,Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,int incimate,int tapsLen);
	int  ReSample(Ipp16s *pSrc,Ipp32fc *pDst);
	void ReSample_Free();
	void UpdateFc(Ipp32f rFreqHz,Ipp32s nLeng,Ipp32f insample);

	void Link11CLEWdemode_ini(Ipp32f rFreqHz,Ipp32s nLeng,Ipp32f Insample,Ipp32f Outsample,int tapsLen);
	void Link11CLEWdemode_free();
	void Link11CLEWdemode(Ipp16s *pSrc,int pSrcLen,Ipp32f TH1,Ipp8u *mByte,int &bytelen,Ipp32fc *expsym,int &expsymlen,int &burstnum,bool &detect);
	void Link11CLEWdetect(Ipp16s *pSrc,int pSrcLen,Ipp32fc *detectout,int &detectoutlen);

	void fftdetect(Ipp32fc *pSrc,Ipp16s symL,Ipp32f *fdopule,Ipp32f *decsum);
	void duopule_estimate(Ipp32fc *pSrc,Ipp16s pSrcLen,Ipp32f f1,Ipp32f *fdopule);
	void Remove_OFDM_fre(Ipp32fc *Buf_32fc,Ipp16s Buf_len,Ipp32f frequency);
	void Timing_estimate(Ipp32fc *pSrc,Ipp16s *idx);
	float do_fir(float *base, int cur, double *coeff, int len);

	void FFT_OFDM(Ipp32fc *Buf_32fc,Ipp32fc *OFDM_fftsym,Ipp32f *energy);
	void Dec_demode(Ipp32fc *OFDM_fftsym,Ipp32s *bitout,Ipp16s pos,Ipp32fc *expsym);
	void OFDM_phase_VV(Ipp32f *OFDM_phase,Ipp32f *OFDM_ref,Ipp32f *decphase,Ipp16s nLeng,Ipp16s M);

	void Decode(Ipp32s *codein,Ipp16s *err);
	bool Islike(Ipp32s *codeinA,Ipp32s *codeinB);
	void DeAddress(Ipp32s *codein,Ipp16s codeinnum,int *address);

	void SaveToBit_ini();
	void SaveToBit(Ipp32s *code,short codenum,Ipp8u *mByte,int &Byteleng);

	void timingPLL(Ipp32fc *pFFT,int fidxLen,int WinLen,int M,Ipp32f &phase);
	void Remove_timephase(Ipp32fc *pSrcDst,Ipp32f tphase,int *fidx,int fidxLen);

};
