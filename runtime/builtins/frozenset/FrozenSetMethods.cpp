//
// Created by semyo on 18.06.2026.
//
#include "FrozenSetValue.h"
#include "StrValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ProtocolHelpers.h"

namespace {

    Value makeContainsMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__contains__",

            [frozenSet](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__contains__");

                return Value(
                    frozenSet->contains(args[0])
                );
            }
        );
    }

    Value makeUnionMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "union",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                return frozenSet->unionSet(args);
            }
        );
    }

    Value makeIntersectionMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "intersection",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                return frozenSet->intersection(args);
            }
        );
    }

    Value makeDifferenceMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "difference",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                return frozenSet->difference(args);
            }
        );
    }

    Value makeSymmetricDifferenceMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "symmetric_difference",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "symmetric_difference");

                return frozenSet->symmetricDifference(args[0]);
            }
        );
    }

    Value makeOrMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__or__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__or__");

                return frozenSet->bitOr(args[0]);
            }
        );
    }

    Value makeRorMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__ror__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ror__");

                return frozenSet->ror(args[0]);
            }
        );
    }

    Value makeRandMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__rand__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__rand__");

                return frozenSet->rand(args[0]);
            }
        );
    }

    Value makeAndMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__and__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__and__");

                return frozenSet->bitAnd(args[0]);
            }
        );
    }

    Value makeSubMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__sub__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__sub__");

                return frozenSet->sub(args[0]);
            }
        );
    }

    Value makeRsubMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__rsub__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__rsub__");

                return frozenSet->rsub(args[0]);
            }
        );
    }

    Value makeXorMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__xor__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__xor__");

                return frozenSet->bitXor(args[0]);
            }
        );
    }

    Value makeRxorMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__rxor__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__rxor__");

                return frozenSet->rxor(args[0]);
            }
        );
    }

    Value makeEqMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(

            "__eq__",

            [frozenSet](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(frozenSet->equal(args[0]));
            }
        );
    }

    Value makeNeMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__ne__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(frozenSet->notEqual(args[0]));
            }
        );
    }

    Value makeLtMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__lt__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__lt__");

                return Value(frozenSet->less(args[0]));
            }
        );
    }

    Value makeLeMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__le__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__le__");

                return Value(frozenSet->lessOrEqual(args[0]));
            }
        );
    }

    Value makeGtMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__gt__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__gt__");

                return Value(
                    frozenSet->greater(args[0])
                );
            }
        );
    }

    Value makeGeMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__ge__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ge__");

                return Value(frozenSet->greaterOrEqual(args[0]));
            }
        );
    }

    Value makeHashMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "__hash__",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "__hash__");

                return Value(Value::BigInt(frozenSet->hash()));
            }
        );
    }

    Value makeIsSubsetMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "issubset",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "issubset");

                return Value(frozenSet->isSubset(args[0]));
            }
        );
    }

    Value makeIsSupersetMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "issuperset",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
                -> Value {

                expectArgs(args, 1, "issuperset");

                return Value(frozenSet->isSuperset(args[0]));
            }
        );
    }

    Value makeIsDisjointMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "isdisjoint",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "isdisjoint");

                return Value(frozenSet->isDisjoint(args[0]));
            }
        );
    }

    Value makeCopyMethod(const Value& obj) {

        auto frozenSet = extract<Value::FrozenSetPtr>(obj);

        return makeBuiltin(
            "copy",

            [frozenSet](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "copy");

                return frozenSet->copy();
            }
        );
    }

    const MethodMap FROZENSET_METHODS = {
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::FrozenSetPtr>),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__iter__", makeIterMethod),
        REGISTER_METHOD("__or__", makeOrMethod),
        REGISTER_METHOD("__ror__", makeRorMethod),
        REGISTER_METHOD("__and__", makeAndMethod),
        REGISTER_METHOD("__rand__", makeRandMethod),
        REGISTER_METHOD("__sub__", makeSubMethod),
        REGISTER_METHOD("__rsub__", makeRsubMethod),
        REGISTER_METHOD("__xor__", makeXorMethod),
        REGISTER_METHOD("__rxor__", makeRxorMethod),
        REGISTER_METHOD("__eq__", makeEqMethod),
        REGISTER_METHOD("__ne__", makeNeMethod),
        REGISTER_METHOD("__lt__", makeLtMethod),
        REGISTER_METHOD("__le__", makeLeMethod),
        REGISTER_METHOD("__gt__", makeGtMethod),
        REGISTER_METHOD("__ge__", makeGeMethod),
        REGISTER_METHOD("__hash__", makeHashMethod),
        REGISTER_METHOD("union", makeUnionMethod),
        REGISTER_METHOD("intersection", makeIntersectionMethod),
        REGISTER_METHOD("difference", makeDifferenceMethod),
        REGISTER_METHOD("symmetric_difference", makeSymmetricDifferenceMethod),
        REGISTER_METHOD("issubset", makeIsSubsetMethod),
        REGISTER_METHOD("issuperset", makeIsSupersetMethod),
        REGISTER_METHOD("isdisjoint", makeIsDisjointMethod),
        REGISTER_METHOD("copy", makeCopyMethod)
    };

}

Value getFrozenSetAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, FROZENSET_METHODS, "frozenset");
}