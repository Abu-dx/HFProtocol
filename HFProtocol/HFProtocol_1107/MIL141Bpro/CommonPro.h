#pragma once



struct InterleaverPam
{
	int   Rows,Cols;
	int   irs,ics;
	int   dirs,dics;
	int   irf,icf;
	int   dirf,dicf;
};

struct OutBits{
	UINT  data;
	OutBits* pPer;
};
struct ConvStatus{
	int num_Errs;//错误数量
	OutBits outBits;
public:
	ConvStatus(){
		num_Errs = 0;outBits.pPer=0;outBits.data=0;
	}
	inline void AddErr(byte Er){
		num_Errs+=Er;
	}
	inline void PutBits(byte bit){
		outBits.data<<=1;
		outBits.data+=bit;
	}
	inline void NewLink(OutBits*pN){
		*pN = outBits;
		outBits.data=0;
		outBits.pPer = pN;
	}
	inline byte getBits(){
		byte I= outBits.data&1;
		outBits.data>>=1;
		return I;
	}
	inline void UpdateByte(){
		outBits = *outBits.pPer;
	}
};
struct StatusInfo{
	byte outPut[2];
	byte cmIndex[2];
	byte codeData;
public:
	void setIndex(byte index){
		cmIndex[0] = (index<<1)&0x7f;
		cmIndex[1] = cmIndex[0]+1;
		codeData = (index&64)!=0;
		outPut[0] = (((index&1)!=0)+((index&2)!=0)+((index&8) !=0)+((index&16)!=0)+((index&64)!=0))&1;
		outPut[1]=  (((index&1)!=0)+((index&8)!=0)+((index&16)!=0)+((index&32)!=0)+((index&64)!=0))&1;
	}
};


#include "ipps.h"
#include "Preamble.h"
class  CCommonPro
{
public:
	CCommonPro(void);
	~CCommonPro(void);

	//{交织内存管理
	byte* pBuffer;
	int   length_Buffer;
	void UpdateBuffer(int length);//更新缓存长度
	//}

	//{ 解walsh缓存
	Ipp32fc *pConvAB;
	Ipp32f  *pConv;
	Ipp32fc **walshpnsym;
	//}

	byte* pLeft;
	CArray<OutBits*> NeedDeleate;
	OutBits* pOutBits;
	int		 length_Outbits;
	int		 cuOutBits;
	ConvStatus decodeStatus[256];//解卷积码的状态空间
	StatusInfo statusInfo[128];


public:

	int DeInterleaver(byte*pData,InterleaverPam*);//解交织
	int Interleaver(byte*pData,InterleaverPam*);//交织

	int  ConverCode(byte*pSource,int Length,byte*pDest);
	int DeConverCode(byte*pSource,int Length,byte*pDest);
	void ClearOutBitsBuffer();
	inline OutBits* NewOutBits();
	
	void PSK_map(short style, short M, float I_map[],float Q_map[]); /* 星座映射方式  0——Gray，1——antiGray*/
	int  PSK_MOD(short data[],short data_len,short M,Ipp32fc *out);/* 输入数据向量 *//* 输入输入长度 *//* 调制阶数 2，4，8*/	/* 输出数据 */		

	void CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f bEnergy,Ipp32f &cConv);
	void GenWalshPN015(int time);
	void GenWalshPN3(int time);
	void DeWalsh(Ipp32fc *pSrc,int num,int nLen,Ipp32f &pMax,int &pIndex);
	
	void Division(byte *pIndata,int N,byte *GPoly,int mK,byte *pOutdata,int &outLen);
	

public:


};