// NATO4529pro.cpp : 定义 DLL 的初始化例程。

#include "stdafx.h"
#include "NATO4529pro.h"
#include <math.h>
#include "Preamble.h"
#include "NATODecoder.h"
#include "RLS.h"
#include "Preambledetect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CNATO4529pro::CNATO4529pro(void)
{
	pPreambleSym = NULL;
	pProbleSym = NULL;	
	pBufInter = NULL;
	probe = NULL;

}

CNATO4529pro::~CNATO4529pro(void)
{
	if (pPreambleSym!=NULL)
	{
		ippsFree(pPreambleSym);
		pPreambleSym = NULL;
	}
	if (pProbleSym!=NULL)
	{
		ippsFree(pProbleSym);
		pProbleSym = NULL;
	}
	if(probe!=NULL)
	{
		ippsFree(probe);
		probe = NULL;
	}
}
void CNATO4529pro::PSK_map(short style, short M, float I_map[],float Q_map[])
{
	switch(M)
	{
	case 2:
		if(style==1)
		{
			I_map[0]=1;Q_map[0]=0;
			I_map[1]=-1;Q_map[1]=0;
		}
		else if(style==0)
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
int CNATO4529pro::PSK_MOD( short data[], short data_len, short M,	 Ipp32fc *out)
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
void CNATO4529pro::Preamble_Gen()
{
	int i,j,k;
	UWleng = 80;

	pPreambleSym = ippsMalloc_32fc(80);
	PSK_MOD(preamble_4529,80,2,pPreambleSym);
	UWEnergy = 1;

	pProbleSym = ippsMalloc_32fc(48);
	probe = ippsMalloc_16s(48);

}
/************************************************************************/
/*   abs(mean(a.*conj(b)))/sqrt(mean(a.*conj(a))*mean(b.*conj(b)));                                                                   */
/************************************************************************/

void CNATO4529pro::CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f *cConv)
{
	
	Ipp32fc pMeanAB;
	Ipp32f pMeanA;

	ippsConj_32fc(pSrcB,pConvAB,nLeng);
	ippsMul_32fc_I(pSrcA,pConvAB,nLeng);
	ippsMean_32fc(pConvAB,nLeng,&pMeanAB,ippAlgHintFast);

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
void CNATO4529pro::Burst_detect_ini(int allLen,int convLen,int deciP,int mproBegin)
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

void CNATO4529pro::Burst_detect_free()
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
	
/************************************************************************/
void CNATO4529pro::Burst_detect(int proBegin,int proLen,Ipp32fc *UWsig,Ipp32f UWEnergy, Ipp16s convLen,Ipp16s deciP,
							Ipp32f Threshold,Ipp16s *BurstPos,int &BurstNum,int &BufPos,int &BufHavePro)
{
	Ipp16s i,j;
	int pPhase=0;
	int pDstLen=0;
	Ipp32fc *pSrcA = ippsMalloc_32fc(convLen);

	Ipp32fc *pSinCos = ippsMalloc_32fc(convLen);
	Ipp32f mfre[3]={1-0.002,0,0.002},mphase=0;
	Ipp32f cconv,maxconv=0;

	int convdataL = convLen*deciP;
	int compareL = 4*deciP;
	int winBufp = 0;
	Ipp32f pMax;
	int pIndex,maxidx;
	
	BurstNum=0;
	ippsCopy_32fc(delayBuf,&proBuf[proBegin],delayL);
	for (i=0;i<proLen - convdataL;i++)
	{
		ippsSampleDown_32fc(&proBuf[proBegin+i],convdataL,pSrcA,&pDstLen,deciP,&pPhase);// 间隔P点抽出数据	
	//	CrossConvCof(pSrcA,UWsig,convLen,UWEnergy,&cconv);// 相关系数

		maxconv=0;
		for (j=0;j<3;j++)
		{
			ippsTone_Direct_32fc(pSinCos, convLen, 1, mfre[j], &mphase, ippAlgHintFast);
			ippsMul_32fc_I(pSrcA,pSinCos, convLen);
			CrossConvCof(pSinCos,UWsig,convLen,UWEnergy,&cconv);// 相关系数
			if(cconv>maxconv)
			{
				maxconv = cconv;
				maxidx = j;
			}
		}

		winBufConv[winBufp] = maxconv;
		winBufp++;
	}
	for (i=0;i<winBufp-compareL;i++)
	{
		if (winBufConv[i]>Threshold && winBufConv[i+1]>Threshold)//检测到信号
		{
			ippsMaxIndx_32f(&winBufConv[i],compareL,&pMax,&pIndex);
			BurstPos[BurstNum] = i+pIndex;
			BurstNum++;
			i = i+compareL;
		}
	}
	delayL = convdataL + compareL;
	ippsCopy_32fc(&proBuf[proBegin+proLen-delayL],delayBuf,delayL);// 结尾数据复制，以便下次处理

	ippsFree(pSrcA);
	ippsFree(pSinCos);
	BufHavePro = winBufp-compareL;
	BufPos = delayL;

}
void CNATO4529pro::DownFre_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,int FFTLen,Ipp32f roll,Ipp32f Baud,Ipp16s P,int SrctapLen)
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
void CNATO4529pro::DownFre_free()
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
void CNATO4529pro::DownFre(Ipp16s *pSrc,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,Ipp32fc *pDst,int &DstLen)
{
	int i;
	int BurstNum;
	int BufHavePro = 0;
	int UWleng = m_Preambledetect->PreambleLen;
	Ipp16s *BurstPos = ippsMalloc_16s(nLeng/UWleng*2);
	Ipp32f *fre = ippsMalloc_32f(nLeng/UWleng*2);
	int OutLen;

	m_Preambledetect->ReSample(pSrc,nLeng,m_Preambledetect->pBuf,OutLen);
	//int dataLen = OutLen+m_Preambledetect->BufPos;
	//m_Preambledetect->Burst_detect(dataLen,m_Preambledetect->pPreambleSym,1,m_Preambledetect->PreambleLen,P,FFTLen,TH1,TH2,
	//	BurstPos,fre,BurstNum,m_Preambledetect->BufPos,BufHavePro);

	int pburst=0;
	for (i=0;i<OutLen;i++)
	{
		/*if(i==BurstPos[pburst] && BurstNum>0)
		{
			pfre = fre[pburst]/P;
			pburst++;
		}*/
		defrephase=defrephase-IPP_2PI*pfre;
		if ((defrephase)>=IPP_2PI || (defrephase)<=-IPP_2PI)
			defrephase=fmod((double)defrephase,(double)IPP_2PI);
		pReal[i]=m_Preambledetect->pBuf[i]*cos(defrephase);
		pImag[i]=m_Preambledetect->pBuf[i]*sin(defrephase);
	}
	ippsFIR64f_32f_I(pReal, OutLen, SrcStateReal);
	ippsFIR64f_32f_I(pImag, OutLen, SrcStateImag);
	ippsRealToCplx_32f(pReal,pImag,pDst,OutLen);
	DstLen = OutLen;
}
void CNATO4529pro::Demode_PSKrealFSE_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,Ipp16s P,int FFTLen,Ipp32f roll,Ipp32f Baud,int SrctapLen,
										 int mdataRate,int mFECType,int mInterType)
{
	int allLen;
	m_Preambledetect = new CPreambledetect;
	SrcLen = SrctapLen;
	DownFre_ini(Insample,Outsample,nLeng,fre,FFTLen,roll,Baud,P,SrctapLen);	
	Preamble_Gen();
	OneFrameLen = 256*P;  //  一段帧长,包括FSE的延时，与FSE阶数有关
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
	contin = 0;
	pfre = fre/Outsample;

	EqualizationRLS_DFE_PLL_ini(0.01,0.707,8,pllphase,18,15,100,0.99);
	SaveTobit_ini();
	pDecoder = new NATODecoder;
	dataRate = mdataRate;
	FECType = mFECType;
	InterType = mInterType;
	if(dataRate==75 ||dataRate==150 || dataRate==300)
	{
		if(InterType==1)
			increasment=1;
		else if(InterType==2)
			increasment =12;
		else
			increasment =12;
	}
	else if(dataRate==600)
	{
		if(InterType==1)
			increasment=2;
		else if(InterType==2)
			increasment =24;
		else
			increasment =24;
	}
	else if(dataRate==1200)
	{
		if(InterType==1)
			increasment=4;
		else if(InterType==2)
			increasment =48;
		else
			increasment =48;
	}
	else
		increasment = 48;
	flushLength = increasment*32*31;
	pBufInter = ippsMalloc_8u(80*8*3+flushLength);
	pBufCoded = ippsMalloc_8u(80*8*3+flushLength);
	pDecoder->setParameter(increasment,true);
}
void CNATO4529pro::Demode_PSKrealFSE_free()
{
	ippsFree(OneFrame);
	Burst_detect_free();
	DownFre_free();
	EqualizationRLS_DFE_PLL_free();
	delete pDecoder;
	ippsFree(pBufInter);
	ippsFree(pBufCoded);
	delete m_Preambledetect;
}

