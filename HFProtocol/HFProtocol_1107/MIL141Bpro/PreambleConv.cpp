#include "StdAfx.h"
#include "Preamble.h"
#include "PreambleConv.h"
#include <math.h>
#include "MIL141Bpro.h"

CPreambleConv::CPreambleConv(void)
{
}

CPreambleConv::~CPreambleConv(void)
{
}

/************************************************************************/
/*   abs(mean(a.*conj(b)))/sqrt(mean(a.*conj(a))*mean(b.*conj(b)));            
/************************************************************************/
void CPreambleConv::CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f &decConv,Ipp32f &cConv,Ipp32f &threod)
{

	Ipp32fc pMeanAB;
	Ipp32f pMeanA;

	Ipp32f thdelt = 0.1;// pf = 10^(-6)

	ippsConj_32fc(pSrcB,pConvAB,nLeng);
	ippsMul_32fc_I(pSrcA,pConvAB,nLeng);
	ippsMean_32fc(pConvAB,nLeng,&pMeanAB,ippAlgHintFast);

	ippsPowerSpectr_32fc(pSrcA,pConv,nLeng);
	ippsMean_32f(pConv,nLeng,&pMeanA,ippAlgHintFast);

	decConv = sqrt(pMeanAB.re*pMeanAB.re + pMeanAB.im*pMeanAB.im)/sqrt(pMeanA*bEnergy);
	cConv = (pMeanAB.re*pMeanAB.re + pMeanAB.im*pMeanAB.im);
	threod = pMeanA*thdelt;

}

/************************************************************************/
/*   xconv=((a.*conj(b)));
%     mm = abs(fft(xconv,64));
%     mm = mm./mean(mm);
%     yf(n)=max(mm);                                                          
/************************************************************************/
void CPreambleConv::CrossConvFFT(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s ConvLeng,int nFFT,Ipp32f &cConv,int &pIndex)
{

	Ipp32f pMean;
	ippsZero_32fc(pConvAB,nFFT);
	ippsConj_32fc(pSrcB,pConvAB,ConvLeng);	
	ippsMul_32fc_I(pSrcA,pConvAB,ConvLeng);
	ippsFFTFwd_CToC_32fc(pConvAB,pConvFFT,pConvFFTSpec, NULL);
	ippsMagnitude_32fc(pConvFFT, pabsConvFFT, nFFT); 

	ippsMean_32f(pabsConvFFT, nFFT, &pMean, ippAlgHintFast);
	ippsDivC_32f_I(pMean, pabsConvFFT, nFFT);
	ippsMaxIndx_32f(pabsConvFFT, nFFT, &cConv, &pIndex);
}
void CPreambleConv::PreambleConv_ini(int ConvLen,int deciP,int FFTLen)
{
	int nlen;
	nlen = (ConvLen>FFTLen)?ConvLen:FFTLen;

	pConvAB = ippsMalloc_32fc(nlen);
	pConv = ippsMalloc_32f(nlen);
	pConvFFT = ippsMalloc_32fc(nlen);
	pabsConvFFT = ippsMalloc_32f(nlen);	

	int order = NumberOfBitsNeeded(FFTLen);
	ippsFFTInitAlloc_C_32fc(&pConvFFTSpec, order, IPP_FFT_DIV_INV_BY_N,ippAlgHintFast);

}
void CPreambleConv::PreambleConv_free()
{
	ippsFree(pConvAB);
	ippsFree(pConv);
	ippsFree(pConvFFT);
	ippsFree(pabsConvFFT);
	ippsFFTFree_C_32fc(pConvFFTSpec);

}
/************************************************************************/
/* 外面做数据拼接    
/************************************************************************/
void CPreambleConv::PreambleConv(Ipp32fc *pSrc,int nLen,Ipp32fc *preamSym,int ConvLen,int deciP,int FFTLen,
								 int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect)
{
	int i;
	int convdataL = ConvLen*deciP;
	int		pPhase=0;
	int		pdownLen;
	int		fftindex;
	int		maxfftindex;
	int		index;
	Ipp32f	cconv;

	Ipp32fc *pSinCos = ippsMalloc_32fc(ConvLen);
	Ipp32fc *pdown = ippsMalloc_32fc(convdataL);
	Ipp32f	mfre,mphase=0;

	detect = FALSE;

	maxCof = 0;
	for (i=0;i<nLen;i++)
	{
		ippsSampleDown_32fc(&pSrc[i],convdataL,pdown,&pdownLen,deciP,&pPhase);// 间隔P点抽出数据			
		CrossConvFFT(pdown,preamSym,ConvLen,FFTLen,cconv,fftindex);
		if(cconv>maxCof)
		{
			maxCof = cconv;
			maxfftindex = fftindex;
			pIndex = i;
		}
		if(maxCof>20)
			break;
	}
	if(maxCof<5)
	{
		maxCof = 0;
		return;
	}
	maxCof = 0;
	frequency = (Ipp32f)maxfftindex/FFTLen;
	if((frequency>0.01 && frequency<0.5) || (frequency>0.9 && frequency<0.99))
	{
		detect = FALSE;
		return;
	}
	else
	{
		mfre = 1- frequency;

	}
	if(mfre>0.999)
	{
		mfre = 0;
	}

	Ipp32f th;
	int num=0;
	Ipp32f decConv;

	int idx;
	for(i=-2;i<2;i++)
	{
		ippsSampleDown_32fc(&pSrc[pIndex+i],convdataL,pdown,&pdownLen,deciP,&pPhase);// 间隔P点抽出数据
		mphase = 0;
		ippsTone_Direct_32fc(pSinCos, ConvLen, 1, mfre, &mphase, ippAlgHintFast);
		ippsMul_32fc_I(pSinCos,pdown,ConvLen);
		CrossConvCof(pdown,preamSym,ConvLen,1,decConv,cconv,th);
		if(cconv>th)
			num++;
		if(decConv>maxCof)
		{
			maxCof = decConv;
			idx = i;
		}
	}
	if(num>=2)
	{
		detect = TRUE;
		pIndex = pIndex + idx;
	}

	ippsFree(pSinCos);
	ippsFree(pdown);
}


