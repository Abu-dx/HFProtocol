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

class MIL110BDecoder
{
	StatusInfo statusInfo[128];
	byte	scrambleTable[511];
	int     scIndex;
	int		errroBuffer[256];
	ConvStatus decodeStatus[256];//解卷积码的状态空间

	CArray<OutBits*> NeedDeleate;
	OutBits* pOutBits;
	int		 length_Outbits;
	int		 cuOutBits;
	inline OutBits* NewOutBits();
	void	 ClearOutBitsBuffer();

	void CreateScrambleTable();

	byte* pBuffer;
	int   length_Buffer;

	void UpdateBuffer(int length);//更新缓存长度
public:
	MIL110BDecoder(void);
	~MIL110BDecoder(void);

	int UNScrambling(byte*,int Length,int symbolBits);
	void UNScrambling(int symbolBits,int symbolNum,int &scIndex,short symboldata[]);

	int ConverCode(byte*pSource,int Length,byte*pDest);//卷积编码
	int Puncturing64(byte*pSource,int Length,byte*pDest);//删除矩阵，必须是2的倍数
	int DeConverCode(byte*pSource,int Length,byte*pDest);//Veterbi译码
	int DeConverCodeList(byte*pSource,int Length,byte*pDest);//Veterbi译码

	int DeConverCodeDePunctur(byte*pSource,int Length,byte*pDest);//Veterbi译码 解删除矩阵

	int DeInterleaver(byte*pData,int length,int K);//解交织
	int Interleaver(byte*pData,int length,int K);//交织
	int SolveModeEqualtion(int K,int N,int X);//解同与方程 M*K%N=X;
};
