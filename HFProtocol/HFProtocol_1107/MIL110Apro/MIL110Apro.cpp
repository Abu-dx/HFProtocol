// MIL110Apro.cpp : 定义 DLL 的初始化例程。

#include "stdafx.h"
#include "MIL110Apro.h"
#include <math.h>
#include "Preamble.h"
#include "MIL110ADecoder.h"
#include "Preambledetect.h"
#include "RLS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CMIL110Apro::CMIL110Apro(void)
{
	pPreambleSym = NULL;
	pC1C2C3 = NULL;
	pD1D2 = NULL;
	pTypeSym = NULL;
	probe = NULL;
	probeSym = NULL;

}

CMIL110Apro::~CMIL110Apro(void)
{
	if (pPreambleSym!=NULL)
	{
		ippsFree(pPreambleSym);
		pPreambleSym = NULL;
	}
	if (pD1D2!=NULL)
	{
		ippsFree(pD1D2);
		pD1D2 = NULL;
	}
	if (pC1C2C3!=NULL)
	{
		ippsFree(pC1C2C3);
		pC1C2C3 = NULL;
	}

	if (probe!=NULL)
	{
		ippsFree(probe);
		probe = NULL;
	}
	if (probeSym!=NULL)
	{
		ippsFree(probeSym);
		probeSym = NULL;
	}
	
	if (pTypeSym!=NULL)
	{	
		for (int i=0;i<8;i++)
		{
			ippsFree(pTypeSym[i]);
		}
		delete pTypeSym;
		pTypeSym = NULL;
	}

}
void CMIL110Apro::PSK_map(short style, short M, float I_map[],float Q_map[])
{
	switch(M)
	{
	case 2:
		if(style==0)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=-1;Q_map[1]=0;
		}
		else if(style==1)
		{
			I_map[0]=-1;Q_map[0]=0;
			I_map[1]=1;Q_map[1]=0;
		}
		break;
	case 4:
		if(style==0)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=0;Q_map[1]=1;
			I_map[2]=0;Q_map[2]=-1;
			I_map[3]=-1;Q_map[3]=0;
		}
		else if(style==1)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=0;Q_map[1]=1;
			I_map[2]=-1;Q_map[2]=0;
			I_map[3]=0;Q_map[3]=-1;
		}
		break;
	case 8:
		if(style==0)
		{
			I_map[0]=1;		Q_map[0]=0;
			I_map[1]=0.707; Q_map[1]=0.707;
			I_map[2]=-0.707;Q_map[2]=0.707;
			I_map[3]=0;		Q_map[3]=1;
			I_map[4]=0.707;	Q_map[4]=-0.707;
			I_map[5]=0;		Q_map[5]=-1;
			I_map[6]=-1;	Q_map[6]=0;
			I_map[7]=-0.707;Q_map[7]=-0.707;
		}
		else if(style==1)
		{
			I_map[0]=1;		Q_map[0]=0;
			I_map[1]=0.707; Q_map[1]=0.707;
			I_map[2]=0;		Q_map[2]=1;
			I_map[3]=-0.707;Q_map[3]=0.707;
			I_map[4]=-1;	Q_map[4]=0;
			I_map[5]=-0.707;Q_map[5]=-0.707;
			I_map[6]=0;		Q_map[6]=-1;
			I_map[7]=0.707;	Q_map[7]=-0.707;
		}
		break;
	default:
		break;
	}
}
int CMIL110Apro::PSK_MOD( short data[], short data_len, short M,	 Ipp32fc *out)
{
	int i;
	float *I_map,*Q_map;
	I_map = new float[M];
	Q_map = new float[M];

	PSK_map(1,M,I_map,Q_map);  // style = 1 顺序映射
	for (i=0; i<data_len; i++)
	{
		out[i].re = I_map[data[i]];
		out[i].im = Q_map[data[i]];
	}
	free(I_map);
	free(Q_map);
	return(0);
}
void CMIL110Apro::Preamble_Gen()
{
	int i,j,k;
	UWleng = 480;

	pPreambleSym = ippsMalloc_32fc(480);
	PSK_MOD(preamble_110A,288,8,pPreambleSym);
	UWEnergy = 1;

	pD1D2 = ippsMalloc_32fc(64);
	pC1C2C3 = ippsMalloc_32fc(96);

	pTypeSym = new Ipp32fc *[8];
	for (int i=0;i<8;i++)
	{
		pTypeSym[i] = ippsMalloc_32fc(32);
		PSK_MOD(type_110A[i],32,8,pTypeSym[i]);
	}
	ippsCopy_32fc(pTypeSym[0],&pPreambleSym[448],32);

	probe = ippsMalloc_16s(32);
	probeSym = ippsMalloc_32fc(32);

}

void CMIL110Apro::DownFre_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,int FFTLen,Ipp32f roll,Ipp32f Baud,Ipp16s P,int SrctapLen)
{
	pfre = fre/Outsample;
	m_Preambledetect->Preamble_Gen();
	m_Preambledetect->ProtolDetect_ini(nLeng,m_Preambledetect->PreambleLen,FFTLen,Insample,Outsample,P);

	pSrcTap = ippsMalloc_64f(SrctapLen);
	Ipp64f fstop = (Baud*(1+0.75))/(Outsample*2);//滤波器带宽要适当大些，
	ippsFIRGenLowpass_64f(fstop,pSrcTap,SrctapLen,ippWinHamming,ippTrue);

	pReal = ippsMalloc_32f(nLeng*2); 
	pImag = ippsMalloc_32f(nLeng*2); 
	pDelayreal=ippsMalloc_32f(SrctapLen);
	pDelayimag=ippsMalloc_32f(SrctapLen);
	ippsSet_32f(0,pDelayreal,SrctapLen);
	ippsSet_32f(0,pDelayimag,SrctapLen);
	ippsFIRInitAlloc64f_32f(&SrcStateReal, pSrcTap, SrctapLen, pDelayreal);
	ippsFIRInitAlloc64f_32f(&SrcStateImag, pSrcTap, SrctapLen, pDelayimag);	

}
void CMIL110Apro::DownFre_free()
{
	ippsFree(pSrcTap);
	ippsFree(pDelayreal);
	ippsFree(pDelayimag);
	ippsFIRFree64f_32f(SrcStateReal);
	ippsFIRFree64f_32f(SrcStateImag);
	ippsFree(pReal);
	ippsFree(pImag);
	m_Preambledetect->ProtolDetect_free();

}
void CMIL110Apro::DownFre(Ipp16s *pSrc,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,Ipp32fc *pDst,int &DstLen)
{
	int i;
	int BurstNum;
	int BufHavePro = 0;
	int UWleng = m_Preambledetect->PreambleLen;
	Ipp16s *BurstPos = ippsMalloc_16s(nLeng);
	Ipp32f *fre = ippsMalloc_32f(nLeng);
	int OutLen;

	m_Preambledetect->ReSample(pSrc,nLeng,&m_Preambledetect->pBuf[m_Preambledetect->BufPos],OutLen);
	int dataLen = OutLen+m_Preambledetect->BufPos;
	m_Preambledetect->Burst_detect(dataLen,m_Preambledetect->pPreambleSym,1,m_Preambledetect->PreambleLen,P,FFTLen,TH1,TH2,
		BurstPos,fre,BurstNum,m_Preambledetect->BufPos,BufHavePro);

	int pburst=0;
	for (i=0;i<BufHavePro;i++)
	{
		if(i==BurstPos[pburst] && BurstNum>0)
		{
			pfre = fre[pburst]/P;
			pburst++;
		}
		defrephase=defrephase-IPP_2PI*pfre;
		if ((defrephase)>=IPP_2PI || (defrephase)<=-IPP_2PI)
			defrephase=fmod((double)defrephase,(double)IPP_2PI);
		pReal[i]=m_Preambledetect->pBuf[i]*cos(defrephase);
		pImag[i]=m_Preambledetect->pBuf[i]*sin(defrephase);
	}
	ippsFIR64f_32f_I(pReal, BufHavePro, SrcStateReal);
	ippsFIR64f_32f_I(pImag, BufHavePro, SrcStateImag);
	ippsRealToCplx_32f(pReal,pImag,pDst,BufHavePro);
	DstLen = BufHavePro;
	ippsFree(BurstPos);
	ippsFree(fre);
}

/************************************************************************/
/*   abs(mean(a.*conj(b)))/sqrt(mean(a.*conj(a))*mean(b.*conj(b)));                                                                   */
/************************************************************************/

