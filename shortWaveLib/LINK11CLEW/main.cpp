#ifdef _DEBUG
// [VLD COMPILE ERROR] '#include <vld.h>' should appear 
//      before '#include <afxwin.h>' in file stdafx.h
#include <vld.h>
#endif

// 必须首先引入 MFC 头文件，确保 AFX_EXT_CLASS 宏正确定义
// afx.h 和 Windows.h 头文件需在调制协议头文件之前加载，否则
// 会出现变量类型未定义的报错
#include <afx.h>
#include <Windows.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <array>
#include <cctype>
#include <filesystem>
#include <map>


// IPP 计算库头文件
#include "ipps.h"
#include <atlconv.h>


// 短波协议头文件
#include "Link11SLEW.h"
#include "Link11CLEW.h"
#include "MIL141Apro.h"
#include "MIL141Bpro.h"
#include "MIL110Apro.h"
#include "MIL110Bpro.h"
#include "NATO4285pro.h"
#include "NATO4529pro.h"
#pragma comment(lib, "MIL110Apro.lib")
#pragma comment(lib, "MIL110Bpro.lib")
#pragma comment(lib, "MIL141Apro.lib")
#pragma comment(lib, "MIL141Bpro.lib")
#pragma comment(lib, "Link11CLEW.lib")
#pragma comment(lib, "Link11SLEW.lib")
#pragma comment(lib, "NATO4285pro.lib")
#pragma comment(lib, "NATO4529pro.lib")


// 信道化、解调器头文件
#include "SgnlPrcsDll.h"
#include "SignalDemodProbe.h"
#include "DataFIRDF.h"
#include "../common/InputAudio.h"
#include "../common/InputParser.h"
#include "../shortWaveProtocalWaveLib/ProtolDetect.h"



// 全局结构体定义（匹配原逻辑，窄字符）
struct OutList
{
    std::string sigType;
    std::string BeginTime;
    std::string EndTime;
    double frequency;
    double hfFre;
    int dataRate;
    int interLeng;
    std::string interType;
    int PU;
    std::string frameType;
    int CRC;
    std::string encrypt;
    std::string message;
};


OutList gl_OutList;

namespace fs = std::filesystem;

struct RuntimeOverrides
{
    bool hasFrequency = false;
    double frequency = 0.0;
    bool hasDataRate = false;
    int dataRate = 0;
};

RuntimeOverrides g_runtimeOverrides;

// 调制协议枚举类（映射switch case数字到协议类型）
enum class ProtocolType {
    LINK11_CLEW = 1,    // case 1
    MIL110A = 2,        // case 2
    MIL110B = 3,        // case 3
    MIL141A = 4,        // case 4
    MIL141B = 5,        // case 5
    STANAG4285 = 6,     // case 6
    STANAG4529 = 7,     // case 7
    LINK11_SLEW = 8,   // case 11
    KG84_FSK2 = 9,     // case 17
    FSK2_BCH = 10,      // case 18
    PI4QPSK = 11        // case 19
};

void link11CLEW_test(const fs::path& filePath);
void link11SLEW_test(const fs::path& filePath);
void MIL141A_test(const fs::path& filePath);
void MIL141B_test(const fs::path& filePath);
void MIL110A_test(const fs::path& filePath);
void MIL110B_test(const fs::path& filePath);
void STANAG4285_test(const fs::path& filePath);
void STANAG4529_test(const fs::path& filePath);
void KG84_test(const fs::path& filePath);
void FSK2_BCH_test(const fs::path& filePath);
void PI4QPSK_test(const fs::path& filePath);

std::string ProtocolTypeName(ProtocolType protocol)
{
    switch (protocol)
    {
    case ProtocolType::LINK11_CLEW: return "link11clew";
    case ProtocolType::MIL110A: return "mil110a";
    case ProtocolType::MIL110B: return "mil110b";
    case ProtocolType::MIL141A: return "mil141a";
    case ProtocolType::MIL141B: return "mil141b";
    case ProtocolType::STANAG4285: return "4285";
    case ProtocolType::STANAG4529: return "4529";
    case ProtocolType::LINK11_SLEW: return "link11slew";
    case ProtocolType::KG84_FSK2: return "kg84";
    case ProtocolType::FSK2_BCH: return "fsk2bch";
    case ProtocolType::PI4QPSK: return "pi4qpsk";
    default: return "unknown";
    }
}

std::string CanonicalProtocolToken(const std::string& token)
{
    std::string lowered = hfinput::InputParser::ToLower(hfinput::InputParser::Trim(token));
    std::string compact;
    compact.reserve(lowered.size());
    for (char ch : lowered) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            compact.push_back(ch);
        }
    }
    return compact;
}

bool TryParseProtocolToken(const std::string& token, ProtocolType& protocol, bool& isAuto)
{
    const std::string key = CanonicalProtocolToken(token);
    isAuto = false;

    if (key.empty() || key == "auto" || key == "0") {
        isAuto = true;
        return true;
    }

    static const std::map<std::string, ProtocolType> protocolAlias = {
        {"1", ProtocolType::LINK11_CLEW},
        {"link11", ProtocolType::LINK11_CLEW},
        {"link11clew", ProtocolType::LINK11_CLEW},
        {"clew", ProtocolType::LINK11_CLEW},
        {"2", ProtocolType::MIL110A},
        {"mil110a", ProtocolType::MIL110A},
        {"m110a", ProtocolType::MIL110A},
        {"110a", ProtocolType::MIL110A},
        {"3", ProtocolType::MIL110B},
        {"mil110b", ProtocolType::MIL110B},
        {"m110b", ProtocolType::MIL110B},
        {"110b", ProtocolType::MIL110B},
        {"4", ProtocolType::MIL141A},
        {"mil141a", ProtocolType::MIL141A},
        {"141a", ProtocolType::MIL141A},
        {"5", ProtocolType::MIL141B},
        {"mil141b", ProtocolType::MIL141B},
        {"141b", ProtocolType::MIL141B},
        {"6", ProtocolType::STANAG4285},
        {"4285", ProtocolType::STANAG4285},
        {"nato4285", ProtocolType::STANAG4285},
        {"stanag4285", ProtocolType::STANAG4285},
        {"s4285", ProtocolType::STANAG4285},
        {"7", ProtocolType::STANAG4529},
        {"4529", ProtocolType::STANAG4529},
        {"nato4529", ProtocolType::STANAG4529},
        {"stanag4529", ProtocolType::STANAG4529},
        {"s4529", ProtocolType::STANAG4529},
        {"8", ProtocolType::LINK11_SLEW},
        {"link11slew", ProtocolType::LINK11_SLEW},
        {"slew", ProtocolType::LINK11_SLEW},
        {"9", ProtocolType::KG84_FSK2},
        {"kg84", ProtocolType::KG84_FSK2},
        {"kg84fsk2", ProtocolType::KG84_FSK2},
        {"10", ProtocolType::FSK2_BCH},
        {"fsk2bch", ProtocolType::FSK2_BCH},
        {"bch", ProtocolType::FSK2_BCH},
        {"11", ProtocolType::PI4QPSK},
        {"pi4qpsk", ProtocolType::PI4QPSK}
    };

    const auto it = protocolAlias.find(key);
    if (it == protocolAlias.end()) {
        return false;
    }

    protocol = it->second;
    return true;
}

