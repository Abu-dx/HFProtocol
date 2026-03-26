#include "StdAfx.h"
#include "MIL110BDecoder.h"

MIL110BDecoder::MIL110BDecoder(void)
{
	length_Outbits = 0;
	cuOutBits = 0;
	pBuffer = 0;
	length_Buffer = 0;

	CreateScrambleTable();

	for (int i=128;i--;)
		statusInfo[i].setIndex(i);
	int N = 110592,k=17329;
	for (int i=0;i<N;i++)
	{	
		if(i==6)
			i=6;
		__int64 Z = SolveModeEqualtion(k,N,i);
		Z=(Z*k)%N;
		if(Z!=i)
			Z=0;
	}
	
}

MIL110BDecoder::~MIL110BDecoder(void)
{
	if(length_Buffer)
		delete [] pBuffer;
	ClearOutBitsBuffer();
}

void	 MIL110BDecoder::ClearOutBitsBuffer()
{
	for (int i=0;i<NeedDeleate.GetSize();i++)
		delete [] NeedDeleate[i];
	NeedDeleate.RemoveAll();
	length_Outbits = 0;
	cuOutBits = 0;
}

void MIL110BDecoder::UNScrambling(int symbolBits,int symbolNum,int &scIndex,short symboldata[])
{
	byte tempSymbol;
	for (int i=0;i<symbolNum;i++)
	{	
		tempSymbol=scrambleTable[scIndex];
		scIndex = (scIndex+symbolBits)%511;

		symboldata[i] = 0;
		for(int j=0;j<symbolBits;j++){
			symboldata[i]+= tempSymbol&(1<<j);
		}
	}

}
int MIL110BDecoder::UNScrambling(byte*pData,int Length,int symbolBits)
{
	if(Length%symbolBits)
		return 0;
	int newL = Length;
	Length /= symbolBits;
	byte tempSymbol;
	int scarmbleIndex=0,index=0;
	for (int i=0;i<Length;i++,index+=symbolBits)
	{
		tempSymbol=0;
		for(int j=0;j<symbolBits;j++){
			tempSymbol<<=1;
			tempSymbol+= pData[index+j];
		}
		tempSymbol^=scrambleTable[scarmbleIndex];
		scarmbleIndex = (scarmbleIndex+symbolBits)%511;
		for(int j=symbolBits;j--;j){
			pData[index+j]= tempSymbol&1;
			tempSymbol>>=1;
		}
	}
	return newL;
}

void MIL110BDecoder::CreateScrambleTable()
{
	byte register7_0=1,register8=0,temp;

	for (int i=0;i<511;i++)
	{
		scrambleTable[i]=register7_0;

		temp = ((register7_0>>4)+(register7_0&1))&1;
		register7_0 >>=1;
		register7_0 += register8<<7;
		register8 = temp;
		if(register8==0&&register7_0==1)
			register8 =0;
	}

}


