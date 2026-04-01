#pragma once

#include <cstdint>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace hfexperimental {

struct BurstInfo
{
    std::int64_t start_sample = 0;
    std::int64_t end_sample = 0;
    double begin_time = 0.0;
    double end_time = 0.0;
    double duration_ms = 0.0;
    double frequency = 0.0;
    int wave_type = 0;
    std::string frame_type;
    double preamble_score = 0.0;
    double decode_quality = 0.0;
    bool tlc_detected = false;
    bool is_legal_bw0 = false;
    std::string reject_reason;
    std::vector<double> walsh_scores;
    std::string deinterleaved_bits;
    std::string viterbi_bits;
};

struct PduField
{
    std::string name;
    std::string value;
};

struct PduInfo
{
    std::string type;
    std::string bits;
    bool crc_ok = false;
    std::string bit_ordering;
    int candidate_rank = 0;
    double field_consistency_score = 0.0;
    std::vector<PduField> fields;
};

struct ScoreBreakdown
{
    double bw0_score = 0.0;
    double pdu_score = 0.0;
    double crc_score = 0.0;
    double timing_score = 0.0;
    double transaction_score = 0.0;
};

struct DetectionResult
{
    bool detected = false;
    std::string protocol = "unknown";
    std::string token;
    std::string display_name;
    std::string internal_protocol = "unknown";
    double confidence = 0.0;
    double score = 0.0;
    double frequency = 0.0;
    int data_rate = 0;
    int inter_length = 0;
    std::string note;
    std::string sample_assessment = "unknown";

    int total_bursts = 0;
    int bw0_count = 0;
    int bw1_count = 0;
    int bw3_count = 0;
    int tlc_count = 0;
    int other_count = 0;

    int tone_peak_count = 0;
    int strong_bin_count = 0;
    double tone_span = 0.0;

    ScoreBreakdown scores;
    std::vector<BurstInfo> bursts;
    std::vector<PduInfo> pdus;
    std::string transaction_type = "unknown";
    bool timing_ok = false;
    std::vector<std::string> timing_violations;
    std::vector<std::string> event_trace;
    std::vector<std::string> notes;
};

inline double Clamp01(double value)
{
    if (value < 0.0) return 0.0;
    if (value > 1.0) return 1.0;
    return value;
}

inline std::string FrameTypeFromWaveType(int waveType)
{
    switch (waveType)
    {
    case 11: return "BW0";
    case 12: return "BW1";
    case 13: return "BW2";
    case 14: return "BW3";
    case 16: return "BW5";
    case 10: return "TLC";
    default: return "";
    }
}

inline std::string BitsToString(const unsigned char* bits, int bitCount)
{
    std::string output;
    output.reserve(bitCount > 0 ? static_cast<std::size_t>(bitCount) : 0u);
    for (int i = 0; i < bitCount; ++i) {
        output.push_back(bits[i] ? '1' : '0');
    }
    return output;
}

inline std::uint32_t BitsToUInt(const std::string& bits, std::size_t start, std::size_t length)
{
    std::uint32_t value = 0;
    const std::size_t end = start + length;
    for (std::size_t i = start; i < end && i < bits.size(); ++i) {
        value <<= 1;
        if (bits[i] == '1') {
            value |= 1u;
        }
    }
    return value;
}

inline std::string JsonEscape(const std::string& input)
{
    std::string out;
    out.reserve(input.size() + 8);
    for (char ch : input) {
        switch (ch) {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out.push_back(ch); break;
        }
    }
    return out;
}

