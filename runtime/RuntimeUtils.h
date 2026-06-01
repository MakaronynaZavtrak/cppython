//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_RUNTIMEUTILS_H
#define CPPYTHON_RUNTIMEUTILS_H
#include "Value.h"

template<typename T>
T extract(const Value& obj);

template<>
inline Value::ListPtr extract<Value::ListPtr>(const Value& obj) {
    return std::get<Value::ListPtr>(obj.data);
}

template<>
inline Value::DictPtr extract<Value::DictPtr>(const Value& obj) {
    return std::get<Value::DictPtr>(obj.data);
}

template<>
inline Value::TuplePtr extract<Value::TuplePtr>(const Value& obj) {
    return std::get<Value::TuplePtr>(obj.data);
}

template<>
inline Value::SetPtr extract<Value::SetPtr>(const Value& obj) {
    return std::get<Value::SetPtr>(obj.data);
}

template<>
inline Value::IteratorPtr extract<Value::IteratorPtr>(const Value& obj) {
    return std::get<Value::IteratorPtr>(obj.data);
}

template<>
inline Value::StrPtr extract<Value::StrPtr>(const Value& obj) {
    return std::get<Value::StrPtr>(obj.data);
}

template<>
inline Value::BytesPtr extract<Value::BytesPtr>(const Value& obj) {
    return std::get<Value::BytesPtr>(obj.data);
}


template<typename Fn>
Value makeBuiltin(const QString& name, Fn&& fn) {
    return Value(
        std::make_shared<BuiltinFunction>(
            name,
            std::forward<Fn>(fn)
        )
    );
}

#endif //CPPYTHON_RUNTIMEUTILS_H