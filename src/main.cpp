#include "crow_all.h"
#include "config.hpp"
#include "database.hpp"
#include "logger.hpp"
#include "handlers.hpp"
#include <sqlite3.h>

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
    setup_routes(app, short_code_length);

    app.port(8080).multithreaded().run();
    sqlite3_close(db);
    return 0;
}