#include <gtest/gtest.h>
#include <fstream>
#include <random>
#include "../include/database.hpp"
#include "../include/config.hpp"

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