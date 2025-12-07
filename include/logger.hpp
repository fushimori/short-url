#pragma once

#include <string>

void log_to_db(const std::string& level, const std::string& message, const std::string& ip = "", const std::string& user_agent = "");
void log(const std::string& message, const std::string& level = "INFO", const std::string& ip = "", const std::string& user_agent = "");