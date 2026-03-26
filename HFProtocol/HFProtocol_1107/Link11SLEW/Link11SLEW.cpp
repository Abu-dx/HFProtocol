// Link11SLEW.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "Link11SLEW.h"
#include <math.h>
#include "Preamble.h"
#include "Link11Decoder.h"
#include "JCRC.h"
#include "RLS.h"
#include "Preambledetect.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CLink11SLEW::CLink11SLEW(void)
{
	probe_sym = NULL;
	pDecoder = NULL;
	pCRC = NULL;
	m_RLS = NULL;
}

CLink11SLEW::~CLink11SLEW(void)
{
	
}
void CLink11SLEW::PSK_map(short style, short M, float I_map[],float Q_map[])
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
int CLink11SLEW::PSK_MOD( short data[], short data_len, short M,Ipp32fc *out)
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
//////////////////////////
//  N阶根升余弦滤波器
//////////////////////////
void CLink11SLEW::SRcos_Filter(int N, float RollOff, float fBaudRate, float fSampleRate, float *fMF_Coef)
{

	float T,R,fs,fd,Sqrt_t;
	float c, S1,S2,D;
	int i,L;

	L=(N-1)/2;
	fs=fSampleRate;
	fd=fBaudRate;
	R=RollOff;
	T=fs/fd;
	Sqrt_t=sqrt(T);

	for (i=-1*L;i<0;i++)
	{ 
		c=cos((1+R)*IPP_PI*i/T);
		S1=sin((1-R)*IPP_PI*i/T);
		S2=4*R*i/T;
		D=IPP_PI*Sqrt_t* ( 1- (4*R*i/T)*(4*R*i/T) ) ;
		fMF_Coef[i+L]=4*R *(c+S1/S2)/D;
	}
	fMF_Coef[i+L]=4*R*(1+(1-R)*IPP_PI/(4*R))/(IPP_PI*Sqrt_t);
	for (i=1;i<=L+1;i++)
	{ 
		c=cos((1+R)*IPP_PI*i/T);
		S1=sin((1-R)*IPP_PI*i/T);
		S2=4*R*i/T;
		D=IPP_PI*Sqrt_t* ( 1- (4*R*i/T)*(4*R*i/T) ) ;
		fMF_Coef[i+L]=4*R *(c+S1/S2)/D;
	}
	return;
}
/************************************************************************/
/*   abs(mean(a.*conj(b)))/sqrt(mean(a.*conj(a))*mean(b.*conj(b)));                                                                   */
/************************************************************************/

void CLink11SLEW::CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f *cConv)
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
void CLink11SLEW::Burst_detect_ini(int allLen,int convLen,int deciP,int mproBegin)
{
	winBufConv = ippsMalloc_32f(allLen);
	ippsZero_32f(winBufConv,allLen);
	proBuf = ippsMalloc_32fc(allLen);

	delayL = convLen*deciP+20;
	delayBuf = ippsMalloc_32fc(delayL);
	delayL = 0;

	pConvAB = ippsMalloc_32fc(convLen);
	pConv = ippsMalloc_32f(convLen);

	proBegin = mproBegin;
	BufPos = 0;

}

