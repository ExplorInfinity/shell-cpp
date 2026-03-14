#pragma once

#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace History {

    static std::vector<std::string> history;

    inline void loadHistory(const std::vector<std::string> &args) {
        std::ifstream infile(args[2]);
        std::string line;
        while (getline(infile, line))
            history.push_back(line);
        infile.close();
    }

    inline void writeHistory(const std::vector<std::string> &args) {
        std::ofstream outfile(args[2]);
        for (const auto &s : history)
            outfile << s << '\n';
        outfile.close();
    }

    inline void showHistory(const std::vector<std::string> &args) {
        const int recent = (args.size() > 1 ? static_cast<int>(history.size()) - stoi(args[1]) : 0);
        for (int i = recent; i < history.size(); i++)
            std::cout << std::setw(5) << i + 1 << "  " << history[i] << std::endl;
    }

    inline void runHistoryCmd(const std::vector<std::string> &args) {
        if (args.size() > 3) {
            std::cerr << args[0] << ": Invalid number of arguments\n";
            return;
        }

        if (args.size() == 3) {
            if(args[1].size() != 2 || args[1][0] != '-') {
                std::cerr << args[0] << ": invalid option -- " << args[1] << '\n';
                return;
            }


            switch (args[1].back()) {
                case 'r':
                    if (args.size() == 3 && !fs::exists(args[2])) {
                        std::cout << args[0] << ": " << args[2] << ": Invalid file path!\n";
                        break;
                    }
                    loadHistory(args);
                    break;
                case 'w':
                    writeHistory(args);
                    break;
                default:
                    std::cerr << args[0] << ": invalid option -- " << args[1] << '\n';
            }
        }

        else if (args.size() == 2 && args[1][0] == '-') {
            std::cout << args[0] << ": Invalid file path!\n";
        }

        else if (args.size() == 2 && !std::ranges::all_of(args[1], ::isdigit)) {
            std::cout << args[0] << ": " << args[1] << ": Invalid Positive Number\n";
        }

        else showHistory(args);
    }

}