bool TryMapDetectedNameToProtocol(const CStringA& detectedName, ProtocolType& protocol)
{
    bool isAuto = false;
    return TryParseProtocolToken(detectedName.GetString(), protocol, isAuto) && !isAuto;
}

void PrintUsage(const char* exeName)
{
    std::cout
        << "Usage:\n"
        << "  " << exeName << " <input_path> [-p protocol] [--search-dir dir] [--frequency hz] [--data-rate bps]\n"
        << "  " << exeName << "                     (interactive mode)\n\n"
        << "Input format:\n"
        << "  raw PCM short data or 16-bit mono PCM WAV\n\n"
        << "Protocol aliases:\n"
        << "  auto, link11clew, link11slew, mil110a, mil110b, mil141a, mil141b,\n"
        << "  4285, 4529, kg84, fsk2bch, pi4qpsk\n\n"
        << "Optional overrides:\n"
        << "  --frequency 1800    Override initial center frequency in Hz when the decoder supports it.\n"
        << "  --data-rate 2400    Override initial data rate when the decoder supports it.\n\n"
        << "Examples:\n"
        << "  " << exeName << " E:\\data\\file.wav\n"
        << "  " << exeName << " \"E:\\data\\file.wav\" -p link11clew\n"
        << "  " << exeName << " .\\file.wav -p auto\n"
        << "  " << exeName << " sample.wav -p 4285 --frequency 1800 --data-rate 2400\n";
}

bool PromptProtocolMenu(std::string& protocolToken)
{
    std::cout << "\nSelect protocol:\n"
              << "  0) auto detect\n"
              << "  1) link11clew\n"
              << "  2) mil110a\n"
              << "  3) mil110b\n"
              << "  4) mil141a\n"
              << "  5) mil141b\n"
              << "  6) 4285\n"
              << "  7) 4529\n"
              << "  8) link11slew\n"
              << "  9) kg84\n"
              << " 10) fsk2bch\n"
              << " 11) pi4qpsk\n";

    std::string line;
    if (!hfinput::InputParser::ReadLine("Enter number (default 0): ", line)) {
        return false;
    }
    protocolToken = line.empty() ? "auto" : line;
    return true;
}

#if !defined(_WIN64)
bool DetectProtocolForFile(const fs::path& inputPath, ProtocolType& protocol)
{
    std::ifstream fin;
    std::string error;
    hfaudio::AudioInputInfo inputInfo;
    if (!hfaudio::OpenPcm16InputStream(inputPath, fin, inputInfo, error)) {
        std::cerr << error << "\n";
        return false;
    }

    const int protocolCount = 8;
    std::array<BOOL, 10> selected{};
    selected.fill(FALSE);
    for (int i = 0; i < protocolCount; ++i) {
        selected[static_cast<size_t>(i)] = TRUE;
    }

    const int blockLength = 8192;
    const float inSample = 9.6e3f;
    const Ipp32f threshold = 0.45f;

    CProtolDetect detector;
    detector.ProtolDetect_ini(blockLength, inSample, selected.data(), protocolCount);

    short* data = static_cast<short*>(malloc(blockLength * sizeof(short)));
    if (data == NULL) {
        detector.ProtolDetect_free();
        return false;
    }

    bool found = false;
    while (true)
    {
        fin.read(reinterpret_cast<char*>(data), blockLength * sizeof(short));
        const DWORD readSamples = static_cast<DWORD>(fin.gcount()) / sizeof(short);
        if (readSamples == 0) {
            break;
        }

        ProtocolOut result;
        const BOOL detected = detector.ProtolDetect(
            data,
            blockLength,
            threshold,
            selected.data(),
            protocolCount,
            result);

        if (detected && TryMapDetectedNameToProtocol(CStringA(result.ProtocolName), protocol)) {
            found = true;
            break;
        }
    }

    free(data);
    detector.ProtolDetect_free();
    return found;
}
#else
bool DetectProtocolForFile(const fs::path&, ProtocolType&)
{
    std::cerr << "Auto detect is not enabled in this x64 build.\n";
    return false;
}
#endif

void RunProtocol(ProtocolType protocol, const fs::path& filePath)
{
    switch (protocol)
    {
    case ProtocolType::LINK11_CLEW:
        link11CLEW_test(filePath);
        break;
    case ProtocolType::MIL110A:
        MIL110A_test(filePath);
        break;
    case ProtocolType::MIL110B:
        MIL110B_test(filePath);
        break;
    case ProtocolType::MIL141A:
        MIL141A_test(filePath);
        break;
    case ProtocolType::MIL141B:
        MIL141B_test(filePath);
        break;
    case ProtocolType::STANAG4285:
        STANAG4285_test(filePath);
        break;
    case ProtocolType::STANAG4529:
        STANAG4529_test(filePath);
        break;
    case ProtocolType::LINK11_SLEW:
        link11SLEW_test(filePath);
        break;
    case ProtocolType::KG84_FSK2:
        KG84_test(filePath);
        break;
    case ProtocolType::FSK2_BCH:
        FSK2_BCH_test(filePath);
        break;
    case ProtocolType::PI4QPSK:
        PI4QPSK_test(filePath);
        break;
    default:
        std::cerr << "Unsupported protocol.\n";
        break;
    }
}

bool OpenProtocolInput(
    const fs::path& filePath,
    std::ifstream& fin,
    std::uint64_t& sampleCount,
    std::string& error)
{
    hfaudio::AudioInputInfo inputInfo;
    if (!hfaudio::OpenPcm16InputStream(filePath, fin, inputInfo, error)) {
        return false;
    }

    sampleCount = hfaudio::SampleCount(inputInfo);
    return true;
}

// 文件解调星座结果 
#define MAX_SEGMENT_SIZE 8192
short   pDataFiledemode[2][2 * MAX_SEGMENT_SIZE]; 


// SampleToTime函数（窄字符版本，格式化时间）
std::string SampleToTime(DWORD sample, float sampleRate)
{
    double time = static_cast<double>(sample) / sampleRate;
    char szTime[64] = { 0 };
    sprintf(szTime, "%.6f", time);  // 窄字符格式化
    return std::string(szTime);
}