void CMIL110Apro::CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f *cConv)
{
	
	Ipp32fc pMeanAB;
	Ipp32f pMeanA;

	ippsConj_32fc(pSrcB,pConvAB,nLeng);
	ippsMul_32fc_I(pSrcA,pConvAB,nLeng);
	ippsMean_32fc(pConvAB,nLeng,&pMeanAB,ippAlgHintFast);

	//if(pMeanAB.re>1000)
	//{
	//	*cConv = 0;
	//	return;
	//}

	ippsPowerSpectr_32fc(pSrcA,pConv,nLeng);
	ippsMean_32f(pConv,nLeng,&pMeanA,ippAlgHintFast);

	*cConv = sqrt(pMeanAB.re*pMeanAB.re + pMeanAB.im*pMeanAB.im)/sqrt(pMeanA*bEnergy);
}
/************************************************************************/
/*   allLen		  数据块总长度，包括上一数据块结尾和本次数据块
	 proBuf		  数据块缓存,包括上一数据块结尾和本次数据块
	 winBufConv   相关运算结果值缓存
	 pConvAB,pConv  相关运算中间缓存
/************************************************************************/
void CMIL110Apro::Burst_detect_ini(int allLen,int convLen,int deciP,int mproBegin)
{
	winBufConv = ippsMalloc_32f(allLen);
	ippsZero_32f(winBufConv,allLen);
	proBuf = ippsMalloc_32fc(allLen);

	delayL = (convLen+4)*deciP+20;
	delayBuf = ippsMalloc_32fc(delayL);
	delayL = 0;

	pConvAB = ippsMalloc_32fc(convLen);
	pConv = ippsMalloc_32f(convLen);

	proBegin = mproBegin;
	BufPos = 0;
}

void CMIL110Apro::Burst_detect_free()
{
	ippsFree(winBufConv);
	ippsFree(proBuf);
	ippsFree(delayBuf);

	ippsFree(pConvAB);
	ippsFree(pConv);	
}
/************************************************************************/
/*  proBegin	  缓存起始点
	proLen        proBuf中的数据总长度，包括上一数据块结尾和本次数据块
	deciP		  抽取倍数
	convLen		  相关数据长度
	BurstPos	  检测到的突发位置
	BurstNum	  检测到的突发个数
	
	BufPos	      下一数据块的存放起始地址
	BufHavePro	  本次已处理数据长度
	
	先突发检测，检测到信号后进行110A类型识别, 
	convLen长度为480
	检测相关长度为288，
	交织长度识别相关长度为64
	计数器识别相关长度为96
/************************************************************************/
void CMIL110Apro::Burst_detect(int proBegin,int proLen,Ipp32fc *UWsig,Ipp32f UWEnergy, Ipp16s convLen,Ipp16s deciP,
							Ipp32f Threshold,Ipp16s *BurstPos,Ipp16s *nD,Ipp16s *nC,int &BurstNum,int &BufPos,int &BufHavePro)
{
	Ipp16s i,j,maxidx;
	int pPhase=0;
	int pDstLen=0;
	Ipp32fc *pSrcA = ippsMalloc_32fc(convLen);
	Ipp32f cconv;

	int convdataL = convLen*deciP;
	int compareL = 4*deciP;
	int winBufp = 0;
	Ipp32f pMax;
	int pIndex;

	int detectconvL = 288;
	int typeconvL = 64;
	int countconvL = 96;
	int type;	
	BurstNum=0;

	ippsCopy_32fc(delayBuf,&proBuf[proBegin],delayL);
	for (i=0;i<proLen - convdataL;i++)
	{
		ippsSampleDown_32fc(&proBuf[proBegin+i],convdataL,pSrcA,&pDstLen,deciP,&pPhase);// 间隔P点抽出数据
		CrossConvCof(pSrcA,UWsig,detectconvL,UWEnergy,&cconv);// 相关系数
		winBufConv[winBufp] = cconv;	
		winBufp++;
	}

	for (i=0;i<winBufp-compareL;i++)
	{
		if (winBufConv[i]>Threshold && winBufConv[i+1]>Threshold)//检测到信号
		{
			ippsMaxIndx_32f(&winBufConv[i],compareL,&pMax,&pIndex);
			BurstPos[BurstNum] = i+pIndex;

			// 交织类型识别
			ippsSampleDown_32fc(&proBuf[proBegin+BurstPos[BurstNum]+detectconvL*deciP],(typeconvL+countconvL)*deciP,pSrcA,&pDstLen,deciP,&pPhase);// 抽取数据以便类型识别		
			IdentifyD1D2(pSrcA,13,type);
			nD[BurstNum] = type;
			IdentifyC1C2C3(&pSrcA[64],24,type);
			nC[BurstNum] = type;
		//	TRACE("%d_%d/",nD,nC);

			BurstNum++;
			i = i+compareL;
		}
	}
	delayL = convdataL + compareL;
	ippsCopy_32fc(&proBuf[proBegin+proLen-delayL],delayBuf,delayL);// 结尾数据复制，以便下次处理

	ippsFree(pSrcA);
	BufHavePro = winBufp-compareL;
	BufPos = delayL;

}
void CMIL110Apro::IdentifyD1D2(Ipp32fc *pSrc,int typeNum,int &outtype)
{
	int i,idx;
	float maxcof=0,cof;
	for (i=0;i<typeNum;i++)
	{
		ippsCopy_32fc(pTypeSym[D1D2[i][0]],pD1D2,32);
		ippsCopy_32fc(pTypeSym[D1D2[i][1]],&pD1D2[32],32);

		CrossConvCof(pSrc,pD1D2,64,1,&cof);// 相关系数
		if (cof>maxcof)
		{
			maxcof = cof;
			idx = i;
		}
	}
	outtype = idx;	
}
void CMIL110Apro::IdentifyC1C2C3(Ipp32fc *pSrc,int typeNum,int &outtype)
{
	int i,idx;
	float maxcof=0,cof;
	for (i=0;i<typeNum;i++)
	{
		ippsCopy_32fc(pTypeSym[C1C2C3[i][0]],pC1C2C3,32);
		ippsCopy_32fc(pTypeSym[C1C2C3[i][1]],&pC1C2C3[32],32);
		ippsCopy_32fc(pTypeSym[C1C2C3[i][2]],&pC1C2C3[64],32);
	
		CrossConvCof(pSrc,pC1C2C3,96,1,&cof);// 相关系数
		if (cof>maxcof)
		{
			maxcof = cof;
			idx = i;
		}
	}
	outtype = idx;
}
void CMIL110Apro::IdentifyInter(Ipp32fc *pSrc,int typeNum,Ipp16s *probe,int probelen,int &outtype,Ipp32fc *outprobesym)
{
	int i,j,idx=4;
	float maxcof=0,cof;
	Ipp16s *temp = ippsMalloc_16s(probelen);
	for (i=4;i<typeNum;i++)
	{
		for (j=0;j<probelen;j++)
			temp[j] =(walshcode[i][j] + probe[j])%8;
		
		PSK_MOD(temp,probelen,8,pD1D2);
		CrossConvCof(pSrc,pD1D2,probelen,1,&cof);// 相关系数
		if (cof>maxcof)
		{
			maxcof = cof;
			idx = i;
		}
	}
	outtype = idx;

	for (j=0;j<probelen;j++)
		temp[j] =(walshcode[idx][j] + probe[j])%8;
	PSK_MOD(temp,probelen,8,outprobesym);

	ippsFree(temp);

}

