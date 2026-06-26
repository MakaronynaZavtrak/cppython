//
// Created by semyo on 08.05.2026.
//
#include "Runtime.h"

std::shared_ptr<ClassValue> Runtime::objectClass = nullptr;
std::shared_ptr<ClassValue> Runtime::strClass = nullptr;
std::shared_ptr<ClassValue> Runtime::bytesClass = nullptr;
std::shared_ptr<ClassValue> Runtime::bytearrayClass = nullptr;