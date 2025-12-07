#pragma once

#include <string>
#include <sqlite3.h>

extern sqlite3* db;

void init_db();
void insert_url(const std::string& short_code, const std::string& url);
std::string get_url(const std::string& short_code);
std::string get_short_code(const std::string& url);
void delete_url(const std::string& short_code);