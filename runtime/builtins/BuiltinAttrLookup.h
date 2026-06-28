//
// Created by semyo on 25.05.2026.
//

#ifndef CPPYTHON_BUILTINATTRLOOKUP_H
#define CPPYTHON_BUILTINATTRLOOKUP_H
#include "BuiltinMethodRegistry.h"
#include "../../exception/AttributeErrorException.h"

inline Value getBuiltinAttr(
    const Value& obj,
    const QString& attr,
    const MethodMap& methods,
    const QString& typeName) {

    if (const auto it = methods.find(attr);
        it != methods.end()) {

        return it.value()(obj);
    }

    throw AttributeErrorException(
        typeName + " has no attribute '" + attr + "'"
    );
}
#endif //CPPYTHON_BUILTINATTRLOOKUP_H