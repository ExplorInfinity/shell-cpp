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


    inline bool cmdCompletion(std::string &input, const int index) {
        if (index > 0 && cache.size() > index) {
            input = cache[index];
            input += ' ';
            return true;
        }

        if (index > 0) return false;

        std::vector<std::string> &possibilities = cache;
        possibilities.clear();

        builtinCmdCompletion(possibilities, input);
        executableCompletion(possibilities, input);

        std::ranges::sort(possibilities);

        if (possibilities.empty())
            return false;

        input = possibilities[0];
        input += ' ';
        return true;
    }

}