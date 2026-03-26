#pragma once

struct Poly
{
	UINT* pK;
	int  N;
	int  Morder;
public:
inline	void settting(int order,int length,UINT* pBuffer){
	Morder = order;N = length;pK = pBuffer;
}
inline	void Clear(){
	for (int i=N;i--;)
		pK[i]=0;
}
inline	void setDataRL(byte *pD){
	Clear();
	UINT *pT=pK,i;
	pD+=Morder-1;
	for (i=0;i<Morder;i++)
	{
		if(!(i&31)&&i)
			pT++;
		*pT<<=1;
		*pT+=*pD--;
	}
	*pT<<=32-i;

}
inline	byte shiftFirstByte(){
	byte t = (*pK)>>24;
	int i;
	for (i=0;i<N-1;i++)
	{
		pK[i]=(pK[i]<<8)+((pK[i+1])>>24);
	}
	pK[i] = pK[i]<<8;
	return t;
}
inline	void  add(Poly*pp){

	for (int i=0;i<N;i++)
	{
		pK[i]^=pp->pK[i];
	}
}

inline	byte shiftFirstBit(){
	byte t = (*pK)>>31;
	int i;
	for (i=0;i<N-1;i++)
	{
		pK[i]=(pK[i]<<1)+((pK[i+1])>>31);
	}
	pK[i] = pK[i]<<1;
	return t;
}
};
class JCRC
{
	Poly  polyTable[257];
	UINT*  pBuffer;
	int   size_buffer;
	int	  length_Poly;
	Poly* pLeft;
	int   cuLength;
	byte  index;
	byte  indexLength;
public:
	JCRC(void);
	~JCRC(void);
	void setGPoly(byte* polynomial,int length);//设置生成多项式
	int sendData(byte*pData,int length);
	void getLeft(byte*);
	int checkCRC(byte*);
};
