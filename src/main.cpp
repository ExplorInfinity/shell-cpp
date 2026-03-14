#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <ranges>
#include <sstream>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "runner.h"
#include "parser.h"
#include "rawInput.h"
#include "autocomplete.h"

using namespace Runner;
using namespace Parser;
using namespace RawInput;
using namespace AutoComplete;

std::vector<std::string> history;

int main() {
    if (!pathEnv) return -1;

    enableRawInput();
    atexit(disableRawInput);

    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    while (true) {
        std::cout << "$ ";
        watchInput();

        const std::string &cmdLine = history.back();
        auto [cmdPipelines, outfile, append, redirection] = parseString(cmdLine);


        int saved = -1;
        if (redirection != REDIRECTION::NONE) {
            saved = dup(redirection);
            const int fd = open(outfile.c_str(), O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
            dup2(fd, redirection);
            close(fd);
        }

        if (cmdPipelines.size() == 1) {
            if (!runCmd(cmdPipelines[0]))
                return 0;
        }

        else if (cmdPipelines.size() > 1) {
            auto prev_read = STDIN_FILENO;

            std::vector<pid_t> childProcesses;
            childProcesses.reserve(cmdPipelines.size());

            for (int i = 0; i < cmdPipelines.size(); i++) {
                const bool last = (i == cmdPipelines.size()-1);

                int fd[2];
                if (!last) pipe(fd);

                const pid_t pid = fork();
                if (pid == 0) {
                    dup2(prev_read, STDIN_FILENO);
                    if (!last) {
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[0]);
                        close(fd[1]);
                    }

                    if (prev_read != STDIN_FILENO)
                        close(prev_read);

                    runCmd(cmdPipelines[i], false);
                    _exit(0);
                }

                if (pid > 0) {
                    if (!last) {
                        close(fd[1]);
                        prev_read = fd[0];
                    }
                    childProcesses.push_back(pid);
                } else {
                    std::cerr << "Unable to fork process during connecting pipelines\n";
                    return 1;
                }
            }

            if (prev_read != STDIN_FILENO)
                close(prev_read);

            for (const auto pid : childProcesses)
                waitpid(pid, nullptr, 0);
        }

        if (redirection != REDIRECTION::NONE) {
            dup2(saved, redirection);
            close(saved);
        }
    }
}
