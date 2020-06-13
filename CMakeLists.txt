cmake_minimum_required(VERSION 3.15)
project(libflipnote C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

add_library(libflipnote flipnote.c flipnote.h)
add_executable(flipview flipview.c)
target_link_libraries(flipview libflipnote ${SDL2_LIBRARIES} ${CURSES_LIBRARIES})
