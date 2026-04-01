#include "stdafx.h"
#include "BW0demode.h"

#include <cstring>
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
const int kCompareWindowMultiplier = 4;
}

CBW0demode::CBW0demode(void)
{
    m_Preambledetect = new CPreambledetect;
    m_CommonPro = new CCommonPro;

    proBuf = NULL;
    winBufConv = NULL;
    winBufTH = NULL;
    delayBuf = NULL;
    pConvAB = NULL;
    pConv = NULL;
    pSrcTap = NULL;
    pDelayreal = NULL;
    pDelayimag = NULL;
    pReal = NULL;
    pImag = NULL;
    SrcStateReal = NULL;
    SrcStateImag = NULL;

    m_BW0BurstCount = 0;
    m_OpenBurstActive = false;
    std::memset(&m_OpenBurstCandidate, 0, sizeof(m_OpenBurstCandidate));
    std::memset(m_BW0BurstCandidates, 0, sizeof(m_BW0BurstCandidates));
}

CBW0demode::~CBW0demode(void)
{
    delete m_Preambledetect;
    delete m_CommonPro;
}

void CBW0demode::ResetBurstCandidates()
{
    m_BW0BurstCount = 0;
    std::memset(m_BW0BurstCandidates, 0, sizeof(m_BW0BurstCandidates));
}

int CBW0demode::GetBurstCandidateCount() const
{
    return m_BW0BurstCount;
}

const BW0BurstCandidate* CBW0demode::GetBurstCandidates() const
{
    return m_BW0BurstCandidates;
}

void CBW0demode::ClearOpenBurstCandidate()
{
    std::memset(&m_OpenBurstCandidate, 0, sizeof(m_OpenBurstCandidate));
    m_OpenBurstActive = false;
}

void CBW0demode::BeginBurstCandidate(int startSample, Ipp32f centerHz, Ipp32f preambleScore)
{
    std::memset(&m_OpenBurstCandidate, 0, sizeof(m_OpenBurstCandidate));
    m_OpenBurstCandidate.valid = 1;
    m_OpenBurstCandidate.startSample = startSample;
    m_OpenBurstCandidate.endSample = startSample;
    m_OpenBurstCandidate.centerHz = centerHz;
    m_OpenBurstCandidate.headType = wMIL141BBW0;
    m_OpenBurstCandidate.preambleScore = preambleScore;
    m_OpenBurstCandidate.decodeQuality = 0.0f;
    m_OpenBurstActive = true;
}

void CBW0demode::StoreWalshScore(int index, Ipp32f score)
{
    if (!m_OpenBurstActive) {
        return;
    }
    if (index < 0 || index >= MIL141B_BW0_WALSH_SCORE_COUNT) {
        return;
    }
    m_OpenBurstCandidate.walshScores[index] = score;
    if (m_OpenBurstCandidate.walshScoreCount < index + 1) {
        m_OpenBurstCandidate.walshScoreCount = index + 1;
    }
}

void CBW0demode::CaptureDecodeBits(const byte* deinterleavedBits, int deinterleavedCount, const byte* viterbiBits, int viterbiCount)
{
    if (!m_OpenBurstActive) {
        return;
    }

    const int safeDeinterCount = deinterleavedCount < MIL141B_BW0_DEINTERLEAVED_BITS ? deinterleavedCount : MIL141B_BW0_DEINTERLEAVED_BITS;
    const int safeViterbiCount = viterbiCount < MIL141B_BW0_VITERBI_BITS ? viterbiCount : MIL141B_BW0_VITERBI_BITS;

    std::memcpy(m_OpenBurstCandidate.deinterleavedBits, deinterleavedBits, safeDeinterCount * sizeof(unsigned char));
    std::memcpy(m_OpenBurstCandidate.viterbiBits, viterbiBits, safeViterbiCount * sizeof(unsigned char));
    m_OpenBurstCandidate.deinterleavedBitCount = safeDeinterCount;
    m_OpenBurstCandidate.viterbiBitCount = safeViterbiCount;
}

