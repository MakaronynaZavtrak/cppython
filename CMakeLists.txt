cmake_minimum_required(VERSION 3.20)
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(cppython)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)

find_package(Python3 REQUIRED)
message(STATUS "Python3 executable: ${Python3_EXECUTABLE}")

find_package(Boost CONFIG REQUIRED)

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
        sources/Value.cpp
        headers/FunctionValue.h
        headers/Param.h
        headers/ClassValue.h
        headers/InstanceValue.h
        headers/BoundMethod.h
        sources/FunctionValue.cpp
        sources/BoundMethod.cpp
        headers/ObjectFwd.h
        headers/CallRuntime.h
        sources/CallRuntime.cpp
        headers/ClassUtils.h
        sources/ClassUtils.cpp
        headers/SuperValue.h
        headers/BuiltinFunction.h
        sources/BuiltinFunction.cpp
        sources/ClassValue.cpp
        sources/InstanceValue.cpp
        headers/ReprMixin.h
        headers/PropertyValue.h
        sources/PropertyValue.cpp
        headers/Runtime.h
        sources/Runtime.cpp
        headers/StaticMethodValue.h
        headers/ClassMethodValue.h
        sources/StaticMethodValue.cpp
        sources/ClassMethodValue.cpp)

target_include_directories(cppython PRIVATE headers)

target_link_libraries(cppython
        Qt6::Core
        Boost::boost
)
