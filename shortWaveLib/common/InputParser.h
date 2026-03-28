#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace hfinput {

namespace fs = std::filesystem;

struct InputOptions {
    std::string raw_input;
    fs::path resolved_input;
    std::string protocol = "auto";
    std::optional<int> data_rate;
    std::optional<double> frequency;
    std::vector<fs::path> search_dirs;
    bool show_help = false;
    bool interactive = false;
};

class InputParser {
public:
    explicit InputParser(std::vector<fs::path> extra_default_dirs = {})
        : default_search_dirs_(BuildDefaultSearchDirs(extra_default_dirs)) {}

    bool Parse(int argc, char* argv[], InputOptions& out, std::string& error) const {
        out = InputOptions{};
        out.search_dirs = default_search_dirs_;
        out.interactive = (argc <= 1);

        std::string input_token;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i] ? argv[i] : "";

            if (arg == "-h" || arg == "--help") {
                out.show_help = true;
                return true;
            }

            if (StartsWith(arg, "--input=")) {
                input_token = arg.substr(std::string("--input=").size());
                continue;
            }
            if (arg == "-i" || arg == "--input") {
                if (i + 1 >= argc) {
                    error = "Missing value for --input.";
                    return false;
                }
                input_token = argv[++i];
                continue;
            }

            if (StartsWith(arg, "--protocol=")) {
                out.protocol = arg.substr(std::string("--protocol=").size());
                continue;
            }
            if (arg == "-p" || arg == "--protocol") {
                if (i + 1 >= argc) {
                    error = "Missing value for --protocol.";
                    return false;
                }
                out.protocol = argv[++i];
                continue;
            }

            if (StartsWith(arg, "--search-dir=")) {
                out.search_dirs.emplace_back(arg.substr(std::string("--search-dir=").size()));
                continue;
            }
            if (arg == "-I" || arg == "--search-dir") {
                if (i + 1 >= argc) {
                    error = "Missing value for --search-dir.";
                    return false;
                }
                out.search_dirs.emplace_back(argv[++i]);
                continue;
            }

            if (StartsWith(arg, "--data-rate=")) {
                if (!TryParseInt(arg.substr(std::string("--data-rate=").size()), out.data_rate)) {
                    error = "Invalid integer for --data-rate.";
                    return false;
                }
                continue;
            }
            if (arg == "--data-rate") {
                if (i + 1 >= argc || !TryParseInt(argv[++i], out.data_rate)) {
                    error = "Invalid integer for --data-rate.";
                    return false;
                }
                continue;
            }

            if (StartsWith(arg, "--frequency=")) {
                if (!TryParseDouble(arg.substr(std::string("--frequency=").size()), out.frequency)) {
                    error = "Invalid number for --frequency.";
                    return false;
                }
                continue;
            }
            if (arg == "--frequency") {
                if (i + 1 >= argc || !TryParseDouble(argv[++i], out.frequency)) {
                    error = "Invalid number for --frequency.";
                    return false;
                }
                continue;
            }

            if (!arg.empty() && arg.front() == '-') {
                error = "Unknown option: " + arg;
                return false;
            }