void link11CLEW_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }



    // 初始化参数变量，分配IPP内存
    const int nLeng = 10000;
    int headNum = 0, allheadNum = 0;
    int p1 = 0;
    bool brun = true;
    // Ipp16s* data = ippsMalloc_16s(nLeng);
    // Ipp8u* outbyte = ippsMalloc_8u(3000);
    // Ipp32fc* expsym = ippsMalloc_32fc(nLeng);
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * 3000);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng);


    int OutLen = 0, byteLeng = 0;
    bool bdetect = false;


    float fs = 9.6e3;
    float fc = static_cast<float>(g_runtimeOverrides.hasFrequency ? g_runtimeOverrides.frequency : 605.0);
    float Insample = fs;
    float Outsample = 7040.0f;
    float rFreq = fc / Insample;
    float roll = 0.35f;
    int SrctapLen = 1024;
    CLink11CLEW m_Link11CLEW;
    m_Link11CLEW.Link11CLEWdemode_ini(fc + 1100, nLeng, Insample, Outsample, SrctapLen);


    DWORD dataHavePro = 0;  // 已处理的样本数
    int nProgressPos = 0;
    bool headerPrinted = false;  // 标记是否已打印表头

    while (brun)
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(short));
        // 获取实际读取的字节数 → 转换为样本数
        DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(short);

        // 读取到文件末尾，终止循环
        if (nBytesRead == 0) break;

        // 调用Link11CLEW解调接口
        m_Link11CLEW.Link11CLEWdemode(data, nLeng, 1, outbyte, byteLeng, expsym, OutLen, headNum, bdetect);

        if (headNum > 0)
        {
            // 打印表头（仅打印一次）
            if (!headerPrinted) {
                char header[512] = { 0 };
                snprintf(header, sizeof(header), "%-8s  %-15s  %-15s  %-15s  %-8s  %-15s\n",
                    "序号", "信号类型", "开始时间", "结束时间", "PU", "帧类型");
                std::cout << header;
                headerPrinted = true;
            }
            for (int i = 0; i < headNum; i++)
            {
                // 计算时间戳
                gl_OutList.BeginTime = SampleToTime(dataHavePro + m_Link11CLEW.m_burstpos[i].m_begin, fs);
                gl_OutList.EndTime = SampleToTime(dataHavePro + m_Link11CLEW.m_burstpos[i].m_end, fs);
                gl_OutList.frequency = m_Link11CLEW.m_burstpos[i].m_fduopule;
                gl_OutList.sigType = "Link11CLEW";
                gl_OutList.PU = m_Link11CLEW.m_burstpos[i].m_address;

                // 做个简单的判断，如果起始时间大于截止时间，说明检测错误。
                double beginTime = std::stod(gl_OutList.BeginTime);
                double endTime = std::stod(gl_OutList.EndTime);
                if (beginTime > endTime) {
                    continue;
                }
                


                // 确定帧类型
                if (m_Link11CLEW.m_burstpos[i].m_type == 1)
                    gl_OutList.frameType = "主站轮询";
                else if (m_Link11CLEW.m_burstpos[i].m_type == 2)
                    gl_OutList.frameType = "主站报告";
                else if (m_Link11CLEW.m_burstpos[i].m_type == 3)
                    gl_OutList.frameType = "前哨回复";
                else
                    gl_OutList.frameType = "";


                // 格式化文本行（窄字符）
                char szLine[512] = { 0 };
                snprintf(szLine, sizeof(szLine), "%-8d  %-15s  %-15s  %-15s  %-8d  %-15s\n",
                    allheadNum,
                    gl_OutList.sigType.c_str(),
                    gl_OutList.BeginTime.c_str(),
                    gl_OutList.EndTime.c_str(),
                    gl_OutList.PU,
                    gl_OutList.frameType.c_str());

                // 打印检测数据
                std::cout << szLine;

                allheadNum++;
            }

        }

        // 更新进度
        dataHavePro += nBytesRead;
        nProgressPos = static_cast<int>(100.0 * dataHavePro / static_cast<double>(m_FileLength));
        if (nProgressPos >= 100) nProgressPos = 100;

        // 读取字节数不足，终止循环
        if (nBytesRead < nLeng)
        {
            nProgressPos = 100;
            brun = false;
        }
    }

    // 释放IPP内存
    free(data);
    free(outbyte);
    free(expsym);

    // 关闭所有文件流
    fin.close();

    // 解调类资源释放
    m_Link11CLEW.Link11CLEWdemode_free();
     

    // 输出完成提示
    printf("Link11CLEW demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);
}




void link11SLEW_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }

    // 初始化参数变量，分配IPP内存
    const int nLeng = 6000;
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * 3000);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng);

    int OutLen = 0, byteLeng = 0;
    bool bdetect = false;

    // 初始化解调参数
    float fs = 9.6e3;
    float fc = static_cast<float>(g_runtimeOverrides.hasFrequency ? g_runtimeOverrides.frequency : 1.8e3);
    float Insample = fs;
    float Outsample = 9.6e3;
    float rFreq = fc / Insample;
    float roll = 0.35f;
    float Baud = 2.4e3;
    int SrctapLen = 1024;

    // 定义头文件类型对象
    HeadType* headType = new HeadType[10];
    int headNum = 0;
    int allheadNum = 0;


    UINT nBytesRead;
    DWORD dataHavePro = 0;
    int nProgressPos = 0;
    int p1 = 0;
    int p2 = 0;
    bool brun = true;
    bool headerPrinted = false;  // 标记是否已打印表头


    // 实例化 LINK11-SLEW 解调器
    CLink11SLEW m_Link11SLEW;
    m_Link11SLEW.Demode_PSKrealFSE_ini(
        Insample, Outsample, nLeng, fc, 4, 512, roll, Baud, SrctapLen);
    float m_DemodeTh = 0.35;
    
    while (brun)
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(Ipp16s));
        // 获取实际读取的样本数（字节数 → 样本数）
        const DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(Ipp16s);

        m_Link11SLEW.Demode_PSKrealFSE(data, nLeng, 4, 512, 6, m_DemodeTh, OutLen, 
            outbyte, byteLeng, headType, headNum);

        if (headNum > 0)
        {
            // 打印表头（仅打印一次）
            if (!headerPrinted) {
                char header[512] = { 0 };
                snprintf(header, sizeof(header), 
                    "%-8s  %-15s  %-15s  %-12s  %-8s  %-15s  %-8s\n",
                    "序号", "信号类型", "开始时间", "频率", "PU", "帧类型", "CRC");
                std::cout << header;
                headerPrinted = true;
            }
            for (int i = 0; i < headNum; i++)
            {
                gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, Insample);
                gl_OutList.sigType = "Link11SLEW";
                gl_OutList.PU = (int)headType[i].address;
                gl_OutList.CRC = (int)headType[i].crcerr;
                gl_OutList.frequency = headType[i].frequency;
                //if(gl_OutList.CRC==0)
                //	radio = radio+1;

                if (headType[i].Type == 0)
                    gl_OutList.frameType = "主站轮询";
                else if (headType[i].Type == 2)
                    gl_OutList.frameType = "主站报告";
                else if (headType[i].Type == 3)
                    gl_OutList.frameType = "前哨回复";
                else
                    gl_OutList.frameType = "";

                char szLine[512] = { 0 };
                snprintf(szLine, sizeof(szLine), 
                    "%-8d  %-15s  %-15s  %-12.2f  %-8d  %-15s  %-8d\n",
                    allheadNum,
                    gl_OutList.sigType.c_str(),
                    gl_OutList.BeginTime.c_str(), 
                    gl_OutList.frequency, 
                    gl_OutList.PU, 
                    gl_OutList.frameType.c_str(), 
                    gl_OutList.CRC);
                
                std::cout << szLine;
                allheadNum++;
            }
        }

        
        dataHavePro = dataHavePro + nBytesRead;
        nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
        if (nProgressPos >= 100)nProgressPos = 100;
        if (nBytesRead < nLeng)
        {
            nProgressPos = 100;
            break;
        }
    }


    // 释放IPP内存
    free(data);
    free(outbyte);
    free(expsym);

    // 关闭所有文件流
    fin.close();

    // 解调类资源释放
    delete[] headType;
    m_Link11SLEW.Demode_PSKrealFSE_free();


    // 输出完成提示
    printf("Link11SLEW demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);

}


