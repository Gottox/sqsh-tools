cmake_minimum_required(VERSION 3.9)
project(libsquash VERSION 0.0.1 DESCRIPTION "Libsquash")

set(CMAKE_C_FLAGS "-Wall")

find_package(ZLIB REQUIRED)

file(GLOB sources src/format/*.c src/*.c src/extractor/*.c)

add_library(squash SHARED ${sources})
set_property(TARGET squash PROPERTY VERSION ${CMAKE_PROJECT_VERSION})
set_property(TARGET squash PROPERTY SOVERSION 0)
target_link_libraries(squash ${ZLIB_LIBRARY})

add_executable(squashinfo bin/squashinfo.c)
target_link_libraries(squashinfo squash)

add_executable(lssquash bin/lssquash.c)
target_link_libraries(lssquash squash)
