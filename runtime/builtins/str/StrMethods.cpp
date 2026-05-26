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

    Value makeStripMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "strip",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "strip");

                if (args.empty()) {
                    return str->strip();
                }

                return str->strip(args[0].asString("strip")->toString());
            }
        );
    }

    Value makeSplitMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "split",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 2, "split");

                std::optional<QString> sep;
                std::optional<qsizetype> maxsplit;

                if (args.size() >= 1) {

                    if (!args[0].isNone()) {
                        sep = args[0].asString("split")->toString();
                    }
                }

                if (args.size() == 2) {
                    maxsplit = static_cast<qsizetype>(args[1].asBigInt("split"));
                }

                return str->split(sep, maxsplit);
            }
        );
    }

    const MethodMap STR_METHODS = {
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::StrPtr>),
        REGISTER_METHOD("__getitem__", make_getitem_Method),
        REGISTER_METHOD("upper", makeUpperMethod),
        REGISTER_METHOD("lower", makeLowerMethod),
        REGISTER_METHOD("strip", makeStripMethod),
        REGISTER_METHOD("split", makeSplitMethod)
    };
}

Value getStrAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, STR_METHODS, "str");
}
