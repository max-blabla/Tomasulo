cmake_minimum_required(VERSION 3.10)
project(riscv)

add_compile_options(-std=c++11 -ofast)
set(CMAKE_CXX_STANDARD 14)

add_executable(riscv main.cpp)