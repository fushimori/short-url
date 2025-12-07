#include "utils.hpp"
#include "database.hpp"

std::string generate_short(int short_code_length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61);
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::string short_code;
    for (int i = 0; i < short_code_length; ++i) {
        short_code += chars[dis(gen)];
    }
    while (!get_url(short_code).empty()) {
        short_code = "";
        for (int i = 0; i < short_code_length; ++i) {
            short_code += chars[dis(gen)];
        }
    }
    return short_code;
}