#pragma once

#include <vector>
#include <string>

namespace AutoComplete {

    inline std::vector<std::string> builtin_cmds = { "exit", "echo", "type", "pwd" };

    inline bool cmdCompletion(std::string &input) {
        for (const auto &cmd : builtin_cmds) {
            if (cmd.size() < input.size()) continue;

            for (int i = 0; i < input.size(); i++) {
                if (cmd[i] != input[i])
                    return false;
            }

            input = cmd;
            input += ' ';
            return true;
        }

        return false;
    }

}