void MIL141A_test(const fs::path& filePath)
{
    // 原版关键参数统一（替换零散变量）
    const double m_FileInsample = 9600.0;  // 输入采样率
    const double m_FileFc = g_runtimeOverrides.hasFrequency ? g_runtimeOverrides.frequency : 1625.0;  // 中频频率
    const int    nLeng = 6000;    // 每次读取的样本长度

    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }


    // 内存分配（替换ippsMalloc为标准malloc，添加失败检查）
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * nLeng);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng); // 原版未使用，保留但检查


    // 初始化变量（修复重复定义、未定义问题）
    int headNum = 0, allheadNum = 0;
    int p1 = 0;
    bool brun = true;
    int byteLeng = 0;          // 移除重复定义
    int messagenum = 0;        // 解调消息数量
    std::vector<std::string> message(100);
    std::vector<std::string> address(100);
    DWORD dataHavePro = 0;     // 已处理样本数
    int nProgressPos = 0;      // 进度百分比
    bool headerPrinted = false;  // 标记是否已打印表头

    // 初始化解调器
    CMIL141Apro m_MIL141Apro;
    float Insample = m_FileInsample;
    float Outsample = 12000.0;
    float rFreq = m_FileFc / Insample;
    float roll = 0.45f;
    short SrctapLen = 512;
    float Baud = 125;
    m_MIL141Apro.MIL141Ademode_ini(Insample, Outsample, 8, m_FileFc, 125, false, nLeng, 4);


    while (brun)
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(Ipp16s));
        // 获取实际读取的样本数（字节数 → 样本数）
        const DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(Ipp16s);

        // 读取到文件末尾，终止循环
        if (nBytesRead == 0) break;

        // 如果读取的数据不足nLeng，将剩余部分清零（避免未初始化数据导致无效参数错误）
        if (nBytesRead < nLeng) {
            memset(data + nBytesRead, 0, (nLeng - nBytesRead) * sizeof(Ipp16s));
        }

        // 核心解调调用 - 使用nLeng（函数可能期望固定长度，但确保数据已正确初始化）
        m_MIL141Apro.MIL141Ademode(data, nLeng, outbyte, byteLeng, message, address, messagenum);


        // 解调
        if (messagenum > 0 ) {
            // 打印表头（仅打印一次）
            if (!headerPrinted) {
                char header[512] = { 0 };
                snprintf(header, sizeof(header),
                    "%-8s  %-15s  %-15s  %-12s  %-30s",
                    "序号", "信号类型", "开始时间", "频率", "消息");
                std::cout << header << std::endl;
                headerPrinted = true;
            }
            for (int i = 0; i < messagenum; i++) { // 修复i未定义问题
                // 填充输出信息（替换gl_OutList，保留原版逻辑）
                gl_OutList.BeginTime = SampleToTime(dataHavePro, m_FileInsample);
                gl_OutList.sigType = "MIL141A";
                gl_OutList.frequency = m_FileFc;
                gl_OutList.message = message[i] + " " + address[i];

                // 格式化输出字符串（替换CString::Format为snprintf）
                char addwrite[512] = { 0 };
                snprintf(addwrite, sizeof(addwrite),
                    "%-8d  %-15s  %-15s  %-12.2f  %-30s",
                    allheadNum,
                    gl_OutList.sigType.c_str(),
                    gl_OutList.BeginTime.c_str(),
                    gl_OutList.frequency,
                    gl_OutList.message.c_str());

                // 打印到终端
                std::cout << addwrite << std::endl;
                allheadNum++;
            }
        }


        dataHavePro += nBytesRead;
        // 修复进度条逻辑错误（原版>=100设为10是bug，改为设为100）
        nProgressPos = static_cast<int>(100.0 * dataHavePro / static_cast<double>(m_FileLength));
        if (nProgressPos > 100) nProgressPos = 100;

        // 读取到文件末尾，终止循环
        if (nBytesRead < nLeng) {
            nProgressPos = 100;
            brun = false;
        }
    }

    // ========== 12. 资源释放（避免内存泄漏/文件句柄泄漏） ==========
    brun = false;
    m_MIL141Apro.MIL141Ademode_free(); // 释放解调器资源

    // 释放内存
    free(data);
    free(outbyte);
    free(expsym); // 原版遗漏的释放

    // 关闭文件
    fin.close();

    // 输出完成提示
    printf("MIL141A demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);
}


void MIL141B_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }



    // 初始化参数变量，分配IPP内存
    HeadType141B* headType = new HeadType141B[10];
    int headNum = 0, allheadNum = 0;

    const int nLeng = 6000;
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * 3000);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng);
    int OutLen, byteLeng;


    // 解调参数初始化
    float Insample = 9.6e3; 
    float Outsample = 9600.0;
    float rFreq = 1800.0 / Insample;
    float roll = 0.45;
    short SrctapLen = 512;
    float Baud = 2400.0;

    CMIL141Bpro m_MIL141Bpro;
    m_MIL141Bpro.MIL141Bdemode_ini(nLeng, Insample, Outsample, 4, roll, Baud, SrctapLen);
    
    

    UINT nBytesRead;
    DWORD dataHavePro = 0;
    int nProgressPos = 0;
    int p1 = 0, p2;
    BOOL brun = true;
    bool headerPrinted = false;  // 标记是否已打印表头


    while (brun)  // 循环处理AD数据
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(short));
        // 获取实际读取的字节数 → 转换为样本数
        DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(short);

        // 读取到文件末尾，终止循环
        if (nBytesRead == 0) break;


        m_MIL141Bpro.MIL141Bdemode(data, nLeng, 4, pDataFiledemode[p1], OutLen, outbyte, byteLeng, headType, headNum);
        if (headNum > 0)
        {
            // 打印表头（仅打印一次）
            if (!headerPrinted) {
                char header[512] = { 0 };
                snprintf(header, sizeof(header), "%-8s  %-15s  %-15s  %-15s  %-12s  %-15s\n",
                    "序号", "信号类型", "开始时间", "结束时间", "频率", "帧类型");
                std::cout << header;
                headerPrinted = true;
            }
            for (int i = 0; i < headNum; i++)
            {
                gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, Insample);
                gl_OutList.EndTime.clear();
                gl_OutList.sigType = "MIL141B";
                gl_OutList.frequency = headType[i].fre;

                if (headType[i].BWtype == wMIL141BBW0)
                    gl_OutList.frameType = ("BW0");
                else if (headType[i].BWtype == wMIL141BBW1)
                    gl_OutList.frameType = ("BW1");
                else if (headType[i].BWtype == wMIL141BBW2)
                    gl_OutList.frameType = ("BW2");
                else if (headType[i].BWtype == wMIL141BBW3)
                    gl_OutList.frameType = ("BW3");
                else if (headType[i].BWtype == wMIL141BBW5)
                    gl_OutList.frameType = ("BW5");
                else if (headType[i].BWtype == wMIL141BTLC)
                    gl_OutList.frameType = ("TLC");
                else
                    gl_OutList.frameType = ("");


                // 格式化文本行（窄字符）
                char szLine[512] = { 0 };
                snprintf(szLine, sizeof(szLine), "%-8d  %-15s  %-15s  %-15s  %-12.2f  %-15s\n",
                    allheadNum,
                    gl_OutList.sigType.c_str(),
                    gl_OutList.BeginTime.c_str(),
                    gl_OutList.EndTime.c_str(),
                    gl_OutList.frequency,
                    gl_OutList.frameType.c_str());

                std::cout << szLine;
                allheadNum++;
            }
        }
        dataHavePro = dataHavePro + nBytesRead;
        nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
        if (nProgressPos >= 100)nProgressPos = 100;
        if (nBytesRead < nLeng)
        {
            nProgressPos = 100;
            break;
        }
    }
    brun = FALSE;
    m_MIL141Bpro.MIL141Bdemode_free();
    free(data);
    free(outbyte);
    free(expsym);
    delete[] headType;

    fin.close();

    // 输出完成提示
    printf("MIL141B demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);
    return;

}



