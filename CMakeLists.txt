cmake_minimum_required(VERSION 3.10)
project(UrlShortener)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR})

find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

add_executable(url_shortener main.cpp)

target_link_libraries(url_shortener ${SQLite3_LIBRARIES} pthread)