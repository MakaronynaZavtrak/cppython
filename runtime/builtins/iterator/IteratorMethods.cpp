//
// Created by semyo on 24.05.2026.
//
#include "CallRuntime.h"
#include "GeneratorValue.h"
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

    Value make_send_Method(const Value& obj) {

        const auto iter = extract<Value::IteratorPtr>(obj);
        auto gen = std::dynamic_pointer_cast<GeneratorValue>(iter);

        if (!gen) {
            throw AttributeErrorException(
                "'" + obj.toString() + "' object has no attribute 'send'"
            );
        }

        return makeBuiltin(
            "send",

            [gen](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "send");

                return gen->send(args[0]);
            }
        );
    }

    Value make_throw_Method(const Value& obj) {

        const auto iter = extract<Value::IteratorPtr>(obj);
        auto gen = std::dynamic_pointer_cast<GeneratorValue>(iter);

        if (!gen) {
            throw AttributeErrorException(
                "object has no attribute 'throw'"
            );
        }

        return makeBuiltin(
            "throw",

            [gen](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "throw");

                return gen->throwInto(args[0]);
            }
        );
    }

    Value make_close_Method(const Value& obj) {

        const auto iter = extract<Value::IteratorPtr>(obj);
        auto gen = std::dynamic_pointer_cast<GeneratorValue>(iter);

        if (!gen) {
            throw AttributeErrorException("object has no attribute 'close'");
        }

        return makeBuiltin(
            "close",

            [gen](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 0, "close");

                return gen->close();
            }
        );
    }

    const MethodMap ITERATOR_METHODS = {
        REGISTER_METHOD("__iter__", make_iter_Method),
        REGISTER_METHOD("__next__", make_next_Method),
        REGISTER_METHOD("send", make_send_Method),
        REGISTER_METHOD("throw", make_throw_Method),
        REGISTER_METHOD("close", make_close_Method)
    };

}

Value getIteratorAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, ITERATOR_METHODS, "iterator");
}
