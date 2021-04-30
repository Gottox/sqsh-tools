cmake_minimum_required(VERSION 3.9)
project(libsquash VERSION 1.0.1 DESCRIPTION "Libsquash")

find_package(ZLIB REQUIRED)
file(GLOB sources src/*.c src/compression/*.c)
add_library(squash SHARED ${sources})
target_link_libraries(squash ${ZLIB_LIBRARY})

add_executable(squashinfo bin/squashinfo.c)
target_link_libraries(squashinfo squash)
