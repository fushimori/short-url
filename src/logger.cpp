#include "logger.hpp"
#include "database.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

void log_to_db(const std::string& level, const std::string& message, const std::string& ip, const std::string& user_agent) {
    const char* sql = "INSERT INTO logs (timestamp, level, message, ip, user_agent) VALUES (datetime('now'), ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, level.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, ip.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, user_agent.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cout << "Failed to log to DB" << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

void log(const std::string& message, const std::string& level, const std::string& ip, const std::string& user_agent) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
    log_to_db(level, message, ip, user_agent);
}