void MIL110A_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }

    // 初始化参数变量，分配IPP内存
    const int nLeng = 6000;
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * 3000);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng);

    int OutLen = 0, byteLeng = 0;
    bool bdetect = false;

    // 初始化解调参数
    float fs = 9.6e3;
    float fc = static_cast<float>(g_runtimeOverrides.hasFrequency ? g_runtimeOverrides.frequency : 1.8e3);
    float Insample = fs;
    float Outsample = 9.6e3;
    float rFreq = fc / Insample;
    float roll = 0.35f;
    float Baud = 2.4e3;
    int SrctapLen = 1024;

    // 定义头文件类型对象
    HeadType110A* headType = new HeadType110A[30];
    int headNum = 0;
    int allheadNum = 0;


    // 实例化 110A 解调器
    CMIL110Apro m_MIL110Apro;
    m_MIL110Apro.Demode_PSKrealFSE_ini(
        Insample, Outsample, nLeng, fc, 4, 512, roll, Baud, SrctapLen
    );


    UINT nBytesRead;
    DWORD dataHavePro = 0;
    int nProgressPos = 0;
    int p1 = 0;
    int p2 = 0;
    bool brun = true;
    bool headerPrinted = false;  // 标记是否已打印表头


    float m_DemodeTh = 0.35;
    while (brun)
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(short));
        // 获取实际读取的字节数 → 转换为样本数
        DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(short);

        // 读取到文件末尾，终止循环
        if (nBytesRead == 0) break;

        m_MIL110Apro.Demode_PSKrealFSE(data, nLeng, 4, 512, 6, m_DemodeTh, OutLen, outbyte, byteLeng, headType, headNum);


        if (headNum > 0)
        {
            // 打印表头（仅打印一次）
            if (!headerPrinted) {
                char header[512] = { 0 };
                snprintf(header, sizeof(header), 
                    "%-8s  %-15s  %-15s  %-12s  %-10s  %-10s\n",
                    "序号", "信号类型", "开始时间", "频率", "数据速率", "交织长度");
                std::cout << header;
                headerPrinted = true;
            }
            for (int i = 0; i < headNum; i++)
            {
                gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, Insample);
                gl_OutList.sigType = "MIL110A";
                gl_OutList.frequency = headType[i].fre;
                gl_OutList.dataRate = headType[i].dataRate;
                gl_OutList.interLeng = headType[i].interLeng;

                char szLine[512] = { 0 };
                snprintf(szLine, sizeof(szLine), 
                    "%-8d  %-15s  %-15s  %-12.2f  %-10d  %-10d\n",
                    allheadNum,
                    gl_OutList.sigType.c_str(),
                    gl_OutList.BeginTime.c_str(),
                    gl_OutList.frequency,
                    gl_OutList.dataRate,
                    gl_OutList.interLeng
                );

                std::cout << szLine;
                allheadNum++;
            }
        }
        dataHavePro = dataHavePro + nBytesRead;
        nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
        if (nProgressPos >= 100)nProgressPos = 100;
        if (nBytesRead < nLeng)
        {
            nProgressPos = 100;
            break;
        }

    }

    brun = FALSE;
    m_MIL110Apro.Demode_PSKrealFSE_free();
    free(data);
    free(outbyte);
    free(expsym);
    delete[] headType;

    fin.close();
    printf("MIL110A demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);

    return;
}


void MIL110B_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }

    // 初始化参数变量，分配IPP内存
    const int nLeng = 6000;
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * 3000);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng);

    int OutLen = 0, byteLeng = 0;
    bool bdetect = false;

    // 初始化解调参数
    float fs = 9.6e3;
    float fc = static_cast<float>(g_runtimeOverrides.hasFrequency ? g_runtimeOverrides.frequency : 1.8e3);
    float Insample = fs;
    float Outsample = 9.6e3;
    float rFreq = fc / Insample;
    float roll = 0.35f;
    float Baud = 2.4e3;
    int SrctapLen = 512;

    // 定义头文件类型对象
    HeadType110B* headType = new HeadType110B[30];
    int headNum = 0;
    int allheadNum = 0;


    // 实例化 110B 解调器
    CMIL110Bpro m_MIL110Bpro;
    m_MIL110Bpro.Demode_PSKrealFSE_ini(
        Insample, Outsample, nLeng, fc, 4, 512, roll, Baud, SrctapLen
    );


    UINT nBytesRead;
    DWORD dataHavePro = 0;
    int nProgressPos = 0;
    int p1 = 0;
    int p2 = 0;
    bool brun = true;
    bool headerPrinted = false;  // 标记是否已打印表头


    float m_DemodeTh = 0.35;
    while (brun)
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(short));
        // 获取实际读取的字节数 → 转换为样本数
        DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(short);

        // 读取到文件末尾，终止循环
        if (nBytesRead == 0) break;

        m_MIL110Bpro.Demode_PSKrealFSE(data, nLeng, 4, 512, 6, m_DemodeTh, OutLen, outbyte, byteLeng, headType, headNum);


        if (headNum > 0)
        {
            // 打印表头（仅打印一次）
            if (!headerPrinted) {
                char header[512] = { 0 };
                snprintf(header, sizeof(header), 
                    "%-8s  %-15s  %-15s  %-12s  %-10s  %-10s\n",
                    "序号", "信号类型", "开始时间", "频率", "数据速率", "交织长度");
                std::cout << header;
                headerPrinted = true;
            }
            for (int i = 0; i < headNum; i++)
            {
                gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, Insample);
                gl_OutList.sigType = "MIL110B";
                gl_OutList.frequency = headType[i].fre;
                gl_OutList.dataRate = headType[i].dataRate;
                gl_OutList.interLeng = headType[i].interLeng;

                char szLine[512] = { 0 };
                snprintf(szLine, sizeof(szLine), 
                    "%-8d  %-15s  %-15s  %-12.2f  %-10d  %-10d\n",
                    allheadNum,
                    gl_OutList.sigType.c_str(),
                    gl_OutList.BeginTime.c_str(),
                    gl_OutList.frequency,
                    gl_OutList.dataRate,
                    gl_OutList.interLeng
                );

                std::cout << szLine;
                allheadNum++;
            }
        }
        dataHavePro = dataHavePro + nBytesRead;
        nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
        if (nProgressPos >= 100)nProgressPos = 100;
        if (nBytesRead < nLeng)
        {
            nProgressPos = 100;
            break;
        }

    }

    brun = FALSE;
    m_MIL110Bpro.Demode_PSKrealFSE_free();
    free(data);
    free(outbyte);
    free(expsym);
    delete[] headType;

    fin.close();
    printf("MIL110B demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);

    return;
}


