//
// Created by semyo on 24.05.2026.
//
#include "CallRuntime.h"
#include "IteratorValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../RuntimeUtils.h"

namespace {

    Value make_iter_Method(const Value& obj) {

        return makeBuiltin(
        "__iter__",

        [obj](const std::vector<Value> &args,
              const Kwargs &,
              const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 0, "__iter__");

                return obj;
            }
        );
    }

    Value make_next_Method(const Value& obj) {

        auto iter = extract<Value::IteratorPtr>(obj);

        return makeBuiltin(
            "__next__",

            [iter](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 0, "__next__");

                return iter->next();
            }
        );
    }

    const MethodMap ITERATOR_METHODS = {
        REGISTER_METHOD("__iter__", make_iter_Method),
        REGISTER_METHOD("__next__", make_next_Method)
    };

}

Value getIteratorAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, ITERATOR_METHODS, "iterator");
}
