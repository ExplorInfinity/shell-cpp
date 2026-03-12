#pragma once

#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
extern const char *pathEnv;

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

    inline bool builtinCmdCompletion(std::string &input) {
        for (const auto &cmd : builtin_cmds) {
            if (match(cmd, input)) {
                input = cmd;
                input += ' ';
                return true;
            }
        }

        return false;
    }

    inline bool isExecutable(const fs::directory_entry &entry) {
        const auto perms = entry.status().permissions();
        return (perms & fs::perms::owner_exec) != fs::perms::none;
    }

    inline bool executableCompletion(std::string &input) {

        std::string dir;
        std::stringstream ss(pathEnv);

        while (std::getline(ss, dir, ':')) {
            for (const auto &entry : fs::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file() && isExecutable(entry) && match(entry.path().filename(), input)) {
                    input = entry.path().filename();
                    input += ' ';
                    return true;
                }
            }
        }


        return false;
    }


    inline bool cmdCompletion(std::string &input) {
        if (builtinCmdCompletion(input) || executableCompletion(input))
            return true;

        return false;
    }

}