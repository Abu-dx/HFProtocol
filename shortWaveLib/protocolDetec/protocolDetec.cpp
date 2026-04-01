// Keep MFC headers first so CString/AFX macros are available.
#include <afx.h>
#include <Windows.h>

#include "ipps.h"
#include <atlconv.h>

#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

#include "../shortWaveProtocalWaveLib/ProtolDetect.h"
#include "../common/InputAudio.h"
#include "../common/InputParser.h"
#include "../common/ExperimentalProtocolDetect.h"

#pragma comment(lib, "ProtolDetect.lib")

namespace fs = std::filesystem;

static const int kFilterAuto = -1;
static const int kFilterUnknown = -2;
static const int kFilter3GALE = -3;
static const int kFilterANDVT = -4;
static const int kFilterLink22Candidate = -5;

struct OutList
{
    std::string sigType;
    std::string beginTime;
    double frequency = 0.0;
    int dataRate = 0;
    int interLeng = 0;
};

std::string SampleToTime(DWORD sample, float sampleRate)
{
    const double time = static_cast<double>(sample) / sampleRate;
    char buffer[64] = { 0 };
    sprintf(buffer, "%.6f", time);
    return std::string(buffer);
}

std::string CanonicalToken(const std::string& token)
{
    std::string lowered = hfinput::InputParser::ToLower(hfinput::InputParser::Trim(token));
    std::string compact;
    for (char ch : lowered) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            compact.push_back(ch);
        }
    }
    return compact;
}

int ProtocolFilterIndexFromToken(const std::string& token)
{
    const std::string key = CanonicalToken(token);
    if (key.empty() || key == "auto" || key == "0") return kFilterAuto;
    if (key == "1" || key == "mil110a" || key == "m110a" || key == "110a") return wMIL110A;
    if (key == "2" || key == "mil110b" || key == "m110b" || key == "110b") return wMIL110B;
    if (key == "3" || key == "mil141a" || key == "141a") return wMIL141A;
    if (key == "4" || key == "mil141b" || key == "141b") return wMIL141B;
    if (key == "5" || key == "link11slew" || key == "slew") return wLINK11SLEW;
    if (key == "6" || key == "link11clew" || key == "link11" || key == "clew") return wLINK11CLEW;
    if (key == "7" || key == "4285" || key == "nato4285" || key == "stanag4285" || key == "s4285") return wNATO4285;
    if (key == "8" || key == "4529" || key == "nato4529" || key == "stanag4529" || key == "s4529") return wNATO4529;
    if (key == "9" || key == "3gale" || key == "3g") return kFilter3GALE;
    if (key == "10" || key == "andvt") return kFilterANDVT;
    if (key == "11" || key == "link22candidate" || key == "link22") return kFilterLink22Candidate;
    return kFilterUnknown;
}

std::string ProtocolTokenFromName(const std::string& protocolName)
{
    const std::string key = CanonicalToken(protocolName);
    if (key.find("link11clew") != std::string::npos) return "link11clew";
    if (key.find("link11slew") != std::string::npos) return "link11slew";
    if (key.find("mil110a") != std::string::npos) return "mil110a";
    if (key.find("mil110b") != std::string::npos) return "mil110b";
    if (key.find("mil141a") != std::string::npos) return "mil141a";
    if (key.find("mil141b") != std::string::npos) return "mil141b";
    if (key.find("3gale") != std::string::npos) return "3g-ale";
    if (key.find("andvt") != std::string::npos) return "andvt";
    if (key.find("link22candidate") != std::string::npos) return "link22-candidate";
    if (key.find("4529") != std::string::npos) return "4529";
    if (key.find("4285") != std::string::npos) return "4285";
    return "unknown";
}

void PrintUsage(const char* exeName)
{
    std::cout
        << "Usage:\n"
        << "  " << exeName << " <input_path> [-p protocol] [--search-dir dir]\n"
        << "  " << exeName << "                     (interactive mode)\n\n"
        << "Input format:\n"
        << "  raw PCM short data or 16-bit mono PCM WAV\n\n"
        << "Protocol aliases:\n"
        << "  auto, mil110a, mil110b, mil141a, mil141b, link11slew, link11clew, 4285, 4529, 3g-ale, andvt, link22-candidate\n";
}

bool PromptProtocolMenu(std::string& protocolToken)
{
    std::cout
        << "\nSelect detection filter:\n"
        << "  0) auto\n"
        << "  1) mil110a\n"
        << "  2) mil110b\n"
        << "  3) mil141a\n"
        << "  4) mil141b\n"
        << "  5) link11slew\n"
        << "  6) link11clew\n"
        << "  7) 4285\n"
        << "  8) 4529\n"
        << "  9) 3g-ale\n"
        << " 10) andvt\n"
        << " 11) link22-candidate\n";

    std::string line;
    if (!hfinput::InputParser::ReadLine("Enter number (default 0): ", line)) {
        return false;
    }
    protocolToken = line.empty() ? "auto" : line;
    return true;
}

