//
// Created by semyo on 11.06.2026.
//
#include "ByteArrayValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ProtocolHelpers.h"

namespace {

    Value makeGetItemMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [byteArray](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__getitem__");

                return byteArray->getItem(args[0]);
            }
        );
    }

    Value makeAddMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__add__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__add__");

                return byteArray->add(args[0]);
            }
        );
    }

    Value makeMultiplyMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__mul__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__mul__");

                return byteArray->multiply(args[0]);
            }
        );
    }

    Value makeContainsMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__contains__",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__contains__");

                return Value(
                    byteArray->contains(args[0])
                );
            }
        );
    }

    Value makeEqMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__eq__",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(
                    byteArray->equal(args[0])
                );
            }
        );
    }

    Value makeNeMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__ne__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(
                    byteArray->notEqual(args[0])
                );
            }
        );
    }

    const MethodMap BYTEARRAY_METHODS = {
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::ByteArrayPtr>),
        REGISTER_METHOD("__getitem__", makeGetItemMethod),
        REGISTER_METHOD("__add__", makeAddMethod),
        REGISTER_METHOD("__mul__", makeMultiplyMethod),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__eq__", makeEqMethod),
        REGISTER_METHOD("__ne__", makeNeMethod)
    };

}

Value getByteArrayAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, BYTEARRAY_METHODS, "bytes");
}

Value make__byteArray__ClassBuiltin() {

    return makeBuiltin(
        "__bytes__",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>&)
        -> Value {

            expectArgs(args, 1, "bytearray.__bytes__");

            return args[0].asByteArray("__bytes__")->__bytes__();
        }
    );

}