void CBW0demode::FinalizeBurstCandidate(int endSample)
{
    if (!m_OpenBurstActive) {
        return;
    }

    m_OpenBurstCandidate.endSample = endSample;

    Ipp32f walshSum = 0.0f;
    Ipp32f walshMin = 1.0e9f;
    Ipp32f walshMax = 0.0f;
    for (int i = 0; i < m_OpenBurstCandidate.walshScoreCount; ++i) {
        const Ipp32f value = m_OpenBurstCandidate.walshScores[i];
        walshSum += value;
        if (value < walshMin) walshMin = value;
        if (value > walshMax) walshMax = value;
    }
    const Ipp32f walshMean = m_OpenBurstCandidate.walshScoreCount > 0
        ? walshSum / static_cast<Ipp32f>(m_OpenBurstCandidate.walshScoreCount)
        : 0.0f;
    if (walshMin == 1.0e9f) {
        walshMin = 0.0f;
    }

    m_OpenBurstCandidate.decodeQuality = (m_OpenBurstCandidate.preambleScore * 0.5f) + (walshMean * 0.35f) + (walshMin * 0.15f);

    if (m_BW0BurstCount < MIL141B_MAX_BW0_BURSTS) {
        m_BW0BurstCandidates[m_BW0BurstCount++] = m_OpenBurstCandidate;
    }

    ClearOpenBurstCandidate();
}

void CBW0demode::CrossConvCof(Ipp32fc* pSrcA, Ipp32fc* pSrcB, Ipp16s nLeng, Ipp32f& cConv, Ipp32f& threod)
{
    Ipp32fc pMeanAB;
    Ipp32f pMeanA;
    const Ipp32f thdelt = 0.0554f;

    ippsConj_32fc(pSrcB, pConvAB, nLeng);
    ippsMul_32fc_I(pSrcA, pConvAB, nLeng);
    ippsMean_32fc(pConvAB, nLeng, &pMeanAB, ippAlgHintFast);

    ippsPowerSpectr_32fc(pSrcA, pConv, nLeng);
    ippsMean_32f(pConv, nLeng, &pMeanA, ippAlgHintFast);

    cConv = (pMeanAB.re * pMeanAB.re + pMeanAB.im * pMeanAB.im);
    threod = pMeanA * thdelt;
}

void CBW0demode::Burst_detect_ini(int allLen, int convLen, int deciP, int mproBegin)
{
    winBufConv = ippsMalloc_32f(allLen);
    winBufTH = ippsMalloc_32f(allLen);
    ippsZero_32f(winBufConv, allLen);
    proBuf = ippsMalloc_32fc(allLen);

    delayL = convLen * deciP + 20;
    delayBuf = ippsMalloc_32fc(delayL);
    delayL = 0;

    pConvAB = ippsMalloc_32fc(convLen);
    pConv = ippsMalloc_32f(convLen);

    proBegin = mproBegin;
    BufPos = 0;
}

void CBW0demode::Burst_detect_free()
{
    ippsFree(proBuf);
    ippsFree(delayBuf);
    ippsFree(pConvAB);
    ippsFree(pConv);
    ippsFree(winBufConv);
    ippsFree(winBufTH);
}

