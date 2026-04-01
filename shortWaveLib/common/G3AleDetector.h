#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "InputAudio.h"
#include "G3AleTypes.h"
#include "G3AlePdu.h"
#include "G3AleTiming.h"
#include "../shortWaveProtocalWaveLib/MIL141Bpro.h"

namespace hfexperimental {
namespace g3ale {

inline bool IsNearBurst(const BurstInfo& existing, const BurstInfo& candidate)
{
    return existing.wave_type == candidate.wave_type && std::llabs(existing.start_sample - candidate.start_sample) <= 200;
}

inline void AppendBurstUnique(std::vector<BurstInfo>& bursts, const BurstInfo& burst)
{
    for (std::size_t i = 0; i < bursts.size(); ++i) {
        if (IsNearBurst(bursts[i], burst)) {
            if (burst.end_sample > bursts[i].end_sample) bursts[i].end_sample = burst.end_sample;
            if (burst.end_time > bursts[i].end_time) bursts[i].end_time = burst.end_time;
            if (burst.duration_ms > bursts[i].duration_ms) bursts[i].duration_ms = burst.duration_ms;
            if (!burst.deinterleaved_bits.empty()) bursts[i].deinterleaved_bits = burst.deinterleaved_bits;
            if (!burst.viterbi_bits.empty()) bursts[i].viterbi_bits = burst.viterbi_bits;
            if (!burst.walsh_scores.empty()) bursts[i].walsh_scores = burst.walsh_scores;
            if (burst.decode_quality > bursts[i].decode_quality) bursts[i].decode_quality = burst.decode_quality;
            if (burst.preamble_score > bursts[i].preamble_score) bursts[i].preamble_score = burst.preamble_score;
            if (burst.is_legal_bw0) bursts[i].is_legal_bw0 = true;
            if (!burst.reject_reason.empty()) bursts[i].reject_reason = burst.reject_reason;
            return;
        }
    }
    bursts.push_back(burst);
}

inline bool HasNearbyTlc(const std::vector<BurstInfo>& bursts, const BurstInfo& bw0)
{
    for (std::size_t i = 0; i < bursts.size(); ++i) {
        if (bursts[i].wave_type != wMIL141BTLC) {
            continue;
        }
        const double deltaSec = bw0.begin_time - bursts[i].begin_time;
        if (deltaSec >= 0.0 && deltaSec <= 1.2) {
            return true;
        }
    }
    return false;
}

inline bool HasStableWalshDecode(const BurstInfo& burst)
{
    if (burst.walsh_scores.size() != 13u) {
        return false;
    }
    double minValue = burst.walsh_scores[0];
    double maxValue = burst.walsh_scores[0];
    for (std::size_t i = 1; i < burst.walsh_scores.size(); ++i) {
        if (burst.walsh_scores[i] < minValue) minValue = burst.walsh_scores[i];
        if (burst.walsh_scores[i] > maxValue) maxValue = burst.walsh_scores[i];
    }
    return maxValue >= 0.20 && minValue >= 0.05;
}

inline bool HasStableViterbiOutput(const BurstInfo& burst)
{
    if (burst.viterbi_bits.size() != 26u) {
        return false;
    }
    int transitions = 0;
    for (std::size_t i = 1; i < burst.viterbi_bits.size(); ++i) {
        if (burst.viterbi_bits[i] != burst.viterbi_bits[i - 1]) {
            ++transitions;
        }
    }
    return transitions >= 3;
}

inline std::string DiagnoseBw0RejectReason(const BurstInfo& burst)
{
    if (burst.duration_ms < 300.0 || burst.duration_ms > 700.0) {
        return "duration_out_of_range";
    }
    if (burst.preamble_score < 0.12) {
        return "weak_preamble";
    }
    if (!HasStableWalshDecode(burst)) {
        return "no_stable_walsh_decode";
    }
    if (burst.deinterleaved_bits.size() != 52u) {
        return "bad_deinterleave";
    }
    if (burst.viterbi_bits.size() != 26u) {
        return "not_26bit_candidate";
    }
    if (!HasStableViterbiOutput(burst)) {
        return "bad_viterbi_output";
    }
    return std::string();
}

inline std::string DetermineSampleAssessment(const DetectionResult& result)
{
    if (result.bw0_count > 0) {
        for (std::size_t i = 0; i < result.bursts.size(); ++i) {
            if (result.bursts[i].wave_type == wMIL141BBW0 && result.bursts[i].is_legal_bw0) {
                return "sample_contains_usable_bw0";
            }
        }
        return "sample_contains_bw0_but_frontend_cannot_lock";
    }
    if (result.tlc_count > 0 || result.bw1_count > 0 || result.bw3_count > 0) {
        return "sample_not_suitable_as_v1_primary_positive";
    }
    return "unknown";
}

inline DetectionResult AnalyzeFile(const std::filesystem::path& path, std::string& error)
{
    DetectionResult result;
    result.token = "mil141b";
    result.display_name = "MIL141B";
    result.protocol = "unknown";
    result.internal_protocol = "unknown";

    std::ifstream fin;
    hfaudio::AudioInputInfo inputInfo;
    if (!hfaudio::OpenPcm16InputStream(path, fin, inputInfo, error)) {
        return result;
    }

    const int nLeng = 6000;
    std::vector<Ipp16s> data(static_cast<std::size_t>(nLeng), 0);
    std::vector<Ipp8u> outbyte(3000);
    std::vector<Ipp16s> expout(static_cast<std::size_t>(nLeng) * 2u, 0);
    std::vector<HeadType141B> headType(32);

    CMIL141Bpro detector;
    detector.MIL141Bdemode_ini(nLeng, 9600, 9600, 4, 0.45f, 2400.0f, 512);

    std::int64_t processedSamples = 0;
    int outLen = 0;
    int byteLen = 0;
    int headNum = 0;

    while (true)
    {
        fin.read(reinterpret_cast<char*>(data.data()), nLeng * sizeof(Ipp16s));
        const int readSamples = static_cast<int>(fin.gcount() / sizeof(Ipp16s));
        if (readSamples == 0) {
            break;
        }
        if (readSamples < nLeng) {
            std::fill(data.begin() + readSamples, data.end(), 0);
        }

        headNum = 0;
        detector.MIL141Bdemode(data.data(), nLeng, 4, expout.data(), outLen, outbyte.data(), byteLen, headType.data(), headNum);

        for (int i = 0; i < headNum; ++i)
        {
            if (headType[i].BWtype == wMIL141BBW0) {
                continue;
            }
            BurstInfo burst;
            burst.start_sample = processedSamples + static_cast<std::int64_t>(headType[i].position);
            burst.end_sample = burst.start_sample;
            burst.begin_time = inputInfo.sample_rate == 0 ? 0.0 : static_cast<double>(burst.start_sample) / static_cast<double>(inputInfo.sample_rate);
            burst.end_time = burst.begin_time;
            burst.duration_ms = 0.0;
            burst.frequency = headType[i].fre;
            burst.wave_type = headType[i].BWtype;
            burst.frame_type = FrameTypeFromWaveType(headType[i].BWtype);
            AppendBurstUnique(result.bursts, burst);
        }

        const int candidateCount = detector.GetBW0BurstCandidateCount();
        const BW0BurstCandidate* candidates = detector.GetBW0BurstCandidates();
        for (int i = 0; i < candidateCount; ++i)
        {
            if (candidates == NULL || !candidates[i].valid) {
                continue;
            }

            BurstInfo burst;
            burst.start_sample = processedSamples + static_cast<std::int64_t>(candidates[i].startSample);
            burst.end_sample = processedSamples + static_cast<std::int64_t>(candidates[i].endSample);
            burst.begin_time = inputInfo.sample_rate == 0 ? 0.0 : static_cast<double>(burst.start_sample) / static_cast<double>(inputInfo.sample_rate);
            burst.end_time = inputInfo.sample_rate == 0 ? 0.0 : static_cast<double>(burst.end_sample) / static_cast<double>(inputInfo.sample_rate);
            burst.duration_ms = (burst.end_time - burst.begin_time) * 1000.0;
            burst.frequency = candidates[i].centerHz;
            burst.wave_type = candidates[i].headType;
            burst.frame_type = FrameTypeFromWaveType(candidates[i].headType);
            burst.preamble_score = candidates[i].preambleScore;
            burst.decode_quality = candidates[i].decodeQuality;
            burst.deinterleaved_bits = BitsToString(candidates[i].deinterleavedBits, candidates[i].deinterleavedBitCount);
            burst.viterbi_bits = BitsToString(candidates[i].viterbiBits, candidates[i].viterbiBitCount);
            for (int scoreIndex = 0; scoreIndex < candidates[i].walshScoreCount; ++scoreIndex) {
                burst.walsh_scores.push_back(candidates[i].walshScores[scoreIndex]);
            }
            AppendBurstUnique(result.bursts, burst);
        }

        processedSamples += readSamples;
        if (readSamples < nLeng) {
            break;
        }
    }

    detector.MIL141Bdemode_free();

    if (result.bursts.empty()) {
        result.note = "No MIL141-family bursts detected.";
        result.notes.push_back(result.note);
        return result;
    }

    std::sort(result.bursts.begin(), result.bursts.end(), [](const BurstInfo& left, const BurstInfo& right) {
        return left.start_sample < right.start_sample;
    });

    double frequencySum = 0.0;
    double bestLegalBw0Quality = 0.0;
    for (std::size_t i = 0; i < result.bursts.size(); ++i) {
        result.bursts[i].duration_ms = (result.bursts[i].end_time - result.bursts[i].begin_time) * 1000.0;
        frequencySum += result.bursts[i].frequency;
        switch (result.bursts[i].wave_type)
        {
        case wMIL141BBW0: ++result.bw0_count; break;
        case wMIL141BBW1: ++result.bw1_count; break;
        case wMIL141BBW3: ++result.bw3_count; break;
        case wMIL141BTLC: ++result.tlc_count; break;
        default: ++result.other_count; break;
        }
        if (result.bursts[i].wave_type == wMIL141BBW0) {
            result.bursts[i].tlc_detected = HasNearbyTlc(result.bursts, result.bursts[i]);
            result.bursts[i].reject_reason = DiagnoseBw0RejectReason(result.bursts[i]);
            result.bursts[i].is_legal_bw0 = result.bursts[i].reject_reason.empty();
            if (result.bursts[i].is_legal_bw0 && result.bursts[i].decode_quality > bestLegalBw0Quality) {
                bestLegalBw0Quality = result.bursts[i].decode_quality;
            }
        }
    }
    result.total_bursts = static_cast<int>(result.bursts.size());
    result.frequency = frequencySum / static_cast<double>(result.bursts.size());

    int candidateRank = 1;
    for (std::size_t i = 0; i < result.bursts.size(); ++i)
    {
        if (result.bursts[i].wave_type != wMIL141BBW0 || result.bursts[i].viterbi_bits.size() != 26u) {
            continue;
        }
        PduInfo pdu;
        if (ParsePduCandidate(result.bursts[i].viterbi_bits, candidateRank, pdu)) {
            result.pdus.push_back(pdu);
            ++candidateRank;
        }
    }

    double maxPduScore = 0.0;
    double crcHits = 0.0;
    int recognizedPdus = 0;
    for (std::size_t i = 0; i < result.pdus.size(); ++i) {
        const double thisScore = ScorePdu(result.pdus[i]);
        if (thisScore > maxPduScore) {
            maxPduScore = thisScore;
        }
        if (result.pdus[i].type != "unknown_pdu") {
            ++recognizedPdus;
        }
        if (result.pdus[i].crc_ok) {
            crcHits += 1.0;
        }
    }

    const int legalBw0Count = static_cast<int>(std::count_if(result.bursts.begin(), result.bursts.end(), [](const BurstInfo& burst) {
        return burst.wave_type == wMIL141BBW0 && burst.is_legal_bw0;
    }));

    result.scores.bw0_score = Clamp01((legalBw0Count > 0 ? 0.55 : 0.0) + (bestLegalBw0Quality > 0.0 ? 0.20 : 0.0) + (bestLegalBw0Quality > 0.3 ? 0.10 : 0.0) + (bestLegalBw0Quality > 0.6 ? 0.15 : 0.0));
    result.scores.pdu_score = maxPduScore;
    result.scores.crc_score = result.pdus.empty() ? 0.0 : Clamp01(crcHits / static_cast<double>(result.pdus.size()));

    const TimingAnalysis timing = AnalyzeTiming(result.bursts, result.pdus);
    result.transaction_type = timing.transaction_type;
    result.timing_ok = timing.timing_ok;
    result.scores.timing_score = timing.timing_score;
    result.scores.transaction_score = timing.transaction_score;
    result.timing_violations = timing.violations;
    result.event_trace = timing.event_trace;
    result.notes.insert(result.notes.end(), timing.violations.begin(), timing.violations.end());

    result.sample_assessment = DetermineSampleAssessment(result);
    result.notes.push_back(std::string("sample_assessment=") + result.sample_assessment);
    result.notes.push_back("strict_v1_status=strict_v1_safe_fallback_complete");
    {
        std::string observed = "observed_burst_family=";
        if (result.tlc_count > 0) observed += "TLC";
        if (result.bw1_count > 0) observed += (observed.back() == '=' ? "" : ",") + std::string("BW1");
        if (result.bw3_count > 0) observed += (observed.back() == '=' ? "" : ",") + std::string("BW3");
        if (result.bw0_count > 0) observed += (observed.back() == '=' ? "" : ",") + std::string("BW0");
        if (result.other_count > 0) observed += (observed.back() == '=' ? "" : ",") + std::string("OTHER");
        if (observed.back() == '=') observed += "none";
        result.notes.push_back(observed);
    }

    if (recognizedPdus == 0) {
        result.notes.push_back("no_valid_le_pdu");
    }

    result.confidence = Clamp01(
        result.scores.bw0_score * 0.30 +
        result.scores.pdu_score * 0.25 +
        result.scores.crc_score * 0.20 +
        result.scores.timing_score * 0.15 +
        result.scores.transaction_score * 0.10);
    result.score = result.confidence;

    if (result.scores.bw0_score >= 0.65 &&
        result.scores.pdu_score >= 0.65 &&
        (result.scores.crc_score >= 0.60 || result.scores.pdu_score >= 0.85) &&
        result.scores.timing_score >= 0.55 &&
        result.scores.transaction_score >= 0.55)
    {
        result.protocol = "3g-ale";
        result.internal_protocol = "3g-ale";
        result.detected = true;
        result.token = "3g-ale";
        result.display_name = "3G-ALE";
        result.note = "Confirmed by BW0 + LE_PDU + CRC/field consistency + timing.";
        result.notes.push_back(result.note);
        return result;
    }

    if (result.bw0_count > 0) {
        result.protocol = "mil141b";
        result.internal_protocol = "mil141b_bw0_candidate";
        result.detected = true;
        result.token = "mil141b";
        result.display_name = "MIL141B";
        result.note = "BW0 detected but protocol evidence is not sufficient for 3G-ALE.";
        result.notes.push_back(result.note);
        return result;
    }

    result.protocol = "mil141b";
    result.internal_protocol = "mil141b";
    result.detected = true;
    result.token = "mil141b";
    result.display_name = "MIL141B";
    result.note = "MIL141B-family bursts detected without legal BW0 evidence.";
    result.notes.push_back(result.note);
    return result;
}

}  // namespace g3ale
}  // namespace hfexperimental


