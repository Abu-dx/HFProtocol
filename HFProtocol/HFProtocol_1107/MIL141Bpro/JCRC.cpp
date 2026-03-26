#include "StdAfx.h"
#include "JCRC.h"


JCRC::JCRC()
{
	pBuffer = 0;
	size_buffer=0;
	pLeft = polyTable+256;
}

JCRC::~JCRC()
{
	if(pBuffer)
		delete [] pBuffer;
}

void JCRC::setGPoly(byte* polynomial,int length)
{
	/////////////////////分配内存/////////////////////////
	int N = (length>>5)+(length&31?1:0);
	if(N*257>size_buffer){
		if(pBuffer)
			delete [] pBuffer;
		size_buffer = N*257;
		pBuffer = new UINT[size_buffer];
	}
	for (int i=257;i--;)
		polyTable[i].settting(length,N,pBuffer+i*N);

	length_Poly = length;

	byte *pTemp=new byte[length+8];//length+8
	for (int i=0,j,t;i<256;i++)
	{
		t =i;
		for (j=length;j<length+8;j++)//产生数据多项式
		{
			pTemp[j]=t&1;
			t>>=1;
		}
		for (j=length;j--;){
			pTemp[j]=0;
		}

		for (j=length+7;j>=length;j--)
		{
			if(pTemp[j])
				for(t=1;t<=length;t++)
					pTemp[j-t]=pTemp[j-t]==polynomial[t-1]?0:1;
		}
		polyTable[i].setDataRL(pTemp);//从低位到高位
	}
	delete [] pTemp;
}

int JCRC::sendData(byte*pData,int length)
{
	length-=length_Poly;

	index =0;
	indexLength = (8-length&7)&7;
	pLeft->Clear();
	for (int i=0;i<length;i++)
	{
		index<<=1;++indexLength;
		index+=*pData++;
		if(indexLength==8){
			index^=pLeft->shiftFirstByte();
			indexLength=0;
			pLeft->add(polyTable+index);
		}
	}
	return checkCRC(pData);
}
void JCRC::getLeft(byte*pD)
{
	for (int i=0;i<length_Poly;i++)
		*pD++=pLeft->shiftFirstBit();
}
int JCRC::checkCRC(byte*pD)
{
	int num=0;
	for (int i=0;i<length_Poly;i++)
		if(pLeft->shiftFirstBit()!=*pD++)
			num++;
	return num;
	//		return false;
	//return true;
}