void CMIL110Apro::Demode_PSKrealFSE_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,Ipp16s P,int FFTLen,Ipp32f roll,Ipp32f Baud,int SrctapLen)
{
	int i,j,allLen;
	m_Preambledetect = new CPreambledetect;
	m_RLS = new RLS;

	Preamble_Gen();
	SrcLen = SrctapLen;
	DownFre_ini(Insample,Outsample,nLeng,fre,FFTLen,roll,Baud,P,SrctapLen);	
	OneFrameLen = (UWleng+5)*P;  //  一段帧长,包括FSE的延时，与FSE阶数有关
	OneFrame = ippsMalloc_32fc(OneFrameLen);
	allLen = nLeng*2 + (UWleng+10)*P + OneFrameLen*2;	
	Burst_detect_ini(allLen,UWleng,P,OneFrameLen*2);
	
	Timing_ini();
	defrephase = 0;
	frephase = 0;
	pllphase = 0;
	probeNum = 0;
	pBuf = 0;
	pBurst = 0;
	flag = 0;

	pfre = fre/Outsample;

	EqualizationRLS_DFE_PLL_ini(0.01,0.707,8,pllphase,18,15,100,0.99);
	SaveTobit_ini();

	pDecode = new MIL110ADecoder;
	pBufInter = ippsMalloc_8u(23040+1000);
	InterLength = 0;

}
void CMIL110Apro::Demode_PSKrealFSE_free()
{
	ippsFree(OneFrame);
	Burst_detect_free();
	DownFre_free();
	EqualizationRLS_DFE_PLL_free();
	delete pDecode;
	delete m_Preambledetect;
	ippsFree(pBufInter);
	delete m_RLS;

}
void CMIL110Apro::Demode_PSKrealFSE(Ipp16s *pSrcDst,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,
									int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType110A *headType,int &headNum)
{
	int i,j;
	Ipp16s *BurstPos = ippsMalloc_16s(nLeng/UWleng*2+1);
	Ipp16s *nD = ippsMalloc_16s(nLeng/UWleng*2+1);
	Ipp16s *nC = ippsMalloc_16s(nLeng/UWleng*2+1);
	int BurstNum;
	int BufHavePro = 0;	
	int M;
	Ipp16s num=0;

	int dataLen;
	DownFre(pSrcDst,nLeng,P,FFTLen,TH1,TH2,&proBuf[proBegin+BufPos],dataLen);
	dataLen = dataLen+BufPos;
	Burst_detect(proBegin,dataLen,pPreambleSym,1,UWleng,P,TH2,BurstPos,nD,nC,BurstNum,BufPos,BufHavePro);
	DeHead110A(BurstPos,nD,BurstNum,headType,headNum);
	
	int TimeOutLen,mbyteleng;
	outLeng = 0;
	byteLeng = 0;
	pBurst = 0;
	////////////
	while(pBuf + (UWleng+5)*P<BufHavePro)
	{
		TimeOutLen = 0;
		if (flag==0 && BurstNum>0 && pBuf<=BurstPos[pBurst] && BurstPos[pBurst]<=pBuf+OneFrameLen)
		{
			nInterleaver = nD[pBurst];
			nCount = nC[pBurst];
			if (nCount==1 || nCount==20 )// 短交织，计数器为1；长交织，计数器为22
			{
				// 选择类型码复制到前导中		
				CopyInterToPreamble(nInterleaver);
				CopyCountToPreamble(nCount);
				flag =1;
				pBuf = BurstPos[pBurst];
			}
			else
			{
				flag=0;
				pBurst++;
				BurstNum--;
				pBuf = pBuf + OneFrameLen;
			}
		}
		else if (flag==1 &&  (nCount==1 || nCount==20))// 找到前导进行同步估计
		{
			// 同步初始化
			OneFrameLen =  (UWleng+5)*P;
		 	Synchronization_PSK(&proBuf[proBegin+pBuf],OneFrameLen,P,pPreambleSym,UWleng,5,OneFrame,TimeOutLen,&outbyte[byteLeng],mbyteleng,1);
 			byteLeng = byteLeng +mbyteleng;
			pBuf = pBuf + OneFrameLen;
			OneFrameLen = UWleng*P;
			flag = 2;
			nCount--;
			pBurst++;
			BurstNum--;
		}
		else if (flag==2 && nCount>=0)
		{
			CopyCountToPreamble(nCount);// 更新前导
			// 数据辅助均衡
			Synchronization_PSK(&proBuf[proBegin+pBuf],OneFrameLen,P,pPreambleSym,UWleng,5,OneFrame,TimeOutLen,&outbyte[byteLeng],mbyteleng,2);
			byteLeng = byteLeng +mbyteleng;
			pBuf = pBuf + OneFrameLen;
			nCount--;
			if (nCount<0)
				flag=3;
			pBurst++;
			BurstNum--;
		}
		//  找到下一个前导
		else if (flag==3 && BurstNum>0 && pBuf<=BurstPos[pBurst] && BurstPos[pBurst]<=pBuf+OneFrameLen)
		{
			TimeOutLen = 0;
			flag=0;
		}
		else if (flag==3)  // 同步跟踪
		{
			Synchronization_PSK(&proBuf[proBegin+pBuf],OneFrameLen,P,pPreambleSym,UWleng,5,OneFrame,TimeOutLen,&outbyte[byteLeng],mbyteleng,0);
			byteLeng = byteLeng +mbyteleng;				
			pBuf = pBuf + OneFrameLen;
		}
		else
			pBuf = pBuf + OneFrameLen;

		// 同步输出
		if (TimeOutLen>0)
		{
			for (i=0;i<TimeOutLen;i++)
			{
				pSrcDst[outLeng + 2*i]=Ipp16s(OneFrame[i].re*1000);
				pSrcDst[outLeng + 2*i+1]=Ipp16s(OneFrame[i].im*1000);
			}
			outLeng = outLeng + 2*TimeOutLen;
		}                       
	}
	if (flag==0 && BurstNum>0)
	{
		nInterleaver = nD[pBurst];
		nCount = nC[pBurst];
		if (nCount==1 || nCount==20 )// 短交织，计数器为1；长交织，计数器为22
		{
			// 选择类型码复制到前导中		
			CopyInterToPreamble(nInterleaver);
			CopyCountToPreamble(nCount);
			flag =1;
			pBuf = BurstPos[pBurst];
		}
		else
		{
			flag=0;
			pBuf = BurstPos[pBurst];
			pBurst++;
			BurstNum--;
		}
	}
	
	// 处理缓存结尾, 将结尾数据复制到缓存头部，以备下次使用 
    ippsCopy_32fc(&proBuf[proBegin+pBuf],&proBuf[proBegin-(BufHavePro-pBuf)],BufHavePro-pBuf);
	pBuf = -(BufHavePro-pBuf);
	ippsFree(BurstPos);ippsFree(nD);ippsFree(nC);
}

