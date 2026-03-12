#pragma once

#include <sstream>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;
inline const char *pathEnv = std::getenv("PATH");

namespace Executable {
    inline std::optional<std::string> doesExecutableExist(const std::string &cmd) {
        std::string dir;
        std::stringstream ss(pathEnv);

        while (std::getline(ss, dir, ':')) {
            std::string candidate = dir + '/';
            candidate += cmd;

            if (fs::exists(candidate)) {
                const fs::perms perms = fs::status(candidate).permissions();

                if ((perms & fs::perms::owner_exec) == fs::perms::none)
                    continue;

                return candidate;
            }
        }

        return {};
    }

    inline bool isExecutable(const fs::directory_entry &entry) {
        const auto perms = entry.status().permissions();
        return (perms & fs::perms::owner_exec) != fs::perms::none;
    }
}