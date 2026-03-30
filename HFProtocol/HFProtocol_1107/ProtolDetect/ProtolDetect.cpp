// ProtolDetect.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "ProtolDetect.h"
#include <math.h>

#include "Detect110A.h"
#include "Detect110B.h"
#include "Detect4285.h"
#include "Detect4529.h"
#include "DetectSLEW.h"
#include "Detect141B.h"
#include "DetectCLEW.h"
#include "ReSampledetect.h"
#include "..\HFProtocol\MIL141Apro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CProtolDetect::CProtolDetect(void)
{
	m_Detect110A = NULL;
	m_Detect110B = NULL;
	m_CDetectSLEW = NULL;
	m_CDetect4285 = NULL;
	m_CDetect4529 = NULL;
	m_CDetect141B = NULL;
	m_CDetectCLEW = NULL;
	m_CReSample = NULL;
	m_CMIL141Apro = NULL;
	pBuf = NULL;
	delayBuf = NULL;
	outbyte = NULL;
	delayL = 0;
	BufPos = 0;
	maxConvLen = 0;
}
CProtolDetect::~CProtolDetect(void)
{
	if(m_Detect110A!=NULL)
	{
		delete m_Detect110A;
		m_Detect110A = NULL;
	}
	if(m_Detect110B!=NULL)
	{
		delete m_Detect110B;
		m_Detect110B = NULL;
	}
	if(m_CDetectSLEW!=NULL)
	{
		delete m_CDetectSLEW;
		m_CDetectSLEW = NULL;
	}
	if(m_CDetect4285!=NULL)
	{
		delete m_CDetect4285;
		m_CDetect4285 = NULL;
	}
	if(m_CDetect4529!=NULL)
	{
		delete m_CDetect4529;
		m_CDetect4529 = NULL;
	}
	if(m_CDetect141B!=NULL)
	{
		delete m_CDetect141B;
		m_CDetect141B = NULL;
	}
	if(m_CDetectCLEW!=NULL)
	{
		m_CDetectCLEW->DetectCLEW_free();
		delete m_CDetectCLEW;
		m_CDetectCLEW = NULL;
	}
	if(m_CReSample!=NULL)
	{
		m_CReSample->ReSample_Free();
		delete m_CReSample;
		m_CReSample = NULL;
	}
	if(m_CMIL141Apro!=NULL)
	{
		m_CMIL141Apro->MIL141Ademode_free();
		delete m_CMIL141Apro;
		m_CMIL141Apro = NULL;
	}
}

/************************************************************************/
/* outsample = 9600   
/************************************************************************/