void STANAG4285_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }

    // 初始化参数变量，分配IPP内存
    const int nLeng = 6000;
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * 3000);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng);

    int OutLen = 0, byteLeng = 0;
    bool bdetect = false;

    // 初始化解调参数
    float fs = 9.6e3;
    float fc = static_cast<float>(g_runtimeOverrides.hasFrequency ? g_runtimeOverrides.frequency : 1.8e3);
    float Insample = fs;
    float Outsample = 9.6e3;
    float rFreq = fc / Insample;
    float roll = 0.35f;
    float Baud = 2.4e3;
    int SrctapLen = 512;

    // 定义头文件类型对象
    HeadType4285* headType = new HeadType4285[nLeng / 80];;
    int headNum = 0;
    int allheadNum = 0;



    UINT nBytesRead;
    DWORD dataHavePro = 0;
    int nProgressPos = 0;
    int p1 = 0;
    int p2 = 0;
    bool brun = true;
    bool headerPrinted = false;  // 标记是否已打印表头

    /*
    * 1. dataRate（数据率，单位：bit/s）
        对于 NATO4285，可选值有 7 个（DlgDemode.cpp:223-226）：
        3600（默认值）
        2400
        1200
        600
        300
        150
        75
      2. FECType（编码类型）
        可选值有 2 个（DlgDemode.cpp:268-272）：
        0 - 对应 UI 选项："否"（不编码）
        1 - 对应 UI 选项："是"（编码，默认值）
      3. InterType（交织长度）
        可选值有 3 个（DlgDemode.cpp:273-279）：
        0 - 对应 UI 选项："No"（无交织）
        1 - 对应 UI 选项："Short"（短交织）
        2 - 对应 UI 选项："Long"（长交织，默认值）
    */
    int dataRate = g_runtimeOverrides.hasDataRate ? g_runtimeOverrides.dataRate : 3600;
    int FECType = 1;
    int InterType = 2;

    // 实例化 SATANG4285 解调器
    CNATO4285pro m_NATO4285pro;
    m_NATO4285pro.Demode_PSKrealFSE_ini(Insample, Outsample, nLeng, 
        fc, 4, 128, roll, Baud, SrctapLen, dataRate, FECType, InterType);

    
    float m_DemodeTh = 0.5;
    while (brun)
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(short));
        // 获取实际读取的字节数 → 转换为样本数
        DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(short);

        // 读取到文件末尾，终止循环
        if (nBytesRead == 0) break;

        m_NATO4285pro.Demode_PSKrealFSE(data, nLeng, 4, 128, 7, 
            m_DemodeTh, OutLen, outbyte, byteLeng, headType, headNum);


        // 打印 outbyte 的十六进制内容
        //if (byteLeng > 0) {
        //    std::cout << "byteLeng=" << byteLeng << ", outbyte (hex):" << std::endl;
        //    for (int i = 0; i < byteLeng; i++) {
        //        if (i > 0 && (i % 16 == 0)) {
        //            std::cout << std::endl;  // 每16字节换行
        //        }
        //        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
        //            << static_cast<unsigned int>(outbyte[i]) << " ";
        //    }
        //    std::cout << std::dec << std::endl << std::endl;  // 恢复十进制输出
        //}

        if (headNum > 0)
        {

            for (int i = 0; i < headNum; i++)
            {
                gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, Insample);
                gl_OutList.sigType = "NATO4285";
                gl_OutList.frequency = headType[i].fre;
                gl_OutList.dataRate = headType[i].dataRate;
                gl_OutList.interType = headType[i].interType;
               

                // 使用 snprintf 格式化输出（固定宽度，左对齐）
                char szLine[512] = { 0 };
                snprintf(szLine, sizeof(szLine), 
                    "%-8d  %-15s  %-15s  %-12.2f  %-10d  %-15s\n",
                    allheadNum,
                    gl_OutList.sigType.c_str(),
                    gl_OutList.BeginTime.c_str(),
                    gl_OutList.frequency,
                    gl_OutList.dataRate,
                    gl_OutList.interType.c_str());
                
                std::cout << szLine;
                allheadNum++;
            }
        }
        dataHavePro = dataHavePro + nBytesRead;
        nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
        if (nProgressPos >= 100)nProgressPos = 100;
        if (nBytesRead < nLeng)
        {
            nProgressPos = 100;
            break;
        }
    }

    brun = FALSE;
    m_NATO4285pro.Demode_PSKrealFSE_free();
    free(data);
    free(outbyte);
    free(expsym);
    delete[] headType;

    fin.close();
    printf("STANAG4285 demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);
}



void STANAG4529_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t m_FileLength = 0;
    if (!OpenProtocolInput(filePath, fin, m_FileLength, openError))
    {
        std::cerr << openError << "\n";
        return;
    }

    // 初始化参数变量，分配IPP内存
    const int nLeng = 6000;
    Ipp16s* data = (Ipp16s*)malloc(sizeof(Ipp16s) * nLeng);
    Ipp8u* outbyte = (Ipp8u*)malloc(sizeof(Ipp8u) * 3000);
    Ipp32fc* expsym = (Ipp32fc*)malloc(sizeof(Ipp32fc) * nLeng);

    int OutLen = 0, byteLeng = 0;
    bool bdetect = false;

    // 初始化解调参数
    float fs = 9.6e3;
    float fc = static_cast<float>(g_runtimeOverrides.hasFrequency ? g_runtimeOverrides.frequency : 2e3);
    float Insample = fs;
    float Outsample = 9.6e3;
    float rFreq = fc / Insample;
    float roll = 0.35f;
    float Baud = 1.2e3;
    int SrctapLen = 2048;

    // 定义头文件类型对象
    HeadType4529* headType = new HeadType4529[nLeng / 80];;
    int headNum = 0;
    int allheadNum = 0;


    UINT nBytesRead;
    DWORD dataHavePro = 0;
    int nProgressPos = 0;
    int p1 = 0;
    int p2 = 0;
    bool brun = true;
    bool headerPrinted = false;  // 标记是否已打印表头

    /*
     * 1. dataRate（数据率）
        NATO4529（case 7）：
        可选值：1800, 1200, 600, 300, 150, 75（6个）
        默认值：1800
      2. FECType（编码类型）—相同
        0 - 对应 UI 选项："否"（不编码）
        1 - 对应 UI 选项："是"（编码）
        默认值：1（编码）
      3. InterType（交织长度）—相同
        0 - 对应 UI 选项："No"（无交织）
        1 - 对应 UI 选项："Short"（短交织）
        2 - 对应 UI 选项："Long"（长交织）
        默认值：2（Long）
    */
    int dataRate = g_runtimeOverrides.hasDataRate ? g_runtimeOverrides.dataRate : 1800;
    int FECType = 1;
    int InterType = 2;

    CNATO4529pro m_NATO4529pro;
    m_NATO4529pro.Demode_PSKrealFSE_ini(Insample, Outsample, nLeng,
        fc, 8, 128, roll, Baud, SrctapLen, dataRate, FECType, InterType);

    float m_DemodeTh = 0.5;
    while (brun)
    {
        // 读取nLeng个short样本（二进制）
        fin.read(reinterpret_cast<char*>(data), nLeng * sizeof(short));
        // 获取实际读取的字节数 → 转换为样本数
        DWORD nBytesRead = static_cast<DWORD>(fin.gcount()) / sizeof(short);

        // 读取到文件末尾，终止循环
        if (nBytesRead == 0) break;

        m_NATO4529pro.Demode_PSKrealFSE(data, nLeng, 
            8, 128, 7, m_DemodeTh, OutLen, outbyte, byteLeng, headType, headNum);


        // 打印 outbyte 的十六进制内容
        //if (byteLeng > 0) {
        //    std::cout << "byteLeng=" << byteLeng << ", outbyte (hex):" << std::endl;
        //    for (int i = 0; i < byteLeng; i++) {
        //        if (i > 0 && (i % 16 == 0)) {
        //            std::cout << std::endl;  // 每16字节换行
        //        }
        //        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2)
        //            << static_cast<unsigned int>(outbyte[i]) << " ";
        //    }
        //    std::cout << std::dec << std::endl << std::endl;  // 恢复十进制输出
        //}

        if (headNum > 0)
        {
            for (int i = 0; i < headNum; i++)
            {
                gl_OutList.BeginTime = SampleToTime(dataHavePro + headType[i].position, Insample);
                gl_OutList.sigType = "NATO4529";
                gl_OutList.frequency = headType[i].fre;
                gl_OutList.dataRate = headType[i].dataRate;
                gl_OutList.interType = headType[i].interType;
            }

            // 使用 snprintf 格式化输出（固定宽度，左对齐）
            char szLine[512] = { 0 };
            snprintf(szLine, sizeof(szLine),
                "%-8d  %-15s  %-15s  %-12.2f  %-10d  %-15s\n",
                allheadNum,
                gl_OutList.sigType.c_str(),
                gl_OutList.BeginTime.c_str(),
                gl_OutList.frequency,
                gl_OutList.dataRate,
                gl_OutList.interType.c_str());

            std::cout << szLine;
            allheadNum++;
        }

        dataHavePro = dataHavePro + nBytesRead;
        nProgressPos = (int)(100.0 * dataHavePro / (double)m_FileLength);
        if (nProgressPos >= 100)nProgressPos = 100;
        if (nBytesRead < nLeng)
        {
            nProgressPos = 100;
            break;
        }
    }

    brun = FALSE;
    m_NATO4529pro.Demode_PSKrealFSE_free();
    free(data);
    free(outbyte);
    free(expsym);
    delete[] headType;

    fin.close();
    printf("STANAG4529 demod complete! Total processed samples: %lu, Total messages: %d\n",
        dataHavePro, allheadNum);
}



