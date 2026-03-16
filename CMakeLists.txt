cmake_minimum_required(VERSION 3.31)
project(web_server)

set(CMAKE_CXX_STANDARD 26)

find_package(nlohmann_json CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ---- Core library (all server code compiled once) ----
add_library(web_server_lib
        src/server.cpp
        src/http/request.cpp
        src/utils/string_utils.cpp
        src/http/http_parser.cpp
        src/http/response.cpp
        src/http/routes_registery.cpp
        src/event_loop.cpp
)

target_include_directories(web_server_lib PUBLIC src)

target_link_libraries(web_server_lib
        PUBLIC
        nlohmann_json::nlohmann_json
)

# ---- Main server executable ----
add_executable(web_server main.cpp)

target_link_libraries(web_server
        PRIVATE
        web_server_lib
)

# ---- Test executable ----
add_executable(web_server_tests
        src/http/http_parser_test.cpp
        src/http/routes_registery_test.cpp
)

target_link_libraries(web_server_tests
        PRIVATE
        web_server_lib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

add_test(NAME AllTests COMMAND web_server_tests)