#include "StdAfx.h"
#include "DetectCLEW.h"
#include "ProtolDetect.h"
#include <math.h>

CDetectCLEW::CDetectCLEW(void)
{
}

CDetectCLEW::~CDetectCLEW(void)
{
}

void CDetectCLEW::DetectCLEW_ini(int nLen,Ipp32f insample)
{
	ReSample_ini(2*nLen,insample,7040.0,8,2048);

	OFDM_fft = 64;
	OFDM_cpxL = 30;
	OFDM_symL = OFDM_fft + OFDM_cpxL;
	// 256点fft检测
	detect_fft = 256;
	bufDetectFFT = ippsMalloc_32fc(detect_fft);
	idx_dopule = floor(605.0/7040*detect_fft);
	idx_16 = floor(2915.0/7040*detect_fft);
	idx_data1 = floor(935.0/7040*detect_fft);
	idx_data14 = floor(2365.0/7040*detect_fft);	
	delt = floor(110.0/7040*detect_fft);
	ippsFFTInitAlloc_C_32fc(&pDetectFFTSpec, 8, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);

	maxDelayLen = 10*OFDM_symL;
	pdataBuf = ippsMalloc_32fc(nLen+maxDelayLen);
	pdataDelay = ippsMalloc_32fc(maxDelayLen);
	datadelayLen = 0;
	pdataPos = 0;
	

}
void CDetectCLEW::DetectCLEW_free()
{
	ReSample_Free();
	ippsFree(bufDetectFFT);
	ippsFFTFree_C_32fc(pDetectFFTSpec);
	ippsFree(pdataBuf);
	ippsFree(pdataDelay);
}
void CDetectCLEW::DetectCLEW(Ipp32fc *pSrc,int nLen,Ipp32f TH,int &pIndex,Ipp32f &decsum,Ipp32f &fdopule)
{

	int i,j,dataLen;
	ReSample(pSrc,nLen,&pdataBuf[pdataPos],dataLen);
	int allLen = pdataPos + dataLen;
	if(allLen<maxDelayLen)// 防止输入数据不足
	{
		pdataPos = allLen;
		return;
	}
	else
	{
		ippsCopy_32fc(pdataDelay,pdataBuf,datadelayLen);
		// 检测处理
		int Num = allLen/OFDM_symL;
		Ipp32f *winBufdec = ippsMalloc_32f(Num);
		Ipp32f *winBufsum = ippsMalloc_32f(Num);
		Ipp32f *mfre = ippsMalloc_32f(Num);
		int pwinnum=0;
		for (i=0; i<allLen-2*OFDM_symL; i=i+OFDM_symL)
		{	
			FFTdetect(&pdataBuf[i],2*OFDM_symL,mfre[pwinnum],winBufdec[pwinnum],winBufsum[pwinnum]);
			pwinnum++;
		}
		int compareN = 8;
		int num=0;
		Ipp32f tempa,maxa=0;
		int BurstNum = 0;
		decsum = 0;
		Ipp32f meansum1,meansum2;
		for (i=0;i<pwinnum-compareN;i++){
			num=0;
			for (j=0;j<5;j++){
				if(winBufdec[i+j]>TH)
					num++;
			}
			if(num>2 && winBufdec[i]>TH && winBufdec[i+1]>TH
				&& winBufdec[i+5+1]<TH && winBufdec[i+5+2]<TH)
			{
				ippsMean_32f(&winBufsum[i],4,&meansum1,ippAlgHintFast);
				ippsMean_32f(&winBufsum[i+5],2,&meansum2,ippAlgHintFast);
				if(meansum1<meansum2)
				{
					BurstNum++;
					ippsMax_32f(&winBufdec[i],5,&tempa);
					if(tempa>maxa)
					{
						maxa = tempa;
						pIndex = i*OFDM_symL;
						decsum = winBufdec[i];
						fdopule = mfre[i+1]*7040;
					}
					i = i + compareN;
				}		
			}
		}
		int BufHavePro;
		if(BurstNum>0)
			BufHavePro = pwinnum*OFDM_symL;
		else
			BufHavePro = (pwinnum-compareN)*OFDM_symL;

		datadelayLen = allLen - BufHavePro;
		TRACE("%d",datadelayLen);
		ippsCopy_32fc(&pdataBuf[allLen-datadelayLen],pdataDelay,datadelayLen);
		pdataPos = datadelayLen;

		ippsFree(winBufdec);
		ippsFree(mfre);
	}
}


