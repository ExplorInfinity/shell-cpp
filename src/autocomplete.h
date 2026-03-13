#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <unordered_map>

#include "executable.h"
using namespace Executable;

namespace fs = std::filesystem;

static std::vector<std::string> cache;

static bool isCompatible(const std::string &cmd, const std::string &input) {
    if (cmd.size() < input.size()) return false;

    for (int i = 0; i < input.size(); i++) {
        if (cmd[i] != input[i])
            return false;
    }

    return true;
}

namespace CommandAutoCompletion {

    inline std::vector<std::string> builtin_cmds = { "exit", "echo", "type", "pwd" };


    inline void builtinCmdCompletion(std::vector<std::string> &possibilities, const std::string &input) {
        for (const auto &cmd : builtin_cmds) {
            if (isCompatible(cmd, input))
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
                if (isExecutable(entry) && isCompatible(filename, input) &&
                    std::ranges::find(builtin_cmds, filename) == builtin_cmds.end())
                {
                    possibilities.push_back(entry.path().filename());
                }
            }
        }
    }


    inline bool cmdCompletion(std::string &input, const bool showOptions) {
        if (showOptions) {
            if (cache.empty())
                return false;

            std::cout << '\n';
            for (const auto &possibility : cache)
                std::cout << possibility << "  ";
            std::cout << "\n$ " << input;

            return true;
        }

        std::vector<std::string> &possibilities = cache;
        possibilities.clear();

        builtinCmdCompletion(possibilities, input);
        executableCompletion(possibilities, input);

        if (possibilities.empty())
            return false;

        if (possibilities.size() == 1) {
            input = possibilities[0];
            input += ' ';
            return true;
        }

        std::ranges::sort(possibilities, [](const std::string &a, const std::string &b) {
            if (a.size() == b.size())
                return a < b;
            return a.size() < b.size();
        });

        if (possibilities[0] == input) {
            input += ' ';
            return true;
        }

        const auto maxSize = possibilities.front().size();
        for (int i = static_cast<int>(possibilities.size()) -1; i >= 0; --i) {
            if (possibilities[i].size() > maxSize)
                possibilities.pop_back();
        }

        if (possibilities.size() > 1)
            return false;

        input = possibilities[0];
        return true;
    }

}

namespace FileAutoCompletion {
    inline bool fileCompletion(std::string &input) {
        const fs::path pwd = fs::current_path();
        for (const fs::directory_entry &entry : fs::directory_iterator(pwd)) {
            if (entry.is_regular_file() && !isExecutable(entry) && isCompatible(entry.path().filename(), input)) {
                input = entry.path().filename();
                input += ' ';
                return true;
            }
        }

        return false;
    }
}

namespace AutoComplete {
    using namespace CommandAutoCompletion;
    using namespace FileAutoCompletion;
}