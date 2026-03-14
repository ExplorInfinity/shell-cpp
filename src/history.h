#pragma once

#include <fstream>

namespace fs = std::filesystem;

namespace History {
    static std::vector<std::string> history;

    inline void loadHistory(const std::vector<std::string> &args) {
        if(args[1] != "-r") {
            std::cerr << args[0] << ": invalid argument -- " << args[1];
            return;
        }

        if (!fs::exists(args[2])) {
            std::cout << args[0] << ": " << args[2] << ": file doesn't exist!";
            return;
        }

        std::ifstream infile(args[2]);
        std::string line;
        while (getline(infile, line)) {
            history.push_back(line);
        }
    }

    inline void showHistory(const std::vector<std::string> &args) {
        const int recent = (args.size() > 1 ? static_cast<int>(history.size()) - stoi(args[1]) : 0);
        for (int i = recent; i < history.size(); i++)
            std::cout << std::setw(5) << i + 1 << "  " << history[i] << std::endl;
    }

}