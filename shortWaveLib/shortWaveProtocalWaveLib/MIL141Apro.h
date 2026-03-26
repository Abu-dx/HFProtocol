#pragma once

// 确保 CString 类型定义正确（MFC 扩展 DLL 需要）
// 必须在包含 ipps.h 之前包含 afxwin.h，以确保 CString 类型定义一致
// 只在 MFC 项目中使用时包含 afxwin.h
#if defined(_AFXDLL) || defined(_AFXEXT)
#include <afxwin.h>         // MFC 核心组件和标准组件
#endif
#include "ipps.h"
#include <string>
#include <vector>

#define X22             0x00400000   /* vector representation of X^{22} */
#define X11             0x00000800   /* vector representation of X^{11} */
#define MASK12          0xfffff800   /* auxiliary vector for testing */
#define GENPOL          0x00000AE3   /* generator polinomial, g(x) */

class CReSampleFSK;
class CFSK8demode;

class  AFX_EXT_CLASS CMIL141Apro
{
public:
	CMIL141Apro(void);
	~CMIL141Apro(void);

public:
	CReSampleFSK* m_Resample;
	CFSK8demode* m_FSKdemode;

	int FFTorder, FFTleng;
	IppsFFTSpec_R_32f* pSpec;
	Ipp32f* fftDst, * fftTemp;

	Ipp32f* reBuf;
	Ipp32f f0;  // 检测出的中心频率

	Ipp8u* Judgeout;
	Ipp8u* Judgedelay;
	int  delayLen;
	int JudgeLen;

	Ipp16s* decoding_table;
	BOOL autoset;

	int flag;
	unsigned char byte_flag, data_byte, bit_num, symbol;



public:
	void MIL141Ademode_ini(float fsOld, float fsNew, int M, float f0, float FSpace, BOOL mautoset, int nLeng, int m_WinType);
	void MIL141Ademode_free();
	void MIL141Ademode(Ipp16s* pSrc, int nLeng, Ipp8u* outbyte, int& byteleng,
		std::vector<std::string>& message_string, std::vector<std::string>& Address_string, int& messagenum);
	void SetF0(float f0, BOOL mautoset);

	void frequence_detect(Ipp32f* pSrc, int nLeng, Ipp32f& f0, int& FSK8);
	void ConvertTobit(Ipp8u* pSrc, int nLeng, Ipp8u* outbyte, int& outLen);

	void FindStart(Ipp8u* pSrc, int nLen, Ipp8u* pDst, int& outLen);
	void FindSame(Ipp8u* pSrcA, Ipp8u* pSrcB, int nLen, int& num);

	void DataProcess(Ipp8u* pSrc, int blocknum, Ipp8u* byteout, int& bytenum, CString* message, CString* Address);
	void vote(Ipp8u* pSrc, Ipp8u* pDst);
	void deinterleaving(Ipp8u* pSrc_vote, Ipp16s* Normal, Ipp16s* Inverted);
	void Golay_endecode(Ipp16s* code, long& encode, Ipp16s* decoding_table);
	void codejoint(long code_normal, long code_invert, long& codeout);
	void message_show(long code, CString& message, CString& Address);

	void SaveTobit_ini();
	void SaveTobit(Ipp8u* pSrc, int nLeng, Ipp8u* outbyte, int& byteLeng, int M);
	void GetFFT_len(int dataLen, int& FFTLeng, int& FFTorder);

	long arr2int(int* a, int r);
	void nextcomb(int n, int r, int* a);
	long get_syndrome(long pattern);
	void Gen_decodetable(Ipp16s* decoding_table);
	long golayDecode(long recd, Ipp16s* decoding_table);

};