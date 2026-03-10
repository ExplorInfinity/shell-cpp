#pragma once
#include <vector>
#include <ranges>
#include <sstream>

namespace Parser {

    struct Command {
        std::string cmd;
        std::vector<std::string> args;
        std::string outfile;
        bool redirection = false;
    };

    inline std::vector<char> doubleQuotesEscapeChars = { '\"', '\\' };

    inline Command parseString(const std::string &s) {
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

        bool redirection = false;
        std::string outfile;

        if (tokens[tokens.size()-2] == ">" || tokens[tokens.size()-2] == "1>") {
            redirection = true;
            outfile = tokens.back();
            tokens.pop_back();
            tokens.pop_back();
        }

        std::string cmd = tokens.front();
        tokens.erase(tokens.begin());

        return { cmd, tokens, outfile, redirection };
    }
}