#include <gtest/gtest.h>
#include <sqlite3.h>
#include <string>
#include <fstream>
#include <random>

sqlite3* db;

void init_db() {
    const char* sql = "CREATE TABLE IF NOT EXISTS urls (short_code TEXT PRIMARY KEY, url TEXT); CREATE UNIQUE INDEX IF NOT EXISTS idx_url ON urls(url); CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp TEXT, level TEXT, message TEXT, ip TEXT, user_agent TEXT);";
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        sqlite3_free(err_msg);
    }
}

void insert_url(const std::string& short_code, const std::string& url) {
    const char* sql = "INSERT OR REPLACE INTO urls (short_code, url) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, short_code.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
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
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
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
                }
            }
        }
        file.close();
    }
    return 6;
}

class UrlShortenerTest : public ::testing::Test {
protected:
    void SetUp() override {
        sqlite3_open(":memory:", &db);
        init_db();
    }

    void TearDown() override {
        sqlite3_close(db);
    }
};

TEST_F(UrlShortenerTest, InsertAndGetUrl) {
    std::string short_code = "abc123";
    std::string url = "http://example.com";
    insert_url(short_code, url);
    EXPECT_EQ(get_url(short_code), url);
}

TEST_F(UrlShortenerTest, GetShortCode) {
    std::string short_code = "def456";
    std::string url = "http://test.com";
    insert_url(short_code, url);
    EXPECT_EQ(get_short_code(url), short_code);
}

TEST_F(UrlShortenerTest, DeleteUrl) {
    std::string short_code = "ghi789";
    std::string url = "http://delete.com";
    insert_url(short_code, url);
    EXPECT_EQ(get_url(short_code), url);
    delete_url(short_code);
    EXPECT_EQ(get_url(short_code), "");
}

TEST_F(UrlShortenerTest, LoadConfigDefault) {
    EXPECT_EQ(load_config(), 6);
}

TEST_F(UrlShortenerTest, LoadConfigFromFile) {
    std::ofstream file("config.txt");
    file << "short_code_length=8" << std::endl;
    file.close();
    EXPECT_EQ(load_config(), 8);
    std::remove("config.txt");
}

TEST_F(UrlShortenerTest, DatabasePersistence) {
    std::string test_db = "test_urls.db";
    sqlite3* db1;
    sqlite3_open(test_db.c_str(), &db1);
    db = db1;
    init_db();

    std::string short_code = "persist123";
    std::string url = "http://persist.com";
    insert_url(short_code, url);
    EXPECT_EQ(get_url(short_code), url);

    sqlite3_close(db1);

    sqlite3* db2;
    sqlite3_open(test_db.c_str(), &db2);
    db = db2;
    init_db();
    EXPECT_EQ(get_url(short_code), url);

    sqlite3_close(db2);
    std::remove(test_db.c_str());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}