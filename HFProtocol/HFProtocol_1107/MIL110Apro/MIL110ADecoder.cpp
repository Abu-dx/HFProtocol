#include "StdAfx.h"
#include "MIL110ADecoder.h"

MIL110ADecoder::MIL110ADecoder(void)
{
	length_Buffer = 0;
	pBuffer = 0;

	length_Outbits = 0;
	cuOutBits = 0;
	for (int i=128;i--;)
		statusInfo[i].setIndex(i);
}

MIL110ADecoder::~MIL110ADecoder(void)
{
	if(pBuffer)
		delete [] pBuffer;
	ClearOutBitsBuffer();
}

void MIL110ADecoder::UpdateBuffer(int length)
{
	if(length<=length_Buffer)
		return;

	if(length_Buffer)
		delete pBuffer;
	length_Buffer = length;
	pBuffer = new byte[length_Buffer];
}

int MIL110ADecoder::DeInterleaver(byte*pData,InterleaverPam*pPam)
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
int MIL110ADecoder::Interleaver(byte*pData,InterleaverPam*pPam)
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




//////////////////////

OutBits* MIL110ADecoder::NewOutBits()
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
void	 MIL110ADecoder::ClearOutBitsBuffer()
{
	for (int i=0;i<NeedDeleate.GetSize();i++)
		delete [] NeedDeleate[i];
	NeedDeleate.RemoveAll();
	length_Outbits = 0;
	cuOutBits = 0;
}
int MIL110ADecoder::ConverCode(byte*pSource,int Length,int repeate,byte*pDest)
{
	byte m_Register=0,*pT;//寄存器
	int index=0,bitOut[2],re;
	//开始输出
	byte buffer[13];
	memset(buffer,0,6);
	memcpy(buffer+6,pSource,6);
	pT = buffer;
	for (int i=6;i--;)
	{
		bitOut[0] = ((pT[0]==1)+(pT[1]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		bitOut[1] = ((pT[0]==1)+(pT[3]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		for (re =repeate;re--;){
			*pDest++=bitOut[0];
			*pDest++=bitOut[1];
		}
		pT++;
	}
	pT = pSource;
	for (int i=Length-6;i--;)
	{
		bitOut[0] = ((pT[0]==1)+(pT[1]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		bitOut[1] = ((pT[0]==1)+(pT[3]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		for (re =repeate;re--;){
			*pDest++=bitOut[0];
			*pDest++=bitOut[1];
		}
		pT++;
	}
	return Length*2*repeate;
}
int MIL110ADecoder::DeConverCode(byte*pSource,int Length,int repeate,byte*pDest)
{
	ClearOutBitsBuffer();
	Length/=repeate;//先去除重复
	Length>>=1;
	ConvStatus* pS1,*pS2;
	int e,re;
	byte* pIndex,*pS = pSource,*pT;
	bool  switchE = false;
	for (int i=0;i<256;i++)
		decodeStatus[i].num_Errs=0;
	for (int i=0,j;i<Length;i++)
	{
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
			pT = pSource;
			for (e=0,re=repeate;re--;)
			{
				e +=pIndex[0]!=*pT++;
				e +=pIndex[1]!=*pT++;
			}
			pS2[j].AddErr(e);
		}
		pSource+=repeate<<1;
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

int  MIL110ADecoder::Puncturing43(byte*pSource,int Length,byte*pDest)
{
	for(int i=0,j=0;i<Length;j=1-j){
		*pDest++= *pSource++;
		switch (j)
		{
		case 0:
			*pDest++=*pSource++;
			break;
		default:
			pSource++;
			break;
		}
		i+=2;
	}
	int newL=(Length/4)*3;
	if((Length/2)%2)
		newL++;
	return newL;
}

int MIL110ADecoder::DeConverCodeDePunctured(byte*pSource,int Length,byte*pDest)
{
	ClearOutBitsBuffer();
	ConvStatus* pS1,*pS2;
	byte* pIndex,*pS = pSource;
	bool  switchE = false;
	for (int i=0;i<256;i++)
		decodeStatus[i].num_Errs=0;
	int puncter=0,newLength=0;
	for (int i=0,j;i<Length;newLength++)
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
			if(newLength%32==0&&newLength)
				pS2[j].NewLink(NewOutBits());
			pS2[j].PutBits(statusInfo[j].codeData);//1 0 1 1 0 0
			//增加误码
			pIndex = statusInfo[j].outPut;
			switch(puncter){
				case  0:
					pS2[j].AddErr((pIndex[0]!=pSource[0])+(pIndex[1]!=pSource[1]));
					break;
				default:
					pS2[j].AddErr(pIndex[0]!=*pSource);
					break;
			}
		}
		switch(puncter){
				case  0:pSource+=2;i+=2;
					break;
				default:pSource++;i++;
					break;
		}
		puncter=1-puncter;
	}
	pS1 = switchE?decodeStatus:decodeStatus+128;

	UINT minErr=-1,index=-1;
	for (int j=128;j--;)
		if(pS1[j].num_Errs<minErr){
			index =j;
			minErr = pS1[j].num_Errs;
		}
		ConvStatus rightPOut = pS1[index];
		index = newLength;
		pDest += newLength-1;
		while(index){
			*pDest-- = rightPOut.getBits();
			if(--index%32==0&&index)
				rightPOut.UpdateByte();
		}
		return newLength;
}

