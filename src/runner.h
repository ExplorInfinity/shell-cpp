#pragma once

#include "config.h"
#include "executable.h"
#include "history.h"

using namespace Config;
using namespace Executable;
using namespace History;

namespace Runner {

    inline bool runCmd(std::vector<std::string> &args, const bool canSpawnChildProcess = true) {
        if (args.empty())
            return true;

        const auto &cmd = args[0];

        if (cmd == "exit")
            return false;

        if (cmd == "echo") {
            if (args.size() > 1) {
                for (int i = 1; i < args.size()-1; i++)
                    std::cout << args[i] << ' ';
                std::cout << args.back();
            }
            std::cout << std::endl;
        }

        else if (cmd == "type") {
            if (args.size() == 1) {
                std::cerr << "Type a valid string literal to search\n";
                return true;
            }

            const auto &search_cmd = args[1];

            if (std::ranges::find(builtin_cmds, search_cmd) != builtin_cmds.end())
                std::cout << search_cmd << " is a shell builtin" << std::endl;

            else {

                if (const auto path = doesExecutableExist(search_cmd)) {
                    std::cout << search_cmd << " is " << path.value() << std::endl;
                } else std::cout << search_cmd << ": not found" << std::endl;

            }
        }

        else if (cmd == "cd") {
            const char* path;

            if (args.empty() || args[1] == "~") {
                path = std::getenv("HOME");
            } else path = args[1].c_str();

            if (chdir(path) != 0)
                std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
        }

        else if (cmd == "pwd") {
            std::cout << std::filesystem::current_path().generic_string() << std::endl;
        }

        else if (cmd == "history") {
            runHistoryCmd(args);
        }

        else if (auto path = doesExecutableExist(cmd)) {

            std::vector<char*> c_args;
            c_args.reserve(args.size() + 1);

            for (auto &arg : args)
                c_args.push_back(arg.data());

            c_args.push_back(nullptr);

            if (!canSpawnChildProcess) {
                execvp(cmd.c_str(), c_args.data());
                return true;
            }

            const pid_t pid = fork();

            if (pid == 0) {
                // Child Process
                execvp(cmd.c_str(), c_args.data());
                _exit(0);
            }

            if (pid > 0) {
                // Parent Process
                wait(nullptr);
            }

            else {
                std::cerr << cmd << ": failed to run" << std::endl;
            }

        }

        else std::cout << cmd << ": command not found" << std::endl;

        return true;
    }

}