/************************************************************************/
/*  数据辅助的同步处理

	bBegin=1, 有同步头，同步参数估计
	bBegin=0，同步跟踪处理

	bBegin==1时
		FrameLen =(UWleng+delay)*P
	bBegin==0时
		FrameLen =UWleng*P
	
/************************************************************************/
void CMIL110Apro::Synchronization_PSK(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp16s P,Ipp32fc *pUWsig,Ipp16s UWleng,int delayCma,
									  Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng,int bBegin)
{
	int i,j,k,p=0,D;
	int pPhase=0;
	int pDownSLen=0;
	Ipp32f energy=0;
	int mbyteleng=0;
	byteleng = 0;
	Ipp8u *pJudge = ippsMalloc_8u(FrameLen/P+20);
	
	int minterlen;
	Ipp32fc *pDownS = ippsMalloc_32fc(FrameLen/2+20);
	Ipp32fc *pProbe;

	if(bBegin==1)// 前导
	{	
		ParaEstimate(FrameBuf,FrameLen,P,pUWsig,mfre,pllphase,pEnergy);
		ippsSampleDown_32fc(FrameBuf,FrameLen,pDownS,&pDownSLen,2,&pPhase);

		frephase = 0;
		mfre = mfre/2;
		Remove_fre(pDownS,pDownSLen,mfre);
		m_RLS->RLS_SetZero();
		EqualizationRLS_DFE_PLL(pDownS,pDownSLen,2,delayCma*2,pEnergy,8,pUWsig,pDst,outleng,TRUE);	
		probeNum = 0;	
		InterLength = 0;

		for (i=0;i<outleng;i++)
			pDst[i] = pDst[i+delayCma];	

	}
	else if(bBegin==2)// 已知序列辅助
	{
		ippsSampleDown_32fc(FrameBuf,FrameLen,pDownS,&pDownSLen,2,&pPhase);
		Remove_fre(pDownS,pDownSLen,mfre);	
		EqualizationRLS_DFE_PLL(pDownS,pDownSLen,2,0,pEnergy,8,pUWsig,pDst,outleng,TRUE);	// 辅助
		
	}
	else if(bBegin==0)// 数据帧 
	{
		if (nInterleaver>10)// 75
		{
			m_RLS->RLSFilter_ini();
			process75B(FrameBuf,FrameLen,pUWsig,UWleng,pDst,outleng,outbyte,byteleng);
		}
		else if (nInterleaver>2)// 1200 --- 150  （20个未知+20个已知）
			Process2020(FrameBuf,FrameLen,pUWsig,UWleng,pDst,outleng,outbyte,byteleng);
		else		// 2400-4800  (32个未知+16个已知) // 判决反馈均衡
			Process3216(FrameBuf,FrameLen,pUWsig,UWleng,pDst,outleng,outbyte,byteleng);
	}
	ippsFree(pJudge);
	ippsFree(pDownS);
}
void CMIL110Apro::process75B(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp32fc *pUWsig,Ipp16s UWleng,
							 Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng)
{
	int i,j,p=0,k;
	int pPhase=0;
	int pDownSLen;
	int moutlen;
	int minterlen;
	int mbyteleng=0;
	byteleng = 0;
	Ipp32f energy;
	Ipp32fc *pDownS = ippsMalloc_32fc(FrameLen/2+20);

	ippsSampleDown_32fc(FrameBuf,FrameLen,pDownS,&pDownSLen,2,&pPhase);
	Remove_fre(pDownS,pDownSLen,mfre);	
	for(i=0;i<15;i++)  // 480 = 15*32
	{
		energy = 0;
		for(j=0;j<pDownSLen;j++)
			energy = energy + sqrt(pDownS[j].re*pDownS[j].re+pDownS[j].im*pDownS[j].im)/pDownSLen;
		if(energy<pEnergy*0.5)
		{
			flag=0;
			return;
		}

		EqualizationFilter(&pDownS[i*64],64,2,pEnergy,&pDst[outleng],moutlen);
		for (j=0;j<32;j++)// 获得扰码
		{
			k = probeNum*32+j;
			k = k%160;
			probe[j] =probe_sequence[k];
		}
		Mapfor75bps(&pDst[i*32],probe,32,nInterleaver,probeNum,probeSym,&pBufInter[InterLength],minterlen);
		InterLength = InterLength+ minterlen;

		// 利用信道探测序列均衡
		EqualizationRLS_DFE_PLL(&pDownS[i*64],64,2,0,pEnergy,8,probeSym,&pDst[outleng],moutlen,TRUE);
		outleng = outleng + moutlen;

		probeNum++;
	}
	ippsFree(pDownS);
	//  解交织
	if (nInterleaver==11 && InterLength==720)  // 75L
	{
		DeInterleaverDecode(nInterleaver,InterLength,minterlen);
		if (minterlen>0)
		{
			SaveTobit(pBufInter,minterlen,&outbyte[byteleng],mbyteleng,1);
			byteleng = byteleng + mbyteleng;
		}
		InterLength = 0;
	}
	else if (nInterleaver==12 && InterLength==90) // 75S
	{
		DeInterleaverDecode(nInterleaver,InterLength,minterlen);
		if (minterlen>0)
		{
			SaveTobit(pBufInter,minterlen,&outbyte[byteleng],mbyteleng,1);
			byteleng = byteleng + mbyteleng;
		}
		InterLength = 0;
	}
}
void CMIL110Apro::Process2020(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp32fc *pUWsig,Ipp16s UWleng,
							  Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng)
{
	int i,j,p=0,k,D;
	int pPhase=0;
	Ipp32fc *pDownS = ippsMalloc_32fc(FrameLen/2+20);
	Ipp8u *pJudge = ippsMalloc_8u(FrameLen/2+20);
	int pDownSLen;
	int moutlen;
	int minterlen;
	byteleng = 0;
	Ipp32f energy;
	int mbyteleng=0;

	ippsSampleDown_32fc(FrameBuf,FrameLen,pDownS,&pDownSLen,2,&pPhase);
	Remove_fre(pDownS,pDownSLen,mfre);	
	while(p+80<=pDownSLen)
	{
		energy = 0;
		for(i=0;i<pDownSLen;i++)                              
			energy = energy + sqrt(pDownS[i].re*pDownS[i].re+pDownS[i].im*pDownS[i].im)/pDownSLen;
		if(energy<pEnergy*0.5)
		{
			flag=0;
			break;
		}
		//{未知数据
		EqualizationRLS_DFE_PLL(&pDownS[p],40,2,0,pEnergy,8,pUWsig,&pDst[outleng],moutlen,FALSE);
		//  去扰
		for (i=0;i<20;i++)
		{
			k = probeNum*40+i;
			k = k%160;
			probe[i] =probe_sequence[k];
		}
		PSK_MOD(probe,20,8,probeSym);
		ippsConj_32fc_I(probeSym,20);
		ippsMul_32fc_I(probeSym,&pDst[outleng],20);
		judge(&pDst[outleng],20,pJudge,4);	
		outleng = outleng + moutlen;
		if(nInterleaver>4 && nInterleaver<=10)// 150--600  BPSK
		{
			MGDecode(pJudge,20,&pBufInter[InterLength],minterlen,1);
			InterLength = InterLength+minterlen;
		}
		else // 1200 QPSK
		{
			MGDecode(pJudge,20,&pBufInter[InterLength],minterlen,2);
			InterLength = InterLength+minterlen;
		}
		//}
        //{信道探测
		if(((nInterleaver%2)==0 && (probeNum%36==34)) || ((nInterleaver%2)==1 && (probeNum%288==286))){
			for (i=0;i<20;i++)// 获得信道探测序列
			{
				k = 20+probeNum*40+i;
				k = k%160;
				probe[i] = (walshcode[D1D2[nInterleaver][0]][i] + probe_sequence[k])%8;
			}
			PSK_MOD(probe,20,8,probeSym);
		}
		else if(((nInterleaver%2)==0 && (probeNum%36==35)) || ((nInterleaver%2)==1 && (probeNum%288==287))){
			for (i=0;i<20;i++)// 获得信道探测序列
			{
				k = 20+probeNum*40+i;
				k = k%160;
				probe[i] = (walshcode[D1D2[nInterleaver][1]][i] + probe_sequence[k])%8;
			}
			PSK_MOD(probe,20,8,probeSym);
		}
		else{
			for (i=0;i<20;i++)// 获得信道探测序列
			{
				k = 20+probeNum*40+i;
				k = k%160;
				probe[i] =probe_sequence[k];
			}
			PSK_MOD(probe,20,8,probeSym);
		}
		// 利用信道探测序列均衡
		EqualizationRLS_DFE_PLL(&pDownS[p+40],20*2,2,0,pEnergy,8,probeSym,&pDst[outleng],moutlen,TRUE);
		ippsConj_32fc_I(probeSym,20);
		ippsMul_32fc_I(probeSym,&pDst[outleng],20);
		judge(&pDst[outleng],20,pJudge,4);
		outleng = outleng + moutlen;
		//}信道探测
		// {解交织
		// 到交织结尾解交织
		if ((nInterleaver%2)==0 && (probeNum%36==35)) // 短交织情况，交织块长720比特，即36个子帧
		{
			DeInterleaverDecode(nInterleaver,InterLength,minterlen);
			if (minterlen>0)
			{
				SaveTobit(pBufInter,minterlen,&outbyte[byteleng],mbyteleng,1);
				byteleng = byteleng + mbyteleng;
			}
			InterLength = 0;
		}
		else if((nInterleaver%2)==1 && (probeNum%288==287))//长交织情况，交织块长5760比特，即288个子帧
		{
			DeInterleaverDecode(nInterleaver,InterLength,minterlen);
			if (minterlen>0)
			{
				SaveTobit(pBufInter,minterlen,&outbyte[byteleng],mbyteleng,1);
				byteleng = byteleng + mbyteleng;
			}
			InterLength = 0;
		}
		//}解交织
		probeNum++;
		p = p + 80;
	}
	ippsFree(pDownS);
	ippsFree(pJudge);
}
void CMIL110Apro::Process3216(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp32fc *pUWsig,Ipp16s UWleng,
							  Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng)
{
	int i,j,p=0,k,D;
	int pPhase=0;
	Ipp32fc *pDownS = ippsMalloc_32fc(FrameLen/2+20);
	Ipp8u *pJudge = ippsMalloc_8u(FrameLen/2+20);
	int pDownSLen;
	int moutlen;
	int minterlen;
	byteleng = 0;
	int mbyteleng=0;
	Ipp32f energy;

	ippsSampleDown_32fc(FrameBuf,FrameLen,pDownS,&pDownSLen,2,&pPhase);
	Remove_fre(pDownS,pDownSLen,mfre);	
	while(p+96<=pDownSLen)
	{
		energy = 0;
		for(i=0;i<pDownSLen;i++)
			energy = energy + sqrt(pDownS[i].re*pDownS[i].re+pDownS[i].im*pDownS[i].im)/pDownSLen;
		if(energy<pEnergy*0.5)
		{
			flag=0;
			break;
		}
		//{未知数据
		EqualizationRLS_DFE_PLL(&pDownS[p],64,2,0,pEnergy,8,pUWsig,&pDst[outleng],moutlen,FALSE);
		//  去扰
		for (i=0;i<32;i++)
		{
			k = probeNum*48+i;
			k = k%160;
			probe[i] =probe_sequence[k];
		}
		PSK_MOD(probe,32,8,probeSym);
		ippsConj_32fc_I(probeSym,32);
		ippsMul_32fc_I(probeSym,&pDst[outleng],32);
		judge(&pDst[outleng],32,pJudge,4);	
		outleng = outleng + moutlen;

		MGDecode(pJudge,32,&pBufInter[InterLength],minterlen,3);
		InterLength = InterLength+minterlen;

		//}
		//{信道探测
		if(((nInterleaver%2)==0 && (probeNum%30==28)) || ((nInterleaver%2)==1 && (probeNum%240==238))){
			for (i=0;i<16;i++)// 获得信道探测序列
			{
				k = 32+probeNum*48+i;
				k = k%160;
				probe[i] = (walshcode[D1D2[nInterleaver][0]][i] + probe_sequence[k])%8;
			}
			PSK_MOD(probe,16,8,probeSym);
		}
		else if(((nInterleaver%2)==0 && (probeNum%30==29)) || ((nInterleaver%2)==1 && (probeNum%240==239))){
			for (i=0;i<16;i++)// 获得信道探测序列
			{
				k = 32+probeNum*48+i;
				k = k%160;
				probe[i] = (walshcode[D1D2[nInterleaver][1]][i] + probe_sequence[k])%8;
			}
			PSK_MOD(probe,16,8,probeSym);
		}
		else{
			for (i=0;i<16;i++)// 获得信道探测序列
			{
				k = 32+probeNum*48+i;
				k = k%160;
				probe[i] =probe_sequence[k];
			}
			PSK_MOD(probe,16,8,probeSym);
		}
		// 利用信道探测序列均衡
		EqualizationRLS_DFE_PLL(&pDownS[p+64],16*2,2,0,pEnergy,8,probeSym,&pDst[outleng],moutlen,TRUE);
		ippsConj_32fc_I(probeSym,16);
		ippsMul_32fc_I(probeSym,&pDst[outleng],16);
		judge(&pDst[outleng],16,pJudge,4);
		outleng = outleng + moutlen;
		//}信道探测
		// {解交织
		// 到交织结尾解交织
		if ((nInterleaver%2)==0 && (probeNum%30==29)) // 短交织情况，交织块长1440个符号，即30个子帧
		{
			DeInterleaverDecode(nInterleaver,InterLength,minterlen);
			if (minterlen>0)
			{
				SaveTobit(pBufInter,minterlen,&outbyte[byteleng],mbyteleng,1);
				byteleng = byteleng + mbyteleng;
			}
			InterLength = 0;
		}
		else if((nInterleaver%2)==1 && (probeNum%240==239))//长交织情况，交织块长11520个符号，即240个子帧
		{
			DeInterleaverDecode(nInterleaver,InterLength,minterlen);
			if (minterlen>0)
			{
				SaveTobit(pBufInter,minterlen,&outbyte[byteleng],mbyteleng,1);
				byteleng = byteleng + mbyteleng;
			}
			InterLength = 0;
		}
		//}解交织
		probeNum++;
		p = p + 96;
	}
	ippsFree(pDownS);
	ippsFree(pJudge);
}
BOOL CMIL110Apro::FindEnd(Ipp8u *pBufInter,int InterLen)
{
	int i,j;
	int num = 0;
	Ipp8u EOM[31] = {1,0,0,1,0,1,1,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,1,0,0,1,0};
	for (i=0;i<InterLen-31;i++)
	{
		num = 0;
		for(j=0;j<31;j++)
			if(pBufInter[i+j] ==EOM[j])
				num++;
		if(num==31)
			return TRUE;
		TRACE("%d_",num);
	}
	return FALSE;
}
void CMIL110Apro::Mapfor75bps(Ipp32fc *pSrc,Ipp16s *scramble,int scrlen,int ninter,int setcount,Ipp32fc *probeSym,Ipp8u *pDst,int &outleng)
{
	int i,n,j;
	float cof,maxcof=0;
	int idx = 0;
	cof = 0;
	int bes = 0;
	if(ninter==12 && setcount%45==44)// 在交织块的结尾用exceptional模式
 		bes = 4;
	else if(ninter==11 && setcount%360==359)// 在交织块的结尾用exceptional模式，其他用normal sets
		bes = 4;
	else
		bes =0;
		
	Ipp16s *temp = ippsMalloc_16s(32);
	for(j=bes+0;j<bes+4;j++)
	{
		for (i=0;i<scrlen;i++)
			temp[i] =(walshcode_75[j][i] + scramble[i])%8;
		PSK_MOD(temp,scrlen,8,pD1D2);
		CrossConvCof(pSrc,pD1D2,scrlen,1,&cof);// 相关系数
		if (cof>maxcof)
		{
			maxcof = cof;
			idx = j;
		}
	}

	for (i=0;i<scrlen;i++)
		temp[i] =(walshcode_75[idx][i] + scramble[i])%8;
	PSK_MOD(temp,scrlen,8,probeSym);

 	if(idx==0 || idx==4){
		pDst[0]=0;pDst[1]=0;}
	else if(idx==1 || idx==5){
		pDst[0]=0;pDst[1]=1;}
	else if(idx==2 || idx==6){
		pDst[0]=1;pDst[1]=1;}
	else if(idx==3 || idx==7){
		pDst[0]=1;pDst[1]=0;}	
	outleng = 2;
	ippsFree(temp);

}
// 只针对定频模式
void CMIL110Apro::DeInterleaverDecode(int nInter,int mInterLength,int &decodeleng)
{
	InterleaverPam pam;
	int repeat=0,i;
	decodeleng = 0;

	int mcodelen=0;
	if (nInter==11)//75 L
	{
		// 解交织
		pam.Cols=36;pam.Rows=20;
		pam.irs=7;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -7;pam.dicf = -3;// ((-7*(R-1))%C+C-1)+7

		pDecode->DeInterleaver(pBufInter,&pam);
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,1,pBufInter);
	}
	else if (nInter==12)//75 S
	{
		// 解交织
		pam.Cols=9;pam.Rows=10;
		pam.irs=7;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -7;pam.dicf = -1;// ((-7*(R-1))%C+C-1)+7

		pDecode->DeInterleaver(pBufInter,&pam);
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,1,pBufInter);
	//	decodeleng = mInterLength;

	}
	else if (mInterLength==720 && (nInter%2)==0 && (nInter>4 && nInter<=10))//150--600 短交织
	{
		// 解交织
		pam.Cols=18;pam.Rows=40;
		pam.irs=9;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -17;pam.dicf = 15;// ((-17*(R-1))%C+C-1)+17

		pDecode->DeInterleaver(pBufInter,&pam);
		// 译码
		if(nInter==6)
			repeat=1;
		else if(nInter==8)
			repeat=2;
		else if(nInter==10)
			repeat=4;
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,repeat,pBufInter);
		InterLength = 0;
	}
	else if (mInterLength==5760 && (nInter%2)==1 && (nInter>4 && nInter<=10))//150--600 长交织
	{
		// 解交织
		pam.Cols=144;pam.Rows=40;
		pam.irs=9;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -17;pam.dicf = -39;

		pDecode->DeInterleaver(pBufInter,&pam);
		// 译码
		if(nInter==5)
			repeat=1;
		else if(nInter==7)
			repeat=2;
		else if(nInter==9)
			repeat=4;
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,repeat,pBufInter);
	//	decodeleng = InterLength;
		InterLength = 0;
	}
	else if (mInterLength==11520 && nInter==3)//1200 长交织
	{
		// 解交织
		pam.Cols=288;pam.Rows=40;
		pam.irs=9;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -17;pam.dicf = -183;// ((-17*(R-1))%C+C-1)+17
		pDecode->DeInterleaver(pBufInter,&pam);
		repeat=1;
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,repeat,pBufInter);
		InterLength = 0;
	}
	else if (mInterLength==1440 && nInter==4)//1200 短交织
	{
		// 解交织
		pam.Cols=36;pam.Rows=40;
		pam.irs=9;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -17;pam.dicf = -3;// ((-17*(R-1))%C+C-1)+17
		pDecode->DeInterleaver(pBufInter,&pam);
		repeat=1;
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,repeat,pBufInter);
		InterLength = 0;
	}
	else if(mInterLength==23040 && nInter==1) // 2400 长交织
	{
		// 解交织
		pam.Cols=576;pam.Rows=40;
		pam.irs=9;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -17;pam.dicf = -471;// ((-17*(R-1))%C+C-1)+17
		pDecode->DeInterleaver(pBufInter,&pam);
		repeat=1;
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,repeat,pBufInter);
		InterLength = 0;
	}
	else if(mInterLength==2880 && nInter==2)// 2400 短交织
	{
		// 解交织
		pam.Cols=72;pam.Rows=40;
		pam.irs=9;pam.dirs=0;pam.ics=0;pam.dics=1;
		pam.irf = 1;pam.dirf = 0;pam.icf = -17;pam.dicf = -39;// ((-17*(R-1))%C+C-1)+17
		pDecode->DeInterleaver(pBufInter,&pam);
		repeat=1;
		decodeleng = pDecode->DeConverCode(pBufInter,pam.Cols*pam.Rows,repeat,pBufInter);
		InterLength = 0;
	}
	else if(nInter==0) //  4800 无交织 无编码
	{
		decodeleng = mInterLength;
		InterLength = 0;
	}

}
void CMIL110Apro::DeHead110A(Ipp16s *BurstPos,Ipp16s *nD,int BurstNum,HeadType110A *headType,int &headNum)
{
	int i;
	short nInterleaver;
	for (i=0;i<BurstNum;i++)
	{
		headType[i].position = BurstPos[i];
		headType[i].fre = pfre*9600.0;
		if(headType[i].fre<600 || headType[i].fre>2000)
			headType[i].fre = 1800;

		nInterleaver = nD[i];
		if(nInterleaver==0)
		{
			headType[i].dataRate=4800;
			headType[i].interLeng = 0;
		}
		else if(nInterleaver==1)
		{
			headType[i].dataRate=2400;
			headType[i].interLeng = 40*576;
		}
		else if(nInterleaver==2)
		{
			headType[i].dataRate=2400;
			headType[i].interLeng = 40*72;
		}
		else if(nInterleaver==3)
		{
			headType[i].dataRate=1200;
			headType[i].interLeng = 40*288;
		}
		else if(nInterleaver==4)
		{
			headType[i].dataRate=1200;
			headType[i].interLeng = 40*36;
		}
		else if(nInterleaver==5)
		{
			headType[i].dataRate=600;
			headType[i].interLeng = 40*144;
		}
		else if(nInterleaver==6)
		{
			headType[i].dataRate=600;
			headType[i].interLeng = 40*18;
		}
		else if(nInterleaver==7)
		{
			headType[i].dataRate=300;
			headType[i].interLeng = 40*144;
		}
		else if(nInterleaver==8)
		{
			headType[i].dataRate=300;
			headType[i].interLeng = 40*18;
		}
		else if(nInterleaver==9)
		{
			headType[i].dataRate=150;
			headType[i].interLeng = 40*144;
		}
		else if(nInterleaver==10)
		{
			headType[i].dataRate=150;
			headType[i].interLeng = 40*18;
		}
		else if(nInterleaver==11)
		{
			headType[i].dataRate=75;
			headType[i].interLeng = 20*36;
		}
		else if(nInterleaver==12)
		{
			headType[i].dataRate=75;
			headType[i].interLeng = 10*9;
		}

	}
	headNum = BurstNum;
}
void CMIL110Apro::CopyInterToPreamble(short nInter)
{
	ippsCopy_32fc(pTypeSym[D1D2[nInter][0]],&pPreambleSym[288],32);
	ippsCopy_32fc(pTypeSym[D1D2[nInter][1]],&pPreambleSym[320],32);	
}
void CMIL110Apro::CopyCountToPreamble(short nCount)
{
	ippsCopy_32fc(pTypeSym[C1C2C3[nCount][0]],&pPreambleSym[352],32);
	ippsCopy_32fc(pTypeSym[C1C2C3[nCount][1]],&pPreambleSym[384],32);
	ippsCopy_32fc(pTypeSym[C1C2C3[nCount][2]],&pPreambleSym[416],32);
}
void CMIL110Apro::ParaEstimate(Ipp32fc *pSrc,int nLeng,int P,Ipp32fc *pUWsig,Ipp32f &fre,Ipp32f &phase,Ipp32f &energy)
{
	Ipp32fc *pTemp = ippsMalloc_32fc(nLeng);
	int outLen;

	Timing_ini();
	Timing_estimate(pUWsig,288,pSrc,P,mtau);
	Timing_recover(pSrc,nLeng,mtau,P,pTemp,outLen);
	Fre_Estimate_LR(pUWsig,pTemp,288,fre);

	frephase = 0;
	Remove_fre(pTemp,288, fre);
	Phase_VV(pTemp,pUWsig,288,phase);

	Ipp32f pEnergy = 0;
	int i;
	for(i=0;i<288;i++)
		pEnergy = pEnergy + sqrt(pTemp[i].re*pTemp[i].re+pTemp[i].im*pTemp[i].im)/288;
	energy = pEnergy;

	ippsFree(pTemp);
	frephase = 0;
}
void CMIL110Apro::Timing_ini()
{
	xi0=0;	xi1=0;	xi2=0;
	xq0=0;	xq1=0;	xq2=0;
	muk = 0;
}
void CMIL110Apro::Timing_estimate(Ipp32fc *pUWsig,Ipp16s UWleng,Ipp32fc *pSrc,Ipp16s P, Ipp32f &tau)
{
	Ipp16s i,j,k=0;
	Ipp32f sumreal=0,sumimag=0;
	Ipp32f real,imag,absf;
	for (i=0;i<UWleng;i++)
	{
		for(j=0;j<P;j++)
		{
			real = pUWsig[i].re*pSrc[i*P+j].re+pUWsig[i].im*pSrc[i*P+j].im;
			imag = pUWsig[i].re*pSrc[i*P+j].im-pUWsig[i].im*pSrc[i*P+j].re;
			absf = real * real +imag * imag;
			sumreal = sumreal + (absf)*cos(IPP_2PI*(Ipp32f)k/P);
			sumimag = sumimag - (absf)*sin(IPP_2PI*(Ipp32f)k/P);
			k++;
		}
	}
	tau=-(Ipp32f)atan2(sumimag,sumreal)/IPP_2PI;
}
/************************************************************************/
/*  定时同步，4倍采样                                                  */
/************************************************************************/
void CMIL110Apro::Timing_recover(Ipp32fc *pSrc,int nleng,Ipp32f tau,Ipp16s P,Ipp32fc *pDst,int &outleng)
{
	Ipp32f fltIdx;
	int mk,i,outn=0;

	if(tau > 0 && tau <= 0.25)
		fltIdx = P*tau+1;
	else if(tau > 0.25 && tau <= 0.5)
		fltIdx = P*tau-1;
	else if(tau > -0.25 && tau <= 0)
		fltIdx = P*tau+3;
	else if(tau > -0.5 && tau <= -0.25)
		fltIdx = P*tau+5;
	for(i=0;i<nleng;i++)
	{			
		fltIdx=fltIdx+1;
		if (fltIdx > P)
		{
			mk=floor(fltIdx);
			muk=fltIdx-mk;
			fltIdx=fltIdx-P;
			pDst[outn].re=sym_interp_i(pSrc[i].re,muk,true);
			pDst[outn].im=sym_interp_q(pSrc[i].im,muk,true);
			outn++;
		}
		else
		{
			sym_interp_i(pSrc[i].re,muk,false);
			sym_interp_q(pSrc[i].im,muk,false);
		}				
	}
	outleng=outn;
}
/************************************************************************/
/*  符号定时插值器 i       当ini 为true时输出内插值
/************************************************************************/
float CMIL110Apro::sym_interp_i(float xi3, float u,bool ini)
{  
	float u2,u3;
	float y=0;
	if(ini)
	{
		u2=u*u;
		u3=u2*u;
		y=xi3*(u3-u)/6+xi2*(u+u2/2-u3/2)+xi1*(1-u/2-u2+u3/2)+xi0*(u2/2-u/3-u3/6);
	}	
	xi0=xi1;
	xi1=xi2;
	xi2=xi3;

	return y;

}
/************************************************************************/
 /*  符号定时插值器 q
/************************************************************************/
float CMIL110Apro::sym_interp_q(float xq3, float u,bool ini)
{
	float u2,u3;
	float y=0;
	if(ini)
	{
		u2=u*u;
		u3=u2*u;
		y=xq3*(u3-u)/6+xq2*(u+u2/2-u3/2)+xq1*(1-u/2-u2+u3/2)+xq0*(u2/2-u/3-u3/6);
	}
	xq0=xq1;
	xq1=xq2;
	xq2=xq3;

	return y;

} 
/************************************************************************/
/* 数据辅助的LR频偏估计                                                  */
/************************************************************************/
void CMIL110Apro::Fre_Estimate_LR(Ipp32fc *pUWsig,Ipp32fc *pSrc,Ipp16s nleng,Ipp32f &fre)
{
	Ipp16s i,N=nleng/2;
	Ipp32fc *removemodule,*RR,pSum;
	removemodule=ippsMalloc_32fc(nleng);
	RR=ippsMalloc_32fc(N+1);

	for (i=0;i<nleng;i++)
	{
		removemodule[i].re=pUWsig[i].re*pSrc[i].re+pUWsig[i].im*pSrc[i].im;
		removemodule[i].im=pUWsig[i].re*pSrc[i].im-pUWsig[i].im*pSrc[i].re;
	}

	ippsAutoCorr_NormB_32fc(removemodule, nleng, RR, N+1);

	ippsSum_32fc(&RR[1], N, &pSum,ippAlgHintFast);
	fre=atan2(pSum.im,pSum.re)/(IPP_PI*(N+1));

	ippsFree(removemodule);
	ippsFree(RR);


}
void CMIL110Apro::Remove_fre(Ipp32fc *pSrc,Ipp32s nLeng, Ipp32f rFreq)
{
	Ipp32f temp;
	Ipp16s i;

	for (i=0;i<nLeng;i++)
	{
		frephase=frephase-IPP_2PI*rFreq;
		if ((frephase)>=IPP_2PI || (frephase)<=-IPP_2PI)
			frephase=fmod((double)frephase,(double)IPP_2PI);

		temp=pSrc[i].re;
		pSrc[i].re=pSrc[i].re*cos(frephase)-pSrc[i].im*sin(frephase);
		pSrc[i].im=pSrc[i].im*cos(frephase)+temp*sin(frephase);
	}

}

