#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "G3AleTypes.h"

namespace hfexperimental {
namespace g3ale {

struct TimingAnalysis
{
    std::string transaction_type = "unknown";
    bool timing_ok = false;
    double timing_score = 0.0;
    double transaction_score = 0.0;
    std::vector<std::string> violations;
    std::vector<std::string> event_trace;
};

inline TimingAnalysis AnalyzeTiming(const std::vector<BurstInfo>& bursts, const std::vector<PduInfo>& pdus)
{
    TimingAnalysis analysis;
    if (pdus.empty()) {
        analysis.violations.push_back("no_valid_le_pdu");
        return analysis;
    }

    const double syncSlotSec = 0.8;
    const double syncToleranceSec = 0.18;
    const double syncDwellSec = 4.0;
    const double asyncListenBeforeTxSec = 2.0;
    const double asyncM = 1.3;
    const double asyncScanWindowSec = asyncListenBeforeTxSec * asyncM * 2.0;
    const double asyncHandshakeMaxSec = 3.0;

    for (std::size_t i = 0; i < pdus.size(); ++i) {
        analysis.event_trace.push_back(std::string("pdu:") + pdus[i].type);
    }

    if (pdus.size() >= 3 && pdus[0].type == "LE_Scanning_Call" && pdus[1].type == "LE_Call" && pdus[2].type == "LE_Handshake")
    {
        analysis.event_trace.push_back("async_sequence:scan_call->call->handshake");
        const double delta1 = bursts.size() >= 2 ? bursts[1].begin_time - bursts[0].begin_time : 0.0;
        const double delta2 = bursts.size() >= 3 ? bursts[2].begin_time - bursts[1].begin_time : 0.0;
        if (delta1 >= 0.0 && delta1 <= asyncScanWindowSec && delta2 >= 0.0 && delta2 <= asyncHandshakeMaxSec) {
            analysis.transaction_type = "async_scan_call";
            analysis.timing_ok = true;
            analysis.timing_score = 1.0;
            analysis.transaction_score = 1.0;
            return analysis;
        }
        if (delta1 > asyncScanWindowSec) {
            analysis.violations.push_back("scan_call_followup_timing_invalid");
        }
        if (delta2 > asyncHandshakeMaxSec) {
            analysis.violations.push_back("handshake_timing_out_of_range");
        }
    }

    if (pdus.size() >= 2 && pdus[0].type == "LE_Call" && pdus[1].type == "LE_Handshake")
    {
        analysis.event_trace.push_back("async_sequence:call->handshake");
        const double delta = bursts.size() >= 2 ? bursts[1].begin_time - bursts[0].begin_time : 0.0;
        if (delta >= 0.0 && delta <= asyncHandshakeMaxSec) {
            analysis.transaction_type = "async_unicast_call";
            analysis.timing_ok = true;
            analysis.timing_score = 0.90;
            analysis.transaction_score = 0.90;
            return analysis;
        }
        analysis.violations.push_back("unicast_handshake_timing_invalid");
    }

    if (pdus.size() >= 2 && bursts.size() >= 2)
    {
        bool slotLike = true;
        double dwellSpan = 0.0;
        for (std::size_t i = 1; i < pdus.size() && i < bursts.size(); ++i) {
            const double delta = bursts[i].begin_time - bursts[i - 1].begin_time;
            if (std::fabs(delta - syncSlotSec) > syncToleranceSec) {
                slotLike = false;
                analysis.violations.push_back("sync_slot_spacing_invalid");
                break;
            }
        }
        if (!bursts.empty()) {
            dwellSpan = bursts[std::min<std::size_t>(bursts.size() - 1, 4)].begin_time - bursts[0].begin_time;
        }
        if (slotLike) {
            analysis.event_trace.push_back("sync_sequence:slot-aligned");
            analysis.transaction_type = "sync_call";
            analysis.timing_ok = true;
            analysis.timing_score = std::fabs(dwellSpan - syncDwellSec) <= 0.6 ? 0.95 : 0.80;
            analysis.transaction_score = 0.85;
            return analysis;
        }
    }

    if (!pdus.empty() && pdus.back().type == "LE_Handshake") {
        for (std::size_t i = 0; i < pdus.back().fields.size(); ++i) {
            if (pdus.back().fields[i].name == "command") {
                if (pdus.back().fields[i].value == "link_release") {
                    analysis.transaction_type = "link_release";
                    analysis.transaction_score = 0.65;
                    analysis.event_trace.push_back("command:link_release");
                }
                if (pdus.back().fields[i].value == "return_to_scan") {
                    analysis.transaction_type = "return_to_scan";
                    analysis.transaction_score = 0.65;
                    analysis.event_trace.push_back("command:return_to_scan");
                }
            }
        }
    }

    if (analysis.transaction_type == "unknown") {
        analysis.violations.push_back("no_supported_transaction_pattern");
    }
    return analysis;
}

}  // namespace g3ale
}  // namespace hfexperimental