void CNATO4529pro::Demode_PSKrealFSE(Ipp16s *pSrcDst,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,
									 int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType4529 *headType,int &headNum)
{
	int i,j;
	Ipp16s *BurstPos = ippsMalloc_16s(nLeng/UWleng*2+1);
	int BurstNum;
	int BufHavePro = 0;	

	int dataLen;
	DownFre(pSrcDst,nLeng,P,FFTLen,TH1,TH2,&proBuf[proBegin+BufPos],dataLen);
	dataLen = dataLen+BufPos;
	Burst_detect(proBegin,dataLen,pPreambleSym,1,UWleng,P,TH2,BurstPos,BurstNum,BufPos,BufHavePro);
	DeHead4529(BurstPos,BurstNum,headType,headNum);

	int TimeOutLen,mbyteleng;
	outLeng = 0;
	byteLeng = 0;
	////////////////////
	pBurst = 0;	
	while(pBuf + OneFrameLen<BufHavePro)
	{
		TimeOutLen = 0;
		//第一次找起点
		if (flag==0 && BurstNum>0 && pBuf<=BurstPos[pBurst] && BurstPos[pBurst]<=pBuf+OneFrameLen)
		{
			flag = 1;
			pBuf = BurstPos[pBurst];
			pBurst++;
			BurstNum--;
		}
		else if (flag==1)// 找到前导进行同步估计
		{
		 	Synchronization_PSK(&proBuf[proBegin+pBuf],OneFrameLen,P,pPreambleSym,UWleng,5,OneFrame,TimeOutLen,&outbyte[byteLeng],mbyteleng,1);
			byteLeng = byteLeng +mbyteleng;
			pBuf = pBuf + OneFrameLen;
			flag = 2;
			contin = 1;
		}
		//  找到下一个前导
		else if (flag==2 && BurstNum>0 && pBuf<=BurstPos[pBurst]+10 && BurstPos[pBurst]<=pBuf+OneFrameLen && contin==1)
		{
			pBuf = BurstPos[pBurst];
			Synchronization_PSK(&proBuf[proBegin+pBuf],OneFrameLen,P,pPreambleSym,UWleng,5,OneFrame,TimeOutLen,&outbyte[byteLeng],mbyteleng,1);
			flag = 2;
			pBuf = pBuf + OneFrameLen;
			byteLeng = byteLeng +mbyteleng;		
			pBurst++;
			BurstNum--;	
		}
		else if(flag==2)
		{
			pBuf = pBuf + OneFrameLen;
			contin = 0;
			flag=0;
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
	while(BurstNum)
	{
		if(BurstPos[pBurst]>BufHavePro-OneFrameLen)
		{
			pBuf = BurstPos[pBurst];
			if(contin==1)
				flag=2;
			else
				flag=1;
			break;
		}
		pBurst++;
		BurstNum--;	
	}
	
	// 处理缓存结尾, 将结尾数据复制到缓存头部，以备下次使用 
	ippsCopy_32fc(&proBuf[proBegin+pBuf],&proBuf[proBegin-(BufHavePro-pBuf)],BufHavePro-pBuf);
	pBuf = -(BufHavePro-pBuf);
	ippsFree(BurstPos);
}
void CNATO4529pro::DeHead4529(Ipp16s *BurstPos,int BurstNum,HeadType4529 *headType,int &headNum)
{
	int i;
	int moduType,interType;
	for (i=0;i<BurstNum;i++)
	{
		headType[i].position = BurstPos[i];
		headType[i].fre = pfre*9600.0;		
		headType[i].dataRate = dataRate;
		if(InterType==0)
			headType[i].interType = "NO";
		else if(InterType==1)
			headType[i].interType = "SHORT";
		else if(InterType==2)
			headType[i].interType = "LONG";
	}
	headNum = BurstNum;
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
void CNATO4529pro::Synchronization_PSK(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp16s P,Ipp32fc *pUWsig,Ipp16s UWleng,int delayCma,
									  Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng,int bBegin)
{
	int i,j,k,p=0;
	int pPhase=0;
	int pDownSLen=0;
	int mbyteleng=0;
	byteleng = 0;
	Ipp32fc *pDownS = ippsMalloc_32fc(FrameLen/2+50);
	Ipp32fc *pTemp = ippsMalloc_32fc(FrameLen/2+50);
	int TempLen;
	Ipp8u *pJudge = ippsMalloc_8u(FrameLen/P+50);
	int judgelen=0;
	int moutleng=0;
	outleng = 0;
	InterLength=0;
	

	ippsSampleDown_32fc(FrameBuf,FrameLen,pTemp,&TempLen,2,&pPhase);
	P = P/2;
	// 取出前导序列同步估计
	ParaEstimate(pTemp,80*P,P,pUWsig,mfre,pllphase,pEnergy);
	ippsSampleDown_32fc(pTemp,TempLen,pDownS,&pDownSLen,2,&pPhase);
	for (i=0;i<2*delayCma;i++)
		pDownS[pDownSLen+i] = pDownS[i];
	pDownSLen = pDownSLen + 2*delayCma;

	frephase = 0;
	mfre = mfre/2;
	Remove_fre(pDownS,pDownSLen,mfre);

	// 前导均衡
	m_RLS->RLS_SetZero();
	EqualizationRLS_DFE_PLL(pDownS,(UWleng+delayCma)*2,2,delayCma*2,pEnergy,pUWsig,&pDst[outleng],moutleng,TRUE);	
//	outleng = outleng + moutleng;

	// 处理数据
	 for (i=0;i<3;i++)
	 {
		 p=(UWleng+delayCma)*2+i*48*2;
		 EqualizationRLS_DFE_PLL(&pDownS[p],32*2,2,0,pEnergy,pUWsig,&pDst[outleng],moutleng,FALSE);
		 ////  去扰
		 for (j=0;j<32;j++)
		 {
			 k = i*48+j;
			 k = k%176;
			 probe[j] =screamble_4529[k];
		 }
		 PSK_MOD(probe,32,8,pProbleSym);
		 ippsConj_32fc_I(pProbleSym,32);
		 ippsMul_32fc_I(pProbleSym,&pDst[outleng],32);
		 judge(&pDst[outleng],32,&pJudge[judgelen],4);	
		 outleng = outleng + moutleng;
		 judgelen = judgelen + 32;

		 // 信道探测
		 for (j=0;j<16;j++)
		 {
			k = 32+i*48+j;
			k = k%176;
			probe[j] = screamble_4529[k];
		 }					
		 PSK_MOD(probe,16,8,pProbleSym);
		 p=(UWleng+delayCma)*2+i*48*2 + 32*2;
		 EqualizationRLS_DFE_PLL(&pDownS[p],16*2,2,0,pEnergy,pProbleSym,&pDst[outleng],moutleng,TRUE);
	//	 outleng = outleng + moutleng;
	 }
	 // 32个数据符号
	 p=(UWleng+delayCma)*2+i*48*2;
	 EqualizationRLS_DFE_PLL(&pDownS[p],32*2,2,0,pEnergy,pUWsig,&pDst[outleng],moutleng,FALSE);
	 ////  去扰
	 for (j=0;j<32;j++)
	 {
		 k = i*48+j;
		 k = k%176;
		 probe[j] =screamble_4529[k];
	 }
	 PSK_MOD(probe,32,8,pProbleSym);
	 ippsConj_32fc_I(pProbleSym,32);
	 ippsMul_32fc_I(pProbleSym,&pDst[outleng],32);
	 judge(&pDst[outleng],32,&pJudge[judgelen],4);	
	 outleng = outleng + moutleng;
	 judgelen = judgelen + 32;


	 // 解交织译码
	 if((dataRate==75 || dataRate==150 || dataRate==300)&& FECType==1)// 有编码
		 MapFor4285(pJudge,judgelen,pBufInter,InterLength,1);// BPSK
	 else if(dataRate==600 && FECType==1)
		 MapFor4285(pJudge,judgelen,pBufInter,InterLength,2);// QPSK
	 else if(dataRate==1200 && FECType==1)
		 MapFor4285(pJudge,judgelen,pBufInter,InterLength,3);// 8PSK
	 else if(dataRate==600 && FECType==0)
		 MapFor4285(pJudge,judgelen,pBufInter,InterLength,1);// BPSK
	 else if(dataRate==1200 && FECType==0)
		 MapFor4285(pJudge,judgelen,pBufInter,InterLength,2);// QPSK
	 else if(dataRate==1800 && FECType==0)
		 MapFor4285(pJudge,judgelen,pBufInter,InterLength,3);// 8PSK
	 else
		 MapFor4285(pJudge,judgelen,pBufInter,InterLength,3);// 8PSK


	 SaveTobit(pBufInter,InterLength,&outbyte[byteleng],mbyteleng,1);
	 byteleng = byteleng + mbyteleng;

	//if(FECType==1)//有编码
	//{
	//	pDecoder->DeConverInterleaver(pBufInter,InterLength,pBufCoded);
	//	SaveTobit(pBufCoded,InterLength,&outbyte[byteleng],mbyteleng,1);
	//	byteleng = byteleng + mbyteleng;
	//}
	//else// 无交织编码
	//{
	//	SaveTobit(pBufInter,InterLength,&outbyte[byteleng],mbyteleng,1);
	//	byteleng = byteleng + mbyteleng;
	//}

	ippsFree(pDownS);
	ippsFree(pJudge);
	ippsFree(pTemp);
}

void CNATO4529pro::EqualizationRLS_DFE_PLL_ini(Ipp32f wn, Ipp32f kesai,Ipp16s M_phase,Ipp32f pll_phase,
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

	pllphase=pll_phase;
	m_RLS = new RLS;
	m_RLS->RLS_ini(mOrderA,mOrderB,mlamde,mdelta);

}

void CNATO4529pro::EqualizationRLS_DFE_PLL_free()
{
	m_RLS->RLS_free();
	if(m_RLS){
		delete m_RLS;
		m_RLS = NULL;
	}
}
//  分数间隔判决反馈均衡
void CNATO4529pro::EqualizationRLS_DFE_PLL(Ipp32fc *pSrc,int nLeng,int P,int delay,Ipp32f mEnergy,Ipp32fc *UWsig,Ipp32fc *pDst,int &outLen,BOOL aid)
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
				m_RLS->RLS_Update(xtemp,UWsig[outLen],pDst[outLen],TRUE,aid);

				eph= m_RLS->xBuf[m_RLS->OrderA].im*pDst[outLen].re - m_RLS->xBuf[m_RLS->OrderA].re*pDst[outLen].im;
				y_p=y_p + (kp_p+ki_p)*eph - kp_p*eold;
				eold=eph;
				pllphase=pllphase+y_p*ko_p;	
				outLen++;
			}
			else
				m_RLS->RLS_Update(xtemp,UWsig[outLen],pDst[outLen],FALSE,aid);
		}
	}
	else
	{
		for (i=0;i<nLeng;i++)
		{	
			xtemp.re=(pSrc[i].re*cos(pllphase)-pSrc[i].im*sin(pllphase))/mEnergy;
			xtemp.im=(pSrc[i].im*cos(pllphase)+pSrc[i].re*sin(pllphase))/mEnergy;
			if(i%P==0){
				m_RLS->RLS_Update(xtemp,UWsig[outLen],pDst[outLen],TRUE,aid);

				eph= m_RLS->xBuf[m_RLS->OrderA].im*pDst[outLen].re - m_RLS->xBuf[m_RLS->OrderA].re*pDst[outLen].im;
				y_p=y_p + (kp_p+ki_p)*eph - kp_p*eold;
				eold=eph;
				pllphase=pllphase+y_p*ko_p;	
				outLen++;
			}
			else
				m_RLS->RLS_Update(xtemp,UWsig[outLen],pDst[outLen],FALSE,aid);
		}
	}
}

