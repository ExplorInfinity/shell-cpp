#pragma once
#include <vector>
#include <ranges>

static std::vector<std::string> redirectionTokens = { ">", "1>", "2>", ">>", "1>>", "2>>" };
static std::vector<char> doubleQuotesEscapeChars = { '\"', '\\' };

namespace Parser {

    enum REDIRECTION { NONE, STDIN, STDERR };

    struct Command {
        std::string cmd;
        std::vector<std::string> args;
        std::string outfile;
        bool append = false;
        REDIRECTION redirection = NONE;
    };


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

        REDIRECTION redirection = NONE;
        std::string outfile;

        bool append = false;
        const std::string &t = tokens[tokens.size()-2];
        if ( tokens.size() > 2 && std::ranges::find(redirectionTokens, t) != redirectionTokens.end()) {

            if (t == ">" || t == "1>" || t == ">>" || t == "1>>")
                redirection = STDIN;
            else if (t == "2>" || t == "2>>")
                redirection = STDERR;

            if (t.size() >= 2 && t.substr(t.size()-2) == ">>")
                append = true;

            outfile = tokens.back();
            tokens.pop_back();
            tokens.pop_back();
        }

        std::string cmd = tokens.front();
        tokens.erase(tokens.begin());

        return { cmd, tokens, outfile, append, redirection };
    }
}