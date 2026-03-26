// FSKpro.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "FSK8demode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CFSK8demode::CFSK8demode(void)
{
}

CFSK8demode::~CFSK8demode(void)
{

}


void CFSK8demode::FSKdemode_ini(float fs,float Baud,int M,float f0,float FSpace,int nLeng,int m_WinType)
{

	mfs = fs;
	mWinLeng = int(mfs/Baud);
	mBaud = Baud;
	Morder = M;
	mStep = int((float)mWinLeng/8 + 0.5);
	int FFTorder;
	GetFFT_len(mWinLeng,mFFTLeng,FFTorder);

	SetFpara(M,f0,FSpace);

	DSTFT_ini(nLeng*2,mFFTLeng,FFTorder,mWinLeng,m_WinType);
	//SaveTobit_ini();
	sameNum = 0;
	old_judge = 0;


}
void CFSK8demode::SetFpara(int M,float f0,float FSpace)
{
	MF.f0 = f0;
	MF.FSpace = FSpace/2;
	if (M==2)
	{
		MF.f1 = f0 - 1 * FSpace;
		MF.f2 = f0 + 1 * FSpace;
	}
	else if (M==4)
	{
		MF.f1 = f0 - 3 * FSpace;
		MF.f2 = f0 - 1 * FSpace;
		MF.f3 = f0 + 1 * FSpace;
		MF.f4 = f0 + 3 * FSpace;
	}
	else if (M==8)
	{
		MF.f1 = f0 - 7 * FSpace;
		MF.f2 = f0 - 5 * FSpace;
		MF.f3 = f0 - 3 * FSpace;
		MF.f4 = f0 - 1 * FSpace;
		MF.f5 = f0 + 1 * FSpace;
		MF.f6 = f0 + 3 * FSpace;
		MF.f7 = f0 + 5 * FSpace;
		MF.f8 = f0 + 7 * FSpace;
	}

	TH_F1 = MF.f1 + FSpace;
	TH_F2 = MF.f2 + FSpace;
	TH_F3 = MF.f3 + FSpace;
	TH_F4 = MF.f4 + FSpace;
	TH_F5 = MF.f5 + FSpace;
	TH_F6 = MF.f6 + FSpace;
	TH_F7 = MF.f7 + FSpace; 
}
void CFSK8demode::FSKdemode_free()
{
	DSTFT_free();
	//SaveTobit_ini();
}
void CFSK8demode::FSKdemode(Ipp32f *pSrc,int nLeng,Ipp8u *outJudge,int &outLeng)
{

	int dftLeng;
	DSTFT(pSrc,nLeng,mStep,mFFTLeng,mWinLeng,WinBuf,maxIndex,dftLeng);

	FSKJudge(maxIndex,dftLeng,mFFTLeng,mfs,Morder);
	FSKSynchronization(maxIndex,dftLeng,outJudge,outLeng);

}