inline std::string FormatDetectionResultJson(const DetectionResult& result)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    oss << "{\n";
    oss << "  \"protocol\": \"" << JsonEscape(result.protocol) << "\",\n";
    oss << "  \"sample_assessment\": \"" << JsonEscape(result.sample_assessment) << "\",\n";
    oss << "  \"confidence\": " << result.confidence << ",\n";
    oss << "  \"scores\": {\n";
    oss << "    \"bw0_score\": " << result.scores.bw0_score << ",\n";
    oss << "    \"pdu_score\": " << result.scores.pdu_score << ",\n";
    oss << "    \"crc_score\": " << result.scores.crc_score << ",\n";
    oss << "    \"timing_score\": " << result.scores.timing_score << ",\n";
    oss << "    \"transaction_score\": " << result.scores.transaction_score << "\n";
    oss << "  },\n";
    oss << "  \"bursts\": [\n";
    for (std::size_t i = 0; i < result.bursts.size(); ++i) {
        const BurstInfo& burst = result.bursts[i];
        oss << "    {\n";
        oss << "      \"type\": \"" << JsonEscape(burst.frame_type) << "\",\n";
        oss << "      \"start_sample\": " << burst.start_sample << ",\n";
        oss << "      \"end_sample\": " << burst.end_sample << ",\n";
        oss << "      \"start_sec\": " << burst.begin_time << ",\n";
        oss << "      \"end_sec\": " << burst.end_time << ",\n";
        oss << "      \"duration_ms\": " << burst.duration_ms << ",\n";
        oss << "      \"center_hz\": " << burst.frequency << ",\n";
        oss << "      \"preamble_score\": " << burst.preamble_score << ",\n";
        oss << "      \"decode_quality\": " << burst.decode_quality << ",\n";
        oss << "      \"tlc_detected\": " << (burst.tlc_detected ? "true" : "false") << ",\n";
        oss << "      \"is_legal_bw0\": " << (burst.is_legal_bw0 ? "true" : "false") << ",\n";
        oss << "      \"reject_reason\": \"" << JsonEscape(burst.reject_reason) << "\",\n";
        oss << "      \"walsh_scores\": [";
        for (std::size_t j = 0; j < burst.walsh_scores.size(); ++j) {
            if (j != 0) oss << ", ";
            oss << burst.walsh_scores[j];
        }
        oss << "],\n";
        oss << "      \"deinterleaved_bits\": \"" << JsonEscape(burst.deinterleaved_bits) << "\",\n";
        oss << "      \"viterbi_bits\": \"" << JsonEscape(burst.viterbi_bits) << "\"\n";
        oss << "    }" << (i + 1 < result.bursts.size() ? "," : "") << "\n";
    }
    oss << "  ],\n";
    oss << "  \"pdus\": [\n";
    for (std::size_t i = 0; i < result.pdus.size(); ++i) {
        const PduInfo& pdu = result.pdus[i];
        oss << "    {\n";
        oss << "      \"type\": \"" << JsonEscape(pdu.type) << "\",\n";
        oss << "      \"bits\": \"" << JsonEscape(pdu.bits) << "\",\n";
        oss << "      \"crc_ok\": " << (pdu.crc_ok ? "true" : "false") << ",\n";
        oss << "      \"field_consistency_score\": " << pdu.field_consistency_score << ",\n";
        oss << "      \"fields\": {";
        for (std::size_t j = 0; j < pdu.fields.size(); ++j) {
            oss << (j == 0 ? "" : ", ") << "\"" << JsonEscape(pdu.fields[j].name) << "\": \"" << JsonEscape(pdu.fields[j].value) << "\"";
        }
        oss << "}\n";
        oss << "    }" << (i + 1 < result.pdus.size() ? "," : "") << "\n";
    }
    oss << "  ],\n";
    oss << "  \"transaction_type\": \"" << JsonEscape(result.transaction_type) << "\",\n";
    oss << "  \"timing_ok\": " << (result.timing_ok ? "true" : "false") << ",\n";
    oss << "  \"timing_violations\": [";
    for (std::size_t i = 0; i < result.timing_violations.size(); ++i) {
        if (i != 0) oss << ", ";
        oss << "\"" << JsonEscape(result.timing_violations[i]) << "\"";
    }
    oss << "],\n";
    oss << "  \"event_trace\": [";
    for (std::size_t i = 0; i < result.event_trace.size(); ++i) {
        if (i != 0) oss << ", ";
        oss << "\"" << JsonEscape(result.event_trace[i]) << "\"";
    }
    oss << "],\n";
    oss << "  \"notes\": [";
    for (std::size_t i = 0; i < result.notes.size(); ++i) {
        if (i != 0) oss << ", ";
        oss << "\"" << JsonEscape(result.notes[i]) << "\"";
    }
    oss << "]\n";
    oss << "}";
    return oss.str();
}

}  // namespace hfexperimental

