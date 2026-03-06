#pragma once
#include <vector>
#include <ranges>
#include <sstream>

namespace Parser {

    inline std::vector<char> doubleQuotesEscapeChars = { '\"', '\\' };

    inline std::vector<std::string> parseString(const std::string &s) {
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

    inline std::vector<std::string> getAllTokens(std::stringstream &ss) {
        std::string s;
        std::getline(ss, s);

        return parseString(s);
    }
}