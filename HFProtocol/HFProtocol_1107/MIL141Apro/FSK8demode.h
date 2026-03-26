#pragma once
#include "ipps.h"

typedef struct{
	float f0;      
	float f1;   
	float f2;      
	float f3; 
	float f4;      
	float f5;   
	float f6;      
	float f7; 
	float f8; 
	int Mod;   
	float SRate;            //Data Rate
	float FSpace;
	int Fmaxd;
	int syn;
	int Re;

}  fsk_stack;
class  CFSK8demode
{
public:
	CFSK8demode(void);
	~CFSK8demode(void);

private:

	Ipp32f *proBuf;	//Êý¾Ý¿é»º´æ
	Ipp32f *delayBuf;
	int	    delayL;

	int *maxIndex;
	Ipp32f *WinBuf;
	IppsFFTSpec_R_32f* spec;
	Ipp32f *fft_32f_Src,*fft_32f_Dst,*fft_32f_Temp;

	fsk_stack MF;
	float mfs;
	int mFFTLeng;
	int mStep;
	float mBaud;
	int mWinLeng;
	int Morder;
	float TH_F1,TH_F2,TH_F3,TH_F4,TH_F5,TH_F6,TH_F7;
	Ipp8u old_judge;
	int sameNum;

	unsigned char byte_flag, data_byte, bit_num, symbol;


public:

	void FSKdemode(Ipp32f *pSrc,int nLeng,Ipp8u *outJudge,int &outLeng);
	void FSKdemode_ini(float fs,float Baud,int M,float f0,float FSpace,int nLeng,int m_WinType);
	void FSKdemode_free();
	void SetFpara(int M,float f0,float FSpace);

private:

	void DSTFT_ini(int nLeng,int FFTLeng,int FFTorder,int WinLeng,int m_WinType);
	void DSTFT_free();
	void DSTFT(Ipp32f *pSrc,int nLeng,int Step,int FFTLeng,int WinLeng,Ipp32f *WinBuf,int *maxIndex,int &outLeng);
	void FSKJudge(int *pSrcDst,int nLeng,int FFTLeng,float insample,int M);
	void FSKSynchronization(int *pSrc,int nLeng,Ipp8u *pDst,int &outLeng);
	
	void GetFFT_len(int dataLen,int &FFTLeng,int &FFTorder);

};