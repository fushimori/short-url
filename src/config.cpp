#include "config.hpp"
#include <fstream>

int load_config() {
    std::ifstream file("config.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("short_code_length=") == 0) {
                try {
                    return std::stoi(line.substr(18));
                } catch (...) {
                }
            }
        }
        file.close();
    }
    return 6;
}