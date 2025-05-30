cmake_minimum_required(VERSION 3.20)
project(cppython)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)

find_package(Python3 REQUIRED)
message(STATUS "Python3 executable: ${Python3_EXECUTABLE}")


add_executable(cppython main.cpp
        headers/Interpreter.h
        headers/Lexer.h
        headers/Parser.h
        sources/Lexer.cpp
        sources/Parser.cpp
        sources/Interpreter.cpp
        headers/Environment.h
        headers/Value.h
        sources/Environment.cpp
        sources/Value.cpp)

target_include_directories(cppython PRIVATE headers)

target_link_libraries(cppython Qt6::Core)