int MIL110BDecoder::ConverCode(byte*pSource,int Length,byte*pDest)
{
	byte m_Register=0,*pT=pSource;//寄存器
	int index=0;
	//开始输出
	for (int stopL=Length-6;index<stopL;index++)
	{
		*pDest++=((pT[0]==1)+(pT[1]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		*pDest++=((pT[0]==1)+(pT[3]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		pT++;
	}
	byte buffer[12];
	memcpy(buffer,pT,6);
	memcpy(buffer+6,pSource,6);
	pT = buffer;
	for (;index<Length;index++)
	{
		*pDest++=((pT[0]==1)+(pT[1]==1)+(pT[3]==1)+(pT[4]==1)+(pT[6]==1))&1;
		*pDest++=((pT[0]==1)+(pT[3]==1)+(pT[4]==1)+(pT[5]==1)+(pT[6]==1))&1;
		pT++;
	}
	return Length<<1;
}
int MIL110BDecoder::DeConverCode(byte*pSource,int Length,byte*pDest)
{
	if(Length%2) return 0;//2,1,2 卷积码
	Length>>=1;

	UpdateBuffer(Length<<8);
	byte* pBuffer = this->pBuffer,*pBuffer1,*pBuffer2,*pIndex,*pCu,*pLast,*pS=pSource;
	int  *pError1,*pError2,tempErr;
	bool  switchE = false;
	for (int i=0;i<256;i++)
		errroBuffer[i]=0;
	int test=13;
	for (int i=0,j;i<Length;i++)
	{
		//交替无码缓存和输出缓存
		pError1 = switchE?errroBuffer:errroBuffer+128;
		pError2 =!switchE?errroBuffer:errroBuffer+128;
		pBuffer1= switchE?pBuffer:pBuffer+(Length<<7);
		pBuffer2=!switchE?pBuffer:pBuffer+(Length<<7);
		switchE = 1 - switchE;
		for (j=128;j--;)
		{
			//上一个状态
			pIndex = statusInfo[j].cmIndex;
			byte index=pError1[pIndex[0]]<=pError1[pIndex[1]]?pIndex[0]:pIndex[1];
			pCu = pBuffer1 + Length*j;//当前的输出数据位置
			pLast = pBuffer2 + Length*index;//上一个状态输出的位置
			memcpy(pCu,pLast,i);

			pCu[i]=statusInfo[j].codeData;

			//增加无码
			pError2[j] = pError1[index];
			pIndex = statusInfo[j].outPut;
			pError2[j]+=(pIndex[0]!=pSource[0])+(pIndex[1]!=pSource[1]);

			if(j==test)
				test=test;
		}
		pSource+=2;
	}
	pSource=pS;
	pBuffer1= !switchE?pBuffer:pBuffer+(Length<<7);
	for (int i=0,j;i<1;i++)
	{
		//交替无码缓存和输出缓存
		pError1 = switchE?errroBuffer:errroBuffer+128;
		pError2 =!switchE?errroBuffer:errroBuffer+128;
		pBuffer1= switchE?pBuffer:pBuffer+(Length<<7);
		pBuffer2=!switchE?pBuffer:pBuffer+(Length<<7);
		switchE = 1 - switchE;
		for (j=128;j--;)
		{
			//上一个状态
			pIndex = statusInfo[j].cmIndex;
			byte index=pError1[pIndex[0]]<=pError1[pIndex[1]]?pIndex[0]:pIndex[1];
			pCu = pBuffer1 + Length*j;//当前的输出数据位置
			pLast = pBuffer2 + Length*index;//上一个状态输出的位置
			memcpy(pCu,pLast,Length);

			//增加无码
			pError2[j] = pError1[index];
			pIndex = statusInfo[j].outPut;
			pError2[j]+=(pIndex[0]!=pSource[0])+(pIndex[1]!=pSource[1]);
			if(j==test)
				test=test;
		}
		pSource+=2;
	}
	pError1 = switchE?errroBuffer:errroBuffer+128;

	UINT minErr=-1,index=-1;
	for (int j=128;j--;)
		if(pError1[j]<minErr){
			index =j;
			minErr = pError1[j];
		}
		pBuffer1+=Length*index;
		memcpy(pDest+6,pBuffer1,Length-6);
		memcpy(pDest,pBuffer1+Length-6,6);
		return Length;
}

int MIL110BDecoder::SolveModeEqualtion(int K,int N,int X)
{
	if(!X)	return 0;
	//X是余数 K是乘法因子 N是模
	K %= N;X %= N;

	bool t=TRUE;
	int m_N=1,m_M=0,a=0,b=0;
	int k;
	while(!(K==1||N==1||X==K||X==N)){
		if(t){
			k = N/K;N%=K;
			m_M-=m_N*k;b-=a*k;
			k = X/K + 1;
			b-=k;
			X = K*k - X;
		}else{
			k = K/N;K%=N;
			m_N-=m_M*k;a-=b*k;
			k = X/N + 1;
			a-=k;
			X = N*k - X;
		}
		t = 1-t;
	}
	if(K==1)	
		b-=X;
	else if(N==1)
		a-=X;
	else if(X==K)
		t?b--:b++;
	else
		t?a++:a--;

	return a*m_M-b*m_N;
}

int MIL110BDecoder::DeInterleaver(byte*pData,int length,int K)
{
	UpdateBuffer(length);
	int  index=0;
	for(int i=0;i<length;i++){
		pBuffer[i]=pData[index];
		index=(index+K)%length;
	}
	memcpy(pData,pBuffer,length);
	return length;
}
int MIL110BDecoder::Interleaver(byte*pData,int length,int K)
{
	UpdateBuffer(length);
	int  index=0;
	for(int i=0;i<length;i++){
		pBuffer[index]=pData[i];
		index=(index+K)%length;
	}
	memcpy(pData,pBuffer,length);
	return length;
}

void MIL110BDecoder::UpdateBuffer(int length)
{
	if(length<=length_Buffer)
		return;

	if(length_Buffer)
		delete pBuffer;
	length_Buffer = length;
	pBuffer = new byte[length_Buffer];
}

int MIL110BDecoder::DeConverCodeList(byte*pSource,int Length,byte*pDest)
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
			pIndex = statusInfo[j].cmIndex;
			byte index=pS1[pIndex[0]].num_Errs<=pS1[pIndex[1]].num_Errs?pIndex[0]:pIndex[1];
			pS2[j]=pS1[index];
			if(i%32==0&&i)
				pS2[j].NewLink(NewOutBits());
			pS2[j].PutBits(statusInfo[j].codeData);
			//增加无码
			pIndex = statusInfo[j].outPut;
			pS2[j].AddErr((pIndex[0]!=pSource[0])+(pIndex[1]!=pSource[1]));
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
	for (int i=6;i--;){
		pDest[i] = rightPOut.getBits();
		if(--index%32==0&&index)
			rightPOut.UpdateByte();
	}
	pDest += Length-1;
	while(index){
		*pDest-- = rightPOut.getBits();
		if(--index%32==0&&index)
			rightPOut.UpdateByte();
	}
	return Length;
}

OutBits* MIL110BDecoder::NewOutBits()
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
int MIL110BDecoder::DeConverCodeDePunctur(byte*pSource,int Length,byte*pDest)
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
				case  1:
					pS2[j].AddErr(pIndex[0]!=*pSource);
					break;
				default:
					pS2[j].AddErr(pIndex[1]!=*pSource);
					break;
			}
		}
		switch(puncter){
				case  0:pSource+=2;i+=2;
					break;
				default:pSource++;i++;
					break;
		}
		puncter=(puncter+1)%3;
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
	for (int i=6;i--;){
		pDest[i] = rightPOut.getBits();
		if(--index%32==0&&index)
			rightPOut.UpdateByte();
	}
	pDest += newLength-1;
	while(index){
		*pDest-- = rightPOut.getBits();
		if(--index%32==0&&index)
			rightPOut.UpdateByte();
	}
	return newLength;
}

int MIL110BDecoder::Puncturing64(byte*pSource,int Length,byte*pDest)
{
	if(Length%2)return 0;
	for(int i=0,j=0;i<Length;j=(j+1)%3){
		switch (j)
		{
		case 0:
			*pDest++= *pSource++;*pDest++= *pSource++;
			break;
		case  1:
			*pDest++= *pSource++;pSource++;
			break;
		default:
			pSource++;*pDest++= *pSource++;
			break;
		}
		i+=2;
	}
	int newL=(Length/6)*4;
	switch ((Length/2)%3)
	{
	case 1:newL+=2;
		break;
	case 2:newL+=3;
		break;
	default:
		break;
	}
	return newL;
}