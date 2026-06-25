//
// Created by semyo on 24.05.2026.
//
#include "CallRuntime.h"
#include "TupleValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../ProtocolHelpers.h"
#include "../../RuntimeUtils.h"

namespace {

    Value make_getitem_Method(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
                    -> Value {

                expectArgs(args, 1, "__getitem__");

                return tuple->getItem(args[0]);
            }
        );
    }

    Value makeHashMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__hash__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
                    -> Value {

                expectArgs(args, 0, "__hash__");

                return Value(Value::BigInt(tuple->hash()));
            }
        );
    }

    Value makeContainsMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__contains__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__contains__");

                return Value(tuple->contains(args[0]));
            }
        );
    }

    Value makeAddMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__add__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__add__");

                return tuple->add(args[0]);
            }
        );
    }

    Value makeMulMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__mul__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__mul__");

                return tuple->multiply(args[0]);
            }
        );
    }

    Value makeEqualMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__eq__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(tuple->equal(args[0]));
            }
        );
    }

    Value makeNotEqualMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__ne__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(tuple->notEqual(args[0]));
            }
        );
    }

    Value makeLessMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__lt__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__lt__");

                return Value(tuple->less(args[0]));
            }
        );
    }

    Value makeLessOrEqualMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__le__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__le__");

                return Value(tuple->lessOrEqual(args[0]));
            }
        );
    }

    Value makeGreaterMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__gt__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__gt__");

                return Value(tuple->greater(args[0]));
            }
        );
    }

    Value makeGreaterOrEqualMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__ge__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__ge__");

                return Value(tuple->greaterOrEqual(args[0]));
            }
        );
    }

    Value makeRMulMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "__rmul__",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__rmul__");

                return tuple->rmul(args[0]);
            }
        );
    }

    Value make_setitem_Method(const Value&) {
        throw std::runtime_error("TypeError: 'tuple' object does not support item assignment");
    }

    Value makeCountMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "count",

            [tuple](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
                    -> Value {

                expectArgs(args, 1, "count");

                return tuple->count(args[0]);
            }
        );
    }

    Value makeIndexMethod(const Value& obj) {

        auto tuple = extract<Value::TuplePtr>(obj);

        return makeBuiltin(
            "index",

            [tuple](const std::vector<Value>& args,
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

                return tuple->index(args[0], start, end);
            }
        );
    }

    const MethodMap TUPLE_METHODS = {
        REGISTER_METHOD("__getitem__", make_getitem_Method),
        REGISTER_METHOD("__setitem__", make_setitem_Method),
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::TuplePtr>),
        REGISTER_METHOD("__hash__", makeHashMethod),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__add__", makeAddMethod),
        REGISTER_METHOD("__mul__", makeMulMethod),
        REGISTER_METHOD("__rmul__", makeRMulMethod),
        REGISTER_METHOD("__eq__", makeEqualMethod),
        REGISTER_METHOD("__ne__", makeNotEqualMethod),
        REGISTER_METHOD("__lt__", makeLessMethod),
        REGISTER_METHOD("__le__", makeLessOrEqualMethod),
        REGISTER_METHOD("__gt__", makeGreaterMethod),
        REGISTER_METHOD("__ge__", makeGreaterOrEqualMethod),
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("index", makeIndexMethod)
    };

}

Value getTupleAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, TUPLE_METHODS, "tuple");
}
