cmake_minimum_required(VERSION 3.16)
project(freecell C)
set (freecell C_STANDARD 99)
file(GLOB sources "src/*.h" "src/*.c")
add_executable(freecell ${sources})