void CMIL110Apro::Phase_VV(Ipp32fc *pSrc,Ipp32fc *pUWsig,Ipp16s len,Ipp32f &phase)
{
	Ipp16s i;
	Ipp32fc temp;

	temp.re=0;temp.im=0;
	for (i=0;i<len;i++)
	{
		temp.re=temp.re+pSrc[i].re*pUWsig[i].re+pSrc[i].im*pUWsig[i].im;
		temp.im = temp.im+pSrc[i].re*pUWsig[i].im-pSrc[i].im*pUWsig[i].re;
	}
	phase=(Ipp32f)atan2(temp.im,temp.re);

}

void CMIL110Apro::EqualizationRLS_DFE_PLL_ini(Ipp32f wn, Ipp32f kesai,Ipp16s M_phase,Ipp32f pll_phase,
													 int mOrderA,int mOrderB,Ipp32f mdelta,Ipp32f mlamde)
{
	wn_p=wn;
	kesai_p=kesai;
	ko_p=1.0;
	kd_p=5.25;
	kp_p=4*kesai_p*wn_p/((1+2*kesai_p*wn_p+wn_p*wn_p)*ko_p*kd_p);
	ki_p=kp_p*wn_p/kesai_p;  
	mod=2*IPP_PI/(M_phase); 
	eold=0;
	y_p=0;

	m_RLS->RLS_ini(mOrderA,mOrderB,mlamde,mdelta);


}

