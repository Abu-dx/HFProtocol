#pragma once

#include <string>

namespace hfexperimental {
namespace g3ale {

struct CrcDefinition
{
    const char* name;
    const char* polynomial_bits;
    int width;
    const char* input_order;
    const char* output_order;
};

inline const CrcDefinition& Crc4Definition()
{
    static const CrcDefinition kDef = {
        "crc4",
        "11011",   // provisional until Appendix C.5.2.2.6 is verified page-by-page in code comments/doc
        4,
        "transmitted-order msb-first",
        "msb-first"
    };
    return kDef;
}

inline const CrcDefinition& Crc8Definition()
{
    static const CrcDefinition kDef = {
        "crc8",
        "110011011",   // provisional until Appendix C.5.2.2.6 is verified page-by-page in code comments/doc
        8,
        "transmitted-order msb-first",
        "msb-first"
    };
    return kDef;
}

inline std::string ComputeCrcBits(const std::string& nonCrcBits, const std::string& generator)
{
    const std::size_t crcWidth = generator.size() - 1u;
    std::string working = nonCrcBits;
    working.append(crcWidth, '0');

    for (std::size_t i = 0; i < nonCrcBits.size(); ++i) {
        if (working[i] != '1') {
            continue;
        }
        for (std::size_t j = 0; j < generator.size(); ++j) {
            working[i + j] = (working[i + j] == generator[j]) ? '0' : '1';
        }
    }

    return working.substr(working.size() - crcWidth, crcWidth);
}

inline bool CheckCrcBits(const std::string& nonCrcBits, const std::string& crcBits, const CrcDefinition& definition, std::string& expected)
{
    expected = ComputeCrcBits(nonCrcBits, definition.polynomial_bits);
    return expected == crcBits;
}

}  // namespace g3ale
}  // namespace hfexperimental
