#include "StdAfx.h"
#include "ReSample.h"

CReSample::CReSample(void)
{
}

CReSample::~CReSample(void)
{
}
//  包含下变频，匹配滤波，重采样
//原始滤波器阶数tapsLen		多相分解后滤波阶数taps_onech   
void CReSample::ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen)
{

	m_Insample = insample;
	m_Outsample = outsample;

	m_incimate = incimate;
	m_taps_onech = tapsLen/incimate;  //  整除
	m_Decimate =Ipp64f(insample*incimate)/outsample;
	TapsLen = tapsLen;

	ResampleBuf = ippsMalloc_32fc(nLeng+TapsLen);
	pDelay = ippsMalloc_32fc(TapsLen);
	delayLen = 0;
	ippsHilbertInitAlloc_16s32fc(&pSpec, nLeng, ippAlgHintNone); ///  指定变换长度后不能变化

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
void CReSample::ReSample_Free()
{
	ippsFree(ResampleBuf);
	ippsFree(pDelay);
	ippsHilbertFree_16s32fc(pSpec);
	for (int i=0;i<m_incimate;i++)
	{
		delete ReSample_taps[i];
	}
	delete ReSample_taps;
}
void  CReSample::ReSample(Ipp16s *pSrc,int nLeng,Ipp32fc *pDst,int &outLeng)
{
	int i,m=0;

	if(m_Insample == m_Outsample)
	{
		ippsHilbert_16s32fc(pSrc,pDst,pSpec);
		outLeng = nLeng;
		return;
	}

	ippsCopy_32fc(pDelay,ResampleBuf,delayLen);
	ippsHilbert_16s32fc(pSrc,&ResampleBuf[delayLen],pSpec);

	int len = nLeng+delayLen;

	for (i=0;i<len-m_taps_onech;i++)
	{	
		while(address>=0)
		{	
			ippsDotProd_32f32fc(ReSample_taps[address],&ResampleBuf[i],m_taps_onech,&pDst[m]);
			faddress=faddress-m_Decimate;
			address=int(faddress+0.5);
			m++;
		}
		faddress=faddress+m_incimate;
		address=int(faddress+0.5);
	}

	delayLen=len-i;
	ippsCopy_32fc(&ResampleBuf[i],pDelay,delayLen);
	outLeng = m;
}