void CBW0demode::Burst_detect(int proLen, Ipp32fc* UWsig, Ipp16s convLen, Ipp16s deciP,
    Ipp16s* BurstPos, Ipp32f* BurstScore, int& BurstNum, int& BufPosOut, int& BufHavePro)
{
    const int convdataL = convLen * deciP;
    const int compareL = kCompareWindowMultiplier * deciP;
    int winBufp = 0;
    int pPhase = 0;
    int pDstLen = 0;
    Ipp32f pMax = 0.0f;
    int pIndex = 0;
    Ipp32f cconv = 0.0f;
    Ipp32f threod = 0.0f;

    Ipp32fc* pSrcA = ippsMalloc_32fc(convLen);
    BurstNum = 0;

    ippsCopy_32fc(delayBuf, &proBuf[proBegin], delayL);
    for (int i = 0; i < proLen - convdataL; ++i)
    {
        ippsSampleDown_32fc(&proBuf[proBegin + i], convdataL, pSrcA, &pDstLen, deciP, &pPhase);
        CrossConvCof(pSrcA, UWsig, convLen, cconv, threod);
        winBufConv[winBufp] = cconv;
        winBufTH[winBufp] = threod;
        ++winBufp;
    }

    for (int i = 0; i < winBufp - compareL; ++i)
    {
        if (winBufConv[i] > winBufTH[i] && winBufConv[i + 1] > winBufTH[i + 1])
        {
            ippsMaxIndx_32f(&winBufConv[i], compareL, &pMax, &pIndex);
            BurstPos[BurstNum] = static_cast<Ipp16s>(i + pIndex);
            BurstScore[BurstNum] = pMax;
            ++BurstNum;
            i += compareL;
        }
    }

    delayL = convdataL + compareL;
    ippsCopy_32fc(&proBuf[proBegin + proLen - delayL], delayBuf, delayL);

    ippsFree(pSrcA);
    BufHavePro = winBufp - compareL;
    BufPosOut = delayL;
}

void CBW0demode::DownFre_ini(Ipp32s nLeng, Ipp32f roll, Ipp32f Baud, Ipp16s P, int SrctapLen)
{
    pSrcTap = ippsMalloc_64f(SrctapLen);
    const Ipp64f fstop = (Baud * (1 + 0.75)) / (9600 * 2);
    ippsFIRGenLowpass_64f(fstop, pSrcTap, SrctapLen, ippWinHamming, ippTrue);

    pReal = ippsMalloc_32f(nLeng * 2);
    pImag = ippsMalloc_32f(nLeng * 2);
    pDelayreal = ippsMalloc_32f(SrctapLen);
    pDelayimag = ippsMalloc_32f(SrctapLen);
    ippsSet_32f(0, pDelayreal, SrctapLen);
    ippsSet_32f(0, pDelayimag, SrctapLen);
    ippsFIRInitAlloc64f_32f(&SrcStateReal, pSrcTap, SrctapLen, pDelayreal);
    ippsFIRInitAlloc64f_32f(&SrcStateImag, pSrcTap, SrctapLen, pDelayimag);
}

void CBW0demode::DownFre_free()
{
    ippsFree(pSrcTap);
    ippsFree(pDelayreal);
    ippsFree(pDelayimag);
    ippsFIRFree64f_32f(SrcStateReal);
    ippsFIRFree64f_32f(SrcStateImag);
    ippsFree(pReal);
    ippsFree(pImag);
}

void CBW0demode::DownFre(Ipp16s* pSrc, int nLeng, Ipp32f frequency, Ipp32fc* pDst, int& DstLen)
{
    for (int i = 0; i < nLeng; ++i)
    {
        defrephase = defrephase - IPP_2PI * frequency / 9600.0f;
        if (defrephase >= IPP_2PI || defrephase <= -IPP_2PI) {
            defrephase = static_cast<Ipp32f>(fmod(static_cast<double>(defrephase), static_cast<double>(IPP_2PI)));
        }
        pReal[i] = pSrc[i] * cos(defrephase);
        pImag[i] = pSrc[i] * sin(defrephase);
    }
    ippsFIR64f_32f_I(pReal, nLeng, SrcStateReal);
    ippsFIR64f_32f_I(pImag, nLeng, SrcStateImag);
    ippsRealToCplx_32f(pReal, pImag, pDst, nLeng);
    DstLen = nLeng;
}

