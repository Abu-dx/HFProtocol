
#include "StdAfx.h"
#include "DetectSLEW.h"
#include "PreambleConv.h"
#include "ProtolDetect.h"

CDetectSLEW::CDetectSLEW(void)
{
	m_CPreambleConv = new CPreambleConv;
	PreambleSym = ippsMalloc_32fc(192);
	m_CPreambleConv->Preamble_Gen(wLINK11SLEW,PreambleSym,PreambleLen);
	m_CPreambleConv->PreambleConv_ini(PreambleLen,4,512);
}

CDetectSLEW::~CDetectSLEW(void)
{
	m_CPreambleConv->PreambleConv_free();
	ippsFree(PreambleSym);
	delete m_CPreambleConv;
	
}
/************************************************************************/
/*   nLen 为相关点数
/************************************************************************/
void CDetectSLEW::DetectSLEW(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect)
{
	m_CPreambleConv->PreambleConv(pSrc,nLen,PreambleSym,PreambleLen,4,512,pIndex,maxCof,frequency,detect);
	frequency = frequency*2400;
	if((frequency>10 && frequency<1200) || (frequency>2100 && frequency<2400))
	{
		detect = FALSE;
		return;
	}
}
