
#include "StdAfx.h"
#include "Detect4285.h"
#include "PreambleConv.h"
#include "ProtolDetect.h"

CDetect4285::CDetect4285(void)
{
	m_CPreambleConv = new CPreambleConv;
	PreambleSym = ippsMalloc_32fc(80);
	m_CPreambleConv->Preamble_Gen(wNATO4285,PreambleSym,PreambleLen);
	m_CPreambleConv->PreambleConv_ini(PreambleLen,4,128);
}

CDetect4285::~CDetect4285(void)
{
	m_CPreambleConv->PreambleConv_free();
	ippsFree(PreambleSym);
	delete m_CPreambleConv;
	
}
/************************************************************************/
/*   nLen 为相关点数
/************************************************************************/
void CDetect4285::Detect4285(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect)
{
	m_CPreambleConv->PreambleConv(pSrc,nLen,PreambleSym,PreambleLen,4,128,pIndex,maxCof,frequency,detect);
	frequency = frequency*2400;
	if((frequency>10 && frequency<1200) || (frequency>2100 && frequency<2400))
	{
		detect = FALSE;
		return;
	}
}