void CMIL110Apro::EqualizationRLS_DFE_PLL_free()
{
	m_RLS->RLS_free();
	if(m_RLS){
		delete m_RLS;
		m_RLS = NULL;
	}
}
void CMIL110Apro::EqualizationFilter(Ipp32fc *pSrc,int nLeng,int P,Ipp32f mEnergy,Ipp32fc *pDst,int &outLen)
{
	int i;
	Ipp32fc xtemp;
	Ipp32f eph;

	outLen=0;
	for (i=0;i<nLeng;i++)
	{	
		xtemp.re=(pSrc[i].re*cos(pllphase)-pSrc[i].im*sin(pllphase))/mEnergy;
		xtemp.im=(pSrc[i].im*cos(pllphase)+pSrc[i].re*sin(pllphase))/mEnergy;
		if(i%P==0){
			m_RLS->RLSFilter(xtemp,pDst[outLen],TRUE);
			eph= m_RLS->xBuffilter[m_RLS->OrderA].im*pDst[outLen].re - m_RLS->xBuffilter[m_RLS->OrderA].re*pDst[outLen].im;
			y_p=y_p + (kp_p+ki_p)*eph - kp_p*eold;
			eold=eph;
			pllphase=pllphase+y_p*ko_p;	
			outLen++;
		}
		else
			m_RLS->RLSFilter(xtemp,pDst[outLen],FALSE);
	}
}
//  分数间隔判决反馈均衡
void CMIL110Apro::EqualizationRLS_DFE_PLL(Ipp32fc *pSrc,int nLeng,int P,int delay,Ipp32f mEnergy,int M,Ipp32fc *UWsig,Ipp32fc *pDst,int &outLen,BOOL aid)
{
	int i;
	Ipp32fc xtemp;
	Ipp32f eph;

	outLen=0;
	if(aid)
	{
		for (i=0;i<nLeng;i++)
		{	
			xtemp.re=(pSrc[i].re*cos(pllphase)-pSrc[i].im*sin(pllphase))/mEnergy;
			xtemp.im=(pSrc[i].im*cos(pllphase)+pSrc[i].re*sin(pllphase))/mEnergy;
			if(i%P==0 && i>=delay){
				m_RLS->RLS_Update(xtemp,UWsig[outLen],M,pDst[outLen],TRUE,aid);

				eph= m_RLS->xBuf[m_RLS->OrderA].im*pDst[outLen].re - m_RLS->xBuf[m_RLS->OrderA].re*pDst[outLen].im;
				y_p=y_p + (kp_p+ki_p)*eph - kp_p*eold;
				eold=eph;
				pllphase=pllphase+y_p*ko_p;	
				outLen++;
			}
			else
				m_RLS->RLS_Update(xtemp,UWsig[outLen],M,pDst[outLen],FALSE,aid);
		}
	}
	else
	{
		for (i=0;i<nLeng;i++)
		{	
			xtemp.re=(pSrc[i].re*cos(pllphase)-pSrc[i].im*sin(pllphase))/mEnergy;
			xtemp.im=(pSrc[i].im*cos(pllphase)+pSrc[i].re*sin(pllphase))/mEnergy;
			if(i%P==0){
				m_RLS->RLS_Update(xtemp,UWsig[outLen],M,pDst[outLen],TRUE,aid);

				eph= m_RLS->xBuf[m_RLS->OrderA].im*pDst[outLen].re - m_RLS->xBuf[m_RLS->OrderA].re*pDst[outLen].im;
				y_p=y_p + (kp_p+ki_p)*eph - kp_p*eold;
				eold=eph;
				pllphase=pllphase+y_p*ko_p;	
				outLen++;
			}
			else
				m_RLS->RLS_Update(xtemp,UWsig[outLen],M,pDst[outLen],FALSE,aid);
		}
	}
}

