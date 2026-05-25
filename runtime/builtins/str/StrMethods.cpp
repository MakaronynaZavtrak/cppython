//
// Created by semyo on 24.05.2026.
//

#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ProtocolHelpers.h"

namespace {

    const MethodMap STR_METHODS = {
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin)
    };
}

Value getStrAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, STR_METHODS, "str");
}