void CProtolDetect::ProtolDetect_ini(int nLeng,Ipp32f insample,BOOL *selProtol,int ProtolNum)
{
	maxConvLen = 800;
	m_CReSample= new CReSampledetect;
	m_CReSample->ReSample_ini(nLeng,insample,9600,16,1024);

	int i,kname;
	for (i=0;i<ProtolNum;i++)
	{
		if(!selProtol[i])
			continue;

		if (i==wMIL110A)
			m_Detect110A = new CDetect110A;
		else if(i==wMIL110B)
			m_Detect110B = new CDetect110B;
		else if(i==wLINK11SLEW)
			m_CDetectSLEW = new CDetectSLEW;
		else if(i==wNATO4285)
			m_CDetect4285 = new CDetect4285;
		else if(i==wNATO4529)
			m_CDetect4529 = new CDetect4529;
		else if(i==wMIL141B)
			m_CDetect141B = new CDetect141B;
		else if(i==wLINK11CLEW)
		{
			m_CDetectCLEW = new CDetectCLEW;
			m_CDetectCLEW->DetectCLEW_ini(nLeng,9600);
		}
		else if(i==wMIL141A)
		{
			m_CMIL141Apro = new CMIL141Apro;
			m_CMIL141Apro->MIL141Ademode_ini(insample,12000,8,1625,125,TRUE,nLeng,4);
		}
	}

	pBuf = ippsMalloc_32fc(2*nLeng);
	delayBuf = ippsMalloc_32fc(4*maxConvLen);
	outbyte = ippsMalloc_8u(nLeng);
	delayL = 0;
	BufPos = 0;
	
}
void CProtolDetect::ProtolDetect_free()
{
	
	ippsFree(pBuf);
	ippsFree(delayBuf);
	ippsFree(outbyte);

	if(m_Detect110A!=NULL)
	{
		delete m_Detect110A;
		m_Detect110A = NULL;
	}
	if(m_Detect110B!=NULL)
	{
		delete m_Detect110B;
		m_Detect110B = NULL;
	}
	if(m_CDetectSLEW!=NULL)
	{
		delete m_CDetectSLEW;
		m_CDetectSLEW = NULL;
	}
	if(m_CDetect4285!=NULL)
	{
		delete m_CDetect4285;
		m_CDetect4285 = NULL;
	}
	if(m_CDetect4529!=NULL)
	{
		delete m_CDetect4529;
		m_CDetect4529 = NULL;
	}
	if(m_CDetect141B!=NULL)
	{
		delete m_CDetect141B;
		m_CDetect141B = NULL;
	}
	if(m_CDetectCLEW!=NULL)
	{
		m_CDetectCLEW->DetectCLEW_free();
		delete m_CDetectCLEW;
		m_CDetectCLEW = NULL;
	}
	if(m_CReSample!=NULL)
	{
		m_CReSample->ReSample_Free();
		delete m_CReSample;
		m_CReSample = NULL;
	}
	if(m_CMIL141Apro!=NULL)
	{
		m_CMIL141Apro->MIL141Ademode_free();
		delete m_CMIL141Apro;
		m_CMIL141Apro = NULL;
	}

}
BOOL CProtolDetect::ProtolDetect(Ipp16s *pSrc,int nLeng,Ipp32f TH,BOOL *selProtol,int ProtolNum,ProtocolOut &m_ProtocolOut)
{
	int i,kname;
	int outLen,allLen;
	m_CReSample->ReSample(pSrc,nLeng,&pBuf[BufPos],outLen);
	allLen = BufPos + outLen;

	if(allLen<4*maxConvLen)// 防止输入数据不足
	{
		BufPos = allLen;
		return FALSE;
	}
	else
	{
		ippsCopy_32fc(delayBuf,pBuf,delayL);
		delayL = 4*maxConvLen;

		// 数据相关处理，数据在pBuf中，处理相关长度为  allLen-delayL
		int pIndex = 0,index = 0,proIndex = -1;
		Ipp32f maxCof = 0,cof = 0;
		Ipp32f decsumCLEW = 0;
		int indexCLEW = 0,index141A = 0;
		Ipp32f freCLEW = 0,fre141A = 0;
		Ipp32f frequency = 0,fre = 0;
		int dataRate = 0,interLeng = 0,waveType = 0;
		int mdataRate = 0,minterLeng = 0,mwaveType = 0;
		int bytelen = 0;
		
		int messagelen = 0;
		BOOL detect[10] = { FALSE };
		bool encrpe = false;
		BOOL pskdetect = FALSE;
		bool detect141A = false;
		for(i=0;i<ProtolNum;i++)
		{
			detect[i] = FALSE;
			if(!selProtol[i])
				continue;
			index = 0;
			cof = 0;
			fre = 0;
			mdataRate = 0;
			minterLeng = 0;
			mwaveType = 0;
			if (i==wMIL110A)
				m_Detect110A->Detect110A(pBuf,allLen-delayL,index,cof,fre,mdataRate,minterLeng,detect[i]);
			else if(i==wMIL110B)
				m_Detect110B->Detect110B(pBuf,allLen-delayL,index,cof,fre,mdataRate,minterLeng,detect[i]);
			else if(i==wLINK11SLEW)
				m_CDetectSLEW->DetectSLEW(pBuf,allLen-delayL,index,cof,fre,detect[i]);
			else if(i==wNATO4285)
				m_CDetect4285->Detect4285(pBuf,allLen-delayL,index,cof,fre,detect[i]);
			else if(i==wNATO4529)
				m_CDetect4529->Detect4529(pBuf,allLen-delayL,index,cof,fre,detect[i]);
			else if(i==wMIL141B)
				m_CDetect141B->Detect141B(pBuf,allLen-delayL,index,cof,fre,mwaveType,detect[i]);
			else if(i==wLINK11CLEW)
				m_CDetectCLEW->DetectCLEW(pBuf,allLen-delayL,1,indexCLEW,decsumCLEW,freCLEW);
			else if(i==wMIL141A)
			{
				// CString message[200];
				// CString address[200];
				std::vector<std::string> message(200);
				std::vector<std::string> address(200);
				bytelen = 0;
				messagelen = 0;
				m_CMIL141Apro->MIL141Ademode(pSrc,nLeng,outbyte,bytelen,message,address,messagelen);
				index141A = 0;
				fre141A = m_CMIL141Apro->f0;
				if(messagelen>0)
					detect141A = true;
			}
			else
				cof = 0;
			
			if(cof>maxCof)
			{
				maxCof = cof;
				pIndex = index;
				frequency = fre;
				proIndex = i;
				dataRate = mdataRate;
				interLeng = minterLeng;
				waveType = mwaveType;
				pskdetect = detect[i];
			}
		}
		if (!pskdetect)
		{
			if(decsumCLEW>1)
			{
				pIndex = indexCLEW;
				frequency = freCLEW;
				proIndex = wLINK11CLEW;
			}
			else if(detect141A)
			{
				pIndex = index141A;
				frequency = fre141A;
				proIndex = wMIL141A;
			}
		}

		ippsCopy_32fc(&pBuf[allLen-delayL],delayBuf,delayL);
		BufPos = delayL;

		if(proIndex < 0 || proIndex >= ProtolNum || !selProtol[proIndex])
			return FALSE;

		kname = proIndex;
		m_ProtocolOut.frequency = frequency;
		m_ProtocolOut.maxCor = maxCof;
		m_ProtocolOut.index = pIndex;

		if(kname==wNATO4285 || kname==wNATO4529)
		{
			if(maxCof>=(TH+0.01) && kname==wNATO4529)
			{
				m_ProtocolOut.ProtocolName = "NATO4529";
				return TRUE;
			}
			else if(maxCof>=(TH+0.2) && kname==wNATO4285)
			{
				m_ProtocolOut.ProtocolName = "NATO4285";
				return TRUE;
			}
			else
				return FALSE;
		}
		else if(kname==wLINK11CLEW)
		{
			m_ProtocolOut.maxCor = decsumCLEW;
			m_ProtocolOut.ProtocolName = "Link11CLEW";
			return TRUE;
		}
		else if(kname==wMIL141A)
		{
			m_ProtocolOut.ProtocolName = "MIL141A";
			m_ProtocolOut.dataRate = 125;
			return TRUE;
		}
		else if(kname==wMIL110A)
		{
			if((frequency>5 && frequency<1200) || (frequency>2100 && frequency<2400))
			{
				return FALSE;
			}
			m_ProtocolOut.ProtocolName = "MIL110A";
			m_ProtocolOut.dataRate  = dataRate;
			m_ProtocolOut.InterLen = interLeng;
			if(maxCof>0.5)
				return TRUE;
			else
				return FALSE;
		}
		else if(kname==wMIL110B)
		{
			if((frequency>5 && frequency<1200) || (frequency>2100 && frequency<2400))
			{
				return FALSE;
			}
			m_ProtocolOut.ProtocolName = "MIL110B";
			m_ProtocolOut.dataRate  = dataRate;
			m_ProtocolOut.InterLen = interLeng;
			if(maxCof>0.5)
				return TRUE;
			else
				return FALSE;
		}
		else if(kname==wLINK11SLEW)
		{
			if((frequency>5 && frequency<1200) || (frequency>2100 && frequency<2400))
			{
				return FALSE;
			}
			m_ProtocolOut.ProtocolName = "Link11SLEW";
			if(maxCof>0.5)
				return TRUE;
			else
				return FALSE;
		}
		else if(kname==wMIL141B)
		{
			if((frequency>5 && frequency<1200) || (frequency>2100 && frequency<2400))
			{
				return FALSE;
			}
			m_ProtocolOut.ProtocolName = "MIL141B";
			m_ProtocolOut.waveType = waveType;
			if(maxCof>0.5)
				return TRUE;
			else
				return FALSE;
		}
		else
			return FALSE;
	}

}
