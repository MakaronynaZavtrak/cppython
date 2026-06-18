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

    const MethodMap FROZENSET_METHODS = {
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::FrozenSetPtr>),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__iter__", makeIterMethod),
        REGISTER_METHOD("union", makeUnionMethod),
        REGISTER_METHOD("intersection", makeIntersectionMethod),
        REGISTER_METHOD("difference", makeDifferenceMethod),
        REGISTER_METHOD("symmetric_difference", makeSymmetricDifferenceMethod)
    };

}

Value getFrozenSetAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, FROZENSET_METHODS, "frozenset");
}