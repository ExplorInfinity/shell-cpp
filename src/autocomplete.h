#pragma once

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

static std::vector<std::string> cache;

static bool hasPrefix(const std::string_view s, const std::string_view prefix) {
    if (s.size() < prefix.size()) return false;

    for (int i = 0; i < prefix.size(); i++) {
        if (tolower(s[i]) != tolower(prefix[i]))
            return false;
    }

    return true;
}

static void lcpSort(std::vector<std::string> &possibilities) {
    std::ranges::sort(possibilities, [](const std::string &a, const std::string &b) {
        if (a.size() == b.size())
            return a < b;
        return a.size() < b.size();
    });
}

namespace CommandAutoCompletion {

    inline void builtinCmdCompletion(std::vector<std::string> &possibilities, const std::string &input) {
        for (const auto &cmd : builtin_cmds) {
            if (hasPrefix(cmd, input))
                possibilities.push_back(cmd);
        }
    }

    inline void executableCompletion(std::vector<std::string> &possibilities, const std::string &input) {

        std::string dir;
        std::stringstream ss(pathEnv);

        while (std::getline(ss, dir, ':')) {
            std::error_code ec;
            for (const auto &entry : fs::recursive_directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
                if (!entry.is_regular_file(ec) || ec) continue;

                const std::string filename = entry.path().filename();
                if (isExecutable(entry) && hasPrefix(filename, input) &&
                    std::ranges::find(builtin_cmds, filename) == builtin_cmds.end())
                {
                    possibilities.push_back(entry.path().filename());
                }
            }
        }
    }


    inline bool cmdCompletion(std::string &cmd, std::string &input, const bool showOptions) {
        if (showOptions) {
            if (cache.empty())
                return false;

            std::cout << '\n';
            for (const auto &possibility : cache)
                std::cout << possibility << "  ";
            std::cout << "\n$ " << input;

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

        lcpSort(possibilities);

        if (possibilities[0] == cmd) {
            cmd += ' ';
            return true;
        }

        int size;
        const auto minSize = possibilities.front().size();
        for (size = static_cast<int>(possibilities.size()) - 1; size >= 0; --size) {
            if (possibilities[size].size() > minSize)
                break;
        }

        if (size > 0)
            return false;

        cmd = possibilities[0];
        return true;
    }

}

namespace FileAutoCompletion {
    inline std::pair<std::string, std::string> parseFilePath(const std::string &filePath) {
        for (int i = static_cast<int>(filePath.size()) - 1; i >= 0; --i) {
            if (filePath[i] == '/')
                return { filePath.substr(0, i + 1), filePath.substr(i + 1) };
        }

        return { "", filePath };
    }

    inline std::string removeSuffix(const std::string &s) {
        if (s.back() == '/')
            return s.substr(0, s.size()-1);

        return s.substr(0, s.rfind('.'));
    }

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
            if ((isFile || entry.is_directory()) && hasPrefix(entry.path().filename().c_str(), fileName))
                possibilities.push_back(subdir + entry.path().filename().c_str() + (isFile ? ' ' : '/'));
        }

        if (possibilities.empty())
            return false;

        if (possibilities.size() == 1) {
            input = possibilities[0];
            return true;
        }

        lcpSort(possibilities);

        int size;
        const auto minSize = removeSuffix(possibilities.front()).size();
        for (size = static_cast<int>(possibilities.size()) - 1; size >= 0; --size) {
            if (removeSuffix(possibilities[size]).size() == minSize)
                break;
        }

        if (size > 0)
            return false;

        possibilities[0].pop_back();
        input = possibilities[0];
        return true;
    }
}

namespace AutoComplete {
    using namespace CommandAutoCompletion;
    using namespace FileAutoCompletion;
}