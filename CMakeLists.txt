cmake_minimum_required(VERSION 3.12.0)
project(toy)
set(CMAKE_CXX_STANDARD 17)
set(source
    Lexer.cpp
    Parser.cpp
    toy.cpp
)
add_executable(toy ${source})