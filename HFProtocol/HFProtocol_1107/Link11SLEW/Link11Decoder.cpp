#include "StdAfx.h"
#include "Link11Decoder.h"

Link11Decoder::Link11Decoder(void)
{
	length_Outbits = 0;
	cuOutBits = 0;
	pCRCTable = 0;
	for (int i=128;i--;)
		statusInfo1[i].setIndex(i);
	for (int i=128;i--;)
		statusInfo2[i].setIndex(i);
}

Link11Decoder::~Link11Decoder(void)
{
	if(pCRCTable)delete [] pCRCTable;
	ClearOutBitsBuffer();
}
void Link11Decoder::initCRC(byte* polynomial,int length,int lData)//高到低
{
	//N次多项式
	length_Poly = length;
	length_Data = lData;
	if(pCRCTable)delete [] pCRCTable;
	pCRCTable = new byte[length*256+length+8];

	pLeft=pCRCTable+length*256;//length+8
	for (int i=0,j,t;i<256;i++)
	{
		if(i==0xb0)
			i=i;
		t =i;
		for (j=length;j<length+8;j++)//产生数据多项式
		{
			pLeft[j]=t&1;
			t>>=1;
		}
		for (j=length;j--;){
			pLeft[j]=0;
		}

		for (j=length+7;j>=length;j--)
		{
			if(pLeft[j])
				for(t=1;t<=length;t++)
					pLeft[j-t]=pLeft[j-t]==polynomial[t-1]?0:1;
		}
		memcpy(pCRCTable+i*length,pLeft,length);//从低位到高位
	}
	index =0;
	indexLength = (8-length_Data&7)&7;
	cuLength=0;
	for (int i=0;i<length;i++)
		pLeft[i]=0;//低位到高位
}

void Link11Decoder::sendData(byte*pData,int length)
{
	cuLength+=length;
	byte* pTemp;
	for (int i=0;i<length;i++)
	{
		index<<=1;++indexLength;
		if(length_Poly-indexLength>=0)
			index+=pData[i]==pLeft[length_Poly-indexLength]?0:1;
		else
			index+=pData[i]==0?0:1;
		if(indexLength==8){
			indexLength=0;
			pTemp = pCRCTable+index*length_Poly;
			for (int i=length_Poly;i--;)
			{
				if(i-8>=0)
					pLeft[i]=pTemp[i]==pLeft[i-8]?0:1;
				else
					pLeft[i]=pTemp[i]==0?0:1;
			}
		}
	}
}

byte* Link11Decoder::getResult(byte*pL)
{
	for (int i=0;i<length_Poly;i++)
		if(pLeft[i]!=pL[length_Poly-1-i])
			return 0;
	return pLeft;
}


int Link11Decoder::ConverCode1(byte*pSource,int Length,byte*pDest)
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

int Link11Decoder::ConverCode2(byte*pSource,int Length,byte*pDest)
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
		*pDest++=((pT[0]==1)+(pT[1]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		*pDest++=((pT[0]==1)+(pT[2]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		pT++;
	}
	pT = pSource;
	for (int i=Length-6;i--;)
	{
		*pDest++=((pT[0]==1)+(pT[1]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		*pDest++=((pT[0]==1)+(pT[2]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		pT++;
	}
	return Length<<1;
}
int Link11Decoder::Puncturing43(byte*pSource,int Length,byte*pDest)
{
	for(int i=0,j=0;i<Length;j=1-j){
		switch (j)
		{
		case 0:
			*pDest++= *pSource++;*pDest++= *pSource++;
			break;
		default:
			*pDest++= *pSource++;pSource++;
			break;
		}
		i+=2;
	}
	int newL=(Length/4)*3;
	if((Length/2)%2)
		newL++;
	return newL;
}



int Link11Decoder::DeConverCode(byte*pSource,int Length,byte*pDest)
{
	if(Length%2) return 0;//2,1,2 卷积码
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
			pIndex = statusInfo1[j].cmIndex;
			byte index=pS1[pIndex[0]].num_Errs<=pS1[pIndex[1]].num_Errs?pIndex[0]:pIndex[1];
			pS2[j]=pS1[index];
			if(i%32==0&&i)
				pS2[j].NewLink(NewOutBits());
			pS2[j].PutBits(statusInfo1[j].codeData);
			//增加无码
			pIndex = statusInfo1[j].outPut;
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
			pIndex = statusInfo1[j].cmIndex;
			byte index=pS1[pIndex[0]].num_Errs<=pS1[pIndex[1]].num_Errs?pIndex[0]:pIndex[1];
			pS2[j]=pS1[index];
			//增加无码
			pIndex = statusInfo1[j].outPut;
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


int Link11Decoder::DeConverCodeDePunctur2(byte*pSource,int Length,byte*pDest)
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
			pIndex = statusInfo2[j].cmIndex;
			byte index=pS1[pIndex[0]].num_Errs<=pS1[pIndex[1]].num_Errs?pIndex[0]:pIndex[1];
			pS2[j]=pS1[index];
			if(newLength%32==0&&newLength)
				pS2[j].NewLink(NewOutBits());
			pS2[j].PutBits(statusInfo2[j].codeData);//1 0 1 1 0 0
			//增加误码
			pIndex = statusInfo2[j].outPut;
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


void	 Link11Decoder::ClearOutBitsBuffer()
{
	for (int i=0;i<NeedDeleate.GetSize();i++)
		delete [] NeedDeleate[i];
	NeedDeleate.RemoveAll();
	length_Outbits = 0;
	cuOutBits = 0;
}

OutBits* Link11Decoder::NewOutBits()
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