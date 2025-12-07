#include "database.hpp"
#include <iostream>

sqlite3* db;

void init_db() {
    const char* sql = "CREATE TABLE IF NOT EXISTS urls (short_code TEXT PRIMARY KEY, url TEXT); CREATE UNIQUE INDEX IF NOT EXISTS idx_url ON urls(url); CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp TEXT, level TEXT, message TEXT, ip TEXT, user_agent TEXT);";
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cout << "Failed to create tables: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
}

void insert_url(const std::string& short_code, const std::string& url) {
    const char* sql = "INSERT OR REPLACE INTO urls (short_code, url) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, short_code.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cout << "Failed to insert URL" << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

std::string get_url(const std::string& short_code) {
    const char* sql = "SELECT url FROM urls WHERE short_code = ?;";
    sqlite3_stmt* stmt;
    std::string url;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, short_code.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }
    return url;
}

std::string get_short_code(const std::string& url) {
    const char* sql = "SELECT short_code FROM urls WHERE url = ?;";
    sqlite3_stmt* stmt;
    std::string short_code;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            short_code = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }
    return short_code;
}

void delete_url(const std::string& short_code) {
    const char* sql = "DELETE FROM urls WHERE short_code = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, short_code.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cout << "Failed to delete URL" << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}