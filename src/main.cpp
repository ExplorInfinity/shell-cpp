#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

int main() {
  const char* pathEnv = std::getenv("PATH");
  if (!pathEnv) return -1;

  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // TODO: Uncomment the code below to pass the first stage
  while (true) {
    std::cout << "$ ";
    std::string cmd;
    std::cin >> cmd;

    if (cmd == "exit") break;

    if (cmd == "echo") {
      std::getline(std::cin, cmd);
      std::cout << cmd.substr(1) << std::endl;
    }

    else if (cmd == "type") {
      std::cin >> cmd;

      if (cmd == "echo" || cmd == "exit" || cmd == "type")
        std::cout << cmd << " is a shell builtin" << std::endl;
      else {
        std::string dir;
        std::stringstream ss(pathEnv);

        bool found = false;
        while (std::getline(ss, dir, ':')) {
          std::string candidate = dir + '/' + cmd;
          if (fs::exists(candidate)) {
            fs::perms perms = fs::status(candidate).permissions();

            if ((perms & fs::perms::owner_exec) == fs::perms::none)
              continue;

            std::cout << cmd << " is " << candidate << std::endl;
            found = true;
            break;
          }
        }

        if (!found) std::cout << cmd << ": not found" << std::endl;
      }
    }

    else std::cout << cmd << ": command not found" << std::endl;
  }
}
