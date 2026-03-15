cmake_minimum_required(VERSION 3.31)
project(web_server)

set(CMAKE_CXX_STANDARD 26)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(web_server main.cpp
        src/server.cpp
        src/server.h
        src/http/request.cpp
        src/http/request.h
        src/utils/string_utils.cpp
        src/utils/string_utils.h
        src/http/http_parser.cpp
        src/http/http_parser.h
        src/http/response.cpp
        src/http/response.h
        src/http/routes_registery.cpp
        src/http/routes_registery.h
        src/event_loop.cpp
        src/event_loop.h
        src/http/conn.h
        src/http/http_status.h
)

add_executable(web_server_tests
        src/http/http_parser.cpp
        src/http/http_parser.h
        src/http/http_parser_test.cpp
        src/http/request.cpp
        src/http/request.h
        src/http/response.cpp
        src/http/response.h
        src/http/routes_registery.cpp
        src/http/routes_registery.h
)

target_link_libraries(web_server_tests
        gtest
        gtest_main
)

enable_testing()
add_test(NAME AllTests COMMAND web_server_tests)