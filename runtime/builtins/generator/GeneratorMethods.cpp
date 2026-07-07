//
// Created by semyo on 07.07.2026.
//

#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ProtocolHelpers.h"

namespace {

    const MethodMap GENERATOR_METHODS = {
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
    };
}

Value getGeneratorAttr(const Value &obj, const QString &attr) {

    return getBuiltinAttr(obj, attr, GENERATOR_METHODS, "generator");
}
