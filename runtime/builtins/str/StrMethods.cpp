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

    Value makeJoinMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "join",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "join");

                return str->join(args[0]);
            }
        );
    }

    Value makeReplaceMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "replace",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 2, 3, "replace");

                if (args.size() == 2) {
                    return str->replace(args[0], args[1]);
                }

                return str->replace(args[0], args[1], args[2]);
            }
        );
    }

    Value makeStartswithMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "startswith",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "startswith");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return str->startswith(args[0], start, end);
            }
        );
    }

    Value makeEndswithMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "endswith",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "endswith");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return str->endswith(args[0], start, end);
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
        REGISTER_METHOD("split", makeSplitMethod),
        REGISTER_METHOD("join", makeJoinMethod),
        REGISTER_METHOD("replace", makeReplaceMethod),
        REGISTER_METHOD("startswith", makeStartswithMethod),
        REGISTER_METHOD("endswith", makeEndswithMethod)
    };
}

Value getStrAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, STR_METHODS, "str");
}
