
#include "StdAfx.h"
#include "Detect141B.h"
#include "PreambleConv.h"
#include "ProtolDetect.h"

CDetect141B::CDetect141B(void)
{
	m_CPreambleConv = new CPreambleConv;
	m_CPreambleConvtlc = new CPreambleConv;


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
void CDetect141B::Detect141B(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,int &waveType,BOOL &detect)
{
	int mpIndex;
	Ipp32f mmaxCof,mfrequency;
	BOOL mdetect;
	m_CPreambleConvtlc->PreambleConv(pSrc,nLen,TLCSym,TLCLen,4,512,pIndex,maxCof,frequency,detect);
	if(!detect)
	{
		m_CPreambleConv->PreambleConv(pSrc,nLen,BW3Sym,BW3Len,4,1024,pIndex,maxCof,frequency,detect);
		if(detect)
			waveType = wMIL141BBW3;
	}
	else //  找到TLC
	{	
		// 识别波形
		m_CPreambleConv->PreambleConv(&pSrc[pIndex+240*4],(500-BW0Len),BW0Sym,BW0Len,4,1024,mpIndex,mmaxCof,mfrequency,mdetect);
		if(mdetect)
		{
			waveType = wMIL141BBW0;
			maxCof = mmaxCof;
			frequency = mfrequency;
		}
		else
		{
			m_CPreambleConv->PreambleConv(&pSrc[pIndex+240*4],(500-BW1Len),BW1Sym,BW1Len,4,1024,mpIndex,mmaxCof,mfrequency,mdetect);
			if(mdetect)
			{
				waveType = wMIL141BBW1;
				maxCof = mmaxCof;
				frequency = mfrequency;
			}
			else
				waveType = wMIL141BTLC;
		}
	}
	frequency = frequency*2400;
	if(detect)
	{
		if((frequency>5 && frequency<1200) || (frequency>2100 && frequency<2400))
		{
			detect = FALSE;
			return;
		}
	}
	

}