void CBW0demode::Demode_ini(Ipp32s nLeng, Ipp16s P, Ipp32f roll, Ipp32f Baud, int SrctapLen)
{
    defrephase = 0;
    DownFre_ini(nLeng, roll, Baud, P, SrctapLen);
    m_Preambledetect->Preamble_Gen(4);
    preambleLen = m_Preambledetect->PreambleLen * P;

    const int allLen = nLeng * 2 + preambleLen;
    Burst_detect_ini(allLen, preambleLen / P, P, preambleLen);

    pidx = 0;
    pBurst = 0;
    flag = 0;
    walshTime = 0;
    byteloadLen = 0;
    walshdataBufLen = 64 * P;

    SaveTobit_ini();
    ResetBurstCandidates();
    ClearOpenBurstCandidate();
}

void CBW0demode::Demode_free()
{
    DownFre_free();
    Burst_detect_free();
}

void CBW0demode::Demode(Ipp16s* pSrc, int nLeng, Ipp16s P, Ipp32f frequency, int& outLeng, Ipp8u* outbyte, int& byteLeng, HeadType141B* headType, int& headNum)
{
    const int UWleng = m_Preambledetect->PreambleLen;
    preambleLen = UWleng * P;

    Ipp16s* BurstPos = ippsMalloc_16s(nLeng / preambleLen * 2 + 1);
    Ipp32f* BurstScore = ippsMalloc_32f(nLeng / preambleLen * 2 + 1);
    int BurstNum = 0;
    int BufHavePro = 0;
    int dataLen = 0;

    ResetBurstCandidates();

    DownFre(pSrc, nLeng, frequency, &proBuf[proBegin + BufPos], dataLen);
    dataLen = dataLen + BufPos;
    Burst_detect(dataLen, m_Preambledetect->pPreambleSym, UWleng, P, BurstPos, BurstScore, BurstNum, BufPos, BufHavePro);

    headNum = 0;
    int mbyteleng = 0;
    outLeng = 0;
    byteLeng = 0;
    Ipp32f pMax = 0.0f;

    pBurst = 0;
    while (pidx + preambleLen < BufHavePro)
    {
        if (flag == 0 && BurstNum > 0 && pidx <= BurstPos[pBurst] && BurstPos[pBurst] <= pidx + preambleLen)
        {
            flag = 1;
            pidx = BurstPos[pBurst];
            walshTime = 0;
            byteloadLen = 0;

            BeginBurstCandidate(pidx, frequency, BurstScore[pBurst]);

            headType[headNum].position = pidx;
            headType[headNum].fre = frequency;
            headType[headNum].BWtype = wMIL141BBW0;
            ++headNum;

            ++pBurst;
            --BurstNum;
        }
        else if (flag == 1)
        {
            pidx = pidx + preambleLen - 4;
            flag = 2;
        }
        else if (flag == 2)
        {
            DeWalsh(&proBuf[proBegin + pidx], walshdataBufLen, P, 8, walshTime, &byteload[byteloadLen], mbyteleng, pMax);
            StoreWalshScore(walshTime, pMax);
            byteloadLen += mbyteleng;
            ++walshTime;
            pidx += walshdataBufLen;
            if (walshTime == 13)
            {
                DeInterDecode(byteload, byteloadLen, outbyte, byteLeng);
                FinalizeBurstCandidate(pidx);
                walshTime = 0;
                byteloadLen = 0;
                flag = 0;
            }
        }
        else
        {
            pidx = pidx + preambleLen;
        }
    }

    while (BurstNum)
    {
        if (BurstPos[pBurst] > BufHavePro - preambleLen)
        {
            pidx = BurstPos[pBurst];
            flag = 0;
            break;
        }
        ++pBurst;
        --BurstNum;
    }

    ippsCopy_32fc(&proBuf[proBegin + pidx], &proBuf[proBegin - (BufHavePro - pidx)], BufHavePro - pidx);
    pidx = -(BufHavePro - pidx);

    ippsFree(BurstPos);
    ippsFree(BurstScore);
}

