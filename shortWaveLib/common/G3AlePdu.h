#pragma once

#include <string>

#include "G3AleTypes.h"
#include "G3AleCrc.h"

namespace hfexperimental {
namespace g3ale {

inline const char* WalshToDeinterleaveBitOrdering()
{
    return "walsh nibble order = b3 b2 b1 b0 from pIndex; each Walsh decision contributes 4 bits in that order";
}

inline const char* DeinterleaveBitOrdering()
{
    return "deinterleaver input order follows 13 Walsh symbols x 4 bits; output is row/column restored bitstream, left-to-right";
}

inline const char* ViterbiBitOrdering()
{
    return "viterbi output is consumed in decoded trellis order and packed msb-first into the 26-bit LE_PDU candidate";
}

inline const char* FigureC17BitOrdering()
{
    return "Figure C-17 field extraction uses msb-first, left-to-right transmitted bit order on the 26-bit candidate";
}

inline std::string CombinedBitOrderingDescription()
{
    return std::string(WalshToDeinterleaveBitOrdering()) + "; " +
        DeinterleaveBitOrdering() + "; " +
        ViterbiBitOrdering() + "; " +
        FigureC17BitOrdering();
}

inline std::string CallTypeLabel(std::uint32_t callType)
{
    switch (callType)
    {
    case 0: return "packet_data";
    case 1: return "long_packet_data";
    case 2: return "modem_circuit";
    case 3: return "voice_circuit";
    case 4: return "high_priority_packet_data";
    case 5: return "high_priority_long_packet_data";
    case 6: return "high_priority_modem_circuit";
    case 7: return "high_priority_voice_circuit";
    default: return "unknown";
    }
}

inline std::string HandshakeCommandLabel(std::uint32_t command)
{
    switch (command)
    {
    case 0: return "continue_handshake";
    case 1: return "commence_traffic_setup";
    case 2: return "channel_test";
    case 3: return "link_release";
    case 4: return "traffic_setup_confirm";
    case 5: return "return_to_scan";
    case 6: return "data";
    case 7: return "abort_handshake";
    default: return "unknown";
    }
}

inline double NormalizeFieldConsistency(double score)
{
    return Clamp01(score);
}

inline bool ParsePduCandidate(const std::string& bits, int candidateRank, PduInfo& outPdu)
{
    outPdu = PduInfo{};
    outPdu.bits = bits;
    outPdu.bit_ordering = CombinedBitOrderingDescription();
    outPdu.candidate_rank = candidateRank;

    if (bits.size() != 26u) {
        outPdu.type = "unknown_pdu";
        outPdu.field_consistency_score = 0.0;
        outPdu.fields.push_back({ "reject_reason", "not_26bit_candidate" });
        return false;
    }

    const std::string prefix = bits.substr(0, 2);
    if (prefix == "10")
    {
        outPdu.type = "LE_Call";
        const std::uint32_t callType = BitsToUInt(bits, 2, 3);
        const std::uint32_t callerMember = BitsToUInt(bits, 5, 6);
        const std::uint32_t callerGroup = BitsToUInt(bits, 11, 5);
        const std::uint32_t calledMember = BitsToUInt(bits, 16, 6);
        outPdu.fields.push_back({ "call_type_code", std::to_string(callType) });
        outPdu.fields.push_back({ "call_type", CallTypeLabel(callType) });
        outPdu.fields.push_back({ "caller_member", std::to_string(callerMember) });
        outPdu.fields.push_back({ "caller_group", std::to_string(callerGroup) });
        outPdu.fields.push_back({ "called_member", std::to_string(calledMember) });
        std::string expectedCrc;
        outPdu.crc_ok = CheckCrcBits(bits.substr(0, 22), bits.substr(22, 4), Crc4Definition(), expectedCrc);
        outPdu.fields.push_back({ "crc_expected", expectedCrc });
        outPdu.fields.push_back({ "crc_actual", bits.substr(22, 4) });
        outPdu.field_consistency_score = NormalizeFieldConsistency(0.55 + (callType <= 7 ? 0.15 : 0.0) + (outPdu.crc_ok ? 0.30 : 0.0));
        return true;
    }

    if (prefix == "00")
    {
        outPdu.type = "LE_Handshake";
        const std::uint32_t linkId = BitsToUInt(bits, 2, 6);
        const std::uint32_t command = BitsToUInt(bits, 8, 3);
        const std::uint32_t argument = BitsToUInt(bits, 11, 7);
        outPdu.fields.push_back({ "link_id", std::to_string(linkId) });
        outPdu.fields.push_back({ "command_code", std::to_string(command) });
        outPdu.fields.push_back({ "command", HandshakeCommandLabel(command) });
        outPdu.fields.push_back({ "argument", std::to_string(argument) });
        std::string expectedCrc;
        outPdu.crc_ok = CheckCrcBits(bits.substr(0, 18), bits.substr(18, 8), Crc8Definition(), expectedCrc);
        outPdu.fields.push_back({ "crc_expected", expectedCrc });
        outPdu.fields.push_back({ "crc_actual", bits.substr(18, 8) });
        outPdu.field_consistency_score = NormalizeFieldConsistency(0.60 + (command <= 7 ? 0.10 : 0.0) + (outPdu.crc_ok ? 0.30 : 0.0));
        return true;
    }

    if (prefix == "01")
    {
        if (bits.substr(2, 5) == "11111")
        {
            outPdu.type = "LE_Scanning_Call";
            outPdu.fields.push_back({ "called_station_address", std::to_string(BitsToUInt(bits, 7, 11)) });
            std::string expectedCrc;
            outPdu.crc_ok = CheckCrcBits(bits.substr(0, 18), bits.substr(18, 8), Crc8Definition(), expectedCrc);
            outPdu.fields.push_back({ "crc_expected", expectedCrc });
            outPdu.fields.push_back({ "crc_actual", bits.substr(18, 8) });
            outPdu.field_consistency_score = NormalizeFieldConsistency(0.70 + (outPdu.crc_ok ? 0.30 : 0.0));
            return true;
        }
        if (bits.substr(2, 3) == "110")
        {
            outPdu.type = "LE_Broadcast";
            const std::uint32_t callType = BitsToUInt(bits, 12, 3);
            outPdu.fields.push_back({ "channel", std::to_string(BitsToUInt(bits, 5, 7)) });
            outPdu.fields.push_back({ "call_type_code", std::to_string(callType) });
            outPdu.fields.push_back({ "call_type", CallTypeLabel(callType) });
            outPdu.fields.push_back({ "countdown", std::to_string(BitsToUInt(bits, 15, 3)) });
            std::string expectedCrc;
            outPdu.crc_ok = CheckCrcBits(bits.substr(0, 18), bits.substr(18, 8), Crc8Definition(), expectedCrc);
            outPdu.fields.push_back({ "crc_expected", expectedCrc });
            outPdu.fields.push_back({ "crc_actual", bits.substr(18, 8) });
            outPdu.field_consistency_score = NormalizeFieldConsistency(0.60 + (callType <= 7 ? 0.10 : 0.0) + (outPdu.crc_ok ? 0.30 : 0.0));
            return true;
        }
    }

    outPdu.type = "unknown_pdu";
    outPdu.crc_ok = false;
    outPdu.field_consistency_score = 0.20;
    outPdu.fields.push_back({ "reject_reason", "bit_order_unknown" });
    return true;
}

inline double ScorePdu(const PduInfo& pdu)
{
    double score = 0.0;
    if (pdu.type == "LE_Scanning_Call" || pdu.type == "LE_Call" || pdu.type == "LE_Handshake" || pdu.type == "LE_Broadcast") {
        score += 0.55;
    }
    else if (pdu.type == "unknown_pdu") {
        score += 0.10;
    }
    score += pdu.field_consistency_score * 0.30;
    if (pdu.crc_ok) {
        score += 0.15;
    }
    return Clamp01(score);
}

}  // namespace g3ale
}  // namespace hfexperimental