            if (input_token.empty()) {
                input_token = arg;
            } else {
                error = "Multiple input paths provided. Keep a single path argument.";
                return false;
            }
        }

        out.protocol = ToLower(Trim(StripQuotes(out.protocol)));
        if (out.protocol.empty()) {
            out.protocol = "auto";
        }

        out.search_dirs = NormalizeSearchDirs(out.search_dirs);
        if (!input_token.empty()) {
            out.raw_input = Trim(StripQuotes(input_token));
            if (!ResolveInputPath(out.raw_input, out.search_dirs, out.resolved_input, error)) {
                return false;
            }
        }

        return true;
    }

    static bool ResolveInputPath(
        const std::string& user_input,
        const std::vector<fs::path>& search_dirs,
        fs::path& resolved_path,
        std::string& error) {
        std::string cleaned = Trim(StripQuotes(user_input));
        if (cleaned.empty()) {
            error = "Input path is empty.";
            return false;
        }

        fs::path input_path(cleaned);
        input_path = input_path.lexically_normal();
        input_path.make_preferred();

        std::vector<fs::path> candidates;
        if (input_path.has_parent_path() || input_path.is_absolute() ||
            input_path.has_root_name() || input_path.has_root_directory()) {
            candidates.push_back(input_path);
        } else {
            candidates.push_back(input_path);
            for (const auto& dir : NormalizeSearchDirs(search_dirs)) {
                candidates.push_back(dir / input_path);
            }
        }

        std::error_code ec;
        for (auto candidate : candidates) {
            candidate = candidate.lexically_normal();
            candidate.make_preferred();
            const auto abs_candidate = fs::absolute(candidate, ec);
            if (ec) {
                ec.clear();
                continue;
            }
            if (fs::exists(abs_candidate, ec) && !ec && fs::is_regular_file(abs_candidate, ec) && !ec) {
                resolved_path = abs_candidate.lexically_normal();
                resolved_path.make_preferred();
                return true;
            }
            ec.clear();
        }

        error = "Cannot find input file: " + cleaned;
        return false;
    }

    static std::vector<fs::path> BuildDefaultSearchDirs(const std::vector<fs::path>& extra_dirs = {}) {
        std::vector<fs::path> dirs;
        std::error_code ec;
        const fs::path cwd = fs::current_path(ec);
        if (!ec) {
            dirs.push_back(cwd);
            dirs.push_back(cwd / "test_data");
            dirs.push_back(cwd.parent_path() / "test_data");
            dirs.push_back(cwd / "shortWave-Package" / "data");
            dirs.push_back(cwd.parent_path() / "shortWave-Package" / "data");
        }
        dirs.push_back(fs::path("D:/0_kunshan/短波数据db"));
        dirs.insert(dirs.end(), extra_dirs.begin(), extra_dirs.end());
        return NormalizeSearchDirs(dirs);
    }

    static bool ReadLine(const std::string& prompt, std::string& value) {
        std::cout << prompt;
        if (!std::getline(std::cin, value)) {
            return false;
        }
        value = Trim(value);
        return true;
    }

    static std::string ToLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }

    static std::string Trim(const std::string& value) {
        if (value.empty()) {
            return {};
        }
        size_t begin = 0;
        while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
            ++begin;
        }
        size_t end = value.size();
        while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
            --end;
        }
        return value.substr(begin, end - begin);
    }

    static std::string StripQuotes(const std::string& value) {
        if (value.size() >= 2) {
            const char first = value.front();
            const char last = value.back();
            if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
                return value.substr(1, value.size() - 2);
            }
        }
        return value;
    }

private:
    static bool StartsWith(const std::string& value, const std::string& prefix) {
        return value.size() >= prefix.size() &&
            value.compare(0, prefix.size(), prefix) == 0;
    }

    static std::vector<fs::path> NormalizeSearchDirs(const std::vector<fs::path>& dirs) {
        std::vector<fs::path> normalized;
        for (const auto& dir : dirs) {
            if (dir.empty()) {
                continue;
            }
            std::error_code ec;
            fs::path abs = fs::absolute(dir, ec);
            if (ec) {
                abs = dir;
                ec.clear();
            }
            abs = abs.lexically_normal();
            abs.make_preferred();

            const auto key = ToLower(abs.generic_string());
            bool exists = false;
            for (const auto& already : normalized) {
                if (ToLower(already.generic_string()) == key) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                normalized.push_back(abs);
            }
        }
        return normalized;
    }

    static bool TryParseInt(const std::string& text, std::optional<int>& out_value) {
        try {
            size_t pos = 0;
            int value = std::stoi(text, &pos);
            if (pos != text.size()) {
                return false;
            }
            out_value = value;
            return true;
        } catch (...) {
            return false;
        }
    }

    static bool TryParseDouble(const std::string& text, std::optional<double>& out_value) {
        try {
            size_t pos = 0;
            double value = std::stod(text, &pos);
            if (pos != text.size()) {
                return false;
            }
            out_value = value;
            return true;
        } catch (...) {
            return false;
        }
    }

    std::vector<fs::path> default_search_dirs_;
};

}  // namespace hfinput
