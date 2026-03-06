#include <iostream>
#include <string>
#include <vector>
#include <ranges>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

#include "parser.h"
using namespace Parser;

namespace fs = std::filesystem;
const char *pathEnv = std::getenv("PATH");

std::optional<std::string> doesExecutableExist(const std::string &cmd) {
    std::string dir;
    std::stringstream ss(pathEnv);

    while (std::getline(ss, dir, ':')) {
        std::string candidate = dir + '/' + cmd;
        if (fs::exists(candidate)) {
            const fs::perms perms = fs::status(candidate).permissions();

            if ((perms & fs::perms::owner_exec) == fs::perms::none)
                continue;

            return candidate;
        }
    }

    return {};
}

std::vector<std::string> builtin_cmds = { "exit", "echo", "type", "pwd" };

int main() {
    if (!pathEnv) return -1;

    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        std::cout << "$ ";
        std::string cmdLine;

        std::getline(std::cin, cmdLine);
        auto cmdTokens = parseString(cmdLine);
        const auto &cmd = cmdTokens[0];

        if (cmd == "exit") break;

        if (cmd == "echo") {
            for (int i = 1; i < cmdTokens.size(); i++)
                std::cout << cmdTokens[i] << ' ';
            std::cout << std::endl;
        }

        else if (cmd == "type") {
            const auto &search_cmd = cmdTokens[1];

            if (std::ranges::find(builtin_cmds, search_cmd) != builtin_cmds.end())
                std::cout << cmd << " is a shell builtin" << std::endl;

            else {

                if (auto path = doesExecutableExist(search_cmd)) {
                    std::cout << cmd << " is " << path.value() << std::endl;
                } else std::cout << cmd << ": not found" << std::endl;

            }
        }

        else if (cmd == "cd") {
            const char* path;

            if (cmdTokens.empty() || cmdTokens[1] == "~") {
                path = std::getenv("HOME");
            } else path = cmdTokens[1].c_str();

            if (chdir(path) != 0)
                std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
        }

        else if (cmd == "pwd") {
            std::cout << std::filesystem::current_path().generic_string() << std::endl;
        }

        else if (auto path = doesExecutableExist(cmd)) {

            std::vector<char*> args;
            args.reserve(cmdTokens.size() + 1);

            std::string arg;
            args.push_back(const_cast<char*>(cmd.c_str()));
            for (int i = 1; i < cmdTokens.size(); i++)
                args.push_back(cmdTokens[i].data());

            args.push_back(nullptr);

            pid_t pid = fork();

            if (pid == 0) {
                // Child Process
                execvp(cmd.c_str(), args.data());
                return 0;
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
    }
}
