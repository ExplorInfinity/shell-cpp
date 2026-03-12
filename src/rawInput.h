#pragma once

#include <termios.h>
#include <unistd.h>
#include <string>

#include "autocomplete.h"
using namespace AutoComplete;

static termios orig_termios;

namespace RawInput {

    enum KEYS {
        DELETE = 127,
        BACKSPACE = 8,
        TAB = '\t',
        NEWLINE = '\n'
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
        termios check;
        tcgetattr(STDIN_FILENO, &check);
        std::cerr << ((check.c_lflag & ICANON) ? "canonical ON\n" : "canonical OFF\n");
    }

    inline void clearTerminalLine() {
        std::cout << "\r\33[2K$ ";
    }

    inline std::string watchInput() {
        char ch;
        std::string input;
        while (read(STDIN_FILENO, &ch, 1) == 1) {
            switch (ch) {
                case TAB:
                    if (input.empty()) break;
                    clearTerminalLine();
                    input = cmdCompletion(input);
                    std::cout << input;
                    break;

                case NEWLINE:
                    std::cout << '\n';
                    return input;

                case DELETE:
                case BACKSPACE:
                    if (!input.empty()) {
                        input.pop_back();
                        std::cout << "\b \b";
                    }
                    break;

                default:
                    std::cout << ch;
                    input += ch;
            }
        }

        return "";
    }
}