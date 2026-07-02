//
// Created by semyo on 08.05.2026.
//
#include "Runtime.h"

#include "classes/RegisterClasses.h"
#include "exceptions/RegisterExceptionClasses.h"

void Runtime::initialize(const std::shared_ptr<Environment> &env) {

    registerObjectClass(env);

    registerStringClass(env);

    registerBytesClass(env);

    registerByteArrayClass(env);

    registerExceptionClasses(env);

}

std::shared_ptr<ClassValue> Runtime::objectClass = nullptr;
std::shared_ptr<ClassValue> Runtime::strClass = nullptr;
std::shared_ptr<ClassValue> Runtime::bytesClass = nullptr;
std::shared_ptr<ClassValue> Runtime::bytearrayClass = nullptr;
std::vector<Value::InstancePtr> Runtime::exceptionStack;
std::shared_ptr<ClassValue> Runtime::baseExceptionClass = nullptr;
std::shared_ptr<ClassValue> Runtime::exceptionClass = nullptr;
std::shared_ptr<ClassValue> Runtime::arithmeticErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::attributeErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::indexErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::keyErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::lookupErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::nameErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::overflowErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::runtimeErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::stopIterationClass = nullptr;
std::shared_ptr<ClassValue> Runtime::syntaxErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::typeErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::unicodeDecodeClass = nullptr;
std::shared_ptr<ClassValue> Runtime::valueErrorClass = nullptr;