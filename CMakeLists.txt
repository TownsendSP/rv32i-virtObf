cmake_minimum_required(VERSION 3.30)
project(CHAsm)

set(CMAKE_CXX_STANDARD 11)

add_executable(CHAsm main.cpp
        ./vmcore.cpp
        ./vmcore.h
)
