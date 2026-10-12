cmake_minimum_required(VERSION 3.10)
project(df_channel_r_recovered LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

add_library(df_channel_core
    src/protocol.cpp
    src/script_store.cpp
    src/channel_registry.cpp
    src/legacy_codec.cpp
    src/channel_service.cpp
    src/channel_client.cpp
    src/daemon_config.cpp)
target_include_directories(df_channel_core PUBLIC include)
target_link_libraries(df_channel_core PUBLIC OpenSSL::Crypto ZLIB::ZLIB Threads::Threads)
if(MSVC)
    target_compile_options(df_channel_core PRIVATE /W4)
else()
    target_compile_options(df_channel_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(df_channel_r src/main.cpp src/channel_daemon.cpp)
target_link_libraries(df_channel_r PRIVATE df_channel_core)

add_executable(channel_core_test tests/channel_core_test.cpp)
target_link_libraries(channel_core_test PRIVATE df_channel_core)

enable_testing()
add_test(NAME channel_core_test COMMAND channel_core_test)

if(UNIX)
    set(CHANNEL_INFO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../artifacts/channel_0828/channel_info_0828_authoritative.etc"
        CACHE FILEPATH "authoritative 0828 channel_info.etc")
    set(CHANNEL_DAEMON_TEST_ROOT "" CACHE PATH
        "external test root containing cfg/smoke.cfg with caller-selected ports")
    add_executable(channel_daemon_integration_test tests/channel_daemon_integration_test.cpp)
    target_link_libraries(channel_daemon_integration_test PRIVATE df_channel_core)
    if(CHANNEL_DAEMON_TEST_ROOT)
        add_test(NAME channel_daemon_integration_test
            COMMAND channel_daemon_integration_test $<TARGET_FILE:df_channel_r>
                    "${CHANNEL_INFO_FILE}" "${CHANNEL_DAEMON_TEST_ROOT}")
    endif()
endif()
