
#include "stdafx.h"
#include "BW1demode.h"
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CBW1demode::CBW1demode(void)
{
	m_Preambledetect = new CPreambledetect;
	m_CommonPro = new CCommonPro;
}
CBW1demode::~CBW1demode(void)
{
	delete m_Preambledetect;
	delete m_CommonPro;
}

/************************************************************************/
/*   abs(mean(a.*conj(b)))/sqrt(mean(a.*conj(a))*mean(b.*conj(b)));                                                                   */
/************************************************************************/
void CBW1demode::CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f &cConv,Ipp32f &threod)
{	
	Ipp32fc pMeanAB;
	Ipp32f pMeanA;
	Ipp32f thdelt = 0.0554;// pf = 10^(-5)

	ippsConj_32fc(pSrcB,pConvAB,nLeng);
	ippsMul_32fc_I(pSrcA,pConvAB,nLeng);
	ippsMean_32fc(pConvAB,nLeng,&pMeanAB,ippAlgHintFast);

	ippsPowerSpectr_32fc(pSrcA,pConv,nLeng);
	ippsMean_32f(pConv,nLeng,&pMeanA,ippAlgHintFast);

	cConv = (pMeanAB.re*pMeanAB.re + pMeanAB.im*pMeanAB.im);
	threod = pMeanA*thdelt;
}

