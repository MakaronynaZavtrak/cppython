//
// Created by semyo on 24.05.2026.
//

#include "CallRuntime.h"
#include "SetValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../ProtocolHelpers.h"
#include "../../RuntimeUtils.h"

namespace {


    Value makeOrMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "__or__",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "__or__");

                set->add(args[0]);

                return {};
            }
        );
    }

    Value makeAddMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "add",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "add");

                set->add(args[0]);

                return {};
            }
        );
    }

    Value makeRemoveMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "remove",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "remove");

                set->remove(args[0]);

                return {};
            }
        );
    }

    Value makeDiscardMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "discard",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "discard");

                set->discard(args[0]);

                return {};
            }
        );
    }

    Value makeUnionMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "union",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                    return set->unionSet(args);

            }
        );
    }

    Value makeIntersectionMethod(const Value &obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "intersection",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {
                return set->intersection(args);
            }
        );
    }

    Value makeDifferenceMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "difference",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                return set->difference(args);
            }
        );
    }

    Value makeSymmetricDifferenceMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
        "symmetric_difference",

        [set](const std::vector<Value> &args,
              const Kwargs &,
              const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "symmetric_difference");

                return set->symmetricDifference(args[0]);
            }
        );
    }

    Value makeIssubsetMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "issubset",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "issubset");

                return Value(set->isSubset(args[0]));
            }
        );
    }

    Value makeIssupersetMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "issuperset",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "issuperset");

                return Value(set->isSuperset(args[0]));
            }
        );
    }

    Value makeIsdisjointMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "isdisjoint",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "isdisjoint");

                return Value(set->isDisjoint(args[0]));
            }
        );
    }

    Value makeCopyMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "copy",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 0, "copy");

                return set->copy();
            }
        );
    }

    Value makeClearMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "clear",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 0, "clear");

                set->clear();

                return {};
            }
        );
    }

    Value makePopMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "pop",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 0, "pop");

                return set->pop();
            }
        );
    }

    Value makeUpdateMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "update",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                set->update(args);

                return {};
            }
        );
    }

    Value makeDifferenceUpdateMethod(const Value &obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "difference_update",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                set->differenceUpdate(args);

                return {};
            }
        );
    }

    Value makeIntersectionUpdateMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "intersection_update",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                set->intersectionUpdate(args);

                return {};
            }
        );
    }

    Value makeSymmetricDifferenceUpdateMethod(const Value& obj) {

        auto set = extract<Value::SetPtr>(obj);

        return makeBuiltin(
            "symmetric_difference_update",

            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "symmetric_difference_update");

                set->symmetricDifferenceUpdate(args[0]);

                return {};
            }
        );
    }

    const MethodMap SET_METHODS = {
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::SetPtr>),
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__or__", makeOrMethod),
        REGISTER_METHOD("add", makeAddMethod),
        REGISTER_METHOD("remove", makeRemoveMethod),
        REGISTER_METHOD("discard", makeDiscardMethod),
        REGISTER_METHOD("union", makeUnionMethod),
        REGISTER_METHOD("intersection", makeIntersectionMethod),
        REGISTER_METHOD("difference", makeDifferenceMethod),
        REGISTER_METHOD("symmetric_difference", makeSymmetricDifferenceMethod),
        REGISTER_METHOD("issubset", makeIssubsetMethod),
        REGISTER_METHOD("issuperset", makeIssupersetMethod),
        REGISTER_METHOD("isdisjoint", makeIsdisjointMethod),
        REGISTER_METHOD("difference_update", makeDifferenceUpdateMethod),
        REGISTER_METHOD("intersection_update", makeIntersectionUpdateMethod),
        REGISTER_METHOD("symmetric_difference_update", makeSymmetricDifferenceUpdateMethod),
        REGISTER_METHOD("pop", makePopMethod),
        REGISTER_METHOD("update", makeUpdateMethod),
        REGISTER_METHOD("clear", makeClearMethod),
        REGISTER_METHOD("copy", makeCopyMethod)
    };

}

Value getSetAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, SET_METHODS, "set");
}
