#include "StdAfx.h"
#include "ReSampleFSK.h"

CReSampleFSK::CReSampleFSK(void)
{
}

CReSampleFSK::~CReSampleFSK(void)
{
}
void CReSampleFSK::IppReSampleIni_16s(int nLeng, int history, int nInRate, int nOutRate)
{
	nHistory = history;
	ippsResamplePolyphaseFixedInitAlloc_16s(&state,nInRate,nOutRate,2*nHistory,0.95f,9.0f,ippAlgHintAccurate);
	inBuf=ippsMalloc_16s(nLeng*2+2);
	ippsZero_16s(inBuf,nLeng*2+2);
	time = nHistory;
	lastread = nHistory;

}
void CReSampleFSK::IppReSamplefree_16s()
{
	ippsFree(inBuf);
	ippsResamplePolyphaseFixedFree_16s(state);
}
void CReSampleFSK::IppReSample_16s(Ipp16s *pSrc,int nLeng,Ipp16s *pDst,int &outLen)
{
	ippsCopy_16s(pSrc,inBuf+lastread,nLeng);
	lastread+=nLeng;
	IppStatus restate;
	restate = ippsResamplePolyphaseFixed_16s(state,inBuf,lastread-nHistory-(int)time,pDst,0.98f,&time,&outLen);

	ippsMove_16s(inBuf+(int)time-nHistory,inBuf,lastread+nHistory-(int)time);
	lastread-=(int)time-nHistory;

	time-=(int)time-nHistory;
}

//  包含下变频，匹配滤波，重采样
//原始滤波器阶数tapsLen		多相分解后滤波阶数taps_onech   
//void CReSampleFSK::ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen)
//{
//	m_insample = insample;
//	m_outsample = outsample;
//	m_incimate = incimate;
//	m_taps_onech = tapsLen/incimate;  //  整除
//	m_Decimate =Ipp64f(insample*incimate)/outsample;
//	TapsLen = tapsLen;
//
//	ResampleBuf = ippsMalloc_32f(nLeng+TapsLen);
//	pDelay = ippsMalloc_32f(TapsLen);
//	delayLen = 0;
//
//	if(m_incimate>m_Decimate)
//		m_fstop=1/(Ipp64f)(m_incimate*2);
//	else
//		m_fstop = 1/(Ipp64f)(m_Decimate*2);
//
//	pTaps_64f = ippsMalloc_64f(tapsLen);
//	ippsFIRGenLowpass_64f(m_fstop, pTaps_64f, tapsLen, ippWinHamming, ippTrue);	//	窗化法生成低通滤波器
//
//	ReSample_taps=new float*[m_incimate];
//	int i, j;
//	for(j=0; j<m_incimate; j++)
//	{
//		ReSample_taps[j]=new float[m_taps_onech];
//		for(i=0; i<m_taps_onech; i++)
//		{
//			ReSample_taps[j][i]=(Ipp32f)pTaps_64f[i*m_incimate+j];
//		}
//	}
//	ippsFree(pTaps_64f);
//	history=0;
//	faddress=0;
//	address=0;
//
//}
//void CReSampleFSK::ReSample_Free()
//{
//	ippsFree(ResampleBuf);
//	ippsFree(pDelay);
//	for (int i=0;i<m_incimate;i++)
//	{
//		free(ReSample_taps[i]);
//	}
//	free(ReSample_taps);
//}
//void  CReSampleFSK::ReSample(Ipp16s *pSrc,int nLeng,Ipp32f *pDst,int &outLeng)
//{
//	int i,m=0;
//
//	if(m_insample==m_outsample)
//	{
//		ippsConvert_16s32f(pSrc,pDst,nLeng);
//		outLeng = nLeng;
//		return;
//	}
//
//	ippsCopy_32f(pDelay,ResampleBuf,delayLen);
//	ippsConvert_16s32f(pSrc,&ResampleBuf[delayLen],nLeng);
//
//	int len = nLeng+delayLen;
//
//	for (i=0;i<len-m_taps_onech+1;i++)
//	{	
//		while(address>=0)
//		{	
//			ippsDotProd_32f(ReSample_taps[address],&ResampleBuf[i],m_taps_onech,&pDst[m]);
//			faddress=faddress-m_Decimate;
//			address=int(faddress+0.5);
//			m++;
//		}
//		faddress=faddress+m_incimate;
//		address=int(faddress+0.5);
//	}
//
//	delayLen=m_taps_onech-1;
//	ippsCopy_32f(&ResampleBuf[i],pDelay,delayLen);
//	outLeng = m;
//}