// ProtolDetect.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "Preambledetect.h"
#include <math.h>
#include "Preamble.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CPreambledetect::CPreambledetect(void)
{
	pPreambleSym = NULL;
}
CPreambledetect::~CPreambledetect(void)
{
	if(pPreambleSym!=NULL)
	{
		ippsFree(pPreambleSym);
		pPreambleSym = NULL;
	}
}
void CPreambledetect::PSK_map(short style, short M, float I_map[],float Q_map[])
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
void CPreambledetect::PSK_MOD(short data[],short data_len,short M,Ipp32fc *out)
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

int CPreambledetect::NumberOfBitsNeeded(short PowerOfTwo)
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

/************************************************************************/
/*   abs(mean(a.*conj(b)))/sqrt(mean(a.*conj(a))*mean(b.*conj(b)));            
/************************************************************************/

void CPreambledetect::CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f &cConv)
{

	Ipp32fc pMeanAB;
	Ipp32f pMeanA;

	ippsConj_32fc(pSrcB,pConvAB,nLeng);
	ippsMul_32fc_I(pSrcA,pConvAB,nLeng);
	ippsMean_32fc(pConvAB,nLeng,&pMeanAB,ippAlgHintFast);

	ippsPowerSpectr_32fc(pSrcA,pConv,nLeng);
	ippsMean_32f(pConv,nLeng,&pMeanA,ippAlgHintFast);

	cConv = sqrt(pMeanAB.re*pMeanAB.re + pMeanAB.im*pMeanAB.im)/sqrt(pMeanA*bEnergy);

}
/************************************************************************/
/*   xconv=((a.*conj(b)));
%     mm = abs(fft(xconv,64));
%     mm = mm./mean(mm);
%     yf(n)=max(mm);                                                          
/************************************************************************/
void CPreambledetect::CrossConvFFT(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s ConvLeng,int nFFT,Ipp32f &cConv,int &pIndex)
{

	Ipp32f pMean;
	ippsZero_32fc(pConvAB,nFFT);
	ippsConj_32fc(pSrcB,pConvAB,ConvLeng);	
	ippsMul_32fc_I(pSrcA,pConvAB,ConvLeng);
//	ippsMul_32fc(pSrcA,pSrcB,pConvAB,ConvLeng);
	ippsFFTFwd_CToC_32fc(pConvAB,pConvFFT,pConvFFTSpec, NULL);
	ippsMagnitude_32fc(pConvFFT, pabsConvFFT, nFFT); 

	ippsMean_32f(pabsConvFFT, nFFT, &pMean, ippAlgHintFast);
	ippsDivC_32f_I(pMean, pabsConvFFT, nFFT);
	ippsMaxIndx_32f(pabsConvFFT, nFFT, &cConv, &pIndex);

}
void CPreambledetect::Burst_detect_ini(int bufLen,int ConvLen,int deciP,int FFTLen)
{
	delayL = ConvLen*deciP*2;
	int len = bufLen+delayL;

	pBuf = ippsMalloc_32f(len);
	winBufConv = ippsMalloc_32f(len);
	ConvIndex = ippsMalloc_16s(len);
		
	delayBuf = ippsMalloc_32f(delayL);
	delayL = 0;
	BufPos = 0;

	if(ConvLen>FFTLen)
	{
		pConvAB = ippsMalloc_32fc(ConvLen);
		pConv = ippsMalloc_32f(ConvLen);
		pConvFFT = ippsMalloc_32fc(ConvLen);
		pabsConvFFT = ippsMalloc_32f(ConvLen);
		
	}
	else
	{
		pConvAB = ippsMalloc_32fc(FFTLen);
		pConv = ippsMalloc_32f(FFTLen);
		pConvFFT = ippsMalloc_32fc(FFTLen);
		pabsConvFFT = ippsMalloc_32f(FFTLen);
	}
	
	int order = NumberOfBitsNeeded(FFTLen);
	ippsFFTInitAlloc_C_32fc(&pConvFFTSpec, order, IPP_FFT_DIV_INV_BY_N,ippAlgHintFast);

}
void CPreambledetect::Burst_detect_free()
{
	ippsFree(pBuf);
	ippsFree(delayBuf);
	ippsFree(winBufConv);
	ippsFree(ConvIndex);

	ippsFree(pConvAB);
	ippsFree(pConv);
	ippsFree(pConvFFT);
	ippsFree(pabsConvFFT);
	ippsFFTFree_C_32fc(pConvFFTSpec);
	
}
/***********************************************************************
bufLen        proBuf中的数据总长度，包括上一数据块结尾和本次数据块
/************************************************************************/
void CPreambledetect::Burst_detect(int bufLen,Ipp32fc *UWsig,Ipp32f UWEnergy, int convLen,int deciP,int nFFT,
								Ipp32f TH1,Ipp32f TH2,Ipp16s *BurstPos,Ipp32f *fre,int &BurstNum,int &BufPos,int &BufHavePro)
{
	Ipp16s i;
	int pPhase=0;
	int pDstLen=0;
	int convdataL = convLen*deciP;
	int compareL = 2*deciP;
	int winBufp = 0;
	Ipp32f pMax;
	int pIndex;
	Ipp32f cconv;
	Ipp32fc *ptemp = ippsMalloc_32fc(bufLen);
	Ipp32fc *pdown = ippsMalloc_32fc(convdataL);
	Ipp32f mphase=0;

	Ipp32fc *pSinCos = ippsMalloc_32fc(convLen);
	Ipp32f mfre;
	
	BurstNum=0;	
	ippsCopy_32f(delayBuf,pBuf,delayL);

	ippsHilbertInitAlloc_32f32fc(&pSpec, bufLen, ippAlgHintNone); ///  指定变换长度后不能变化
	ippsHilbert_32f32fc(pBuf,ptemp,pSpec);

	for (i=0;i<bufLen - convdataL;i++)
	{
		ippsSampleDown_32fc(&ptemp[i],convdataL,pdown,&pDstLen,deciP,&pPhase);// 间隔P点抽出数据			
		CrossConvFFT(pdown,UWsig,convLen,nFFT,cconv,pIndex);
		winBufConv[winBufp] = cconv;
		ConvIndex[winBufp] = pIndex;
		winBufp++;
	}
	i=0;
	while (i<winBufp-compareL)
	{
		/*ippsMaxIndx_32f(&winBufConv[i],compareL,&pMax,&pIndex);
		if(pMax>TH1){
			if(((winBufConv[i+pIndex-1]+winBufConv[i+pIndex]+winBufConv[i+pIndex+1])/3)>TH1){
				BurstPos[BurstNum] = i+pIndex;
				fre[BurstNum] = (Ipp32f)ConvIndex[i+pIndex]/nFFT;
				BurstNum++;
				i = i+compareL;
				continue;
			}
		}*/
		if (winBufConv[i]>TH1 && winBufConv[i+1]>TH1)
		{
			ippsMaxIndx_32f(&winBufConv[i],compareL,&pMax,&pIndex);

			ippsSampleDown_32fc(&ptemp[i+pIndex],convdataL,pdown,&pDstLen,deciP,&pPhase);// 间隔P点抽出数据
			if(ConvIndex[i+pIndex]>0)
				mfre = 1 - (Ipp32f)ConvIndex[i+pIndex]/nFFT;
			else
				mfre = 0;
			mphase = 0;
			ippsTone_Direct_32fc(pSinCos, convLen, 1, mfre, &mphase, ippAlgHintFast);
			ippsMul_32fc_I(pSinCos,pdown,convLen);
			CrossConvCof(pdown,UWsig,convLen,1,cconv);
			if(cconv>TH2)
			{
				BurstPos[BurstNum] = i+pIndex;
				fre[BurstNum] = (Ipp32f)ConvIndex[i+pIndex]/nFFT;
				if(fre[BurstNum]>0.4 && fre[BurstNum]<1)
				{
					BurstNum++;
					i = i+compareL;
					continue;
				}
			}

		}
		i++;
	}
	delayL = convdataL + compareL;
	ippsCopy_32f(&pBuf[bufLen-delayL],delayBuf,delayL);// 结尾数据复制，以便下次处理
	BufHavePro = winBufp-compareL;
	BufPos = delayL;
	ippsFree(ptemp);
	ippsFree(pdown);
	ippsHilbertFree_32f32fc(pSpec);
	ippsFree(pSinCos);
}


