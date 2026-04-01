#pragma once

#include "ipps.h"
#include "CommonPro.h"
#include "Preambledetect.h"
#include "MIL141Bpro.h"

class CBW0demode
{
public:
    CBW0demode(void);
    ~CBW0demode(void);

public:
    CPreambledetect* m_Preambledetect;
    CCommonPro* m_CommonPro;

    Ipp32f pfre;
    Ipp64f* pSrcTap;
    int SrcLen;
    IppsFIRState64f_32f* SrcStateReal;
    IppsFIRState64f_32f* SrcStateImag;
    Ipp32f* pDelayreal;
    Ipp32f* pDelayimag;
    Ipp32f* pReal;
    Ipp32f* pImag;

    Ipp32fc* proBuf;
    int BufPos;
    int proBegin;
    Ipp32f* winBufConv;
    Ipp32f* winBufTH;
    Ipp32fc* delayBuf;
    int delayL;
    Ipp32fc* pConvAB;
    Ipp32f* pConv;

    int preambleLen;
    Ipp16s walshdataBufLen;
    int walshTime;
    byte byteload[52];
    int byteloadLen;

    int pidx;
    int pBurst;
    int flag;
    Ipp32f defrephase;

    unsigned char byte_flag;
    unsigned char data_byte;
    unsigned char bit_num;
    unsigned char symbol;

    BW0BurstCandidate m_BW0BurstCandidates[MIL141B_MAX_BW0_BURSTS];
    int m_BW0BurstCount;
    BW0BurstCandidate m_OpenBurstCandidate;
    bool m_OpenBurstActive;

public:
    void Demode_ini(Ipp32s nLeng, Ipp16s P, Ipp32f roll, Ipp32f Baud, int SrctapLen);
    void Demode(Ipp16s* pSrc, int nLeng, Ipp16s P, Ipp32f frequency, int& outLeng, Ipp8u* outbyte, int& byteLeng, HeadType141B* headType, int& headNum);
    void Demode_free();

    int GetBurstCandidateCount() const;
    const BW0BurstCandidate* GetBurstCandidates() const;
    void ResetBurstCandidates();

private:
    void CrossConvCof(Ipp32fc* pSrcA, Ipp32fc* pSrcB, Ipp16s nLeng, Ipp32f& cConv, Ipp32f& threod);
    void Burst_detect_ini(int allLen, int convLen, int deciP, int mproBegin);
    void Burst_detect(int proLen, Ipp32fc* UWsig, Ipp16s convLen, Ipp16s deciP, Ipp16s* BurstPos, Ipp32f* BurstScore, int& BurstNum, int& BufPos, int& BufHavePro);
    void Burst_detect_free();

    void DownFre_ini(Ipp32s nLeng, Ipp32f roll, Ipp32f Baud, Ipp16s P, int SrctapLen);
    void DownFre_free();
    void DownFre(Ipp16s* pSrc, int nLeng, Ipp32f frequency, Ipp32fc* pDst, int& DstLen);

    void DeWalsh(Ipp32fc* pSrc, int nLen, int P, int winLen, int time, byte* outbyte, int& outbytelen, Ipp32f& pMax);
    void DeInterDecode(byte* intbyte, int nLen, byte* pDest, int& pDestlen);
    void SaveTobit_ini();
    void SaveTobit(Ipp8u* pSrc, int nLeng, Ipp8u* outbyte, int& byteLeng, Ipp8s modutype);

    void ClearOpenBurstCandidate();
    void BeginBurstCandidate(int startSample, Ipp32f centerHz, Ipp32f preambleScore);
    void StoreWalshScore(int index, Ipp32f score);
    void CaptureDecodeBits(const byte* deinterleavedBits, int deinterleavedCount, const byte* viterbiBits, int viterbiCount);
    void FinalizeBurstCandidate(int endSample);
};

