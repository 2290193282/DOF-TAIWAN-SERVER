cmake_minimum_required(VERSION 3.20)

project(df_bridge_r_restored LANGUAGES CXX)

option(DFBRIDGE_WITH_MYSQL "Build the production d_channel MySQL repository" OFF)

find_package(Threads REQUIRED)

add_library(dfbridge_core
    src/bridge_core.cpp
    src/channel_info.cpp
    src/config.cpp
    src/protocol.cpp
    src/script_thread.cpp
)

target_include_directories(dfbridge_core PUBLIC include)
target_compile_features(dfbridge_core PUBLIC cxx_std_11)
target_compile_definitions(dfbridge_core PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(dfbridge_core PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(dfbridge_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(dfbridge_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(DFBRIDGE_WITH_MYSQL)
    find_path(MYSQL_INCLUDE_DIR mysql.h PATH_SUFFIXES mysql mariadb)
    find_library(MYSQL_CLIENT_LIBRARY NAMES mysqlclient libmysql)
    if(NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_CLIENT_LIBRARY)
        message(FATAL_ERROR "DFBRIDGE_WITH_MYSQL=ON requires mysql.h and libmysqlclient")
    endif()
    target_sources(dfbridge_core PRIVATE src/mysql_repository.cpp)
    target_include_directories(dfbridge_core PRIVATE "${MYSQL_INCLUDE_DIR}")
    target_link_libraries(dfbridge_core PUBLIC "${MYSQL_CLIENT_LIBRARY}")
    target_compile_definitions(dfbridge_core PUBLIC DFBRIDGE_WITH_MYSQL=1)
endif()

add_executable(df_bridge_r src/main.cpp src/service.cpp)
target_link_libraries(df_bridge_r PRIVATE dfbridge_core)

enable_testing()
add_executable(dfbridge_tests tests/bridge_tests.cpp)
target_link_libraries(dfbridge_tests PRIVATE dfbridge_core)
add_test(NAME dfbridge_core COMMAND dfbridge_tests)
