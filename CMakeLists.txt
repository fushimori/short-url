cmake_minimum_required(VERSION 3.10)
project(UrlShortener)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR})

find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
add_executable(url_shortener src/main.cpp ${SOURCES})

target_link_libraries(url_shortener ${SQLite3_LIBRARIES} pthread)

enable_testing()
add_executable(url_shortener_tests tests/tests.cpp ${SOURCES})
target_link_libraries(url_shortener_tests ${SQLite3_LIBRARIES} GTest::gtest_main pthread)
add_test(NAME UrlShortenerTests COMMAND url_shortener_tests)