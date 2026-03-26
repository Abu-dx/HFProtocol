
#include "stdafx.h"
#include "CommonPro.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CCommonPro::CCommonPro(void)
{
	length_Buffer = 0;
	pBuffer = 0;

	length_Outbits = 0;
	cuOutBits = 0;
	for (int i=128;i--;)
		statusInfo[i].setIndex(i);

	pConvAB = ippsMalloc_32fc(64);
	pConv = ippsMalloc_32f(64);

	walshpnsym = new Ipp32fc *[16];
	for (int i=0;i<16;i++)
		walshpnsym[i] = ippsMalloc_32fc(64);
}
CCommonPro::~CCommonPro(void)
{
	
	ClearOutBitsBuffer();
	if(pBuffer)
		delete [] pBuffer;
	for(int i=0;i<16;i++)
		ippsFree(walshpnsym[i]);
	delete walshpnsym;
	ippsFree(pConvAB);
	ippsFree(pConv);
}
void CCommonPro::UpdateBuffer(int length)
{
	if(length<=length_Buffer)
		return;

	if(length_Buffer)
		delete pBuffer;
	length_Buffer = length;
	pBuffer = new byte[length_Buffer];
}
int CCommonPro::DeInterleaver(byte*pData,InterleaverPam*pPam)
{
	int length = pPam->Cols*pPam->Rows;
	UpdateBuffer(length);

	byte *pIn = pData;
	for (int s=0,r=0,c=0;s++<length;)
	{
		pBuffer[r*pPam->Cols+c] = *pIn++;
		c = (c+pPam->icf+(s%pPam->Rows?0:pPam->dicf))%pPam->Cols;
		r = (r+pPam->irf+(s%pPam->Cols?0:pPam->dirf))%pPam->Rows;		
		if(c<0) c+=pPam->Cols;
		if(r<0) r+=pPam->Rows;
	}

	pIn = pData;
	for (int s=0,r=0,c=0;s++<length;)
	{
		*pIn++ = pBuffer[r*pPam->Cols+c];
		c = (c+pPam->ics+(s%pPam->Rows?0:pPam->dics))%pPam->Cols;
		r = (r+pPam->irs+(s%pPam->Cols?0:pPam->dirs))%pPam->Rows;		
		if(c<0) c+=pPam->Cols;
		if(r<0) r+=pPam->Rows;
	}
	return length;
}
int CCommonPro::Interleaver(byte*pData,InterleaverPam*pPam)
{
	int length = pPam->Cols*pPam->Rows;
	UpdateBuffer(length);

	byte *pIn = pData;
	for (int s=0,r=0,c=0;s++<length;)
	{	
		pBuffer[r*pPam->Cols+c] = *pIn++;
		c = (c+pPam->ics+(s%pPam->Rows?0:pPam->dics))%pPam->Cols;
		r = (r+pPam->irs+(s%pPam->Cols?0:pPam->dirs))%pPam->Rows;		
		if(c<0) c+=pPam->Cols;
		if(r<0) r+=pPam->Rows;
	}

	pIn = pData;
	for (int s=0,r=0,c=0;s++<length;)
	{
		*pIn++ = pBuffer[r*pPam->Cols+c];
		c = (c+pPam->icf+(s%pPam->Rows?0:pPam->dicf))%pPam->Cols;
		r = (r+pPam->irf+(s%pPam->Cols?0:pPam->dirf))%pPam->Rows;		
		if(c<0) c+=pPam->Cols;
		if(r<0) r+=pPam->Rows;
	}
	return length;
}
// (133,171)卷积码  摇尾卷积码，最后6比特作为初始寄存器的值
int CCommonPro::ConverCode(byte*pSource,int Length,byte*pDest)
{
	byte m_Register=0,*pT;//寄存器
	int index=0;
	//开始输出
	byte buffer[13];
	memcpy(buffer,pSource+Length-6,6);
	memcpy(buffer+6,pSource,6);
	pT = buffer;
	for (int i=6;i--;)
	{
		*pDest++=((pT[0]==1)+(pT[1]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		*pDest++=((pT[0]==1)+(pT[3]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		pT++;
	}
	pT = pSource;
	for (int i=Length-6;i--;)
	{
		*pDest++=((pT[0]==1)+(pT[1]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		*pDest++=((pT[0]==1)+(pT[3]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		pT++;
	}
	return Length<<1;
}
void CCommonPro::ClearOutBitsBuffer()
{
	for (int i=0;i<NeedDeleate.GetSize();i++)
		delete [] NeedDeleate[i];
	NeedDeleate.RemoveAll();
	length_Outbits = 0;
	cuOutBits = 0;
}
int CCommonPro::DeConverCode(byte*pSource,int Length,byte*pDest)
{
	if(Length%2) return 0;//2,1,6 卷积码
	ClearOutBitsBuffer();
	Length>>=1;
	ConvStatus* pS1,*pS2;
	byte* pIndex,*pS = pSource;
	bool  switchE = false;
	for (int i=0;i<256;i++)
		decodeStatus[i].num_Errs=0;
	for (int i=0,j;i<Length;i++)
	{
		//交替无码缓存和输出缓存
		pS1 = switchE?decodeStatus:decodeStatus+128;
		pS2 =!switchE?decodeStatus:decodeStatus+128;
		switchE = 1 - switchE;
		for (j=128;j--;)
		{
			//上一个状态
			pIndex = statusInfo[j].cmIndex;
			byte index=pS1[pIndex[0]].num_Errs<=pS1[pIndex[1]].num_Errs?pIndex[0]:pIndex[1];
			pS2[j]=pS1[index];
			if(i%32==0&&i)
				pS2[j].NewLink(NewOutBits());
			pS2[j].PutBits(statusInfo[j].codeData);
			//增加无码
			pIndex = statusInfo[j].outPut;
			pS2[j].AddErr((pIndex[0]!=pSource[0])+(pIndex[1]!=pSource[1]));
			//	TRACE("第%d步 状态%d 误码%d\r\n",i+1,j,pS2[j].num_Errs);
		}
		pSource+=2;
	}
	//提高尾部比特的准确性
	pSource=pS;
	for (int i=0,j;i<6;i++)
	{
		//交替无码缓存和输出缓存
		pS1 = switchE?decodeStatus:decodeStatus+128;
		pS2 =!switchE?decodeStatus:decodeStatus+128;
		switchE = 1 - switchE;
		for (j=128;j--;)
		{
			//上一个状态
			pIndex = statusInfo[j].cmIndex;
			byte index=pS1[pIndex[0]].num_Errs<=pS1[pIndex[1]].num_Errs?pIndex[0]:pIndex[1];
			pS2[j]=pS1[index];
			//增加无码
			pIndex = statusInfo[j].outPut;
			pS2[j].AddErr((pIndex[0]!=pSource[0])+(pIndex[1]!=pSource[1]));
		}
		pSource+=2;
	}
	pS1 = switchE?decodeStatus:decodeStatus+128;

	UINT minErr=-1,index=-1;
	for (int j=128;j--;)
		if(pS1[j].num_Errs<minErr){
			index =j;
			minErr = pS1[j].num_Errs;
		}
		ConvStatus rightPOut = pS1[index];
		index = Length;
		pDest += Length-1;
		while(index){
			*pDest-- = rightPOut.getBits();
			if(--index%32==0&&index)
				rightPOut.UpdateByte();
		}
		return Length;
}
OutBits* CCommonPro::NewOutBits()
{
	if(length_Outbits==cuOutBits){
		pOutBits = new OutBits[1024];
		NeedDeleate.Add(pOutBits);
		length_Outbits = 1024;
		cuOutBits = 0;
	}
	cuOutBits++;
	return pOutBits+cuOutBits-1;
}

void CCommonPro::PSK_map(short style, short M, float I_map[],float Q_map[])
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
int CCommonPro::PSK_MOD( short data[], short data_len, short M,Ipp32fc *out)
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
/************************************************************************/
/*   abs(mean(a.*conj(b)))/sqrt(mean(a.*conj(a))*mean(b.*conj(b)));                                                                   */
/************************************************************************/
void CCommonPro::CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f &cConv)
{	
	Ipp32fc pMeanAB;
	Ipp32f pMeanA;

	ippsConj_32fc(pSrcB,pConvAB,nLeng);
	ippsMul_32fc_I(pSrcA,pConvAB,nLeng);
	ippsMean_32fc(pConvAB,nLeng,&pMeanAB,ippAlgHintFast);

	ippsPowerSpectr_32fc(pSrcA,pConv,nLeng);
	ippsMean_32f(pConv,nLeng,&pMeanA,ippAlgHintFast);

	cConv = sqrt(pMeanAB.re*pMeanAB.re + pMeanAB.im*pMeanAB.im)/sqrt(pMeanA*bEnergy);
}
void CCommonPro::GenWalshPN015(int time)
{
	int i,j,k,m;
	short data[64];
	for (i=0;i<16;i++)
	{
		for(m=0;m<4;m++){
			for(j=0;j<16;j++){
				k = (j + m*16 + time*64)%256;
				data[(k%64)] = (WalshCode[i][j] + BW015PN[k])%8;
			}
		}
		PSK_MOD(data, 64,8,walshpnsym[i]);
	}
}
void CCommonPro::GenWalshPN3(int time)
{
	int i,j,k,m;
	short data[16];
	for (i=0;i<16;i++)
	{
		for(j=0;j<16;j++){
			k = (j +  time*16)%32;
			data[j] = (WalshCode[i][j] + BW3PN[k])%8;
		}
		PSK_MOD(data, 16,8,walshpnsym[i]);
	}
	
}

void CCommonPro::DeWalsh(Ipp32fc *pSrc,int num,int nLen,Ipp32f &pMax,int &pIndex)
{
	int i;
	Ipp32f *conv = ippsMalloc_32f(num);
	for (i=0;i<num;i++)
	{
		CrossConvCof(pSrc,walshpnsym[i],nLen,1,conv[i]);
	}
	ippsMaxIndx_32f(conv,num,&pMax,&pIndex);
	ippsFree(conv);
}

void CCommonPro::Division(byte *pIndata,int N,byte *GPoly,int mK,byte *pOutdata,int &outLen)
{
	// 除法过程根据除法电路设计(如：刘玉君《信道编码》P97除法电路图4.5)
	// pTemp[]按降幂存放校验位,即pTemp[0]表示高次幂
	// mK ：GPoly的最高次幂 , GPoly从0到mK共mK+1个元素，GPoly[0]表示低
	// 用pIndata的前mK 比特初始化寄存器
	int		i, j;
	int		feedback;
	BYTE	*pTemp;
	outLen = 0;
	pTemp = new BYTE[mK];
	//memcpy(pTemp,pIndata,mK);	//m_k bit寄存器
	ZeroMemory(pTemp,mK);
	for(i=0; i<N; i++)
	{
		pOutdata[outLen] = pTemp[0];
		outLen++;
		feedback = pIndata[i]^pTemp[0];
		if(feedback != 0)
		{
			for(j=mK-1; j>0; j--)
			{
				if (GPoly[j] != 0)
					pTemp[mK-1-j] = pTemp[mK-1-j + 1] ^ feedback;
				else
					pTemp[mK-1-j] = pTemp[mK-1-j + 1];
			}
			pTemp[mK-1] = GPoly[0] && feedback;
		} 
		else 
		{
			for (j=0; j<mK-1; j++)
				pTemp[j] = pTemp[j + 1];
			pTemp[mK-1] = 0;
		}
	}
	delete pTemp;
}
