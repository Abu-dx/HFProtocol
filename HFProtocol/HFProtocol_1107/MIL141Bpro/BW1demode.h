#pragma once

#include "ipps.h"
#include "CommonPro.h"
#include "Preambledetect.h"
#include "MIL141Bpro.h"
#include "Viterbi.h"

class CBW1demode
{
public:
	CBW1demode(void);
	~CBW1demode(void);

public:
	CPreambledetect *m_Preambledetect;
	CCommonPro *m_CommonPro;

	Viterbi *m_Viterbi;

	
	// 变频，匹配滤波
	Ipp32f pfre;
	Ipp64f *pSrcTap;
	int SrcLen;
	IppsFIRState64f_32f *SrcStateReal,*SrcStateImag;
	Ipp32f *pDelayreal,*pDelayimag;
	Ipp32f *pReal,*pImag;

	//  突发检测
	Ipp32fc *proBuf;	//数据块缓存
	int     BufPos;
	int     proBegin;
	Ipp32f  *winBufConv;//相关运算结果缓存
	Ipp32f  *winBufTH;
	Ipp32fc *delayBuf;
	int	    delayL;
	Ipp32fc *pConvAB;
	Ipp32f  *pConv;
	Ipp32f pEnergy;

	int preambleLen;
	Ipp16s  walshdataBufLen;  // 有效载荷缓存长度
	int walshTime;
	byte byteload[144];
	int byteloadLen;

	int pidx,pBurst;
	int flag;

	// pll
	Ipp32f defrephase;
	unsigned char byte_flag, data_byte, bit_num, symbol;

	int saveposition;
	float savefrequency;
	int deflag;

public:

	void Demode_ini(Ipp32s nLeng,Ipp16s P,Ipp32f roll,Ipp32f Baud,int SrctapLen);
	void Demode(Ipp16s *pSrc,int nLeng,Ipp16s P,Ipp32f frequency,int &outLeng,Ipp8u *outbyte,int &byteLeng,HeadType141B *headType,int &headNum);
	void Demode_free();

private:

	void CrossConvCof(Ipp32fc *pSrcA,Ipp32fc *pSrcB,Ipp16s nLeng,Ipp32f &cConv,Ipp32f &threod);
	void Burst_detect_ini(int allLen,int convLen,int deciP,int mproBegin);
	void Burst_detect(int proBegin,int proLen,Ipp32fc *UWsig,Ipp16s convLen,Ipp16s deciP,
		Ipp16s *BurstPos,int &BurstNum,int &BufPos,int &BufHavePro);
	void Burst_detect_free();

	void DownFre_ini(Ipp32s nLeng,Ipp32f roll,Ipp32f Baud,Ipp16s P,int SrctapLen);
	void DownFre_free();
	void DownFre(Ipp16s *pSrc,int nLeng,Ipp32f frequency,Ipp32fc *pDst,int &DstLen);

	void DeWalsh(Ipp32fc *pSrc,int nLen,int P,int winLen,int time,byte *outbyte,int &outbytelen,Ipp32f &pMax);
	void DeInterDecode1(byte *intbyte,int nLen,byte *pDest,int &pDestlen);
	void DeInterDecode5(byte *intbyte,int nLen,byte *pDest,int &pDestlen);

	void SaveTobit_ini();
	void SaveTobit(Ipp8u *pSrc,int nLeng,Ipp8u *outbyte,int &byteLeng,Ipp8s modutype);
};