void CDetectCLEW::FFTdetect(Ipp32fc *pSrc,int nLen,Ipp32f &fdopule,Ipp32f &decsum,Ipp32f &sum16)
{
	int i,j;
	float suma,sumb,sumc;
	Ipp32f *fftDst,maxf,*decs;
	decs = ippsMalloc_32f(8);
	int pIndex;

	fftDst = ippsMalloc_32f(detect_fft);

	ippsZero_32fc(bufDetectFFT,detect_fft);
	ippsCopy_32fc(pSrc,bufDetectFFT,nLen);
	ippsFFTFwd_CToC_32fc_I(bufDetectFFT,pDetectFFTSpec,NULL);
	ippsMagnitude_32fc(bufDetectFFT,fftDst,detect_fft);

	// 幅度比值
	int ibegin,iend;
	for (i=-4;i<4;i++)
	{
		suma = 0;
		ibegin = idx_dopule+i*delt-delt/2;
		iend = idx_dopule+i*delt+delt/2;
		for (j= ibegin;j<iend;j++)
			suma = suma + fftDst[j]/detect_fft;

		sumb = 0;
		ibegin = idx_16+i*delt-delt/2;
		iend = idx_16+i*delt+delt/2;
		for (j= ibegin;j<iend;j++)
			sumb = sumb + fftDst[j]/detect_fft;

		sumc = 0;
		ibegin = idx_data1+i*delt-delt/2;
		iend = idx_data14+i*delt+delt/2;
		for (j= ibegin;j<iend;j++)
			sumc = sumc + fftDst[j]/detect_fft;

		decs[i+4] = (suma+sumb)/sumc;
	}
	ippsMaxIndx_32f(decs,8,&decsum,&pIndex);

	sum16 = 0;
	ibegin = idx_data1+pIndex*delt-delt/2;
	iend = idx_data14+pIndex*delt+delt/2;
	for (j= ibegin;j<iend;j++)
		sum16 = sum16 + fftDst[j]/detect_fft;

	fdopule =(float)(idx_dopule+pIndex-4)/detect_fft;
	ippsFree(decs);
	ippsFree(fftDst);

}

void CDetectCLEW::ReSample_ini(Ipp32s nLeng,Ipp32f insample,Ipp32f outsample,Ipp16s incimate,Ipp16s tapsLen)
{
	m_incimate = incimate;
	m_taps_onech = tapsLen/incimate;  //  整除
	m_Decimate =Ipp64f(insample*incimate)/outsample;
	TapsLen = tapsLen;

	ResampleBuf = ippsMalloc_32fc(nLeng+TapsLen);
	pDelay = ippsMalloc_32fc(TapsLen);
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
void CDetectCLEW::ReSample_Free()
{
	ippsFree(ResampleBuf);
	ippsFree(pDelay);
	for (int i=0;i<m_incimate;i++)
	{
		delete ReSample_taps[i];
	}
	delete ReSample_taps;
}
void  CDetectCLEW::ReSample(Ipp32fc *pSrc,int nLeng,Ipp32fc *pDst,int &outLeng)
{
	int i,m=0;
	Ipp32fc temp;

	ippsCopy_32fc(pDelay,ResampleBuf,delayLen);
	ippsCopy_32fc(pSrc,&ResampleBuf[delayLen],nLeng);

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