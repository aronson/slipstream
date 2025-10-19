cmake_minimum_required(VERSION 3.13)
project(slipstream C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS Off)

set(PICOQUIC_FETCH_PTLS ON)

set(DISABLE_DEBUG_PRINTF OFF)
set(ENABLE_ASAN OFF)
set(ENABLE_UBSAN OFF)
set(BUILD_DEMO OFF)
set(BUILD_HTTP OFF)
set(BUILD_LOGREADER OFF)

set(BUILD_TESTING OFF)

set(PICOQUIC_ADDITIONAL_C_FLAGS "" CACHE STRING "Additional C flags for picoquic")
set(CMAKE_C_FLAGS "-fexceptions ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    list(APPEND PICOQUIC_COMPILE_DEFINITIONS BUILD_LOGLIB)
    set(BUILD_LOGLIB ON)
else()
    set(BUILD_LOGLIB OFF)
endif()

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()
add_subdirectory(extern/picoquic)

# Common source files for both client and server
set(COMMON_SOURCES
        src/slipstream_inline_dots.c
        src/slipstream_sockloop.c
        src/slipstream_utils.c
        include/slipstream.h
        include/slipstream_inline_dots.h
        include/slipstream_server_cc.h
        include/slipstream_slot.h
        include/slipstream_sockloop.h
        include/slipstream_utils.h

        extern/lua-resty-base-encoding/b32_data.h
        extern/lua-resty-base-encoding/base32.c
        extern/lua-resty-base-encoding/modp_stdint.h
        include/lua-resty-base-encoding-base32.h

        extern/SPCDNS/src/dns.h
        extern/SPCDNS/src/codec.c
        extern/SPCDNS/src/mappings.c
        extern/SPCDNS/src/mappings.h
        extern/SPCDNS/src/netsimple.c
        extern/SPCDNS/src/netsimple.h
        extern/SPCDNS/src/output.c
        extern/SPCDNS/src/output.h

        extern/quick_arg_parser/quick_arg_parser.hpp
)

# Client binary
add_executable(slipstream-client
        src/slipstream_client_cli.cpp
        src/slipstream_client.c
        ${COMMON_SOURCES}
)
target_link_libraries(slipstream-client PRIVATE m)
target_link_libraries(slipstream-client PRIVATE picoquic-core)
if (BUILD_LOGLIB)
    target_link_libraries(slipstream-client PRIVATE picoquic-log)
endif ()
target_include_directories(slipstream-client PRIVATE include extern extern/quick_arg_parser)
set_picoquic_compile_settings(slipstream-client)

# Server binary
add_executable(slipstream-server
        src/slipstream_server_cli.cpp
        src/slipstream_server.c
        src/slipstream_server_cc.c
        ${COMMON_SOURCES}
)
target_link_libraries(slipstream-server PRIVATE m)
target_link_libraries(slipstream-server PRIVATE picoquic-core)
if (BUILD_LOGLIB)
    target_link_libraries(slipstream-server PRIVATE picoquic-log)
endif ()
target_include_directories(slipstream-server PRIVATE include extern extern/quick_arg_parser)
set_picoquic_compile_settings(slipstream-server)