void CFSK8demode::DSTFT_ini(int nLeng,int FFTLeng,int FFTorder,int WinLeng,int m_WinType)
{
	int allLen = nLeng*2 + FFTLeng*2;
	proBuf = ippsMalloc_32f(allLen);
	delayL = FFTLeng*2;
	delayBuf = ippsMalloc_32f(delayL);
	delayL = 0;

	maxIndex = ippsMalloc_32s(allLen);

	ippsFFTInitAlloc_R_32f(&spec, FFTorder, IPP_FFT_NODIV_BY_ANY, ippAlgHintAccurate );
	fft_32f_Src = ippsMalloc_32f(FFTLeng);
	fft_32f_Temp = ippsMalloc_32f(FFTLeng*2);
	fft_32f_Dst = ippsMalloc_32f(FFTLeng*2);

	WinBuf = ippsMalloc_32f(WinLeng);
	ippsSet_32f(1,WinBuf,WinLeng);
	switch (m_WinType)
	{
	case 0:
		break;
	case 1:
		ippsWinHamming_32f_I(WinBuf,WinLeng);//汉明窗
		break;
	case 2:
		ippsWinBartlett_32f_I(WinBuf,WinLeng);
		break;
	case 3:
		ippsWinBlackman_32f_I(WinBuf,WinLeng,-0.25);
		break;
	case 4:
		ippsWinBlackmanStd_32f_I(WinBuf,WinLeng);
		break;
	case 5:
		ippsWinBlackmanOpt_32f_I(WinBuf,WinLeng);
		break;
	case 6:
		ippsWinHann_32f_I(WinBuf,WinLeng);
		break;
	case 7:
		ippsWinKaiser_32f_I(WinBuf,WinLeng,1.0f);
		break;
	default:
		break;
	}
	
}
void CFSK8demode::DSTFT_free()
{
	ippsFree(proBuf);
	ippsFree(delayBuf);
	ippsFFTFree_R_32f(spec);
	ippsFree(fft_32f_Src);
	ippsFree(fft_32f_Temp);
	ippsFree(fft_32f_Dst);
	ippsFree(WinBuf);
	ippsFree(maxIndex);
}
/************************************************************************
   nLeng      当前送入的数据长度
/************************************************************************/
void CFSK8demode::DSTFT(Ipp32f *pSrc,int nLeng,int Step,int FFTLeng,int WinLeng,Ipp32f *WinBuf,int *maxIndex,int &outLeng)
{
	int i;
	Ipp32s max_temp=0;
	Ipp32f pMax;
	

	ippsCopy_32f(delayBuf,proBuf,delayL);
	ippsCopy_32f(pSrc,&proBuf[delayL],nLeng);
	int allLeng = delayL + nLeng;
	
	outLeng = 0;
	for (i=0;i<allLeng-WinLeng;i=i+Step)
	{
		ippsZero_32f(fft_32f_Src,FFTLeng);
		ippsCopy_32f(&proBuf[i],fft_32f_Src,WinLeng);
		ippsMul_32f_I(WinBuf,fft_32f_Src,WinLeng);

		ippsFFTFwd_RToCCS_32f( fft_32f_Src, fft_32f_Temp, spec, NULL );
		ippsMagnitude_32fc( (Ipp32fc*)fft_32f_Temp, fft_32f_Dst, FFTLeng/2);

		//pMax = 0;
		//for(k=fstart;k<=fend;k++)
		//{
		//	if(fft_32f_Dst[k]>pMax)
		//	{
		//		pMax = fft_32f_Dst[k];
		//		maxIndex[outLeng] = k;
		//	}
		//}
		ippsMaxIndx_32f(fft_32f_Dst,FFTLeng/2,&pMax,&maxIndex[outLeng]);
		outLeng++;	
	}
	delayL = allLeng - outLeng*Step;
	ippsCopy_32f(&proBuf[allLeng-delayL],delayBuf,delayL);

}
void CFSK8demode::FSKJudge(int *pSrcDst,int nLeng,int FFTLeng,float insample,int M)
{
	int i;
	float f;
	switch(M)
	{
	case 2:
		for (i=0;i<nLeng;i++)
		{
			f = pSrcDst[i]*insample/FFTLeng;
			if(f < TH_F1)
				pSrcDst[i] = 0;
			else 
				pSrcDst[i] = 1; 	
		}
		break;
	case 4:
		for (i=0;i<nLeng;i++)
		{
			f = pSrcDst[i]*insample/FFTLeng;
			if(f<TH_F1)
				pSrcDst[i] = 2;
			else if(f>=TH_F1 && f<TH_F2)
				pSrcDst[i] = 3;
			else if(f>=TH_F2 && f<TH_F3)
				pSrcDst[i] = 1;
			else if(f>=TH_F3 )
				pSrcDst[i] = 0;
		}
		break;
	case 8:
		for (i=0;i<nLeng;i++)
		{
			f = pSrcDst[i]*insample/FFTLeng;
			if(f < TH_F1)
				pSrcDst[i] = 0;
			else if(f >=TH_F1 && f < TH_F2)
				pSrcDst[i] = 1;
			else if(f >= TH_F2 && f < TH_F3)
				pSrcDst[i] = 3;
			else if(f >= TH_F3 && f < TH_F4)
				pSrcDst[i] = 2;
			else if(f >= TH_F4 && f < TH_F5)
				pSrcDst[i] = 6;
			else if(f >= TH_F5 && f < TH_F6)
				pSrcDst[i] = 7;
			else if(f >= TH_F6 && f < TH_F7)
				pSrcDst[i] = 5;
			else if(f >= TH_F7 )
				pSrcDst[i] = 4;	
			//TRACE("%d_\n",pSrcDst[i]);
		}
		break;
	}
}
void CFSK8demode::FSKSynchronization(int *pSrc,int nLeng,Ipp8u *pDst,int &outLeng)
{
	int i,n;
	outLeng = 0;
	for (i=0;i<nLeng;i++)
	{
		if (sameNum>0)
		{
			if (pSrc[i] == old_judge)
				sameNum++;
			if((pSrc[i]!= old_judge) || (sameNum>80))
			{
				n =short(float(sameNum)/8 + 0.5);
				if(n>0)
				{
					ippsSet_8u(old_judge,&pDst[outLeng],n);
					outLeng = outLeng + n;
					sameNum = 1;
				}
			}
		}
		else
			sameNum = sameNum + 1;
		old_judge = pSrc[i];
		
	}
	/*for (i=0;i<outLeng;i++)
	{
		int nn = short(outLeng/48);
		for (int j=0;j<nn;j++)
		{
			if (i == 23+49*j)
				TRACE("\n");
		}
		
	   TRACE("%d_",pDst[i]);
	}*/

}
/************************************************************************
  根据长度求出最接近的fft点数                                         
/************************************************************************/
void CFSK8demode::GetFFT_len(int dataLen,int &FFTLeng,int &FFTorder)
{
	int FFTm=0;
	int i,j,m,n=0;
	for (i=0,j=1;i<16;i++,j=j*2)
	{		
		m=dataLen&j;
		if(m==j)
		{
			n++;
			FFTm=i;
		}
	}
	if(n!=1)
		FFTm=FFTm+1;
	FFTorder = FFTm;
	FFTLeng= 1<<FFTm;	
}