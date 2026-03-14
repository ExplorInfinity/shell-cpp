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
        if (s[i] != prefix[i])
            return false;
    }

    return true;
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
        std::vector<std::string> &possibilities = cache;

        builtinCmdCompletion(possibilities, cmd);
        executableCompletion(possibilities, cmd);

        if (possibilities.empty())
            return false;

        if (possibilities.size() == 1) {
            cmd = possibilities[0];
            cmd += ' ';
            return true;
        }

        std::ranges::sort(possibilities, [](const std::string &a, const std::string &b) {
            if (a.size() == b.size())
                return a < b;
            return a.size() < b.size();
        });

        if (possibilities[0] == cmd) {
            cmd += ' ';
            return true;
        }

        const auto maxSize = possibilities.front().size();
        for (int i = static_cast<int>(possibilities.size()) - 1; i >= 0; --i) {
            if (possibilities[i].size() > maxSize)
                possibilities.pop_back();
        }

        if (possibilities.size() > 1)
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

        const fs::path cwd = fs::current_path();
        const auto [subdir, fileName] = parseFilePath(input);

        fs::path path = cwd;
        path += '/' + subdir;

        cache.clear();
        auto &possibilities = cache;


        for (const fs::directory_entry &entry : fs::directory_iterator(path)) {
            const bool isFile = entry.is_regular_file() && !isExecutable(entry);
            if ((isFile || entry.is_directory()) && hasPrefix(entry.path().filename().c_str(), fileName))
                possibilities.push_back(subdir + entry.path().filename().c_str() + (isFile ? ' ' : '/'));
        }

        std::ranges::sort(possibilities);

        if (possibilities.empty())
            return false;

        if (possibilities.size() == 1) {
            input = possibilities[0];
            return true;
        }

        const auto maxSize = removeSuffix(possibilities.front()).size();
        for (int i = static_cast<int>(possibilities.size()) - 1; i >= 0; --i) {
            if (removeSuffix(possibilities[i]).size() > maxSize)
                possibilities.pop_back();
        }

        if (possibilities.size() > 1)
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