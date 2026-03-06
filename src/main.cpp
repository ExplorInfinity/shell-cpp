#include <iostream>
#include <string>
#include <vector>
#include <ranges>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

namespace fs = std::filesystem;
const char *pathEnv = std::getenv("PATH");

std::vector<char> doubleQuotesEscapeChars = { '\"', '\\' };

std::vector<std::string> getAllTokens(std::stringstream &ss) {
    std::string s;
    std::getline(ss, s);

    std::vector<std::string> tokens;

    char startQuote = 0;
    bool stringLiteral = false, escapeChar = false;
    std::string token;
    for (const char ch : s) {
        if (escapeChar) {
            if (stringLiteral && std::ranges::find(doubleQuotesEscapeChars, ch) == doubleQuotesEscapeChars.end())
                token += '\\';
            token += ch;
            escapeChar = false;
            continue;
        }

        if ((ch == '\"' || ch == '\'') && (!stringLiteral || startQuote == ch)) {
            stringLiteral = !stringLiteral;
            startQuote = ch;
        }

        else if (ch == '\\' && (startQuote == '\"' || !stringLiteral)) {
            escapeChar = true;
        }

        else if (ch == ' ' && !stringLiteral) {
            if (!token.empty()) tokens.push_back(token);
            token = "";
        }

        else token += ch;
    }

    if (!token.empty())
        tokens.push_back(token);

    return tokens;
}

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
        std::string line, cmd;

        std::getline(std::cin, line);

        std::stringstream cmdStream(line);
        cmdStream >> cmd;

        if (cmd == "exit") break;

        if (cmd == "echo") {
            const auto tokens = getAllTokens(cmdStream);
            for (const auto &token : tokens)
                std::cout << token << ' ';
            std::cout << std::endl;
        }

        else if (cmd == "type") {
            cmdStream >> cmd;

            if (std::ranges::find(builtin_cmds, cmd) != builtin_cmds.end())
                std::cout << cmd << " is a shell builtin" << std::endl;
            else {

                if (auto path = doesExecutableExist(cmd)) {
                    std::cout << cmd << " is " << path.value() << std::endl;
                } else std::cout << cmd << ": not found" << std::endl;

            }
        }

        else if (cmd == "cd") {
            auto tokens = getAllTokens(cmdStream);
            const char* path;

            if (tokens.empty() || tokens[0] == "~") {
                path = std::getenv("HOME");
            } else path = tokens[0].c_str();

            if (chdir(path) != 0)
                std::cerr << "cd: " << path << ": No such file or directory" << std::endl;
        }

        else if (cmd == "pwd") {
            std::cout << std::filesystem::current_path().generic_string() << std::endl;
        }

        else if (auto path = doesExecutableExist(cmd)) {

            auto tokens = getAllTokens(cmdStream);

            std::vector<char*> args;
            args.reserve(tokens.size() + 2);

            std::string arg;
            args.push_back(const_cast<char*>(cmd.c_str()));
            for (auto &token : tokens)
                args.push_back(token.data());

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
