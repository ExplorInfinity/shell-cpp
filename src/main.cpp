#include <iostream>
#include <string>
#include <vector>
#include <ranges>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <fcntl.h>
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
        std::string candidate = dir + '/';
        candidate += cmd;

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
        auto [cmd, args, outfile, append, redirection] = parseString(cmdLine);

        int saved = -1;
        if (redirection != REDIRECTION::NONE) {
            saved = dup(redirection);
            int fd = open(outfile.c_str(), O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
            dup2(fd, redirection);
            close(fd);
        }

        if (cmd == "exit") break;

        if (cmd == "echo") {
            for (const auto &token : args)
                std::cout << token << ' ';
            std::cout << std::endl;
        }

        else if (cmd == "type") {
            const auto &search_cmd = args[0];

            if (std::ranges::find(builtin_cmds, search_cmd) != builtin_cmds.end())
                std::cout << search_cmd << " is a shell builtin" << std::endl;

            else {

                if (auto path = doesExecutableExist(search_cmd)) {
                    std::cout << search_cmd << " is " << path.value() << std::endl;
                } else std::cout << search_cmd << ": not found" << std::endl;

            }
        }

        else if (cmd == "cd") {
            const char* path;

            if (args.empty() || args[0] == "~") {
                path = std::getenv("HOME");
            } else path = args[0].c_str();

            if (chdir(path) != 0)
                std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
        }

        else if (cmd == "pwd") {
            std::cout << std::filesystem::current_path().generic_string() << std::endl;
        }

        else if (auto path = doesExecutableExist(cmd)) {

            std::vector<char*> c_args;
            args.reserve(args.size() + 2);

            c_args.push_back(const_cast<char*>(cmd.c_str()));
            for (auto &arg : args)
                c_args.push_back(arg.data());

            c_args.push_back(nullptr);

            pid_t pid = fork();

            if (pid == 0) {
                // Child Process
                execvp(cmd.c_str(), c_args.data());
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

        if (redirection != REDIRECTION::NONE) {
            dup2(saved, redirection);
            close(saved);
        }
    }
}