void CNATO4529pro::ParaEstimate(Ipp32fc *pSrc,int nLeng,int P,Ipp32fc *pUWsig,Ipp32f &fre,Ipp32f &phase,Ipp32f &energy)
{
	Ipp32fc *pTemp = ippsMalloc_32fc(nLeng);
	int outLen;

	Timing_ini();
	Timing_estimate(pUWsig,80,pSrc,P,mtau);
	Timing_recover(pSrc,nLeng,mtau,P,pTemp,outLen);
	Fre_Estimate_LR(pUWsig,pTemp,80,fre);

	frephase = 0;
	Remove_fre(pTemp,80, fre);
	Phase_VV(pTemp,pUWsig,80,phase);

	Ipp32f pEnergy = 0;
	int i;
	for(i=0;i<80;i++)
		pEnergy = pEnergy + sqrt(pTemp[i].re*pTemp[i].re+pTemp[i].im*pTemp[i].im)/80;
	energy = pEnergy;

	ippsFree(pTemp);
	frephase = 0;
}
void CNATO4529pro::Timing_ini()
{
	xi0=0;	xi1=0;	xi2=0;
	xq0=0;	xq1=0;	xq2=0;
	muk = 0;
}
void CNATO4529pro::Timing_estimate(Ipp32fc *pUWsig,Ipp16s UWleng,Ipp32fc *pSrc,Ipp16s P, Ipp32f &tau)
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
void CNATO4529pro::Timing_recover(Ipp32fc *pSrc,int nleng,Ipp32f tau,Ipp16s P,Ipp32fc *pDst,int &outleng)
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
float CNATO4529pro::sym_interp_i(float xi3, float u,bool ini)
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
float CNATO4529pro::sym_interp_q(float xq3, float u,bool ini)
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
void CNATO4529pro::Fre_Estimate_LR(Ipp32fc *pUWsig,Ipp32fc *pSrc,Ipp16s nleng,Ipp32f &fre)
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
void CNATO4529pro::Remove_fre(Ipp32fc *pSrc,Ipp32s nLeng, Ipp32f rFreq)
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