void PrintExperimentalDetection(const hfexperimental::DetectionResult& result)
{
    std::cout << hfexperimental::FormatDetectionResultJson(result) << "\n";
    std::cout << "DETECTED_PROTOCOL=" << result.token << "\n";
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

    const int selectedFilter = ProtocolFilterIndexFromToken(protocolToken);
    if (selectedFilter == kFilterUnknown) {
        std::cerr << "Unknown protocol token: " << protocolToken << "\n";
        PrintUsage(argv[0]);
        return 1;
    }

    if (selectedFilter == kFilter3GALE) {
        hfexperimental::DetectionResult result;
        if (!hfexperimental::Analyze3GALEByMil141B(resolvedPath, result, error)) {
            std::cerr << error << "\n";
            return 1;
        }
        if (!result.detected) {
            std::cout << "No protocol detected.\n";
            return 2;
        }
        PrintExperimentalDetection(result);
        return 0;
    }
    if (selectedFilter == kFilterANDVT) {
        hfexperimental::DetectionResult result;
        if (!hfexperimental::AnalyzeANDVT(resolvedPath, result, error)) {
            std::cerr << error << "\n";
            return 1;
        }
        if (!result.detected) {
            std::cout << "No protocol detected.\n";
            return 2;
        }
        PrintExperimentalDetection(result);
        return 0;
    }
    if (selectedFilter == kFilterLink22Candidate) {
        hfexperimental::DetectionResult result;
        if (!hfexperimental::DetectLink22BearerCandidate(resolvedPath, result, error)) {
            std::cerr << error << "\n";
            return 1;
        }
        if (!result.detected) {
            std::cout << "No protocol detected.\n";
            return 2;
        }
        PrintExperimentalDetection(result);
        return 0;
    }

    std::ifstream fin;
    hfaudio::AudioInputInfo inputInfo;
    if (!hfaudio::OpenPcm16InputStream(resolvedPath, fin, inputInfo, error)) {
        std::cerr << error << "\n";
        return 1;
    }

    const int protocolCount = 8;
    std::array<BOOL, 10> selected{};
    selected.fill(FALSE);
    if (selectedFilter == kFilterAuto) {
        for (int i = 0; i < protocolCount; ++i) {
            selected[static_cast<size_t>(i)] = TRUE;
        }
    } else {
        selected[static_cast<size_t>(selectedFilter)] = TRUE;
    }

    const int blockLength = 8192;
    const float inSample = inputInfo.sample_rate == 0 ? 9.6e3f : static_cast<float>(inputInfo.sample_rate);
    const float outSample = 9.6e3f;
    const Ipp32f threshold = 0.45f;

    CProtolDetect detector;
    detector.ProtolDetect_ini(blockLength, inSample, selected.data(), protocolCount);

    short* data = static_cast<short*>(malloc(blockLength * sizeof(short)));
    if (data == nullptr) {
        std::cerr << "Memory allocation failed.\n";
        detector.ProtolDetect_free();
        return 1;
    }

    DWORD processedSamples = 0;
    bool foundAny = false;
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
            static_cast<int>(readSamples),
            threshold,
            selected.data(),
            protocolCount,
            result);

        if (detected)
        {
            foundAny = true;
            OutList out;
            out.beginTime = SampleToTime(__int64(processedSamples * 9600.0 / inSample) + result.index, outSample);
            out.sigType = result.ProtocolName.GetString();
            out.dataRate = result.dataRate;
            out.interLeng = result.InterLen;
            out.frequency = result.frequency;

            std::string sigType = out.sigType;
            std::string token = ProtocolTokenFromName(out.sigType);
            if (token == "mil141b") {
                hfexperimental::DetectionResult experimental;
                std::string refineError;
                if (hfexperimental::Analyze3GALEByMil141B(resolvedPath, experimental, refineError) && experimental.protocol == "3g-ale") {
                    sigType = experimental.display_name;
                    token = experimental.token;
                    out.frequency = experimental.frequency;
                }
            }

            std::cout << "Detected signal:\n";
            std::cout << sigType
                << " beginTime=" << out.beginTime
                << " fc=" << out.frequency
                << " dataRate=" << out.dataRate
                << "\n";
            std::cout << "DETECTED_PROTOCOL=" << token << "\n";
        }

        processedSamples += readSamples;
    }

    detector.ProtolDetect_free();
    free(data);

    if (!foundAny) {
        std::cout << "No protocol detected.\n";
        return 2;
    }

    return 0;
}



