#include "StdAfx.h"
#include "Detect141B.h"
#include "PreambleConv.h"
#include "MIL141Bpro.h"

namespace {
bool IsLegal141BFrequency(Ipp32f frequencyNorm)
{
    const Ipp32f frequencyHz = frequencyNorm * 2400.0f;
    return !((frequencyHz > 5.0f && frequencyHz < 1200.0f) || (frequencyHz > 2100.0f && frequencyHz < 2400.0f));
}

void ConsiderCandidate(BOOL candDetect, int candWaveType, int candIndex, Ipp32f candCof, Ipp32f candFrequency,
    BOOL& bestDetect, int& bestWaveType, int& bestIndex, Ipp32f& bestCof, Ipp32f& bestFrequency)
{
    if (!candDetect) {
        return;
    }
    if (!IsLegal141BFrequency(candFrequency)) {
        return;
    }
    if (!bestDetect || candCof > bestCof) {
        bestDetect = TRUE;
        bestWaveType = candWaveType;
        bestIndex = candIndex;
        bestCof = candCof;
        bestFrequency = candFrequency;
    }
}
}

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
/*   nLen Ϊ相关的长度  pSr输入的数据长度要长于 800点*4
/************************************************************************/
void CDetect141B::Detect141B(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &frequency,int &waveType,BOOL &detect)
{
	int mpIndex = 0;
	Ipp32f mfrequency = 0.0f;
	BOOL mdetect = FALSE;
	BOOL mdetectTlc = FALSE;
	BOOL mdetectBw3 = FALSE;
	BOOL mdetectBw0 = FALSE;
	BOOL mdetectBw1 = FALSE;
	Ipp32f maxcof = 0.0f;
	Ipp32f maxcofTlc = 0.0f;
	Ipp32f maxcofBw3 = 0.0f;
	Ipp32f maxcofBw0 = 0.0f;
	Ipp32f maxcofBw1 = 0.0f;
	Ipp32f frequencyTlc = 0.0f;
	Ipp32f frequencyBw3 = 0.0f;
	Ipp32f frequencyBw0 = 0.0f;
	Ipp32f frequencyBw1 = 0.0f;
	int pIndextlc = 0;
	int pIndexbw3 = 0;
	int pIndexbw0 = 0;
	int pIndexbw1 = 0;

	BOOL bestDetect = FALSE;
	int bestWaveType = 0;
	int bestIndex = 0;
	Ipp32f bestCof = 0.0f;
	Ipp32f bestFrequency = 0.0f;

	m_CPreambleConvtlc->PreambleConv(pSrc,nLen,TLCSym,TLCLen,4,512,pIndextlc,maxcofTlc,frequencyTlc,mdetectTlc);
	m_CPreambleConv->PreambleConv(pSrc,nLen,BW3Sym,BW3Len,4,1024,pIndexbw3,maxcofBw3,frequencyBw3,mdetectBw3);
	m_CPreambleConv->PreambleConv(pSrc,nLen,BW0Sym,BW0Len,4,1024,pIndexbw0,maxcofBw0,frequencyBw0,mdetectBw0);
	m_CPreambleConv->PreambleConv(pSrc,nLen,BW1Sym,BW1Len,4,1024,pIndexbw1,maxcofBw1,frequencyBw1,mdetectBw1);

	ConsiderCandidate(mdetectTlc, wMIL141BTLC, pIndextlc, maxcofTlc, frequencyTlc,
		bestDetect, bestWaveType, bestIndex, bestCof, bestFrequency);
	ConsiderCandidate(mdetectBw3, wMIL141BBW3, pIndexbw3, maxcofBw3, frequencyBw3,
		bestDetect, bestWaveType, bestIndex, bestCof, bestFrequency);
	ConsiderCandidate(mdetectBw0, wMIL141BBW0, pIndexbw0, maxcofBw0, frequencyBw0,
		bestDetect, bestWaveType, bestIndex, bestCof, bestFrequency);
	ConsiderCandidate(mdetectBw1, wMIL141BBW1, pIndexbw1, maxcofBw1, frequencyBw1,
		bestDetect, bestWaveType, bestIndex, bestCof, bestFrequency);

	if(mdetectTlc)
	{	
		m_CPreambleConv->PreambleConv(&pSrc[pIndextlc+240*4],(500-BW0Len),BW0Sym,BW0Len,4,1024,mpIndex,maxcof,mfrequency,mdetect);
		ConsiderCandidate(mdetect, wMIL141BBW0, pIndextlc + 240 * 4 + mpIndex, maxcof, mfrequency,
			bestDetect, bestWaveType, bestIndex, bestCof, bestFrequency);

		m_CPreambleConv->PreambleConv(&pSrc[pIndextlc+240*4],(500-BW1Len),BW1Sym,BW1Len,4,1024,mpIndex,maxcof,mfrequency,mdetect);
		ConsiderCandidate(mdetect, wMIL141BBW1, pIndextlc + 240 * 4 + mpIndex, maxcof, mfrequency,
			bestDetect, bestWaveType, bestIndex, bestCof, bestFrequency);
	}

	detect = bestDetect;
	waveType = bestWaveType;
	pIndex = bestIndex;
	frequency = bestFrequency * 2400.0f;
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
// 数据采样率转换
void CDetect141B::MIL141Bdetect(Ipp16s *pSrc,int nLeng,int &pIndex,Ipp32f &frequency,int &waveType,BOOL &detect)
{
	int outLen,allLen;

	ippsHilbertInitAlloc_16s32fc(&pSpec, nLeng, ippAlgHintNone); ///  指定变换长度和性能变化
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
