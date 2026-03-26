
#include "StdAfx.h"
#include "Detect110A.h"
#include "PreambleConv.h"
#include "ProtolDetect.h"
#include "Preamble.h"

CDetect110A::CDetect110A(void)
{
	m_CPreambleConv = new CPreambleConv;
	PreambleSym = ippsMalloc_32fc(288);
	m_CPreambleConv->Preamble_Gen(wMIL110A,PreambleSym,PreambleLen);
	m_CPreambleConv->PreambleConv_ini(PreambleLen,4,512);


	pD1D2 = ippsMalloc_32fc(64);
	pTypeSym = new Ipp32fc *[8];
	for (int i=0;i<8;i++)
	{
		pTypeSym[i] = ippsMalloc_32fc(32);
		m_CPreambleConv->PSK_MOD(type_110A[i],32,8,pTypeSym[i]);
	}

}

CDetect110A::~CDetect110A(void)
{

	ippsFree(pD1D2);
	for (int i=0;i<8;i++)
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
void CDetect110A::Detect110A(Ipp32fc *pSrc,int nLen,int &pIndex,Ipp32f &maxCof,Ipp32f &frequency,int &dataRate,int &interLeng,BOOL &detect)
{
	m_CPreambleConv->PreambleConv(pSrc,nLen,PreambleSym,PreambleLen,4,512,pIndex,maxCof,frequency,detect);	
	if(detect)
	{
		Ipp32fc *pdown = ippsMalloc_32fc(64);
		int pdownLen;
		int pPhase = 0;
		ippsSampleDown_32fc(&pSrc[pIndex+PreambleLen*4],64*4,pdown,&pdownLen,4,&pPhase);// 间隔P点抽出数据

		Ipp32fc *pSinCos = ippsMalloc_32fc(64);
		float mPhase = 0;
		Ipp32f mfre = 1-frequency;
		ippsTone_Direct_32fc(pSinCos, 64, 1, mfre, &mPhase, ippAlgHintFast);
		ippsMul_32fc_I(pSinCos,pdown,64);	
		IdentifyD1D2(pdown,13,dataRate,interLeng);

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

void CDetect110A::IdentifyD1D2(Ipp32fc *pSrc,int typeNum,int &dataRate,int &interLeng)
{
	int i,idx=0;
	float maxcof=0,cof;
	float cc,th;
	for (i=0;i<typeNum;i++)
	{
		ippsCopy_32fc(pTypeSym[D1D2[i][0]],pD1D2,32);
		ippsCopy_32fc(pTypeSym[D1D2[i][1]],&pD1D2[32],32);

		m_CPreambleConv->CrossConvCof(pSrc,pD1D2,64,1,cof,cc,th);// 相关系数
		if (cof>=maxcof)
		{
			maxcof = cof;
			idx = i;
		}
	}
	switch(idx)
	{
	case 0:
		dataRate=4800;		interLeng = 0;		break;
	case 1:
		dataRate=2400;		interLeng = 40*576;		break;
	case 2:
		dataRate=2400;		interLeng = 40*72;		break;
	case 3:
		dataRate=1200;		interLeng = 40*288;		break;
	case 4:
		dataRate=1200;		interLeng = 40*36;		break;
	case 5:
		dataRate=600;		interLeng = 40*144;		break;
	case 6:
		dataRate=600;		interLeng = 40*18;		break;
	case 7:
		dataRate=300;		interLeng = 40*144;		break;
	case 8:
		dataRate=300;		interLeng = 40*18;		break;
	case 9:
		dataRate=150;		interLeng = 40*144;		break;
	case 10:
		dataRate=150;		interLeng = 40*18;		break;
	case 11:
		dataRate=75;		interLeng = 20*36;		break;
	case 12:
		dataRate=75;		interLeng = 40*9;		break;
	default:
		break;
	}
}
