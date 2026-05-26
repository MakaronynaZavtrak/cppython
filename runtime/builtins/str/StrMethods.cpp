//
// Created by semyo on 24.05.2026.
//

#include "StrValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ProtocolHelpers.h"

namespace {

    Value make_getitem_Method(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [str](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__getitem__");

                return str->getItem(args[0]);
            }
        );
    }

    Value makeUpperMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "upper",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "upper");

                return str->upper();
            }
        );
    }

    Value makeLowerMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "lower",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "lower");

                return str->lower();
            }
        );
    }

    const MethodMap STR_METHODS = {
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::StrPtr>),
        REGISTER_METHOD("__getitem__", make_getitem_Method),
        REGISTER_METHOD("upper", makeUpperMethod),
        REGISTER_METHOD("lower", makeLowerMethod)
    };
}

Value getStrAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, STR_METHODS, "str");
}
