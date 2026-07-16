//
// Created by semyo on 12.07.2026.
//

#include "RangeMethods.h"

#include "CallRuntime.h"
#include "RangeValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../ProtocolHelpers.h"
#include "../../RuntimeUtils.h"

namespace {

    Value make_getitem_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__getitem__",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__getitem__");
                return range->getItem(args[0]);
            });
    }

    Value make_len_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__len__",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 0, "__len__");
                return Value(Value::BigInt(range->len()));
            });
    }

    Value make_contains_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__contains__",
            [range](const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {
                expectArgs(args, 1, "__contains__");
                return Value(range->contains(args[0]));
            });
    }

    Value make_eq_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__eq__",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {
                expectArgs(args, 1, "__eq__");
                return Value(range->equal(args[0]));
            });
    }

    Value make_ne_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__ne__",
            [range](const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__ne__");
                return Value(range->notEqual(args[0]));
            });
    }

    Value make_hash_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__hash__",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 0, "__hash__");
                return Value(Value::BigInt(range->hash()));
            });
    }

    Value make_repr_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__repr__",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 0, "__repr__");
                return Value(range->repr());
            });
    }

    Value make_bool_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("__bool__",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 0, "__bool__");
                return Value(range->toBool());
            });
    }

    Value make_count_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("count",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "count");
                return range->count(args[0]);
            });
    }

    Value make_index_Method(const Value& obj) {

        auto range = extract<Value::RangePtr>(obj);

        return makeBuiltin("index",
            [range](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "index");
                return range->index(args[0]);
            });
    }

    Value make_start_Attr(const Value& obj) {
        return Value(extract<Value::RangePtr>(obj)->start);
    }

    Value make_stop_Attr(const Value& obj) {
        return Value(extract<Value::RangePtr>(obj)->stop);
    }

    Value make_step_Attr(const Value& obj) {
        return Value(extract<Value::RangePtr>(obj)->step);
    }

    const MethodMap RANGE_METHODS = {
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__getitem__", make_getitem_Method),
        REGISTER_METHOD("__len__", make_len_Method),
        REGISTER_METHOD("__contains__", make_contains_Method),
        REGISTER_METHOD("__eq__", make_eq_Method),
        REGISTER_METHOD("__ne__", make_ne_Method),
        REGISTER_METHOD("__hash__", make_hash_Method),
        REGISTER_METHOD("__repr__", make_repr_Method),
        REGISTER_METHOD("__bool__", make_bool_Method),
        REGISTER_METHOD("count", make_count_Method),
        REGISTER_METHOD("index", make_index_Method),
        REGISTER_METHOD("start", make_start_Attr),
        REGISTER_METHOD("stop", make_stop_Attr),
        REGISTER_METHOD("step", make_step_Attr)
    };
}

Value getRangeAttr(const Value& obj, const QString& attr) {
    return getBuiltinAttr(obj, attr, RANGE_METHODS, "range");
}