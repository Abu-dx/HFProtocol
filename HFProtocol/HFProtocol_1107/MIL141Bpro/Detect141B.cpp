
#include "StdAfx.h"
#include "Detect141B.h"
#include "PreambleConv.h"
#include "MIL141Bpro.h"

CDetect141B::CDetect141B(void)
{
	m_CPreambleConvtlc = new CPreambleConv;
	m_CPreambleConv = new CPreambleConv;

	TLCSym = ippsMalloc_32fc(240);
	m_CPreambleConvtlc->Preamble_Gen(wMIL141BTLC,TLCSym,TLCLen);
	m_CPreambleConvtlc->PreambleConv_ini(TLCLen,4,512);

	BW0Sym = ippsMalloc_32fc(512);
	m_CPreambleConv->Preamble_Gen(wMIL141BBW0,BW0Sym,BW0Len);
	m_CPreambleConv->PreambleConv_ini(512,4,1024);

	BW1Sym = ippsMalloc_32fc(512);
	m_CPreambleConv->Preamble_Gen(wMIL141BBW1,BW1Sym,BW1Len);

	BW3Sym = ippsMalloc_32fc(512);
	m_CPreambleConv->Preamble_Gen(wMIL141BBW3,BW3Sym,BW3Len);
}

CDetect141B::~CDetect141B(void)
{
	m_CPreambleConv->PreambleConv_free();
	delete m_CPreambleConv;
	m_CPreambleConvtlc->PreambleConv_free();
	delete m_CPreambleConvtlc;

	ippsFree(TLCSym);
	ippsFree(BW0Sym);
	ippsFree(BW1Sym);
	ippsFree(BW3Sym);

}

/************************************************************************/
/*   nLen 为相关点数  pSr缓存中至少要有 （800）*4
/************************************************************************/
void CDetect141B::Detect141B(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &frequency,int &waveType,BOOL &detect)
{
	int mpIndex;
	Ipp32f mfrequency;
	BOOL mdetect,mdetect1,mdetect2;
	Ipp32f maxcof,maxcof1,maxcof2;
	int pIndextlc,pIndexbw3;

	m_CPreambleConvtlc->PreambleConv(pSrc,nLen,TLCSym,TLCLen,4,512,pIndextlc,maxcof1,frequency,mdetect1);

	m_CPreambleConv->PreambleConv(pSrc,nLen,BW3Sym,BW3Len,4,1024,pIndexbw3,maxcof2,frequency,mdetect2);
	

	if(maxcof1>maxcof2 && mdetect1)
	{
		waveType = wMIL141BTLC;
		detect = mdetect1;
		pIndex = pIndextlc;
	}
	else if(maxcof2>maxcof1 && mdetect2)
	{
		waveType = wMIL141BBW3;
		detect = mdetect2;
		pIndex = pIndexbw3;
	}
	else
	{
		detect = FALSE;
		waveType = 0;
	}

	if(waveType==wMIL141BTLC) //  找到TLC
	{	
		// 识别波形
		m_CPreambleConv->PreambleConv(&pSrc[pIndex+240*4],(500-BW0Len),BW0Sym,BW0Len,4,1024,mpIndex,maxcof,mfrequency,mdetect);
		if(mdetect)
		{
			waveType = wMIL141BBW0;
			frequency = mfrequency;
			detect = mdetect;
		}
		else
		{
			m_CPreambleConv->PreambleConv(&pSrc[pIndex+240*4],(500-BW1Len),BW1Sym,BW1Len,4,1024,mpIndex,maxcof,mfrequency,mdetect);
			if(mdetect)
			{
				waveType = wMIL141BBW1;
				frequency = mfrequency;
				detect = mdetect;
			}
			else
				waveType = wMIL141BTLC;
		}
	}
	frequency = frequency*2400;
}


void CDetect141B::MIL141Bdetect_ini(int nLeng)
{
	maxConvLen = 800;
	pBuf = ippsMalloc_32fc(2*nLeng);
	delayBuf = ippsMalloc_32fc(4*maxConvLen);
	delayL = 0;
	BufPos = 0;

}
void CDetect141B::MIL141Bdetect_free()
{
	ippsFree(pBuf);
	ippsFree(delayBuf);

}
// 已完成采样率转换
void CDetect141B::MIL141Bdetect(Ipp16s *pSrc,int nLeng,int &pIndex,Ipp32f &frequency,int &waveType,BOOL &detect)
{
	int outLen,allLen;

	ippsHilbertInitAlloc_16s32fc(&pSpec, nLeng, ippAlgHintNone); ///  指定变换长度后不能变化
	ippsHilbert_16s32fc(pSrc,&pBuf[BufPos],pSpec);
	allLen = BufPos + nLeng;

	if(allLen<4*maxConvLen)// 防止输入数据不足
	{
		BufPos = allLen;
		return;
	}
	else
	{
		ippsCopy_32fc(delayBuf,pBuf,delayL);
		delayL = 4*maxConvLen;
		Detect141B(pBuf,allLen-delayL,pIndex,frequency,waveType,detect);
		ippsCopy_32fc(&pBuf[allLen-delayL],delayBuf,delayL);
		BufPos = delayL;
	}
	ippsHilbertFree_16s32fc(pSpec);
}