void KG84_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t sampleCount = 0;
    if (!OpenProtocolInput(filePath, fin, sampleCount, openError))
    {
        std::cerr << openError << "\n";
        return;
    }
    const size_t m_FileLength = static_cast<size_t>(sampleCount);


    // 读取文件全部数据
    int16_t* sdata = (int16_t*)malloc(sizeof(int16_t) * m_FileLength);
    fin.read(reinterpret_cast<char*>(sdata), sizeof(int16_t) * m_FileLength);


    // 转为 long 类型
    double* data = (double*)malloc(sizeof(double) * m_FileLength);
    for (int i = 0; i < m_FileLength; i++) {
        data[i] = static_cast<double>(sdata[i]);
    }

    // 初始化输出数组
    double* in_cx = new double[2 * m_FileLength];

    // 下变频，转为复信号
    float fs = 9.6e3;
    float df = -2e3;
    double* in_re = new double[m_FileLength];
    double* in_im = new double[m_FileLength];
    double omega = 2 * M_PI * df;
    for (int i = 0; i < m_FileLength; i++)
    {
        in_re[i] = data[i] *  cos(omega * static_cast<double>(i) / fs);
        in_im[i] = data[i] * -sin(omega * static_cast<double>(i) / fs);
    }

    // 低通滤波器
    float pass_freq = 2e3;  // 通带频率 2kHz
    float stop_freq = 3e3;  // 阻带频率 3kHz
    float pass = pass_freq / fs;  // 归一化通带频率
    float stop = stop_freq / fs;  // 归一化阻带频率
    float pa = 0.01;
    float sp = 75;
    CDataFIRDF firlpf;
    firlpf.IinitialLPFPara(FIRLPFDT_Kaiser, pass, stop, pa, sp);
    firlpf.Filter(in_re, in_im, m_FileLength);
    for (int i = 0; i < m_FileLength; i++) {
        in_cx[2 * i + 0] = in_re[i];
        in_cx[2 * i + 1] = in_im[i];
    }


    // KG-84 2FSK 解调
    CSignalDemodProbe demod;
    float demod_rb = 0.075e3;
    float demod_df = 0.875e3;
    demod.InitDemodParam(fs, demod_rb, demod_df, SMT_2FSK, false);
    demod.InputData(in_cx, m_FileLength * 2);
    DWORD symLen = 0;
    char* sym = demod.GetSignalSymbol(symLen);
    std::string symStr;
    for (int i = 0; i < symLen; i++) {
        if (sym[i] == 1) {
            symStr.push_back('1');
        }
        else {
            symStr.push_back('0');
        }
    }

    std::cout << "KG84 Demodulation Result: " << std::endl;
    std::cout << symStr << std::endl;

    // 保存星座图
//#define kg84_channelizer_out    "d:/0_kunshan/kun-data-out/kg84_demod_longiq.dat"
//    long* consta = demod.GetSignalDemod(symLen, false);
//    std::ofstream fout(kg84_channelizer_out, std::ios::binary | std::ios::out);
//    fout.write(reinterpret_cast<char*>(consta), sizeof(long) * symLen);
//    fout.close();


    free(sdata);
    free(data);
    delete[] in_re;
    delete[] in_im;
    delete[] in_cx;
    fin.close();
}


void FSK2_BCH_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t sampleCount = 0;
    if (!OpenProtocolInput(filePath, fin, sampleCount, openError))
    {
        std::cerr << openError << "\n";
        return;
    }
    const size_t m_FileLength = static_cast<size_t>(sampleCount);

    // 读取文件全部数据
    int16_t* sdata = (int16_t*)malloc(sizeof(int16_t) * m_FileLength);
    fin.read(reinterpret_cast<char*>(sdata), sizeof(int16_t) * m_FileLength);


    // 转为 long 类型
    double* data = (double*)malloc(sizeof(double) * m_FileLength);
    for (int i = 0; i < m_FileLength; i++) {
        data[i] = static_cast<double>(sdata[i]);
    }

    // 初始化输出数组
    double* in_cx = new double[2 * m_FileLength];

    // 下变频，转为复信号
    float fs = 9.6e3;
    float df = -2e3;
    double* in_re = new double[m_FileLength];
    double* in_im = new double[m_FileLength];
    double omega = 2 * M_PI * df;
    for (int i = 0; i < m_FileLength; i++)
    {
        in_re[i] = data[i] * cos(omega * static_cast<double>(i) / fs);
        in_im[i] = data[i] * -sin(omega * static_cast<double>(i) / fs);
    }

    // 低通滤波器
    float pass_freq = 2e3;  // 通带频率 2kHz
    float stop_freq = 3e3;  // 阻带频率 3kHz
    float pass = pass_freq / fs;  // 归一化通带频率
    float stop = stop_freq / fs;  // 归一化阻带频率
    float pa = 0.01;
    float sp = 75;
    CDataFIRDF firlpf;
    firlpf.IinitialLPFPara(FIRLPFDT_Kaiser, pass, stop, pa, sp);
    firlpf.Filter(in_re, in_im, m_FileLength);
    for (int i = 0; i < m_FileLength; i++) {
        in_cx[2 * i + 0] = in_re[i];
        in_cx[2 * i + 1] = in_im[i];
    }


    // 2FSK 解调
    CSignalDemodProbe demod;
    float demod_rb = 0.075e3;
    float demod_df = 0.85e3;
    demod.InitDemodParam(fs, demod_rb, demod_df, SMT_2FSK, false);
    demod.InputData(in_cx, m_FileLength * 2);
    DWORD symLen = 0;
    char* sym = demod.GetSignalSymbol(symLen);
    std::string symStr;
    for (int i = 0; i < symLen; i++) {
        if (sym[i] == 1) {
            symStr.push_back('1');
        }
        else {
            symStr.push_back('0');
        }
    }

    std::cout << "2FSK(BCH) Demodulation Result: " << std::endl;
    std::cout << symStr << std::endl;

    // 保存星座图
