//
// Created by semyo on 18.06.2026.
//
#include "FrozenSetValue.h"
#include "StrValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ProtocolHelpers.h"

namespace {

    const MethodMap FROZENSET_METHODS = {
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::FrozenSetPtr>)
    };

}

Value getFrozenSetAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, FROZENSET_METHODS, "frozenset");
}