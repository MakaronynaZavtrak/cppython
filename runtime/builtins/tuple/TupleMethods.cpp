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
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("index", makeIndexMethod),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::TuplePtr>)
    };

}

Value getTupleAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, TUPLE_METHODS, "tuple");
}