//  包含下变频，匹配滤波，重采样
//原始滤波器阶数tapsLen		多相分解后滤波阶数taps_onech   
void CPreambledetect::ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen)
{

	m_Insample = insample;
	m_Outsample = outsample;

	m_incimate = incimate;
	m_taps_onech = tapsLen/incimate;  //  整除
	m_Decimate =Ipp64f(insample*incimate)/outsample;
	mSrctaplen = tapsLen;

	ResampleBuf = ippsMalloc_32f(nLeng+mSrctaplen);
	pDelay = ippsMalloc_32f(mSrctaplen);
	delayLen = 0;
	
	if(m_incimate>m_Decimate)
		m_fstop=1/(Ipp64f)(m_incimate*2);
	else
		m_fstop = 1/(Ipp64f)(m_Decimate*2);

	pTaps_64f = ippsMalloc_64f(tapsLen);
	ippsFIRGenLowpass_64f(m_fstop, pTaps_64f, tapsLen, ippWinHamming, ippTrue);	//	窗化法生成低通滤波器

	ReSample_taps=new float*[m_incimate];
	int i, j;
	for(j=0; j<m_incimate; j++)
	{
		ReSample_taps[j]=new float[m_taps_onech];
		for(i=0; i<m_taps_onech; i++)
		{
			ReSample_taps[j][i]=(Ipp32f)pTaps_64f[i*m_incimate+j];
		}
	}
	ippsFree(pTaps_64f);
	history=0;
	faddress=0;
	address=0;

}
void  CPreambledetect::ReSample(Ipp16s *pSrc,int nLeng,Ipp32f *pDst,int &outLeng)
{
	int i,m=0;
	Ipp32f temp;

	if(m_Insample == m_Outsample)
	{
		ippsConvert_16s32f(pSrc,pDst,nLeng);
		outLeng = nLeng;
		return;
	}

	ippsCopy_32f(pDelay,ResampleBuf,delayLen);
	ippsConvert_16s32f(pSrc,&ResampleBuf[delayLen],nLeng);
	
	int len = nLeng+delayLen;

	for (i=0;i<len-m_taps_onech;i++)
	{	
		while(address>=0)
		{	
			ippsDotProd_32f(ReSample_taps[address],&ResampleBuf[i],m_taps_onech,&temp);
			pDst[m] = temp*2;
			faddress=faddress-m_Decimate;
			address=int(faddress+0.5);
			m++;
		}
		faddress=faddress+m_incimate;
		address=int(faddress+0.5);
	}

	delayLen=len-i;
	ippsCopy_32f(&ResampleBuf[i],pDelay,delayLen);
	outLeng = m;
}
void CPreambledetect::ReSample_Free()
{
	ippsFree(ResampleBuf);
	ippsFree(pDelay);
	for (int i=0;i<m_incimate;i++)
	{
		delete ReSample_taps[i];
	}
	delete ReSample_taps;
}

void CPreambledetect::ProtolDetect_ini(int nLeng,int maxConvLen,int FFTLen,Ipp32f insample,Ipp32f outsample,int deciP)
{
	ReSample_ini(nLeng,insample,outsample,32,8192);
	int BufLen = nLeng*2;
	Burst_detect_ini(BufLen,maxConvLen,deciP,FFTLen);
	
}
void CPreambledetect::ProtolDetect_free()
{
	ReSample_Free();
	Burst_detect_free();
}

void CPreambledetect::Preamble_Gen()
{
	PreambleLen = 80;
	if(pPreambleSym!=NULL)
		ippsFree(pPreambleSym);
	pPreambleSym = ippsMalloc_32fc(PreambleLen);
	PSK_MOD(preamble_4285,PreambleLen,2,pPreambleSym);

}
	

