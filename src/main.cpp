#include <iostream>
#include <string>

int main() {
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
      else
        std::cout << cmd << ": not found" << std::endl;
    }

    else std::cout << cmd << ": command not found" << std::endl;
  }
}
