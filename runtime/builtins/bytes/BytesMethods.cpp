//
// Created by semyo on 01.06.2026.
//
#include "CallRuntime.h"
#include "BytesValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../ProtocolHelpers.h"
#include "../../RuntimeUtils.h"

namespace {

    Value makeGetItemMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__getitem__");

                return bytes->getItem(args[0]);
            }
        );
    }

    Value makeAddMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__add__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__add__");

                return bytes->add(args[0]);
            }
        );
    }

    Value makeMultiplyMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__mul__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__add__");

                return bytes->multiply(args[0]);
            }
        );
    }

    Value makeEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__eq__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(bytes->equal(args[0]));
            }
        );
    }

    const MethodMap BYTES_METHODS = {
        REGISTER_METHOD("__getitem__", makeGetItemMethod),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::BytesPtr>),
        REGISTER_METHOD("__add__", makeAddMethod),
        REGISTER_METHOD("__mul__", makeMultiplyMethod),
        REGISTER_METHOD("__eq__", makeEqualMethod)
    };
}

Value getBytesAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, BYTES_METHODS, "bytes");
}