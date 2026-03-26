#pragma once

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

struct InterleaverPam
{
	int   Rows,Cols;
	int   irs,ics;
	int   dirs,dics;
	int   irf,icf;
	int   dirf,dicf;
};

class MIL110ADecoder
{
	//{交织内存管理
	byte* pBuffer;
	int   length_Buffer;
	void UpdateBuffer(int length);//更新缓存长度
	//}

	//{译码内存管理
	StatusInfo statusInfo[128];
	byte	scrambleTable[511];
	int		errroBuffer[256];
	ConvStatus decodeStatus[256];//解卷积码的状态空间

	CArray<OutBits*> NeedDeleate;
	OutBits* pOutBits;
	int		 length_Outbits;
	int		 cuOutBits;
	inline OutBits* NewOutBits();
	void	 ClearOutBitsBuffer();
	//}

public:
	MIL110ADecoder(void);
	~MIL110ADecoder(void);

	int DeInterleaver(byte*pData,InterleaverPam*);//解交织
	int Interleaver(byte*pData,InterleaverPam*);//交织

	int  Puncturing43(byte*pSource,int Length,byte*pDest);
	int ConverCode(byte*pSource,int Length,int repeate,byte*pDest);

	int DeConverCode(byte*pSource,int Length,int repeate,byte*pDest);
	int DeConverCodeDePunctured(byte*pSource,int Length,byte*pDest);
};