//#define kg84_channelizer_out    "d:/0_kunshan/kun-data-out/kg84_demod_longiq.dat"
//    long* consta = demod.GetSignalDemod(symLen, false);
//    std::ofstream fout(kg84_channelizer_out, std::ios::binary | std::ios::out);
//    fout.write(reinterpret_cast<char*>(consta), sizeof(long) * symLen);
//    fout.close();


    free(sdata);
    free(data);
    delete[] in_re;
    delete[] in_im;
    delete[] in_cx;
    fin.close();

}


void PI4QPSK_test(const fs::path& filePath)
{
    std::ifstream fin;
    std::string openError;
    std::uint64_t sampleCount = 0;
    if (!OpenProtocolInput(filePath, fin, sampleCount, openError))
    {
        std::cerr << openError << "\n";
        return;
    }
    const size_t m_FileLength = static_cast<size_t>(sampleCount);


    // 读取文件全部数据
    int16_t* sdata = (int16_t*)malloc(sizeof(int16_t) * m_FileLength);
    fin.read(reinterpret_cast<char*>(sdata), sizeof(int16_t) * m_FileLength);


    // 转为 long 类型
    double* data = (double*)malloc(sizeof(double) * m_FileLength);
    for (int i = 0; i < m_FileLength; i++) {
        data[i] = static_cast<double>(sdata[i]);
    }

    // 初始化输出数组
    double* in_cx = new double[2 * m_FileLength];

    // 下变频，转为复信号
    float fs = 9.6e3;
    float df = -2.4e3;
    double* in_re = new double[m_FileLength];
    double* in_im = new double[m_FileLength];
    double omega = 2 * M_PI * df;
    for (int i = 0; i < m_FileLength; i++)
    {
        in_re[i] = data[i] * cos(omega * static_cast<double>(i) / fs);
        in_im[i] = data[i] * -sin(omega * static_cast<double>(i) / fs);
    }

    // 低通滤波器
    float pass_freq = 2e3;  // 通带频率 2kHz
    float stop_freq = 3e3;  // 阻带频率 3kHz
    float pass = pass_freq / fs;  // 归一化通带频率
    float stop = stop_freq / fs;  // 归一化阻带频率
    float pa = 0.01;
    float sp = 75;
    CDataFIRDF firlpf;
    firlpf.IinitialLPFPara(FIRLPFDT_Kaiser, pass, stop, pa, sp);
    firlpf.Filter(in_re, in_im, m_FileLength);
    for (int i = 0; i < m_FileLength; i++) {
        in_cx[2 * i + 0] = in_re[i];
        in_cx[2 * i + 1] = in_im[i];
    }


    // PI4-QPSK 解调
    CSignalDemodProbe demod;
    float demod_rb = 0.125e3;
    float demod_df = 0;
    demod.InitDemodParam(fs, demod_rb, demod_df, SMT_PI4DQPSK, false);
    demod.InputData(in_cx, m_FileLength * 2);
    DWORD symLen = 0;
    char* sym = demod.GetSignalSymbol(symLen);
    std::string symStr;
    for (int i = 0; i < symLen; i++) {
        if (sym[i] == 1) {
            symStr.push_back('1');
        }
        else {
            symStr.push_back('0');
        }
    }

    // 打印数据
    std::cout << "PI/4-QPSK Demodulation Result: " << std::endl;
    std::cout << symStr << std::endl;

    // 保存星座图
//#define kg84_channelizer_out    "d:/0_kunshan/kun-data-out/kg84_demod_longiq.dat"
//    long* consta = demod.GetSignalDemod(symLen, false);
//    std::ofstream fout(kg84_channelizer_out, std::ios::binary | std::ios::out);
//    fout.write(reinterpret_cast<char*>(consta), sizeof(long) * symLen);
//    fout.close();


    free(sdata);
    free(data);
    delete[] in_re;
    delete[] in_im;
    delete[] in_cx;
    fin.close();
}






int main(int argc, char* argv[])
{
    hfinput::InputParser parser;
    hfinput::InputOptions options;
    std::string error;
    if (!parser.Parse(argc, argv, options, error)) {
        std::cerr << "Argument error: " << error << "\n\n";
        PrintUsage(argv[0]);
        return 1;
    }
    if (options.show_help) {
        PrintUsage(argv[0]);
        return 0;
    }

    fs::path resolvedPath = options.resolved_input;
    std::string protocolToken = options.protocol;
    g_runtimeOverrides.hasFrequency = options.frequency.has_value();
    g_runtimeOverrides.frequency = options.frequency.value_or(0.0);
    g_runtimeOverrides.hasDataRate = options.data_rate.has_value();
    g_runtimeOverrides.dataRate = options.data_rate.value_or(0);
    if (resolvedPath.empty()) {
        while (true) {
            std::string inputLine;
            if (!hfinput::InputParser::ReadLine("Input path: ", inputLine)) {
                std::cerr << "No input path provided.\n";
                return 1;
            }
            if (hfinput::InputParser::ResolveInputPath(inputLine, options.search_dirs, resolvedPath, error)) {
                break;
            }
            std::cerr << error << "\n";
            std::cerr << "Hint: absolute path, relative path, and filename-only are supported.\n";
        }

        if (!PromptProtocolMenu(protocolToken)) {
            std::cerr << "Protocol selection cancelled.\n";
            return 1;
        }
    }

    ProtocolType protocol = ProtocolType::LINK11_CLEW;
    bool isAuto = false;
    if (!TryParseProtocolToken(protocolToken, protocol, isAuto)) {
        std::cerr << "Unknown protocol token: " << protocolToken << "\n";
        PrintUsage(argv[0]);
        return 1;
    }

    if (isAuto) {
        if (!DetectProtocolForFile(resolvedPath, protocol)) {
            std::cerr << "Auto detect failed. Try -p with a specific protocol.\n";
            return 1;
        }
        std::cout << "Auto detected protocol: " << ProtocolTypeName(protocol) << "\n";
    }

    std::cout << "Input: " << resolvedPath.string() << "\n";
    std::cout << "Protocol: " << ProtocolTypeName(protocol) << "\n";
    RunProtocol(protocol, resolvedPath);
    return 0;
}