void CMIL110Apro::DecPSK(Ipp32fc pSrc,Ipp32fc &pDst)
{

	Ipp32f angle = atan2(pSrc.im,pSrc.re);
	if (angle>=-IPP_PI/8 && angle<IPP_PI/8)
	{
		pDst.re = 1;
		pDst.im = 0;
	}
	else if (angle>=IPP_PI/8 && angle<IPP_PI*3/8)	
	{												
		pDst.re = 0.707;
		pDst.im = 0.707;
	}									   
	else if(angle>=IPP_PI*3/8 && angle<IPP_PI*5/8)		
	{
		pDst.re = 0;
		pDst.im = 1;
	}										    
	else if(angle>=IPP_PI*5/8 && angle<IPP_PI*7/8)	
	{
		pDst.re = -0.707;
		pDst.im = 0.707;
	}										 	
	else if (angle>=-IPP_PI*7/8 && angle<-IPP_PI*5/8)	
	{
		pDst.re = -0.707;
		pDst.im = -0.707;
	}
	else if (angle>=-IPP_PI*5/8 && angle<-IPP_PI*3/8)
	{
		pDst.re = 0;
		pDst.im = -1;
	}
	else if (angle>=-IPP_PI*3/8 && angle<-IPP_PI*1/8)
	{
		pDst.re = 0.707;
		pDst.im = -0.707;
	}
	else
	{
		pDst.re = -1;
		pDst.im = 0;
	}
}

