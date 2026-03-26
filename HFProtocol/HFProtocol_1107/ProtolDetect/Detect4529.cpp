
#include "StdAfx.h"
#include "Detect4529.h"
#include "PreambleConv.h"
#include "ProtolDetect.h"

CDetect4529::CDetect4529(void)
{
	m_CPreambleConv = new CPreambleConv;
	PreambleSym = ippsMalloc_32fc(80);
	m_CPreambleConv->Preamble_Gen(wNATO4529,PreambleSym,PreambleLen);
	m_CPreambleConv->PreambleConv_ini(PreambleLen,8,128);
}

CDetect4529::~CDetect4529(void)
{
	m_CPreambleConv->PreambleConv_free();
	ippsFree(PreambleSym);
	delete m_CPreambleConv;
	
}
/************************************************************************/
/*   nLen 为相关点数
/************************************************************************/
void CDetect4529::Detect4529(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,BOOL &detect)
{
	m_CPreambleConv->PreambleConv(pSrc,nLen,PreambleSym,PreambleLen,8,128,pIndex,maxCof,frequency,detect);

	frequency = frequency*1200;

}
