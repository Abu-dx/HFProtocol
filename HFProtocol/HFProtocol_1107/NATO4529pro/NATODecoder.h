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


class NATODecoder
{
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

	//{交织寄存器
	byte* pBuffer;
	byte* pRowRegister[32];
	int   increasment;
	//}

public:
	NATODecoder(void);
	~NATODecoder(void);


	void setParameter(int increasment,bool isDeconver);

	int  Puncturing43(byte*pSource,int Length,byte*pDest);
	int  DePuncturing34(byte*pSource,int Length,byte*pDest);

	void DeConverInterleaver(byte* pData,int Length,byte* pOut);
	void ConverInterleaver(byte* pData,int Length,byte* pOut);

	int DeConverCode(byte*pSource,int Length,int repeate,byte*pDest);
	int ConverCode(byte*pSource,int Length,int repeate,byte*pDest);
	int DeConverCodeDePunctured(byte*pSource,int Length,byte*pDest);
};
