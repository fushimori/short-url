#include "crow_all.h"
#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <random>
#include <sqlite3.h>
#include <fstream>

sqlite3* db;

void log(const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
}

int load_config() {
    std::ifstream file("config.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("short_code_length=") == 0) {
                try {
                    return std::stoi(line.substr(18));
                } catch (...) {
                    log("Invalid config value, using default");
                }
            }
        }
        file.close();
    }
    return 6;
}

void init_db() {
    const char* sql = "CREATE TABLE IF NOT EXISTS urls (short_code TEXT PRIMARY KEY, url TEXT); CREATE UNIQUE INDEX IF NOT EXISTS idx_url ON urls(url);";
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        log("Failed to create table/index: " + std::string(err_msg));
        sqlite3_free(err_msg);
    } else {
        log("Database initialized");
    }
}

void insert_url(const std::string& short_code, const std::string& url) {
    const char* sql = "INSERT OR REPLACE INTO urls (short_code, url) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, short_code.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            log("Failed to insert URL");
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
            log("Failed to delete URL");
        }
        sqlite3_finalize(stmt);
    }
}

int main() {
    log("Starting URL Shortener server");
    int short_code_length = load_config();
    log("Short code length: " + std::to_string(short_code_length));
    if (sqlite3_open("urls.db", &db) != SQLITE_OK) {
        log("Failed to open database");
        return 1;
    }
    init_db();

    crow::SimpleApp app;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 61);
    const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    auto generate_short = [&]() -> std::string {
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
    };

    CROW_ROUTE(app, "/shorten")
        .methods("POST"_method)
        ([&](const crow::request& req) {
            log("Received shorten request");
            auto body = crow::json::load(req.body);
            if (!body || !body.has("url")) {
                log("Invalid request: missing or invalid JSON");
                return crow::response(400, "Invalid JSON or missing 'url' field");
            }
            std::string url = body["url"].s();
            if (url.empty() || url.find("http") != 0) {
                log("Invalid URL: " + url);
                return crow::response(400, "Invalid URL");
            }
            std::string existing_code = get_short_code(url);
            if (!existing_code.empty()) {
                log("URL already shortened: " + url + " -> " + existing_code);
                crow::json::wvalue response;
                response["short_url"] = "http://localhost:8080/" + existing_code;
                return crow::response(response);
            }
            std::string short_code = generate_short();
            insert_url(short_code, url);
            log("Shortened URL: " + url + " to " + short_code);
            crow::json::wvalue response;
            response["short_url"] = "http://localhost:8080/" + short_code;
            return crow::response(response);
        });

    CROW_ROUTE(app, "/<string>")
        .methods("GET"_method)
        ([&](std::string short_code) {
            if (short_code.empty()) {
                return crow::response(400, "Invalid short code");
            }
            log("Redirect request for: " + short_code);
            std::string url = get_url(short_code);
            if (!url.empty()) {
                log("Redirecting to: " + url);
                crow::response res(302);
                res.add_header("Location", url);
                return res;
            } else {
                log("Short URL not found: " + short_code);
                return crow::response(404, "Short URL not found");
            }
        });

    CROW_ROUTE(app, "/delete/<string>")
        .methods("DELETE"_method)
        ([&](std::string short_code) {
            if (short_code.empty()) {
                return crow::response(400, "Invalid short code");
            }
            log("Delete request for: " + short_code);
            std::string url = get_url(short_code);
            if (!url.empty()) {
                delete_url(short_code);
                log("Deleted URL: " + short_code);
                return crow::response(200, "Deleted");
            } else {
                log("Short URL not found: " + short_code);
                return crow::response(404, "Short URL not found");
            }
        });

    app.port(8080).multithreaded().run();
    sqlite3_close(db);
    return 0;
}