/************************************************************************/
/*   allLen		  数据块总长度，包括上一数据块结尾和本次数据块
	 proBuf		  数据块缓存,包括上一数据块结尾和本次数据块
	 winBufConv   相关运算结果值缓存
	 pConvAB,pConv  相关运算中间缓存
/************************************************************************/
void CBW1demode::Burst_detect_ini(int allLen,int convLen,int deciP,int mproBegin)
{
	winBufConv = ippsMalloc_32f(allLen);
	winBufTH = ippsMalloc_32f(allLen);
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

void CBW1demode::Burst_detect_free()
{
	ippsFree(proBuf);
	ippsFree(delayBuf);
	ippsFree(pConvAB);
	ippsFree(pConv);
	ippsFree(winBufConv);	
	ippsFree(winBufTH);
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
void CBW1demode::Burst_detect(int proBegin,int proLen,Ipp32fc *UWsig,Ipp16s convLen,Ipp16s deciP,
								Ipp16s *BurstPos,int &BurstNum,int &BufPos,int &BufHavePro)
{
	Ipp16s i;
	int pPhase=0;
	int pDstLen=0;
	Ipp32fc *pSrcA = ippsMalloc_32fc(convLen);

	int convdataL = convLen*deciP ;
	int compareL = 4*deciP;
	int winBufp = 0;
	Ipp32f pMax;
	int pIndex;
	Ipp32f cconv;
	Ipp32f threod;
	
	BurstNum=0;
	
	ippsCopy_32fc(delayBuf,&proBuf[proBegin],delayL);
	for (i=0;i<proLen - convdataL;i++)
	{
		ippsSampleDown_32fc(&proBuf[proBegin+i],convdataL,pSrcA,&pDstLen,deciP,&pPhase);// 间隔P点抽出数据
		CrossConvCof(pSrcA,UWsig,convLen,cconv,threod);// 相关系数	
		winBufConv[winBufp] = cconv;
		winBufTH[winBufp] = threod;
		winBufp++;
	}
	for (i=0;i<winBufp-compareL;i++)
	{
		if (winBufConv[i]>winBufTH[i] && winBufConv[i+1]>winBufTH[i+1])
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

void CBW1demode::DownFre_ini(Ipp32s nLeng,Ipp32f roll,Ipp32f Baud,Ipp16s P,int SrctapLen)
{

	pSrcTap = ippsMalloc_64f(SrctapLen);
	Ipp64f fstop = (Baud*(1+0.75))/(9600*2);//滤波器带宽要适当大些，
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
void CBW1demode::DownFre_free()
{
	ippsFree(pSrcTap);
	ippsFree(pDelayreal);
	ippsFree(pDelayimag);
	ippsFIRFree64f_32f(SrcStateReal);
	ippsFIRFree64f_32f(SrcStateImag);
	ippsFree(pReal);
	ippsFree(pImag);

}
void CBW1demode::DownFre(Ipp16s *pSrc,int nLeng,Ipp32f frequency,Ipp32fc *pDst,int &DstLen)
{
	int i;
	int OutLen;
	for (i=0;i<nLeng;i++)
	{
		defrephase=defrephase-IPP_2PI*frequency/9600.0;
		if ((defrephase)>=IPP_2PI || (defrephase)<=-IPP_2PI)
			defrephase=fmod((double)defrephase,(double)IPP_2PI);
		pReal[i]=pSrc[i]*cos(defrephase);
		pImag[i]=pSrc[i]*sin(defrephase);
	}
	ippsFIR64f_32f_I(pReal, nLeng, SrcStateReal);
	ippsFIR64f_32f_I(pImag, nLeng, SrcStateImag);
	ippsRealToCplx_32f(pReal,pImag,pDst,nLeng);
	DstLen = nLeng;
}
void CBW1demode::Demode_ini(Ipp32s nLeng,Ipp16s P,Ipp32f roll,Ipp32f Baud,int SrctapLen)
{
	defrephase = 0;
	DownFre_ini(nLeng,roll,Baud,P,SrctapLen);	
	m_Preambledetect->Preamble_Gen(5);
	preambleLen = m_Preambledetect->PreambleLen*P;

	walshdataBufLen = 64*P;  // 

	int allLen = nLeng*2 + preambleLen;	
	Burst_detect_ini(allLen,preambleLen/P,P,preambleLen);

	pidx = 0;
	pBurst = 0;
	flag = 0;
	walshTime = 0;
	byteloadLen = 0;
	SaveTobit_ini();

}
void CBW1demode::Demode_free()
{
	DownFre_free();
	Burst_detect_free();
}
void CBW1demode::Demode(Ipp16s *pSrc,int nLeng,Ipp16s P,Ipp32f frequency,int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType141B *headType,int &headNum)
{
	int UWleng = m_Preambledetect->PreambleLen;
	preambleLen = UWleng*P;
	Ipp16s *BurstPos = ippsMalloc_16s(nLeng/preambleLen*2+1);
	int BurstNum;
	int BufHavePro = 0;

	int dataLen;
	DownFre(pSrc,nLeng,frequency,&proBuf[proBegin+BufPos],dataLen);
	dataLen = dataLen+BufPos;
	Burst_detect(proBegin,dataLen,m_Preambledetect->pPreambleSym,UWleng,P,BurstPos,BurstNum,BufPos,BufHavePro);

	headNum = 0;
	int mbyteleng;
	outLeng = 0;
	byteLeng = 0;
	Ipp32f pMax;
	////////////////////
	pBurst = 0;	
	while(pidx + preambleLen<BufHavePro)
	{
		//第一次找起点
		if (flag==0 && BurstNum>0 && pidx<=BurstPos[pBurst] && BurstPos[pBurst]<=pidx+preambleLen)
		{
			flag = 1;
			pidx = BurstPos[pBurst];
			pBurst++;
			BurstNum--;
			walshTime = 0;
			byteloadLen = 0;

			

			/*saveposition = pidx;
			savefrequency = frequency;	
			deflag = 1;*/
		}
		else if(flag==1)
		{
			headType[headNum].position = pidx;
			headType[headNum].fre = frequency;
			headType[headNum].BWtype = wMIL141BBW1;
			headNum++;
			pidx = pidx + preambleLen ; 
			flag=2;
		}
		else if(flag==2)
		{
			pidx = pidx + 64*P-4; //  preambleLen = 512*4   实际应为576*4
			flag = 3;
		}
		else if (flag==3)// 找到前导后解walsh
		{
			DeWalsh(&proBuf[proBegin+pidx],walshdataBufLen,P,8,walshTime,&byteload[byteloadLen],mbyteleng,pMax);
			byteloadLen = byteloadLen + mbyteleng;
			walshTime++;
			pidx = pidx + walshdataBufLen;
			if(walshTime==26 && pMax<0.5)// 如果为BW5，此时已经没有相关峰
			{
				DeInterDecode5(byteload,byteloadLen-4,outbyte,byteLeng);
				walshTime = 0;
				byteloadLen = 0;
				flag=0;
				if(deflag==1)
				{
					headType[headNum].position = saveposition;
					headType[headNum].fre = savefrequency;
					headType[headNum].BWtype = wMIL141BBW5;
					headNum++;
					deflag = 0;
				}	
			}
			else if(walshTime==36)
			{
				DeInterDecode1(byteload,byteloadLen,outbyte,byteLeng);  // 换成（3,1,8）卷积码
				walshTime = 0;
				byteloadLen = 0;
				flag=0;

				if(deflag==1)
				{
					headType[headNum].position = saveposition;
					headType[headNum].fre = savefrequency;
					headType[headNum].BWtype = wMIL141BBW1;
					headNum++;
					deflag = 0;
				}
			}
		} 
		else
			pidx = pidx + preambleLen;                  
	}
	while(BurstNum)
	{
		if(BurstPos[pBurst]>BufHavePro-preambleLen)
		{
			pidx = BurstPos[pBurst];
			/*saveposition = pidx;
			savefrequency = frequency;	
			deflag = 1;*/
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
void CBW1demode::DeWalsh(Ipp32fc *pSrc,int nLen,int P,int winLen,int time,byte *outbyte,int &outbytelen,Ipp32f &pMax)
{
	int i;
	Ipp32fc *ptemp = ippsMalloc_32fc(64);
	int dstLen;
	int pphase=0;
	int pIndex,pIndextemp;
	float tempMax;
	pMax = 0;
	//{解walsh
	m_CommonPro->GenWalshPN015(time);
	for(i=0;i<winLen;i++)
	{
		ippsSampleDown_32fc(&pSrc[i],nLen,ptemp,&dstLen,P,&pphase);
		m_CommonPro->DeWalsh(ptemp,16,64,tempMax,pIndextemp);
		if(tempMax>pMax)
		{
			pMax = tempMax;
			pIndex = pIndextemp;
		}
	}
	
	switch(pIndex)
	{
	case 0:
		outbyte[0]=0;outbyte[1]=0;outbyte[2]=0;outbyte[3]=0;
		break;
	case 1:
		outbyte[0]=0;outbyte[1]=0;outbyte[2]=0;outbyte[3]=1;
		break;
	case 2:
		outbyte[0]=0;outbyte[1]=0;outbyte[2]=1;outbyte[3]=0;
		break;
	case 3:
		outbyte[0]=0;outbyte[1]=0;outbyte[2]=1;outbyte[3]=1;
		break;
	case 4:
		outbyte[0]=0;outbyte[1]=1;outbyte[2]=0;outbyte[3]=0;
		break;
	case 5:
		outbyte[0]=0;outbyte[1]=1;outbyte[2]=0;outbyte[3]=1;
		break;
	case 6:
		outbyte[0]=0;outbyte[1]=1;outbyte[2]=1;outbyte[3]=0;
		break;
	case 7:
		outbyte[0]=0;outbyte[1]=1;outbyte[2]=1;outbyte[3]=1;
		break;
	case 8:
		outbyte[0]=1;outbyte[1]=0;outbyte[2]=0;outbyte[3]=0;
		break;
	case 9:
		outbyte[0]=1;outbyte[1]=0;outbyte[2]=0;outbyte[3]=1;
		break;
	case 10:
		outbyte[0]=1;outbyte[1]=0;outbyte[2]=1;outbyte[3]=0;
		break;
	case 11:
		outbyte[0]=1;outbyte[1]=0;outbyte[2]=1;outbyte[3]=1;
		break;
	case 12:
		outbyte[0]=1;outbyte[1]=1;outbyte[2]=0;outbyte[3]=0;
		break;
	case 13:
		outbyte[0]=1;outbyte[1]=1;outbyte[2]=0;outbyte[3]=1;
		break;
	case 14:
		outbyte[0]=1;outbyte[1]=1;outbyte[2]=1;outbyte[3]=0;
		break;
	case 15:
		outbyte[0]=1;outbyte[1]=1;outbyte[2]=1;outbyte[3]=1;
		break;
	default:
		break;

	}
	outbytelen = 4;
	ippsFree(ptemp);
}
void CBW1demode::DeInterDecode1(byte *intbyte,int nLen,byte *pDest,int &pDestlen)
{
	int i;
	//{解交织  16*9
	InterleaverPam m_InterleaverPam;
	m_InterleaverPam.Rows=16;m_InterleaverPam.Cols=9;
	m_InterleaverPam.irs=0;m_InterleaverPam.ics =1 ;m_InterleaverPam.dirs=1;m_InterleaverPam.dics=0;
	m_InterleaverPam.irf=1;m_InterleaverPam.icf =0 ;m_InterleaverPam.dirf=0;m_InterleaverPam.dicf=1;
	int outinterlen = m_CommonPro->DeInterleaver(intbyte,&m_InterleaverPam);
	//}
	//{译码
//	pDestlen = m_CommonPro->DeConverCode(intbyte,outinterlen,pDest);
	//}	
	SaveTobit(intbyte,outinterlen,pDest,pDestlen,1);
}
void CBW1demode::DeInterDecode5(byte *intbyte,int nLen,byte *pDest,int &pDestlen)
{
	int i;
	//{解交织  10*10
	InterleaverPam m_InterleaverPam;
	m_InterleaverPam.Rows=10;m_InterleaverPam.Cols=10;
	m_InterleaverPam.irs=0;m_InterleaverPam.ics =1 ;m_InterleaverPam.dirs=1;m_InterleaverPam.dics=0;
	m_InterleaverPam.irf=1;m_InterleaverPam.icf =0 ;m_InterleaverPam.dirf=0;m_InterleaverPam.dicf=1;
	int outinterlen = m_CommonPro->DeInterleaver(intbyte,&m_InterleaverPam);
	//}
	//{译码
	byte temp[50];
	int temple;
	temple = m_CommonPro->DeConverCode(intbyte,outinterlen,temp);
	//m_Viterbi->Decode(intbyte,outinterlen,temp,&temple);
	SaveTobit(temp,temple,pDest,pDestlen,1);
	//}	
}
void CBW1demode::SaveTobit_ini()
{
	byte_flag = 0x80;
	data_byte = 0;
	bit_num = 8;
	symbol = 0; //bit流组成byte
}
void CBW1demode::SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,Ipp8s modutype)
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

