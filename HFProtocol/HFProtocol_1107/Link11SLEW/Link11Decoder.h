#pragma once
struct StatusInfo1{
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
struct StatusInfo2{
	byte outPut[2];
	byte cmIndex[2];
	byte codeData;
public:
	void setIndex(byte index){
		cmIndex[0] = (index<<1)&0x7f;
		cmIndex[1] = cmIndex[0]+1;
		codeData = (index&64)!=0;
		outPut[0] = (((index&1)!=0)+((index&2)!=0)+((index&16)!=0)+((index&32)!=0)+((index&64)!=0))&1;
		outPut[1]=  (((index&1)!=0)+((index&4)!=0)+((index&8) !=0)+((index&16)!=0)+((index&64)!=0))&1;
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


class Link11Decoder
{

	int length_Poly;
	int length_Data;

	byte* pCRCTable;

	int cuLength;
	byte index;
	byte indexLength;
public:
	Link11Decoder(void);
	~Link11Decoder(void);

	byte* pLeft;
	CArray<OutBits*> NeedDeleate;
	OutBits* pOutBits;
	int		 length_Outbits;
	int		 cuOutBits;
	ConvStatus decodeStatus[256];//解卷积码的状态空间
	StatusInfo1 statusInfo1[128];
	StatusInfo2 statusInfo2[128];


	int  DeConverCode(byte*pSource,int Length,byte*pDest);

	int  ConverCode1(byte*pSource,int Length,byte*pDest);
	int  ConverCode2(byte*pSource,int Length,byte*pDest);

	inline OutBits* NewOutBits();

	int  DeConverCodeDePunctur2(byte*pSource,int Length,byte*pDest);
	int  Puncturing43(byte*pSource,int Length,byte*pDest);
	void ClearOutBitsBuffer();


	void initCRC(byte* polynomial,int length,int length_Data);
	void sendData(byte*pData,int length);
	byte* getResult(byte*pLeft);
};