void CMIL110Apro::SaveTobit_ini()
{
	byte_flag = 0x80;
	data_byte = 0;
	bit_num = 8;
	symbol = 0; //bit流组成byte
}
void CMIL110Apro::SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,Ipp8s modutype)
{
	Ipp32s i,m,j;
	switch(modutype)
	{
	case 1:  //  BPSK
		m=0;
		for (i=0;i<nLeng;i++)
		{
			////////////　bit流组成byte存放　//////////////
			if(pSrc[i]&0x01 == 1)
				data_byte |= byte_flag;
			byte_flag >>= 1;
			if(byte_flag == 0)//字节存满
			{
				outbyte[m]=data_byte;
				m++;				
				byte_flag = 0x80;
				data_byte = 0;
			}	
		}
		break;
	case 2: // QPSK  45
		m=0;
		for (i=0;i<nLeng;i++)
		{
			
			////////////　bit流组成byte存放　//////////////		
			bit_num -= 2;
			data_byte |= (pSrc[i] << bit_num);			
			if(bit_num == 0)
			{
				outbyte[m]=data_byte;
				m++;
				bit_num = 8;
				data_byte = 0;
			}
		}
		break;
	case 3:  //  8PSk
		m=0;
		for(i=0;i<nLeng;i++)
		{
			//将每个码元3bit拆开单bit存放
			for(j = 0; j < 3; j++)
			{
				bit_num = (pSrc[i] >> (2-j)) & 0x01;
				if(bit_num == 1)
					data_byte |= byte_flag;
				byte_flag = byte_flag>>1;
				if(byte_flag == 0)//字节存满
				{
					outbyte[m]=data_byte;
					m++;
					byte_flag = 0x80;
					data_byte = 0;
				}
			}	
		}
		break;
	case 4:  //  16QAM
		m=0;
		for(i=0;i<nLeng;i++)
		{
			//将每个码元4bit拆开单bit存放
			for(j = 0; j < 4; j++)
			{
				bit_num = (pSrc[i] >> (3-j)) & 0x01;
				if(bit_num == 1)
					data_byte |= byte_flag;
				byte_flag = byte_flag>>1;
				if(byte_flag == 0)//字节存满
				{
					outbyte[m]=data_byte;
					m++;
					byte_flag = 0x80;
					data_byte = 0;
				}
			}	
		}
		break;
	case 5:  //  32QAM
		m=0;
		for(i=0;i<nLeng;i++)
		{
			//将每个码元5bit拆开单bit存放
			for(j = 0; j < 5; j++)
			{
				bit_num = (pSrc[i] >> (4-j)) & 0x01;
				if(bit_num == 1)
					data_byte |= byte_flag;
				byte_flag = byte_flag>>1;
				if(byte_flag == 0)//字节存满
				{
					outbyte[m]=data_byte;
					m++;
					byte_flag = 0x80;
					data_byte = 0;
				}
			}	
		}
		break;
	case 6:  //  64QAM
		m=0;
		for(i=0;i<nLeng;i++)
		{
			//将每个码元6bit拆开单bit存放
			for(j = 0; j < 6; j++)
			{
				bit_num = (pSrc[i] >> (5-j)) & 0x01;
				if(bit_num == 1)
					data_byte |= byte_flag;
				byte_flag = byte_flag>>1;
				if(byte_flag == 0)//字节存满
				{
					outbyte[m]=data_byte;
					m++;
					byte_flag = 0x80;
					data_byte = 0;
				}
			}	
		}
		break;
	}
	byteLeng = m;
}

/************************************************************************/
/*   modutype:  1->BPSK
				2->QPSK
				3->8PSK
/************************************************************************/
void CMIL110Apro::judge(Ipp32fc *pSrc,Ipp16s nLeng,Ipp8u *pDst,Ipp8s modutype)
{
	Ipp32f *angle;
	Ipp32s i,m,j;
	angle=ippsMalloc_32f(nLeng);
	ippsPhase_32fc(pSrc, angle, nLeng);
	
	switch(modutype)
	{
	case 1:  //  BPSK
		for (i=0;i<nLeng;i++)
		{
			if(pSrc[i].re>0)
				pDst[i]=0;
			else
				pDst[i]=1;						
		}
		break;
	case 2: // QPSK  45
		for (i=0;i<nLeng;i++)
		{
			if(pSrc[i].re >= 0)
			{
                if (pSrc[i].im>=0)	//			|
					pDst[i] = 0;	//		1	|	0
                else				//	  ------|-------
					pDst[i] = 2;	//		3	|	2
			}						//			|
            else					//
			{
				if (pSrc[i].im<0) 
					pDst[i] = 3;
                else
					pDst[i] = 1;	 
			}
		}
		break;
	case 3: // QPSK  45
		for (i=0;i<nLeng;i++)
		{
			if(pSrc[i].re >= 0)
			{
                if (pSrc[i].im>=0)	//			|
					pDst[i] = 0;	//		2	|	0
                else				//	  ------|-------
					pDst[i] = 1;	//		3	|	1
			}						//			|
            else					//
			{
				if (pSrc[i].im<0) 
					pDst[i] = 3;
                else
					pDst[i] = 2;	 
			}
		}
		break;
	case 4:  // 8PSK
		for (i=0;i<nLeng;i++)
		{
			if (angle[i]>=-IPP_PI/8 && angle[i]<IPP_PI/8)
				pDst[i] = 0;
			else if (angle[i]>=IPP_PI/8 && angle[i]<IPP_PI*3/8)		//		  |	
				pDst[i] = 1;										 //	  3	  2	  1 	
			else if(angle[i]>=IPP_PI*3/8 && angle[i]<IPP_PI*5/8)	//		  |	  		 		
				pDst[i] = 2;										//  --4---|---0----		
			else if(angle[i]>=IPP_PI*5/8 && angle[i]<IPP_PI*7/8)	//		  | 	
				pDst[i] = 3;										//	  5	  6	  7 		
			else if (angle[i]>=-IPP_PI*7/8 && angle[i]<-IPP_PI*5/8)	//		  |	
				pDst[i] = 5;
			else if (angle[i]>=-IPP_PI*5/8 && angle[i]<-IPP_PI*3/8)
				pDst[i] = 6;
			else if (angle[i]>=-IPP_PI*3/8 && angle[i]<-IPP_PI*1/8)
				pDst[i] = 7;
			else
				pDst[i] = 4;
		}
		break;
	}
	ippsFree(angle);
}
/************************************************************************/

/************************************************************************/
void CMIL110Apro::MGDecode(Ipp8u *pSrc,int nLeng,Ipp8u *pDst,int &outLeng,Ipp8s modutype)
{
	int i;
	switch(modutype)
	{
	case 1:  //  BPSK
		for (i=0;i<nLeng;i++)
		{
			if(pSrc[i]==0 || pSrc[i]==1 || pSrc[i]==2 || pSrc[i]==3)
				pDst[i]=0;
			else
				pDst[i]=1;
		}
		outLeng = nLeng;
		break;
	case 2://QPSK
		for (i=0;i<nLeng;i++)
		{
			if(pSrc[i]==0 || pSrc[i]==1){ 
				pDst[2*i] = 0;pDst[2*i+1] = 0;}
			if(pSrc[i]==2 || pSrc[i]==3){ 
				pDst[2*i] = 0;pDst[2*i+1] = 1;}
			if(pSrc[i]==4 || pSrc[i]==5){ 
				pDst[2*i] = 1;pDst[2*i+1] = 1;}
			if(pSrc[i]==6 || pSrc[i]==7){ 
				pDst[2*i] = 1;pDst[2*i+1] = 0;}		
		}
		outLeng = 2*nLeng;
		break;
	case 3:
		for (i=0;i<nLeng;i++)
		{
			if(pSrc[i]==0){ 
				pDst[3*i] = 0;pDst[3*i+1] = 0;pDst[3*i+2] = 0;}
			if(pSrc[i]==1){ 
				pDst[3*i] = 0;pDst[3*i+1] = 0;pDst[3*i+2] = 1;}
			if(pSrc[i]==2){ 
				pDst[3*i] = 0;pDst[3*i+1] = 1;pDst[3*i+2] = 1;}
			if(pSrc[i]==3){ 
				pDst[3*i] = 0;pDst[3*i+1] = 1;pDst[3*i+2] = 0;}
			if(pSrc[i]==4){ 
				pDst[3*i] = 1;pDst[3*i+1] = 1;pDst[3*i+2] = 0;}
			if(pSrc[i]==5){ 
				pDst[3*i] = 1;pDst[3*i+1] = 1;pDst[3*i+2] = 1;}
			if(pSrc[i]==6){ 
				pDst[3*i] = 1;pDst[3*i+1] = 0;pDst[3*i+2] = 1;}
			if(pSrc[i]==7){ 
				pDst[3*i] = 1;pDst[3*i+1] = 0;pDst[3*i+2] = 0;}
		}
		outLeng = 3*nLeng;
		break;
	}

}
