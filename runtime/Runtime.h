//
// Created by semyo on 08.05.2026.
//

#ifndef CPPYTHON_RUNTIME_H
#define CPPYTHON_RUNTIME_H
#include <memory>

class ClassValue;

class Runtime {
public:
    static std::shared_ptr<ClassValue> objectClass;

    static std::shared_ptr<ClassValue> strClass;

    static std::shared_ptr<ClassValue> bytesClass;

    static std::shared_ptr<ClassValue> bytearrayClass;
};
#endif //CPPYTHON_RUNTIME_H