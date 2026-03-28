#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace hfaudio {

namespace fs = std::filesystem;

struct AudioInputInfo {
    fs::path path;
    bool is_wav = false;
    std::uint64_t data_offset = 0;
    std::uint64_t data_bytes = 0;
    std::uint32_t sample_rate = 0;
    std::uint16_t channels = 1;
    std::uint16_t bits_per_sample = 16;
};

inline std::uint16_t ReadLE16(const unsigned char* bytes)
{
    return static_cast<std::uint16_t>(bytes[0]) |
        (static_cast<std::uint16_t>(bytes[1]) << 8);
}

inline std::uint32_t ReadLE32(const unsigned char* bytes)
{
    return static_cast<std::uint32_t>(bytes[0]) |
        (static_cast<std::uint32_t>(bytes[1]) << 8) |
        (static_cast<std::uint32_t>(bytes[2]) << 16) |
        (static_cast<std::uint32_t>(bytes[3]) << 24);
}

inline bool IsRiffWaveHeader(const unsigned char* header)
{
    return std::memcmp(header, "RIFF", 4) == 0 &&
        std::memcmp(header + 8, "WAVE", 4) == 0;
}

inline bool InspectPcm16Input(const fs::path& path, AudioInputInfo& info, std::string& error)
{
    info = AudioInputInfo{};
    info.path = path;

    std::error_code ec;
    const std::uint64_t file_size = fs::file_size(path, ec);
    if (ec) {
        error = "Cannot read file size: " + path.string();
        return false;
    }

    std::ifstream fin(path, std::ios::binary | std::ios::in);
    if (!fin.is_open()) {
        error = "Open input file failed: " + path.string();
        return false;
    }

    unsigned char header[12] = { 0 };
    fin.read(reinterpret_cast<char*>(header), sizeof(header));
    const std::streamsize header_bytes = fin.gcount();
    fin.clear();
    fin.seekg(0, std::ios::beg);

    if (header_bytes == static_cast<std::streamsize>(sizeof(header)) && IsRiffWaveHeader(header)) {
        info.is_wav = true;

        bool found_fmt = false;
        bool found_data = false;
        std::uint16_t audio_format = 0;

        fin.seekg(12, std::ios::beg);
        while (fin && static_cast<std::uint64_t>(fin.tellg()) + 8 <= file_size) {
            unsigned char chunk_header[8] = { 0 };
            fin.read(reinterpret_cast<char*>(chunk_header), sizeof(chunk_header));
            if (fin.gcount() != static_cast<std::streamsize>(sizeof(chunk_header))) {
                break;
            }

            const std::uint32_t chunk_size = ReadLE32(chunk_header + 4);
            const std::streamoff chunk_data_pos = fin.tellg();
            if (std::memcmp(chunk_header, "fmt ", 4) == 0) {
                const std::uint32_t bytes_to_read = std::min<std::uint32_t>(chunk_size, 16u);
                unsigned char fmt_bytes[16] = { 0 };
                fin.read(reinterpret_cast<char*>(fmt_bytes), bytes_to_read);
                if (fin.gcount() != static_cast<std::streamsize>(bytes_to_read) || bytes_to_read < 16) {
                    error = "Invalid WAV fmt chunk: " + path.string();
                    return false;
                }

                audio_format = ReadLE16(fmt_bytes + 0);
                info.channels = ReadLE16(fmt_bytes + 2);
                info.sample_rate = ReadLE32(fmt_bytes + 4);
                info.bits_per_sample = ReadLE16(fmt_bytes + 14);
                found_fmt = true;
            } else if (std::memcmp(chunk_header, "data", 4) == 0) {
                info.data_offset = static_cast<std::uint64_t>(chunk_data_pos);
                info.data_bytes = std::min<std::uint64_t>(
                    chunk_size,
                    file_size > info.data_offset ? file_size - info.data_offset : 0);
                found_data = true;
                break;
            }

            const std::uint64_t aligned_chunk_size = chunk_size + (chunk_size % 2u);
            fin.clear();
            fin.seekg(chunk_data_pos + static_cast<std::streamoff>(aligned_chunk_size), std::ios::beg);
        }

        if (!found_fmt || !found_data) {
            error = "Unsupported or incomplete WAV file: " + path.string();
            return false;
        }
        if (audio_format != 1) {
            error = "Only PCM WAV is supported: " + path.string();
            return false;
        }
        if (info.channels != 1 || info.bits_per_sample != 16) {
            error = "Only 16-bit mono PCM WAV is supported: " + path.string();
            return false;
        }
        return true;
    }

    info.data_offset = 0;
    info.data_bytes = file_size;
    return true;
}

inline bool OpenPcm16InputStream(
    const fs::path& path,
    std::ifstream& stream,
    AudioInputInfo& info,
    std::string& error)
{
    if (!InspectPcm16Input(path, info, error)) {
        return false;
    }

    stream.open(path, std::ios::binary | std::ios::in);
    if (!stream.is_open()) {
        error = "Open input file failed: " + path.string();
        return false;
    }

    stream.seekg(static_cast<std::streamoff>(info.data_offset), std::ios::beg);
    if (!stream.good()) {
        error = "Seek input file failed: " + path.string();
        return false;
    }

    return true;
}

inline std::uint64_t SampleCount(const AudioInputInfo& info)
{
    return info.data_bytes / sizeof(std::int16_t);
}

}  // namespace hfaudio

