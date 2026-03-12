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

namespace AutoComplete {

    inline std::vector<std::string> builtin_cmds = { "exit", "echo", "type", "pwd" };

    inline bool match(const std::string &cmd, const std::string &input) {
        if (cmd.size() < input.size()) return false;

        for (int i = 0; i < input.size(); i++) {
            if (cmd[i] != input[i])
                return false;
        }

        return true;
    }

    inline void builtinCmdCompletion(std::vector<std::string> &possibilities, const std::string &input) {
        for (const auto &cmd : builtin_cmds) {
            if (match(cmd, input))
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
                if (isExecutable(entry) && match(filename, input) &&
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

        std::ranges::sort(possibilities);
        const auto maxSize = possibilities.front().size();
        for (const auto &possibility : possibilities) {
            if (possibility.size() > maxSize)
                possibilities.pop_back();
        }

        if (possibilities.size() > 1)
            return false;

        input = possibilities[0];
        return true;
    }

}