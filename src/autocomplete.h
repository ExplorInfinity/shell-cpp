#pragma once

#include <utility>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "config.h"
#include "executable.h"
using namespace Config;
using namespace Executable;

namespace fs = std::filesystem;

// Custom Class to handle filepaths
struct FilePath {
    std::string filename;
    std::string extension;
    bool isDir = true;

    explicit FilePath(std::string filename): filename(std::move(filename)) {}
    explicit FilePath(std::string filename, std::string extension): FilePath(std::move(filename), std::move(extension), true){}
    explicit FilePath(std::string filename, std::string extension, const bool isDir): filename(std::move(filename)), extension(std::move(extension)), isDir(isDir) {}

    [[nodiscard]] bool is_directory() const {
        return isDir;
    }

    operator std::string() const {
        return filename + extension;
    }
};

inline std::ostream &operator<<(std::ostream &os, const FilePath &filePath) {
    return os << filePath.filename << filePath.extension;
}

// Static Global Cache
static std::vector<FilePath> cache;

// Utility functions
static bool hasPrefix(const std::string_view s, const std::string_view prefix) {
    if (s.size() < prefix.size()) return false;

    for (int i = 0; i < prefix.size(); i++) {
        if (tolower(s[i]) != tolower(prefix[i]))
            return false;
    }

    return true;
}

inline std::pair<std::string, std::string> parseFilePath(const std::string &filePath) {
    for (int i = static_cast<int>(filePath.size()) - 1; i >= 0; --i) {
        if (filePath[i] == '/')
            return { filePath.substr(0, i + 1), filePath.substr(i + 1) };
    }

    return { "", filePath };
}

// LCP (Longest Common Prefix) Logic
static std::string getLCP(std::vector<FilePath> &possibilities) {
    if (possibilities.empty())
        return "";

    std::ranges::sort(possibilities, [](const FilePath &a, const FilePath &b) {
        return static_cast<std::string>(a) < static_cast<std::string>(b);
    });

    const auto first = static_cast<std::string>(possibilities.front()),
                last = static_cast<std::string>(possibilities.back());

    std::size_t i = 0;
    while ( i < first.size() && i < last.size() &&
            tolower(first[i]) == tolower(last[i])) ++i;

    return first.substr(0, i);
}

namespace CommandAutoCompletion {

    inline void builtinCmdCompletion(std::vector<FilePath> &possibilities, const std::string &input) {
        for (const auto &cmd : builtin_cmds) {
            if (hasPrefix(cmd, input))
                possibilities.emplace_back(cmd);
        }
    }

    inline void executableCompletion(std::vector<FilePath> &possibilities, const std::string &input) {

        std::string dir;
        std::stringstream ss(pathEnv);

        while (std::getline(ss, dir, ':')) {
            std::error_code ec;
            for (const auto &entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
                if (!entry.is_regular_file(ec) || ec) continue;

                const std::string filename = entry.path().filename();
                if (isExecutable(entry) && hasPrefix(filename, input) &&
                    std::ranges::find(builtin_cmds, filename) == builtin_cmds.end())
                {
                    possibilities.emplace_back(entry.path().filename());
                }
            }
        }
    }


    inline bool cmdCompletion(std::string &cmd, const std::string &cmdLine, const bool showOptions) {
        if (showOptions) {
            if (cache.empty())
                return false;

            std::cout << '\n';
            for (const auto &possibility : cache)
                std::cout << possibility << "  ";
            std::cout << "\n$ " << cmdLine;

            return true;
        }

        cache.clear();
        auto &possibilities = cache;

        builtinCmdCompletion(possibilities, cmd);
        executableCompletion(possibilities, cmd);

        if (possibilities.empty())
            return false;

        if (possibilities.size() == 1) {
            cmd = possibilities[0];
            cmd += ' ';
            return true;
        }

        const auto lcp = getLCP(possibilities);
        if (lcp.size() == cmd.size())
            return false;

        cmd = lcp;
        return true;
    }

}

namespace FileAutoCompletion {

    inline bool fileCompletion(std::string &input, const std::string &cmdLine, const bool showOptions) {
        if (showOptions) {
            if(cache.empty())
                return false;

            std::cout << '\n';
            for (const auto &s : cache)
                std::cout << s << "  ";

            std::cout << "\n$ " << cmdLine;
        }

        const auto [subdir, fileName] = parseFilePath(input);

        if (!subdir.empty() && !fs::exists(subdir))
            return false;

        const fs::path cwd = fs::current_path();
        fs::path path = cwd;
        if (!subdir.empty()) path += '/' + subdir;

        cache.clear();
        auto &possibilities = cache;

        for (const fs::directory_entry &entry : fs::directory_iterator(path)) {
            const bool isFile = entry.is_regular_file() && !isExecutable(entry);
            if ((isFile || entry.is_directory()) &&
                hasPrefix(entry.path().filename().c_str(), fileName))
            {
                possibilities.emplace_back(subdir + entry.path().stem().string(), (isFile ? entry.path().extension() : "/"), !isFile);
            }
        }

        if (possibilities.empty())
            return false;

        if (possibilities.size() == 1) {
            input = possibilities[0];
            if (!possibilities[0].is_directory()) input += ' ';
            return true;
        }

        const auto lcp = getLCP(possibilities);
        if (lcp.size() == input.size())
            return false;

        input = lcp;
        return true;
    }
}

namespace AutoComplete {
    using namespace CommandAutoCompletion;
    using namespace FileAutoCompletion;
}