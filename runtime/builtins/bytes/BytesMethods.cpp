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

    Value makeNotEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__ne__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(
                    bytes->notEqual(args[0])
                );
            }
        );
    }

    Value makeLessMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__lt__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__lt__");

                return Value(
                    bytes->less(args[0])
                );
            }
        );
    }

    Value makeLessOrEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__le__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__le__");

                return Value(
                    bytes->lessOrEqual(args[0])
                );
            }
        );
    }

    Value makeGreaterMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__gt__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__gt__");

                return Value(
                    bytes->greater(args[0])
                );
            }
        );
    }

    Value makeGreaterOrEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__ge__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ge__");

                return Value(
                    bytes->greaterOrEqual(args[0])
                );
            }
        );
    }

    Value makeContainsMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__contains__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__contains__");

                return Value(
                    bytes->contains(args[0])
                );
            }
        );
    }

    Value makeFindMethod(const Value& obj) {

        return makeBuiltin(
            "find",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {
                expectArgsRange(args, 1, 3, "find");

                Value sub = args[0];

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) start = args[1];

                if (args.size() == 3) end = args[2];

                return obj.asBytes()->find(sub, start, end);
            }
        );
    }

    Value makeIndexMethod(const Value& obj) {

        return makeBuiltin(
            "index",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 1, 3, "index");

                const Value sub = args[0];

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2)
                    start = args[1];

                if (args.size() >= 3)
                    end = args[2];

                return obj.asBytes()->index(sub, start, end);
            }
        );
    }

    Value makeCountMethod(const Value &obj) {

        return makeBuiltin(
            "count",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 1, 3, "count");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return obj.asBytes()->count(args[0], start, end);
            }
        );
    }

    Value makeStartsWithMethod(const Value &obj) {

        return makeBuiltin(
            "startswith",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 1, 3, "startswith");

                const Value prefix = args[0];

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return obj.asBytes()->startsWith(prefix, start, end);
            }
        );
    }

    const MethodMap BYTES_METHODS = {
        REGISTER_METHOD("__getitem__", makeGetItemMethod),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::BytesPtr>),
        REGISTER_METHOD("__add__", makeAddMethod),
        REGISTER_METHOD("__mul__", makeMultiplyMethod),
        REGISTER_METHOD("__eq__", makeEqualMethod),
        REGISTER_METHOD("__ne__", makeNotEqualMethod),
        REGISTER_METHOD("__lt__", makeLessMethod),
        REGISTER_METHOD("__le__", makeLessOrEqualMethod),
        REGISTER_METHOD("__gt__", makeGreaterMethod),
        REGISTER_METHOD("__ge__", makeGreaterOrEqualMethod),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("find", makeFindMethod),
        REGISTER_METHOD("index", makeIndexMethod),
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("startswith", makeStartsWithMethod)
    };
}

Value getBytesAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, BYTES_METHODS, "bytes");
}