int CPreambleConv::NumberOfBitsNeeded(short PowerOfTwo)
{
	int i;
	if ( PowerOfTwo < 2 ) 
	{
		return 0;
	}
	for ( i=0; ; i++ )
		if ( PowerOfTwo & (1 << i) )
			return i;
}

void CPreambleConv::Preamble_Gen(int type,Ipp32fc *pSym,int &pLeng)
{

	if(type==wMIL141BTLC)
	{
		pLeng = 240;
		PSK_MOD(TCLAGC_141B,pLeng,8,pSym);
	}
	else if(type==wMIL141BBW0)
	{
		pLeng = 384;
		PSK_MOD(preamble_BW0,pLeng,8,pSym);
	}
	else if(type==wMIL141BBW1)
	{
		pLeng = 384;
		PSK_MOD(preamble_BW1,pLeng,8,pSym);
	}
	else if(type==wMIL141BBW3)
	{
		pLeng = 384;
		PSK_MOD(preamble_BW3,pLeng,8,pSym);
	}

}
void CPreambleConv::PSK_map(short style, short M, float I_map[],float Q_map[])
{
	switch(M)
	{
	case 2:
		if(style==1)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=-1;Q_map[1]=0;
		}
		else if(style==0)
		{
			I_map[0]=-1;Q_map[0]=0;
			I_map[1]=1;Q_map[1]=0;
		}
		break;
	case 4:
		if(style==0)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=0;Q_map[1]=1;
			I_map[2]=0;Q_map[2]=-1;
			I_map[3]=-1;Q_map[3]=0;
		}
		else if(style==1)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=0;Q_map[1]=1;
			I_map[2]=-1;Q_map[2]=0;
			I_map[3]=0;Q_map[3]=-1;
		}
		break;
	case 8:
		if(style==0)
		{
			I_map[0]=1;		Q_map[0]=0;
			I_map[1]=0.707; Q_map[1]=0.707;
			I_map[2]=-0.707;Q_map[2]=0.707;
			I_map[3]=0;		Q_map[3]=1;
			I_map[4]=0.707;	Q_map[4]=-0.707;
			I_map[5]=0;		Q_map[5]=-1;
			I_map[6]=-1;	Q_map[6]=0;
			I_map[7]=-0.707;Q_map[7]=-0.707;
		}
		else if(style==1)
		{
			I_map[0]=1;		Q_map[0]=0;
			I_map[1]=0.707; Q_map[1]=0.707;
			I_map[2]=0;		Q_map[2]=1;
			I_map[3]=-0.707;Q_map[3]=0.707;
			I_map[4]=-1;	Q_map[4]=0;
			I_map[5]=-0.707;Q_map[5]=-0.707;
			I_map[6]=0;		Q_map[6]=-1;
			I_map[7]=0.707;	Q_map[7]=-0.707;
		}
		break;
	default:
		break;
	}
}
void CPreambleConv::PSK_MOD(short data[],short data_len,short M,Ipp32fc *out)
{
	int i;
	float *I_map,*Q_map;
	I_map = new float[M];
	Q_map = new float[M];

	PSK_map(1,M,I_map,Q_map);  // style = 1 顺序映射
	for (i=0; i<data_len; i++)
	{
		out[i].re = I_map[data[i]];
		out[i].im = Q_map[data[i]];
	}
	free(I_map);
	free(Q_map);
}
