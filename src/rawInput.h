#pragma once

#include <termios.h>
#include <unistd.h>
#include <string>

#include "parser.h"
#include "autocomplete.h"

using namespace Parser;
using namespace AutoComplete;

static termios orig_termios;

namespace RawInput {

    enum KEYS {
        DELETE = 127,
        BACKSPACE = 8,
        TAB = '\t',
        NEWLINE = '\n',
        BELL = '\x07'
    };

    inline void disableRawInput() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }

    inline void enableRawInput() {
        tcgetattr(STDIN_FILENO, &orig_termios);

        termios raw = orig_termios;
        // Disabling canonical mode + echo
        // ICANON → disables line buffering
        // ECHO → stops the terminal from automatically printing characters
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    inline void isRawInputEnabled() {
        termios check{};
        tcgetattr(STDIN_FILENO, &check);
        std::cerr << ((check.c_lflag & ICANON) ? "canonical ON\n" : "canonical OFF\n");
    }

    inline void clearTerminalLine() {
        std::cout << "\r\33[2K$ ";
    }

    inline void handleTab(std::string &input, int &tabCount) {
        if (input.empty()) return;

        const auto &[cmdPipelines, outfile, append, redirection] = parseString(input);

        const auto &args = cmdPipelines.back();
        if (args.empty()) return;

        auto cmd = args[0];

        if (input.back() != ' ' && args.size() == 1) {
            const std::size_t lastInputSize = cmd.size();
            if (!cmdCompletion(cmd, input, ++tabCount > 1))
                std::cout << '\x07';
            else tabCount = 0;

            input += cmd.substr(lastInputSize);
            std::cout << cmd.substr(lastInputSize);
        } else {
            auto file = ((args.size() == 1 || input.back() == ' ') ? "" : args.back());
            const std::size_t lastInputSize = file.size();
            if (!fileCompletion(file, input, ++tabCount > 1))
                std::cout << '\x07';
            else tabCount = 0;

            input += file.substr(lastInputSize);
            std::cout << file.substr(lastInputSize);
        }
    }

    inline std::string watchInput() {
        char ch;
        std::string input;
        int tabCount = 0;
        while (read(STDIN_FILENO, &ch, 1) == 1) {
            switch (ch) {
                case TAB:
                    handleTab(input, tabCount);
                    break;

                case NEWLINE:
                    std::cout << '\n';
                    return input;

                case DELETE:
                case BACKSPACE:
                    if (!input.empty()) {
                        input.pop_back();
                        std::cout << "\b \b";
                        tabCount = 0;
                    }
                    break;

                default:
                    std::cout << ch;
                    input += ch;
                    tabCount = 0;
            }
        }

        return "";
    }
}