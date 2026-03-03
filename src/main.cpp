#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

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

int main() {
    if (!pathEnv) return -1;

    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        std::cout << "$ ";
        std::string line, cmd;

        std::getline(std::cin, line);

        std::stringstream cmdStream(line);
        cmdStream >> cmd;

        if (cmd == "exit") break;

        if (cmd == "echo") {
            std::string printline;
            if (std::getline(cmdStream, printline) && !printline.empty())
                std::cout << cmd.substr(1) << std::endl;
        }

        else if (cmd == "type") {
            std::cin >> cmd;

            if (cmd == "echo" || cmd == "exit" || cmd == "type")
                std::cout << cmd << " is a shell builtin" << std::endl;
            else {

                if (auto path = doesExecutableExist(cmd)) {
                    std::cout << cmd << " is " << path.value() << std::endl;
                } else std::cout << cmd << " : not found" << std::endl;

            }
        }

        else if (auto path = doesExecutableExist(cmd)) {
            pid_t pid = fork();

            std::string arg1, arg2;
            cmdStream >> arg1 >> arg2;

            if (pid == 0) {
                // Child Process
                execl(path->c_str(), cmd.c_str(), arg1.c_str(), arg2.c_str(), nullptr);
            }

            else if (pid > 0) {
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