void CLink11SLEW::Burst_detect_free()
{
	ippsFree(proBuf);
	ippsFree(delayBuf);
	ippsFree(pConvAB);
	ippsFree(pConv);
	ippsFree(winBufConv);	
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
void CLink11SLEW::Burst_detect(int proBegin,int proLen,Ipp32fc *UWsig,Ipp32f UWEnergy, Ipp16s convLen,Ipp16s deciP,
									  Ipp32f Threshold,Ipp16s *BurstPos,int &BurstNum,int &BufPos,int &BufHavePro)
{
	Ipp16s i;
	int pPhase=0;
	int pDstLen=0;
	Ipp32fc *pSrcA = ippsMalloc_32fc(convLen);

	int convdataL = convLen*deciP ;
	int compareL = 2*deciP;
	int winBufp = 0;
	Ipp32f pMax;
	int pIndex;
	Ipp32f cconv;
	
	BurstNum=0;
	
	ippsCopy_32fc(delayBuf,&proBuf[proBegin],delayL);
	for (i=0;i<proLen - convdataL;i++)
	{
		ippsSampleDown_32fc(&proBuf[proBegin+i],convdataL,pSrcA,&pDstLen,deciP,&pPhase);// 间隔P点抽出数据
		CrossConvCof(pSrcA,UWsig,convLen,UWEnergy,&cconv);// 相关系数	
		winBufConv[winBufp] = cconv;
		winBufp++;
	}
	for (i=0;i<winBufp-compareL;i++)
	{
		if (winBufConv[i]>Threshold && winBufConv[i+1]>Threshold)
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
	BufHavePro = winBufp-compareL;
	BufPos = delayL;

}

void CLink11SLEW::DownFre_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,int FFTLen,Ipp32f roll,Ipp32f Baud,Ipp16s P,int SrctapLen)
{
	m_Outsample = Outsample;
	pfre = fre/Outsample;
	m_Preambledetect->Preamble_Gen();
	m_Preambledetect->ProtolDetect_ini(nLeng,m_Preambledetect->PreambleLen,FFTLen,Insample,Outsample,P);

	pSrcTap = ippsMalloc_64f(SrctapLen);
//	SRcos_Filter(SrctapLen,roll,Baud,Baud*P,pSrcTap);
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
void CLink11SLEW::DownFre_free()
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
void CLink11SLEW::DownFre(Ipp16s *pSrc,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,Ipp32fc *pDst,int &DstLen)
{
	int i;
	int BurstNum;
	int BufHavePro = 0;
	int UWleng = m_Preambledetect->PreambleLen;
	Ipp16s *BurstPos = ippsMalloc_16s(nLeng/UWleng*2);
	Ipp32f *fre = ippsMalloc_32f(nLeng/UWleng*2);
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
}

void CLink11SLEW::Demode_PSKrealFSE_ini(Ipp32f Insample,Ipp32f Outsample,Ipp32s nLeng,Ipp32f fre,Ipp16s P,int FFTLen,Ipp32f roll,Ipp32f Baud,int SrctapLen)
{

	m_Preambledetect = new CPreambledetect;
	SrcLen = SrctapLen;
	DownFre_ini(Insample,Outsample,nLeng,fre,FFTLen,roll,Baud,P,SrctapLen);	

	int UWleng = m_Preambledetect->PreambleLen;
	OneFrameLen = (UWleng+5)*P;  //  一段帧长,包括FSE的延时，与FSE阶数有关
	OneFrame = ippsMalloc_32fc(OneFrameLen);

	int allLen = nLeng*2 + (UWleng+10)*P + OneFrameLen*2;	
	Burst_detect_ini(allLen,UWleng,P,OneFrameLen*2);

	probe = ippsMalloc_16s(45);
	probe_sym = ippsMalloc_32fc(45);

	pfre = fre/Outsample;
	defrephase = 0;
	frephase = 0;
	pllphase = 0;
	probeNum = 0;
	Timing_ini();
	EqualizationRLS_DFE_PLL_ini(0.01,0.707,8,pllphase,18,15,100,0.99);
	SaveTobit_ini();
	pDecoder = new Link11Decoder;
	pCRC  = new JCRC;

	byte polynomial[12]={0,1,0,1,0,0,1,1,1,0,0,1};
	pCRC->setGPoly(polynomial,12);

	
	pidx = 0;
	pBurst = 0;
	flag = 0;

}
void CLink11SLEW::Demode_PSKrealFSE_free()
{
	ippsFree(OneFrame);
	ippsFree(probe);
	ippsFree(probe_sym);
	DownFre_free();
	Burst_detect_free();

	EqualizationRLS_DFE_PLL_free();
	delete pDecoder;
	delete pCRC;
	delete m_Preambledetect;

}

void CLink11SLEW::Demode_PSKrealFSE(Ipp16s *pSrcDst,int nLeng,Ipp16s P,int FFTLen,Ipp32f TH1,Ipp32f TH2,
									int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType *headType,int &headNum)
{
	int i;
	int UWleng = m_Preambledetect->PreambleLen;
	Ipp16s *BurstPos = ippsMalloc_16s(nLeng/UWleng*2+1);
	int BurstNum;
	int BufHavePro = 0;

	int dataLen;
	DownFre(pSrcDst,nLeng,P,FFTLen,TH1,TH2,&proBuf[proBegin+BufPos],dataLen);
	dataLen = dataLen+BufPos;
	Burst_detect(proBegin,dataLen,m_Preambledetect->pPreambleSym,1,UWleng,P,TH2,BurstPos,BurstNum,BufPos,BufHavePro);

	/*outLeng = 0;
	for (i=0;i<BufHavePro;i++)
	{
		pSrcDst[outLeng + 2*i]=Ipp16s(proBuf[proBegin+i].re);
		pSrcDst[outLeng + 2*i+1]=Ipp16s(proBuf[proBegin+i].im);
	}
	outLeng = outLeng + 2*BufHavePro;*/

	headNum = 0;
	int mheadNum;
	int TimeOutLen=0,mbyteleng;
	outLeng = 0;
	byteLeng = 0;
	//
	pBurst = 0;	
	while(pidx + (UWleng+5)*P<BufHavePro)
	{
		TimeOutLen = 0;
		//第一次找起点
		if (flag==0 && BurstNum>0 && pidx<=BurstPos[pBurst] && BurstPos[pBurst]<=pidx+OneFrameLen)
		{
			flag = 1;
			pidx = BurstPos[pBurst];
			pBurst++;
			BurstNum--;
		}
		else if (flag==1)// 找到前导进行同步估计
		{
			OneFrameLen = (UWleng+5)*P; 
		    Synchronization_RLSCMA_forSLEW(&proBuf[proBegin+pidx],OneFrameLen,P,m_Preambledetect->pPreambleSym,UWleng,5,OneFrame,TimeOutLen,&outbyte[byteLeng],mbyteleng,&headType[headNum],mheadNum,1);
			byteLeng = byteLeng +mbyteleng;
			headNum = headNum + mheadNum;
			pidx = pidx + OneFrameLen;
			flag = 2;
			OneFrameLen = (UWleng)*P; 

		}//  找到下一个前导
		
		else if (flag==2)  // 同步跟踪
		{
		    Synchronization_RLSCMA_forSLEW(&proBuf[proBegin+pidx],OneFrameLen,P,m_Preambledetect->pPreambleSym,UWleng,5,OneFrame,TimeOutLen,&outbyte[byteLeng],mbyteleng,&headType[headNum],mheadNum,0);
			byteLeng = byteLeng +mbyteleng;
			if(mheadNum>0)
				headType[headNum].position = pidx-OneFrameLen-SrcLen/2;
			headNum = headNum + mheadNum;
			pidx = pidx + OneFrameLen;
			if(BurstNum>0 && pidx<=BurstPos[pBurst] && BurstPos[pBurst]<=pidx+OneFrameLen)
			{
				flag=3;
			}
		}
		else if (flag==3 && BurstNum>0 && pidx<=BurstPos[pBurst] && BurstPos[pBurst]<=pidx+OneFrameLen)
		{
			TimeOutLen = 0;
			flag = 1;
			pidx = BurstPos[pBurst];
			pBurst++;
			BurstNum--;	

		}
		else
			pidx = pidx + OneFrameLen;

		// 同步输出
		if (TimeOutLen>0)
		{
			//TRACE("%d_",TimeOutLen);
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
		if(BurstPos[pBurst]>BufHavePro-(UWleng+5)*P)
		{
			pidx = BurstPos[pBurst];
			flag=1;
			break;
		}
		pBurst++;
		BurstNum--;	
	}
	//// 处理缓存结尾, 将结尾数据复制到缓存头部，以备下次使用 
	ippsCopy_32fc(&proBuf[proBegin+pidx],&proBuf[proBegin-(BufHavePro-pidx)],BufHavePro-pidx);
	pidx = -(BufHavePro-pidx);
	ippsFree(BurstPos);
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
void CLink11SLEW::Synchronization_RLSCMA_forSLEW(Ipp32fc *FrameBuf,Ipp16s FrameLen,Ipp16s P,Ipp32fc *pUWsig,Ipp16s UWleng,int delayCma,
									  Ipp32fc *pDst,int &outleng,Ipp8u *outbyte,int &byteleng,HeadType *headType,int &headNum,bool bBegin)
{
	int i,k,p=0;
	int pDownSLen=0;
	int moutlen=0;
	int mbyteleng=0;
	int bcrc;
	int pPhase = 0;
	byteleng = 0;
	Ipp8u *pJudge = ippsMalloc_8u(FrameLen/P+20);
	Ipp8u *pDephase = ippsMalloc_8u(90*3);
	Ipp8u *pInterl = ippsMalloc_8u(90*3);
	int DephaseLeng;
	Ipp32fc *pDownS = ippsMalloc_32fc(FrameLen/2+20);
	
	headNum = 0;
	if(bBegin)
	{
		ParaEstimate(FrameBuf,FrameLen,P,pUWsig,mfre,pllphase,pEnergy);
		ippsSampleDown_32fc(FrameBuf,FrameLen,pDownS,&pDownSLen,2,&pPhase);

		frephase = 0;
		mfre = mfre/2;
		Remove_fre(pDownS,pDownSLen,mfre);
		m_RLS->RLS_SetZero();
		EqualizationRLS_DFE_PLL(pDownS,pDownSLen,2,delayCma*2,pEnergy,pUWsig,pDst,outleng,TRUE);	
		
		probeNum = 0;
		FrameIdx = 0;
		TRACE("\r\n");
		TRACE("\*");
	}
	else
	{
		pPhase = 0;
		moutlen = 0;outleng = 0;
		ippsSampleDown_32fc(FrameBuf,FrameLen,pDownS,&pDownSLen,2,&pPhase);
		Remove_fre(pDownS,pDownSLen,mfre);		
		//	 判决反馈均衡
		while(p+128<=pDownSLen)
		{
			EqualizationRLS_DFE_PLL(&pDownS[p],90,2,0,pEnergy,pUWsig,&pDst[outleng],moutlen,FALSE);
			
			////  去扰
			for (i=0;i<45;i++)
			{
				k = probeNum*64+i;
				k = k%160;
				probe[i] =SLEWprobe_sequence[k];
			//	probeNum++;
				
			}
			PSK_MOD(probe,45,8,probe_sym);
			ippsConj_32fc_I(probe_sym,45);
			ippsMul_32fc(probe_sym,&pDst[outleng],pDownS,45);
			judge(pDownS,45,pJudge,4);	
			DeBitPair_PhaseDecode(pJudge,45,pDephase,DephaseLeng);
			deinterleaver(pDephase,pInterl);

			if (FrameIdx==0)//header
			{
				DephaseLeng = pDecoder->DeConverCode(pInterl,90,pDephase);
				bcrc = pCRC->sendData(pDephase,DephaseLeng); // CRC校验
				TRACE("%d_",bcrc);
				
				HeadDecode(pDephase,bcrc,headType[headNum]);
				headNum++;
				// 取前33比特		
				DephaseLeng = 33;
				SaveTobit(pDephase,DephaseLeng,&outbyte[byteleng],mbyteleng,1);
				byteleng = byteleng + mbyteleng;
				if((!bcrc) && ((pDephase[0]&0x01)==0))
				{
					flag=3;
					break;
				}
				lastframe = 0;
			}
			else
			{
				int out = FindEnd(pInterl,90);
				if(out==1 || out==0) // 全1
				{
					flag=3;
					DephaseLeng = 48;
					SaveTobit(pDephase,DephaseLeng,&outbyte[byteleng],mbyteleng,1);
					byteleng = byteleng + mbyteleng;
					break;
				}
				DephaseLeng = pDecoder->DeConverCodeDePunctur2(pInterl,90,pDephase);
				bcrc = pCRC->sendData(pDephase,DephaseLeng); // CRC 校验
				TRACE("%d_",bcrc);
				// 取前48比特
				DephaseLeng = 48;
				SaveTobit(pDephase,DephaseLeng,&outbyte[byteleng],mbyteleng,1);
				byteleng = byteleng + mbyteleng;
			}			
			
			// 获得信道探测序列
			for (i=0;i<19;i++)
			{
				k = 45+probeNum*64+i;
				k = k%160;
				probe[i] = SLEWprobe_sequence[k];
			//	probeNum++;
			}					
			PSK_MOD(probe,19,8,probe_sym);

			// 利用信道探测序列均衡
			outleng = outleng + moutlen;
			EqualizationRLS_DFE_PLL(&pDownS[p+90],19*2,2,0,pEnergy,probe_sym,&pDst[outleng],moutlen,TRUE);

			ippsConj_32fc_I(probe_sym,19);
			ippsMul_32fc(probe_sym,&pDst[outleng],pDownS,19);
			judge(pDownS,19,pJudge,4);
			DeBitPair_PhaseDecode(pJudge,19,pDephase,DephaseLeng);
			/*SaveTobit(pDephase,DephaseLeng,&outbyte[byteleng],mbyteleng,1);
			byteleng = byteleng + mbyteleng;*/

			probeNum++;
			outleng = outleng + moutlen;
			p = p + 128;
			FrameIdx++;
		}
		
	}
	ippsFree(pJudge);
	ippsFree(pDephase);
	ippsFree(pInterl);
	ippsFree(pDownS);
}
void CLink11SLEW::ParaEstimate(Ipp32fc *pSrc,int nLeng,int P,Ipp32fc *pUWsig,Ipp32f &fre,Ipp32f &phase,Ipp32f &energy)
{
	Ipp32fc *pTemp = ippsMalloc_32fc(nLeng);
	int outLen;

	Timing_ini();
	Timing_estimate(pUWsig,192,pSrc,P,mtau);
	Timing_recover(pSrc,nLeng,mtau,P,pTemp,outLen);
	Fre_Estimate_LR(pUWsig,pTemp,192,fre);

	frephase = 0;
	Remove_fre(pTemp,192, fre);
	Phase_VV(pTemp,pUWsig,192,phase);

	Ipp32f pEnergy = 0;
	int i;
	for(i=0;i<192;i++)
		pEnergy = pEnergy + sqrt(pTemp[i].re*pTemp[i].re+pTemp[i].im*pTemp[i].im)/192;
	energy = pEnergy*2;

	ippsFree(pTemp);
	frephase = 0;
}
void CLink11SLEW::EqualizationRLS_DFE_PLL_ini(Ipp32f wn, Ipp32f kesai,Ipp16s M_phase,Ipp32f pll_phase,
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

void CLink11SLEW::EqualizationRLS_DFE_PLL_free()
{
	m_RLS->RLS_free();
	if(m_RLS){
		delete m_RLS;
		m_RLS = NULL;
	}
}
//  分数间隔判决反馈均衡，针对PSK
void CLink11SLEW::EqualizationRLS_DFE_PLL(Ipp32fc *pSrc,int nLeng,int P,int delay,Ipp32f mEnergy,Ipp32fc *UWsig,Ipp32fc *pDst,int &outLen,BOOL aid)
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
int CLink11SLEW::FindEnd(Ipp8u *pSrc,int nLeng)
{
	int i;
	int out=2;
	int num0=0,num1=0;
	for (i=0;i<nLeng;i++)
	{
		if((pSrc[i]&0x01)==0)
			num0++;
		if((pSrc[i]&0x01)==1)
			num1++;
	}
	if((num0>=(nLeng-3)))
		out = 0;
	else if((num1>=(nLeng-3)))
		out = 1;
	else
		out = 2;
	return out;

}
void CLink11SLEW::HeadDecode(Ipp8u *pSrc,int crcerr,HeadType &mheadType)
{
	int i;
	if(crcerr==0)  
	{
		mheadType.Type = 0;  // 00 主站询问  10 主站报告  11 前哨回复
		if((pSrc[0]&0x01)==1)
			mheadType.Type |= 0x02;
		if((pSrc[31]&0x01)==1)
			mheadType.Type |= 0x01;

		mheadType.address = 0;
		byte byteflag=0x20;
		for (i=0;i<6;i++)
		{
			if(pSrc[i+1]&0x01==1)
				mheadType.address|=byteflag;
			byteflag >>= 1;
		}
	}
	else
	{
		mheadType.Type = 0x01; // 未知
		mheadType.address = 0;
	}
	mheadType.crcerr = crcerr;

	mheadType.frequency = pfre*m_Outsample;

}

void CLink11SLEW::DecToBit(Ipp8u pSrc,int M,Ipp8u *pDst,int &outLeng)
{
	int i;
	switch(M)
	{
	case 8:
		for (i=0;i<3;i++)
			pDst[i] = (pSrc>>(2-i))&1;
		outLeng = 3;
		break;
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

/************************************************************************/
/* 数据辅助的LR频偏估计                                                  */
/************************************************************************/
void CLink11SLEW::Fre_Estimate_LR(Ipp32fc *pUWsig,Ipp32fc *pSrc,Ipp16s nleng,Ipp32f &fre)
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

void CLink11SLEW::Remove_fre(Ipp32fc *pSrc,Ipp32s nLeng, Ipp32f rFreq)
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

void CLink11SLEW::Phase_VV(Ipp32fc *pSrc,Ipp32fc *pUWsig,Ipp16s len,Ipp32f &phase)
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
/////////////////////////////////////////////////////////////////////////////
//  dfpll_phase: 判决反馈锁相环，用于载频相差恢复                          //
//  arg:判决点经修正后的相位；                                             //
//  mod:躁声容限的绝对值，对于8PSK,躁声容限为-M_PI/8---+M_PI/8,mod=M_PI/4  //
//  ko:数控振荡器增益；                                                    //
//  k1:一阶环路滤波器系数 
//  mode=2*M_PI/(float)PSK_M;  
//  收敛到0相位                                            
/////////////////////////////////////////////////////////////////////////////
void CLink11SLEW::dfpll_phase_ini(Ipp32f wn, Ipp32f kesai,Ipp16s M_phase)
{
	wn_p=wn;
	kesai_p=kesai;
	ko_p=1.0;
	kd_p=1;
	kp_p=4*kesai_p*wn_p/((1+2*kesai_p*wn_p+wn_p*wn_p)*ko_p*kd_p);
	ki_p=kp_p*wn_p/kesai_p;  
	mod=2*IPP_PI/(M_phase); 
	eold=0;
	y_p=0;
}
void CLink11SLEW::dfpll_phase(Ipp32fc *pSrc,Ipp32s nLeng,Ipp32f pll_phase)
{
	float e;
	Ipp32f arg,temp;
	pllphase=pll_phase;
	Ipp32s i;
	for (i=0;i<nLeng;i++)
	{
		temp=pSrc[i].re;
		pSrc[i].re=pSrc[i].re*cos(pllphase)-pSrc[i].im*sin(pllphase);
		pSrc[i].im=pSrc[i].im*cos(pllphase)+temp*sin(pllphase);

		if(pSrc[i].re==0)
			if(pSrc[i].im>=0)arg=IPP_PI/2;
			else arg=-IPP_PI/2;
		else arg=atan2(pSrc[i].im,pSrc[i].re);

		e=fmod(float(arg+IPP_PI),mod);
		if(e>(mod/2.0))
			e=e-mod;
		e=sin(e);
		y_p=y_p + (kp_p+ki_p)*e - kp_p*eold;
		eold=e;
		pllphase=pllphase+y_p*ko_p;	
	}
}

void CLink11SLEW::Timing_ini()
{
	xi0=0;	xi1=0;	xi2=0;
	xq0=0;	xq1=0;	xq2=0;
	muk = 0;

}
void CLink11SLEW::Timing_estimate(Ipp32fc *pUWsig,Ipp16s UWleng,Ipp32fc *pSrc,Ipp16s P, Ipp32f &tau)
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
void CLink11SLEW::Timing_recover(Ipp32fc *pSrc,int nleng,Ipp32f tau,Ipp16s P,Ipp32fc *pDst,int &outleng)
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
float CLink11SLEW::sym_interp_i(float xi3, float u,bool ini)
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
float CLink11SLEW::sym_interp_q(float xq3, float u,bool ini)
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


void CLink11SLEW::DeBitPair_PhaseDecode(Ipp8u *pSrc,int nLeng,Ipp8u *pDst,int &outLeng)
{
/*	Ipp32s i;
	for(i=0;i<nLeng;i++)
	{
		if (pSrc[i]==0 )
		{
			pDst[2*i]=0;
			pDst[2*i+1]=0;
		}
		else if (pSrc[i]==1)
		{
			pDst[2*i]=0;
			pDst[2*i+1]=1;
		}
		else if (pSrc[i]==3 )
		{
			pDst[2*i]=1;
			pDst[2*i+1]=1;
		}
		else if (pSrc[i]==2)
		{
			pDst[2*i]=1;
			pDst[2*i+1]=0;
		}
	}
	outLeng = 2*nLeng;*/
	Ipp32s i;
	for(i=0;i<nLeng;i++)
	{
		if (pSrc[i]==0 || pSrc[i]==1)
		{
			pDst[2*i]=0;
			pDst[2*i+1]=0;
		}
		else if (pSrc[i]==2 || pSrc[i]==3)
		{
			pDst[2*i]=0;
			pDst[2*i+1]=1;
		}
		else if (pSrc[i]==4 || pSrc[i]==5)
		{
			pDst[2*i]=1;
			pDst[2*i+1]=1;
		}
		else if (pSrc[i]==6 || pSrc[i]==7)
		{
			pDst[2*i]=1;
			pDst[2*i+1]=0;
		}
	}
	outLeng = 2*nLeng;
}
void CLink11SLEW::deinterleaver(Ipp8u *pSrc,Ipp8u *pDst)
{
	int i,j;
	for(i=0;i<90;i++)
	{
		j = (i*17)%90;
		pDst[j] = pSrc[i];
	}
}

void CLink11SLEW::SaveTobit_ini()
{
	byte_flag = 0x80;
	data_byte = 0;
	bit_num = 8;
	symbol = 0; //bit流组成byte
}
void CLink11SLEW::SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,Ipp8s modutype)
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
void CLink11SLEW::judge(Ipp32fc *pSrc,Ipp16s nLeng,Ipp8u *pDst,Ipp8s modutype)
{
	Ipp32f *angle;
	Ipp32s i;
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
			if(angle[i]>=-IPP_PI/4 && angle[i]<IPP_PI/4)
                pDst[i] = 0;
			else if (angle[i]>=IPP_PI/4 && angle[i]<3*IPP_PI/4)
				pDst[i] = 1;
			else if (angle[i]>=-3*IPP_PI/4 && angle[i]<-IPP_PI/4)
				pDst[i] = 2;
			else
				pDst[i] = 3;
            
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