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

    Value makeLtMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__lt__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__lt__");

                return Value(
                    byteArray->less(args[0])
                );
            }
        );
    }

    Value makeLeMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__le__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__le__");

                return Value(
                    byteArray->lessOrEqual(args[0])
                );
            }
        );
    }

    Value makeGtMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__gt__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__gt__");

                return Value(
                    byteArray->greater(args[0])
                );
            }
        );
    }

    Value makeGeMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__ge__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ge__");

                return Value(
                    byteArray->greaterOrEqual(args[0])
                );
            }
        );
    }

    Value makeFindMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "find",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "find");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->find(args[0], start, end);
            }
        );
    }

    Value makeRfindMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rfind",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "rfind");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->rfind(args[0], start, end);
            }
        );
    }

    Value makeIndexMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "index",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "index");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->index(args[0], start, end);
            }
        );
    }

    Value makeCountMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "count",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
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

                return byteArray->count(args[0], start, end);
            }
        );
    }

    Value makeStartsWithMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "startswith",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "startswith");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->startsWith(args[0], start, end);
            }
        );
    }

    Value makeEndsWithMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "endswith",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "endswith");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->endsWith(
                    args[0],
                    start,
                    end
                );
            }
        );
    }

    Value makeLStripMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "lstrip",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
                -> Value {

                expectArgsRange(args, 0, 1, "bytearray.lstrip");

                if (args.empty()) {
                    return byteArray->lstrip();
                }

                return byteArray->lstrip(args[0]);
            }
        );
    }

    Value makeRStripMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rstrip",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&
            ) -> Value {

                expectArgsRange(args, 0, 1, "rstrip");

                if (args.empty()) {
                    return byteArray->rstrip();
                }

                return byteArray->rstrip(args[0]);
            }
        );
    }

    Value makeStripMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "strip",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
                -> Value {

                expectArgsRange(args, 0, 1, "strip");

                if (args.empty()) {
                    return byteArray->strip();
                }

                return byteArray->strip(args[0]);
            }
        );
    }

    Value makeRemovePrefixMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "removeprefix",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "removeprefix");

                return byteArray->removeprefix(args[0]);
            }
        );
    }

    Value makeRemoveSuffixMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "removesuffix",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "removesuffix");

                return byteArray->removesuffix(args[0]);
            }
        );
    }

    Value makeReplaceMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "replace",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 2, 3, "replace");

                if (args.size() == 2) {

                    return byteArray->replace(args[0], args[1]);
                }

                return byteArray->replace(
                    args[0],
                    args[1],
                    args[2].toBigInt()
                );
            }
        );
    }

    Value makeSplitMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "split",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "split");

                if (args.size() == 1) {

                    return byteArray->split(args[0]);
                }

                return byteArray->split(args[0], args[1].toBigInt());
            }
        );
    }

    Value makeRSplitMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rsplit",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "rsplit");

                if (args.size() == 1) {

                    return byteArray->rsplit(args[0]);
                }

                return byteArray->rsplit(args[0], args[1].toBigInt());
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
        REGISTER_METHOD("__ne__", makeNeMethod),
        REGISTER_METHOD("__lt__", makeLtMethod),
        REGISTER_METHOD("__le__", makeLeMethod),
        REGISTER_METHOD("__gt__", makeGtMethod),
        REGISTER_METHOD("__ge__", makeGeMethod),
        REGISTER_METHOD("find", makeFindMethod),
        REGISTER_METHOD("rfind", makeRfindMethod),
        REGISTER_METHOD("index", makeIndexMethod),
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("startswith", makeStartsWithMethod),
        REGISTER_METHOD("endswith", makeEndsWithMethod),
        REGISTER_METHOD("lstrip", makeLStripMethod),
        REGISTER_METHOD("rstrip", makeRStripMethod),
        REGISTER_METHOD("strip", makeStripMethod),
        REGISTER_METHOD("removeprefix", makeRemovePrefixMethod),
        REGISTER_METHOD("removesuffix",makeRemoveSuffixMethod),
        REGISTER_METHOD("replace", makeReplaceMethod),
        REGISTER_METHOD("split", makeSplitMethod),
        REGISTER_METHOD("rsplit", makeRSplitMethod)
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