
#include "StdAfx.h"
#include "Detect110B.h"
#include "PreambleConv.h"
#include "ProtolDetect.h"
#include "Preamble.h"

CDetect110B::CDetect110B(void)
{
	m_CPreambleConv = new CPreambleConv;
	PreambleSym = ippsMalloc_32fc(184);
	m_CPreambleConv->Preamble_Gen(wMIL110B,PreambleSym,PreambleLen);
	m_CPreambleConv->PreambleConv_ini(PreambleLen,4,512);

	pTypeSym = new Ipp32fc *[31];
	for (int i=0;i<31;i++)
	{
		pTypeSym[i] = ippsMalloc_32fc(41);
		m_CPreambleConv->PSK_MOD(type_110B[i],41,8,pTypeSym[i]);
	}

}

CDetect110B::~CDetect110B(void)
{
	for (int i=0;i<31;i++)
	{
		ippsFree(pTypeSym[i]);
	}
	delete pTypeSym;

	m_CPreambleConv->PreambleConv_free();
	ippsFree(PreambleSym);
	delete m_CPreambleConv;
	
}
/************************************************************************/
/*   nLen 为相关点数
/************************************************************************/
void CDetect110B::Detect110B(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,int &dataRate,int &interLeng,BOOL &detect)
{
	m_CPreambleConv->PreambleConv(pSrc,nLen,PreambleSym,PreambleLen,4,512,pIndex,maxCof,frequency,detect);

	if(detect)
	{
		Ipp32fc *pdown = ippsMalloc_32fc(41);
		int pdownLen;
		int pPhase = 0;
		ippsSampleDown_32fc(&pSrc[pIndex+215*4],41*4,pdown,&pdownLen,4,&pPhase);// 间隔P点抽出数据

		Ipp32fc *pSinCos = ippsMalloc_32fc(41);
		float mPhase = 0;
		Ipp32f mfre = 1-frequency;
		ippsTone_Direct_32fc(pSinCos, 41, 1, mfre, &mPhase, ippAlgHintFast);
		ippsMul_32fc_I(pSinCos,pdown,41);	
		IdentifyType(pdown,41,pTypeSym,31,dataRate,interLeng);
		ippsFree(pdown);
		ippsFree(pSinCos);
	}
	frequency = frequency*2400;
	if((frequency>10 && frequency<1200) || (frequency>2100 && frequency<2400))
	{
		detect = FALSE;
		return;
	}
}


void CDetect110B::IdentifyType(Ipp32fc *pSrc,int nLeng,Ipp32fc **Typesym,int typeNum,int &dataRate,int &interLeng)
{
	int i,idx;
	float maxcof=0,cof;
	float cc,th;
	for (i=0;i<typeNum;i++)
	{
		m_CPreambleConv->CrossConvCof(pSrc,Typesym[i],nLeng,1,cof,cc,th);// 相关系数
		if (cof>maxcof)
		{
			maxcof = cof;
			idx = i;
		}
	}
	
	int moduType,interType;

	moduType = int(idx/6);
	interType = idx%6;
	if(moduType==0)
		dataRate=3200;
	else if(moduType==1)
		dataRate=4800;
	else if(moduType==2)
		dataRate=6400;
	else if(moduType==3)
		dataRate=8000;
	else if(moduType==4)
		dataRate=9600;

	if(interType==0)
		interLeng = dataRate/6.25;
	else if(interType==1)
		interLeng = dataRate/6.25*3;
	else if(interType==2)
		interLeng = dataRate/6.25*9;
	else if(interType==3)
		interLeng = dataRate/6.25*18;
	else if(interType==4)
		interLeng = dataRate/6.25*36;
	else if(interType==5)
		interLeng = dataRate/6.25*72;

}