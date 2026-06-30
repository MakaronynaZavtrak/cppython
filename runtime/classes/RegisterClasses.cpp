//
// Created by semyo on 30.06.2026.
//
#include "RegisterClasses.h"

#include "ClassValue.h"
#include "Environment.h"
#include "../Runtime.h"
#include "../builtins/bytearray/ByteArrayMethods.h"
#include "../builtins/bytes/BytesMethods.h"
#include "../builtins/str/StrMethods.h"

void registerObjectClass(const std::shared_ptr<Environment>& env) {

    Runtime::objectClass = std::make_shared<ClassValue>("object");

    Runtime::objectClass->name = "object";

    env->set("object", Value(Runtime::objectClass));

    Runtime::objectClass->attributes["__getattribute__"] =
    env->get("__object_getattribute__");

    Runtime::objectClass->attributes["__setattr__"] =
    env->get("__object_setattr__");
}

void registerStringClass(const std::shared_ptr<Environment> &env) {

    Runtime::strClass = std::make_shared<ClassValue>("str");
    Runtime::strClass->name = "str";
    Runtime::strClass->bases.push_back(Runtime::objectClass);

    auto builtin = std::get<Value::BuiltinFunctionPtr>(makeMakeTransStrClassBuiltin().data);

    Runtime::strClass->attributes["maketrans"] = makeMakeTransStrClassBuiltin();

    env->set("str", Value(Runtime::strClass));

    Runtime::strClass->attributes["__call__"] = env->get("__str_call__");

    env->set("__str_type__", Value(Runtime::strClass));
}

void registerBytesClass(const std::shared_ptr<Environment> &env) {

    Runtime::bytesClass = std::make_shared<ClassValue>("bytes");
    Runtime::bytesClass->name = "bytes";
    Runtime::bytesClass->bases.push_back(Runtime::objectClass);

    Runtime::bytesClass->attributes["fromhex"] = makeFromHexClassBuiltin();
    Runtime::bytesClass->attributes["maketrans"] = makeMakeTransBytesClassBuiltin();
    Runtime::bytesClass->attributes["__bytes__"] = make__bytes__ClassBuiltin();

    env->set("bytes", Value(Runtime::bytesClass));
    Runtime::bytesClass->attributes["__call__"] = env->get("__bytes_call__");
    env->set("__bytes_type__", Value(Runtime::bytesClass));
}

void registerByteArrayClass(const std::shared_ptr<Environment> &env) {

    Runtime::bytearrayClass = std::make_shared<ClassValue>("bytearray");
    Runtime::bytearrayClass->name = "bytearray";
    Runtime::bytearrayClass->bases.push_back(Runtime::objectClass);

    env->set("bytearray", Value(Runtime::bytearrayClass));
    Runtime::bytearrayClass->attributes["__call__"] = env->get("__bytearray_call__");
    env->set("__bytearray_type__", Value(Runtime::bytearrayClass));
    Runtime::bytearrayClass->attributes["__bytes__"] = make_byteArray_ClassBuiltin();
    Runtime::bytearrayClass->attributes["fromhex"] = makeByteArrayFromHexBuiltin();
    Runtime::bytearrayClass->attributes["maketrans"] = makeByteArrayMakeTransBuiltin();
}