void CNATO4529pro::Phase_VV(Ipp32fc *pSrc,Ipp32fc *pUWsig,Ipp16s len,Ipp32f &phase)
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


void CNATO4529pro::DecPSK(Ipp32fc pSrc,Ipp32fc &pDst)
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


void CNATO4529pro::DecToBit(Ipp8u pSrc,int M,Ipp8u *pDst,int &outLeng)
{
	int i;
	switch(M)
	{
	case 16:
		for (i=0;i<4;i++)
			pDst[i] = (pSrc>>(3-i))&1;
		outLeng = 4;
		break;
	case 32:
		for (i=0;i<5;i++)
			pDst[i] = (pSrc>>(4-i))&1;
		outLeng = 5;
		break;
	case 64:
		for (i=0;i<6;i++)
			pDst[i] = (pSrc>>(5-i))&1;
		outLeng = 6;
		break;
	}
}

void CNATO4529pro::SaveTobit_ini()
{
	byte_flag = 0x80;
	data_byte = 0;
	bit_num = 8;
	symbol = 0; //bit流组成byte
}
void CNATO4529pro::SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,Ipp8s modutype)
{
	Ipp32s i,m,j;
	switch(modutype)
	{
	case 1:		// 取最低比特
		m=0;
		for (i=0;i<nLeng;i++)
		{
			////////////　bit流组成byte存放　//////////////
			if((pSrc[i]&0x01) == 1)
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
	}
	byteLeng = m;
}

/************************************************************************/
/*   modutype:  1->BPSK
				2->QPSK
				3->8PSK
/************************************************************************/
void CNATO4529pro::judge(Ipp32fc *pSrc,int nLeng,Ipp8u *pDst,Ipp8s modutype)
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
				pDst[i] = 1; 
			else if (angle[i]>=IPP_PI/8 && angle[i]<IPP_PI*3/8)		//		  |	
				pDst[i] = 0;										 //	  3	  2	  1 	
			else if(angle[i]>=IPP_PI*3/8 && angle[i]<IPP_PI*5/8)	//		  |	  		 		
				pDst[i] = 2;										//  --4---|---0----		
			else if(angle[i]>=IPP_PI*5/8 && angle[i]<IPP_PI*7/8)	//		  | 	
				pDst[i] = 3;										//	  5	  6	  7 		
			else if (angle[i]>=-IPP_PI*7/8 && angle[i]<-IPP_PI*5/8)	//		  |	
				pDst[i] = 6;
			else if (angle[i]>=-IPP_PI*5/8 && angle[i]<-IPP_PI*3/8)
				pDst[i] = 4;
			else if (angle[i]>=-IPP_PI*3/8 && angle[i]<-IPP_PI*1/8)
				pDst[i] = 5;
			else
				pDst[i] = 7;
		}
		break;
	}
	ippsFree(angle);
}
void CNATO4529pro::MapFor4285(Ipp8u *pSrc,int nLeng,Ipp8u *pDst,int &outLeng,Ipp8s modutype)
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
				pDst[2*i] = 1;pDst[2*i+1] = 0;}
			if(pSrc[i]==6 || pSrc[i]==7){ 
				pDst[2*i] = 1;pDst[2*i+1] = 1;}		
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
				pDst[3*i] = 0;pDst[3*i+1] = 1;pDst[3*i+2] = 0;}
			if(pSrc[i]==3){ 
				pDst[3*i] = 0;pDst[3*i+1] = 1;pDst[3*i+2] = 1;}
			if(pSrc[i]==4){ 
				pDst[3*i] = 1;pDst[3*i+1] = 0;pDst[3*i+2] = 0;}
			if(pSrc[i]==5){ 
				pDst[3*i] = 1;pDst[3*i+1] = 0;pDst[3*i+2] = 1;}
			if(pSrc[i]==6){ 
				pDst[3*i] = 1;pDst[3*i+1] = 1;pDst[3*i+2] = 0;}
			if(pSrc[i]==7){ 
				pDst[3*i] = 1;pDst[3*i+1] = 1;pDst[3*i+2] = 1;}
		}
		outLeng = 3*nLeng;
		break;
	}

}
