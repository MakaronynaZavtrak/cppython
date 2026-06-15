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
        sources/ClassMethodValue.cpp
        headers/DescriptorUtils.h
        sources/DescriptorUtils.cpp
        headers/ListValue.h
        sources/ListValue.cpp
        headers/DictValue.h
        sources/DictValue.cpp
        sources/TupleValue.cpp
        headers/TupleValue.h
        headers/ListIterator.h
        headers/TupleIterator.h
        headers/StopIterationException.h
        headers/DictKeysView.h
        sources/DictKeysView.cpp
        headers/DictValuesView.h
        headers/DictItemsView.h
        sources/DictValueView.cpp
        sources/DictItemsView.cpp
        headers/DictKeysIterator.h
        headers/DictValuesIterator.h
        headers/DictItemsIterator.h
        headers/SetValue.h
        sources/SetValue.cpp
        headers/IteratorValue.h
        headers/SetIterator.h
        sources/SetIterator.cpp
        sources/ListIterator.cpp
        sources/TupleIterator.cpp
        sources/DictKeysIterator.cpp
        sources/DictValuesIterator.cpp
        sources/DictItemsIterator.cpp
        headers/StringIterator.h
        sources/StringIterator.cpp
        runtime/builtins/list/ListMethods.h
        runtime/builtins/list/ListMethods.cpp
        runtime/builtins/dict/DictMethods.h
        runtime/builtins/dict/DictMethods.cpp
        runtime/builtins/tuple/TupleMethods.h
        runtime/builtins/tuple/TupleMethods.cpp
        runtime/builtins/set/SetMethods.h
        runtime/builtins/set/SetMethods.cpp
        runtime/builtins/iterator/IteratorMethods.h
        runtime/builtins/iterator/IteratorMethods.cpp
        runtime/builtins/str/StrMethods.h
        runtime/builtins/str/StrMethods.cpp
        runtime/builtins/BuiltinMethodFactory.h
        runtime/RuntimeUtils.h
        runtime/builtins/BuiltinMethodRegistry.h
        runtime/builtins/BuiltinAttrLookup.h
        runtime/ProtocolHelpers.h
        runtime/ArgValidation.h
        headers/StrValue.h
        sources/StrValue.cpp
        headers/BytesValue.h
        sources/BytesValue.cpp
        runtime/builtins/bytes/BytesMethods.h
        runtime/builtins/bytes/BytesMethods.cpp
        headers/ObjectValue.h
        headers/BytesIterator.h
        sources/BytesIterator.cpp
        headers/SliceValue.h
        headers/ByteArrayValue.h
        sources/ByteArrayValue.cpp
        runtime/builtins/bytearray/ByteArrayMethods.h
        runtime/builtins/bytearray/ByteArrayMethods.cpp
        headers/ByteArrayIterator.h
        sources/ByteArrayIterator.cpp)

target_include_directories(cppython PRIVATE headers)

target_link_libraries(cppython
        Qt6::Core
        Boost::boost
)
