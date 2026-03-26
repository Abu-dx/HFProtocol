// MIL141Bpro.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "MIL141Bpro.h"
#include "BW0demode.h"
#include "BW1demode.h"
#include "BW3demode.h"
#include "Detect141B.h"
#include "BW2demode.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CMIL141Bpro::CMIL141Bpro(void)
{
	m_CDetect141B = NULL;
	m_BW0demode = NULL;
	m_BW1demode = NULL;
	m_BW3demode = NULL;
	m_BW2demode = NULL;

}
CMIL141Bpro::~CMIL141Bpro(void)
{
	if(m_CDetect141B!=NULL)
	{
		delete m_CDetect141B;
		m_CDetect141B = NULL;
	}
	if(m_BW0demode!=NULL)
	{
		delete m_BW0demode;
		m_BW0demode = NULL;
	}
	if(m_BW1demode!=NULL)
	{
		delete m_BW1demode;
		m_BW1demode = NULL;
	}
	if(m_BW3demode!=NULL)
	{
		delete m_BW3demode;
		m_BW3demode = NULL;
	}
	if(m_BW2demode!=NULL)
	{
		delete m_BW2demode;
		m_BW2demode = NULL;
	}

}

void CMIL141Bpro::MIL141Bdemode_ini(int nLeng,int Insample,int Outsample,Ipp16s P,
									Ipp32f roll,Ipp32f Baud,int SrctapLen)
{
	waveType = 0;
	int allLen = 3*nLeng;
	pBuf = ippsMalloc_16s(allLen);	
	pDelay = ippsMalloc_16s(allLen);
	DelayLen = 0;

	m_Insample = Insample;
	m_Outsample = Outsample;
	
	IppReSampleIni_16s(nLeng,128,Insample,Outsample);

	m_CDetect141B = new CDetect141B;
	m_CDetect141B->MIL141Bdetect_ini(nLeng);

	m_BW0demode = new CBW0demode;
	m_BW0demode->Demode_ini(allLen,P,roll,Baud,SrctapLen);

	m_BW1demode = new CBW1demode;
	m_BW1demode->Demode_ini(allLen,P,roll,Baud,SrctapLen);

	m_BW3demode = new CBW3demode;
	m_BW3demode->Demode_ini(allLen,P,roll,Baud,SrctapLen);

	m_BW2demode = new CBW2demode;
	m_BW2demode->Demode_PSKrealFSE_ini(9600,9600,nLeng,1800,4,512,roll,2400,SrctapLen);

	
}
void CMIL141Bpro::MIL141Bdemode(Ipp16s *pSrc,int nLeng,Ipp16s P,Ipp16s *expout,int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType141B *headType,int &headNum)
{
	int pIndex;
	Ipp32f maxCof,mfrequency;
	BOOL detect;
	int mwaveType;
	int reoulen;
	outLeng = 0;

	if(m_Insample==m_Outsample)
	{
		ippsCopy_16s(pSrc,&pBuf[DelayLen],nLeng);
		reoulen = nLeng;
	}
	else
		IppReSample_16s(pSrc,nLeng,&pBuf[DelayLen],reoulen);
	m_CDetect141B->MIL141Bdetect(&pBuf[DelayLen],reoulen,pIndex,mfrequency,mwaveType,detect);
	
	if(detect)
	{
		if(mwaveType==wMIL141BTLC)
		{
			headNum = 0;
			headType[headNum].BWtype =  mwaveType;
			headType[headNum].fre = mfrequency;
			headType[headNum].position = pIndex;
			headNum++;
			return;
		}
		if(waveType!=mwaveType)
		{
			ippsCopy_16s(pDelay,pBuf,DelayLen);
			pData = pBuf;
			dataLen = DelayLen + reoulen;
		}
		else
		{
			pData = &pBuf[DelayLen];
			dataLen = reoulen;
		}
		frequency = mfrequency;
		waveType = mwaveType;
	}
	else
	{
		pData = &pBuf[DelayLen];
		dataLen = reoulen;
	}
	if(waveType==wMIL141BBW0)
	{
		m_BW0demode->Demode(pData,dataLen,P,frequency,outLeng,outbyte,byteLeng,headType,headNum);
	}
	else if(waveType==wMIL141BBW1)
	{
		m_BW1demode->Demode(pData,dataLen,P,frequency,outLeng,outbyte,byteLeng,headType,headNum);
	}
	else if(waveType==wMIL141BBW3)
	{
		m_BW3demode->Demode(pData,dataLen,P,frequency,outLeng,outbyte,byteLeng,headType,headNum);
	}
	else if(waveType==wMIL141BTLC)
	{
		//m_BW2demode->Demode_PSKrealFSE(pData,dataLen,P,512,6,0.6,expout,outLeng,outbyte,byteLeng,headType,headNum);
		
	}
	else
	{
		byteLeng = 0;
		headNum = 0;
	}
	ippsCopy_16s(&pBuf[DelayLen],pDelay,reoulen);
	DelayLen = reoulen;
	
}
void CMIL141Bpro::MIL141Bdemode_free()
{
	m_CDetect141B->MIL141Bdetect_free();
	delete m_CDetect141B;
	m_CDetect141B = NULL;

	m_BW0demode->Demode_free();
	delete m_BW0demode;
	m_BW0demode = NULL;

	m_BW1demode->Demode_free();
	delete m_BW1demode;
	m_BW1demode = NULL;

	m_BW3demode->Demode_free();
	delete m_BW3demode;
	m_BW3demode = NULL;

	m_BW2demode->Demode_PSKrealFSE_free();
	delete m_BW2demode;
	m_BW2demode = NULL;

	IppReSamplefree_16s();

	ippsFree(pBuf);
	ippsFree(pDelay);

}

void CMIL141Bpro::IppReSampleIni_16s(int nLeng, int history, int nInRate, int nOutRate)
{
	nHistory = history;
	ippsResamplePolyphaseFixedInitAlloc_16s(&state,nInRate,nOutRate,2*nHistory,0.95f,9.0f,ippAlgHintAccurate);
	inBuf=ippsMalloc_16s(nLeng*2+2);
	ippsZero_16s(inBuf,nLeng*2+2);
	time = nHistory;
	lastread = nHistory;

}
void CMIL141Bpro::IppReSamplefree_16s()
{
	ippsFree(inBuf);
	ippsResamplePolyphaseFixedFree_16s(state);
}
void CMIL141Bpro::IppReSample_16s(Ipp16s *pSrc,int nLeng,Ipp16s *pDst,int &outLen)
{
	ippsCopy_16s(pSrc,inBuf+lastread,nLeng);
	lastread+=nLeng;
	IppStatus restate;
	restate = ippsResamplePolyphaseFixed_16s(state,inBuf,lastread-nHistory-(int)time,pDst,0.98f,&time,&outLen);

	ippsMove_16s(inBuf+(int)time-nHistory,inBuf,lastread+nHistory-(int)time);
	lastread-=(int)time-nHistory;

	time-=(int)time-nHistory;


}