void CBW0demode::DeWalsh(Ipp32fc* pSrc, int nLen, int P, int winLen, int time, byte* outbyte, int& outbytelen, Ipp32f& pMax)
{
    Ipp32fc* ptemp = ippsMalloc_32fc(64);
    int dstLen = 0;
    int pphase = 0;
    int pIndex = 0;
    int pIndextemp = 0;
    float tempMax = 0.0f;
    pMax = 0.0f;

    m_CommonPro->GenWalshPN015(time);
    for (int i = 0; i < winLen; ++i)
    {
        ippsSampleDown_32fc(&pSrc[i], nLen, ptemp, &dstLen, P, &pphase);
        m_CommonPro->DeWalsh(ptemp, 16, 64, tempMax, pIndextemp);
        if (tempMax > pMax)
        {
            pMax = tempMax;
            pIndex = pIndextemp;
        }
    }

    outbyte[0] = static_cast<byte>((pIndex >> 3) & 0x01);
    outbyte[1] = static_cast<byte>((pIndex >> 2) & 0x01);
    outbyte[2] = static_cast<byte>((pIndex >> 1) & 0x01);
    outbyte[3] = static_cast<byte>((pIndex >> 0) & 0x01);
    outbytelen = 4;
    ippsFree(ptemp);
}

void CBW0demode::DeInterDecode(byte* intbyte, int nLen, byte* pDest, int& pDestlen)
{
    InterleaverPam m_InterleaverPam;
    m_InterleaverPam.Rows = 4;
    m_InterleaverPam.Cols = 13;
    m_InterleaverPam.irs = 0;
    m_InterleaverPam.ics = 1;
    m_InterleaverPam.dirs = 1;
    m_InterleaverPam.dics = 0;
    m_InterleaverPam.irf = 1;
    m_InterleaverPam.icf = 0;
    m_InterleaverPam.dirf = 0;
    m_InterleaverPam.dicf = 1;

    const int outinterlen = m_CommonPro->DeInterleaver(intbyte, &m_InterleaverPam);

    byte temp[MIL141B_BW0_VITERBI_BITS] = { 0 };
    int tempLen = m_CommonPro->DeConverCode(intbyte, outinterlen, temp);

    CaptureDecodeBits(intbyte, outinterlen, temp, tempLen);
    SaveTobit(temp, tempLen, pDest, pDestlen, 1);
}

void CBW0demode::SaveTobit_ini()
{
    byte_flag = 0x80;
    data_byte = 0;
    bit_num = 8;
    symbol = 0;
}

void CBW0demode::SaveTobit(Ipp8u* pSrc, int nLeng, Ipp8u* outbyte, int& byteLeng, Ipp8s modutype)
{
    Ipp32s m = 0;
    switch (modutype)
    {
    case 1:
        for (Ipp32s i = 0; i < nLeng; ++i)
        {
            if ((pSrc[i] & 0x01) == 1)
                data_byte |= byte_flag;
            byte_flag >>= 1;
            if (byte_flag == 0)
            {
                outbyte[m] = data_byte;
                ++m;
                byte_flag = 0x80;
                data_byte = 0;
            }
        }
        break;
    case 2:
        for (Ipp32s i = 0; i < nLeng; ++i)
        {
            bit_num -= 2;
            data_byte |= (pSrc[i] << bit_num);
            if (bit_num == 0)
            {
                outbyte[m] = data_byte;
                ++m;
                bit_num = 8;
                data_byte = 0;
            }
        }
        break;
    case 3:
        for (Ipp32s i = 0; i < nLeng; ++i)
        {
            for (Ipp32s j = 0; j < 3; ++j)
            {
                bit_num = static_cast<unsigned char>((pSrc[i] >> (2 - j)) & 0x01);
                if (bit_num == 1)
                    data_byte |= byte_flag;
                byte_flag = byte_flag >> 1;
                if (byte_flag == 0)
                {
                    outbyte[m] = data_byte;
                    ++m;
                    byte_flag = 0x80;
                    data_byte = 0;
                }
            }
        }
        break;
    default:
        break;
    }
    byteLeng = static_cast<int>(m);
}
