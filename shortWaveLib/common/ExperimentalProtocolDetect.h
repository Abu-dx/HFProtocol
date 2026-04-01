#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "InputAudio.h"
#include "G3AleTypes.h"
#include "G3AleDetector.h"
#include "../shortWaveProtocalWaveLib/ProtolDetect.h"
#pragma comment(lib, "MIL141Bpro.lib")

namespace hfexperimental {

namespace fs = std::filesystem;

inline bool ReadAllPcm16(
    const fs::path& path,
    std::vector<Ipp16s>& samples,
    hfaudio::AudioInputInfo& inputInfo,
    std::string& error)
{
    std::ifstream fin;
    if (!hfaudio::OpenPcm16InputStream(path, fin, inputInfo, error)) {
        return false;
    }

    const std::uint64_t sampleCount = hfaudio::SampleCount(inputInfo);
    if (sampleCount == 0) {
        error = "Input file has no PCM samples: " + path.string();
        return false;
    }

    if (sampleCount > static_cast<std::uint64_t>((std::numeric_limits<std::size_t>::max)())) {
        error = "Input file is too large to analyze in-memory: " + path.string();
        return false;
    }

    samples.resize(static_cast<std::size_t>(sampleCount));
    fin.read(reinterpret_cast<char*>(samples.data()), static_cast<std::streamsize>(sampleCount * sizeof(Ipp16s)));
    const std::size_t readSamples = static_cast<std::size_t>(fin.gcount() / sizeof(Ipp16s));
    if (readSamples == 0) {
        error = "Failed to read PCM samples from: " + path.string();
        samples.clear();
        return false;
    }

    samples.resize(readSamples);
    return true;
}

inline double GoertzelMagnitude(
    const Ipp16s* samples,
    std::size_t sampleCount,
    double sampleRate,
    double frequency)
{
    if (sampleCount == 0 || sampleRate <= 0.0) {
        return 0.0;
    }

    const double omega = 2.0 * 3.14159265358979323846 * frequency / sampleRate;
    const double coeff = 2.0 * std::cos(omega);
    double s0 = 0.0;
    double s1 = 0.0;
    double s2 = 0.0;
    for (std::size_t i = 0; i < sampleCount; ++i) {
        s0 = static_cast<double>(samples[i]) + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    const double power = s1 * s1 + s2 * s2 - coeff * s1 * s2;
    return power > 0.0 ? std::sqrt(power) : 0.0;
}

inline bool Analyze3GALEByMil141B(
    const fs::path& path,
    DetectionResult& result,
    std::string& error)
{
    result = g3ale::AnalyzeFile(path, error);
    return error.empty();
}

inline bool AnalyzeANDVT(
    const fs::path& path,
    DetectionResult& result,
    std::string& error)
{
    result = DetectionResult{};
    result.token = "andvt";
    result.display_name = "ANDVT";
    result.protocol = "unknown";

    std::vector<Ipp16s> samples;
    hfaudio::AudioInputInfo inputInfo;
    if (!ReadAllPcm16(path, samples, inputInfo, error)) {
        return false;
    }

    const std::size_t window = 4096;
    const std::size_t hop = 512;
    if (samples.size() < window) {
        result.note = "Input is shorter than one analysis window.";
        result.notes.push_back(result.note);
        return true;
    }

    struct EnergyWindow
    {
        double energy = 0.0;
        std::size_t index = 0;
    };

    std::vector<EnergyWindow> energyWindows;
    for (std::size_t i = 0; i + window <= samples.size(); i += hop) {
        double energy = 0.0;
        for (std::size_t k = 0; k < window; ++k) {
            const double value = static_cast<double>(samples[i + k]);
            energy += value * value;
        }
        energyWindows.push_back(EnergyWindow{ energy, i });
    }

    std::sort(energyWindows.begin(), energyWindows.end(), [](const EnergyWindow& left, const EnergyWindow& right) {
        return left.energy > right.energy;
    });

    std::vector<std::size_t> starts;
    for (const EnergyWindow& item : energyWindows) {
        bool farEnough = true;
        for (std::size_t existing : starts) {
            if ((existing > item.index ? existing - item.index : item.index - existing) < window / 2) {
                farEnough = false;
                break;
            }
        }
        if (farEnough) {
            starts.push_back(item.index);
        }
        if (starts.size() == 3) {
            break;
        }
    }

    const double sampleRate = inputInfo.sample_rate == 0 ? 9600.0 : static_cast<double>(inputInfo.sample_rate);
    std::vector<double> frequencies;
    for (double freq = 300.0; freq <= 3300.0; freq += 25.0) {
        frequencies.push_back(freq);
    }

    double peakCountSum = 0.0;
    double strongCountSum = 0.0;
    double spanSum = 0.0;
    double centerFrequencySum = 0.0;

    for (std::size_t start : starts) {
        std::vector<double> magnitudes(frequencies.size(), 0.0);
        for (std::size_t i = 0; i < frequencies.size(); ++i) {
            magnitudes[i] = GoertzelMagnitude(samples.data() + start, window, sampleRate, frequencies[i]);
        }

        const double maxMagnitude = *std::max_element(magnitudes.begin(), magnitudes.end());
        if (maxMagnitude <= 0.0) {
            continue;
        }

        std::vector<std::size_t> strongBins;
        int peakCount = 0;
        for (std::size_t i = 0; i < magnitudes.size(); ++i) {
            if (magnitudes[i] > maxMagnitude * 0.45) {
                strongBins.push_back(i);
            }
            if (i > 0 && i + 1 < magnitudes.size() &&
                magnitudes[i] > maxMagnitude * 0.45 &&
                magnitudes[i] >= magnitudes[i - 1] &&
                magnitudes[i] >= magnitudes[i + 1]) {
                ++peakCount;
            }
        }

        double weightedFrequency = 0.0;
        double weightSum = 0.0;
        for (std::size_t i = 0; i < magnitudes.size(); ++i) {
            if (magnitudes[i] > maxMagnitude * 0.45) {
                weightedFrequency += frequencies[i] * magnitudes[i];
                weightSum += magnitudes[i];
            }
        }

        const double span = strongBins.empty() ? 0.0 : frequencies[strongBins.back()] - frequencies[strongBins.front()];
        peakCountSum += static_cast<double>(peakCount);
        strongCountSum += static_cast<double>(strongBins.size());
        spanSum += span;
        centerFrequencySum += weightSum > 0.0 ? weightedFrequency / weightSum : 0.0;
    }

    const double windowCount = starts.empty() ? 1.0 : static_cast<double>(starts.size());
    const double averagePeakCount = peakCountSum / windowCount;
    const double averageStrongCount = strongCountSum / windowCount;
    const double averageSpan = spanSum / windowCount;
    const double averageCenterFrequency = centerFrequencySum / windowCount;
    const double peakRatio = averageStrongCount > 0.0 ? averagePeakCount / averageStrongCount : 0.0;

    result.tone_peak_count = static_cast<int>(std::lround(averagePeakCount));
    result.strong_bin_count = static_cast<int>(std::lround(averageStrongCount));
    result.tone_span = averageSpan;
    result.frequency = averageCenterFrequency;

    double score = 0.0;
    if (averagePeakCount >= 30.0) score += 0.40;
    if (averageSpan >= 2600.0) score += 0.25;
    if (peakRatio >= 0.72) score += 0.20;
    if (averageCenterFrequency >= 1400.0 && averageCenterFrequency <= 2200.0) score += 0.15;

    result.score = Clamp01(score);
    result.confidence = result.score;
    result.detected = averagePeakCount >= 30.0 && averageSpan >= 2600.0 && peakRatio >= 0.72 && result.score >= 0.75;
    result.protocol = result.detected ? "andvt" : "unknown";
    result.token = result.detected ? "andvt" : "unknown";
    result.note =
        "tonePeaks=" + std::to_string(result.tone_peak_count) +
        " strongBins=" + std::to_string(result.strong_bin_count) +
        " spanHz=" + std::to_string(static_cast<int>(std::lround(result.tone_span))) +
        " peakRatio=" + std::to_string(peakRatio);
    result.notes.push_back(result.note);
    return true;
}

inline bool DetectLink22BearerCandidate(
    const fs::path& path,
    DetectionResult& result,
    std::string& error)
{
    result = DetectionResult{};
    result.token = "link22-candidate";
    result.display_name = "Link22 bearer candidate";
    result.protocol = "unknown";

    std::ifstream fin;
    hfaudio::AudioInputInfo inputInfo;
    if (!hfaudio::OpenPcm16InputStream(path, fin, inputInfo, error)) {
        return false;
    }

    const int protocolCount = 8;
    std::vector<BOOL> selected(10, FALSE);
    selected[wNATO4529] = TRUE;

    const int blockLength = 8192;
    const float inSample = inputInfo.sample_rate == 0 ? 9.6e3f : static_cast<float>(inputInfo.sample_rate);
    const Ipp32f threshold = 0.45f;

    CProtolDetect detector;
    detector.ProtolDetect_ini(blockLength, inSample, selected.data(), protocolCount);

    std::vector<short> data(blockLength);
    double bestScore = 0.0;
    double bestFrequency = 0.0;
    bool hit = false;
    while (true)
    {
        fin.read(reinterpret_cast<char*>(data.data()), blockLength * sizeof(short));
        const DWORD readSamples = static_cast<DWORD>(fin.gcount()) / sizeof(short);
        if (readSamples == 0) {
            break;
        }

        ProtocolOut protocolOut;
        if (detector.ProtolDetect(data.data(), static_cast<int>(readSamples), threshold, selected.data(), protocolCount, protocolOut)) {
            const std::string protocolName = protocolOut.ProtocolName.GetString();
            if (protocolName.find("4529") != std::string::npos) {
                hit = true;
                if (protocolOut.maxCor > bestScore) {
                    bestScore = protocolOut.maxCor;
                    bestFrequency = protocolOut.frequency;
                }
            }
        }
    }

    detector.ProtolDetect_free();

    result.detected = hit;
    result.score = bestScore;
    result.confidence = bestScore;
    result.frequency = bestFrequency;
    result.protocol = hit ? "link22-candidate" : "unknown";
    result.note = hit
        ? "Manual candidate only, based on STANAG 4539 / NATO4529 bearer hit."
        : "No STANAG 4539 bearer candidate detected.";
    result.notes.push_back(result.note);
    return true;
}

}  // namespace hfexperimental
