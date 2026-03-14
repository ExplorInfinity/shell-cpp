#pragma once

namespace Pipeline {

    inline void createPipeline(std::vector<std::string> &agrs) {
        int fd[2];
        pipe(fd);

        const auto pid = fork();

        if (pid == 0) {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
        } else if (pid > 0) {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
        } else {
            std::cerr << "Unable to fork while creating Pipeline\n";
        }
    }

}