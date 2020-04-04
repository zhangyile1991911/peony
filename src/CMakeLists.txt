cmake_minimum_required(VERSION 3.1)
project(peony VERSION 1.1.1 LANGUAGES CXX)

add_library(peony "")

#Options
option(PEONY_BUILD_TEST "Build tests" OFF)


# environment
set_target_properties(
    peony PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# include path
target_include_directories(peony PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# source codes
file(GLOB NET_INC ${CMAKE_CURRENT_SOURCE_DIR}/net/*.hpp)
file(GLOB NET_SRC ${CMAKE_CURRENT_SOURCE_DIR}/net/*.cpp)

file(GLOB BASE_INC ${CMAKE_CURRENT_SOURCE_DIR}/base/*.hpp)
file(GLOB BASE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/base/*.cpp)

file(GLOB_RECURSE PLOG_INC ${CMAKE_CURRENT_SOURCE_DIR}/hybrid/plog/*.hpp)

file(GLOB_RECURSE GOOGLE_DISPATCH_SRC ${CMAKE_CURRENT_SOURCE_DIR}/hybrid/google/*.*)

file(GLOB_RECURSE PEONY_PROTO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/proto/MsgPacket.pb.cc)
if(PEONY_BUILD_TEST)
    add_subdirectory(test)
endif(PEONY_BUILD_TEST)

target_include_directories(peony PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/hybrid ${CMAKE_CURRENT_SOURCE_DIR}/proto)
target_sources(peony PRIVATE
        ${NET_INC} ${NET_SRC}
        ${BASE_INC} ${BASE_SRC}
        ${PLOG_INC}
        ${GOOGLE_DISPATCH_SRC}
        ${PEONY_PROTO_SRC})

# link library
find_package(Threads)
target_link_libraries(peony ${CMAKE_THREAD_LIBS_INIT})

# code warnings
if(UNIX)
    target_compile_options(peony PUBLIC -g -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers)
endif()