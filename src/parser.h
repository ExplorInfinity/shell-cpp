#pragma once
#include <vector>
#include <ranges>
#include <algorithm>

static std::vector<std::string> redirectionTokens = { ">", "1>", "2>", ">>", "1>>", "2>>" };
static std::vector<char> doubleQuotesEscapeChars = { '\"', '\\' };

namespace Parser {

    enum REDIRECTION { NONE, STDIN, STDERR };

    struct Command {
        std::vector<std::vector<std::string>> cmdPipelines;
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

        // Checking Output Redirection

        REDIRECTION redirection = NONE;
        std::string outfile;

        bool append = false;
        if (tokens.size() > 2 &&
            std::ranges::find(redirectionTokens, tokens[tokens.size()-2]) != redirectionTokens.end())
        {
            const std::string &t = tokens[tokens.size()-2];

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

        // Creating different Pipelines
        const auto count = std::ranges::count(tokens, "|");

        std::vector<std::vector<std::string>> cmdPipelines(count + 1);
        int cmdIndex = static_cast<int>(count);
        for (int i = static_cast<int>(tokens.size())-1; i >= 0; --i) {
            if (tokens[i] == "|") {
                for (int j = i + 1; j < tokens.size(); j++)
                    cmdPipelines[cmdIndex].push_back(std::move(tokens[j]));
                tokens.resize(i);
                --cmdIndex;
            }
        }
        for (const auto &token : tokens)
            cmdPipelines[cmdIndex].push_back(token);

        return { cmdPipelines, outfile, append, redirection };
    }
}