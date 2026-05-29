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

                if (!args.empty()) {

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

    Value makeFindMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "find",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "find");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return str->find(args[0], start, end);
            }
        );
    }

    Value makeCountMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "count",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "count");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return str->count(args[0], start, end);
            }
        );
    }

    Value makeIndexMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "index",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "index");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return str->index(args[0], start, end);
            }
        );
    }

    Value makeRfindMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "rfind",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "rfind");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return str->rfind(args[0], start, end);
            }
        );
    }

    Value makeRindexMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "rindex",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "rindex");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return str->rindex(args[0], start, end);
            }
        );
    }

    Value makeCapitalizeMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "capitalize",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "capitalize");

                return str->capitalize();
            }
        );
    }

    Value makeTitleMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "title",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "title");

                return str->title();
            }
        );
    }

    Value makeSwapcaseMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "swapcase",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "swapcase");

                return str->swapcase();
            }
        );
    }

    Value makeIsAlphaMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "isalpha",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isalpha");

                return str->isalpha();
            }
        );
    }

    Value makeIsDigitMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "isdigit",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isdigit");

                return str->isdigit();
            }
        );
    }

    Value makeIsAlnumMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "isalnum",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isalnum");

                return str->isalnum();
            }
        );
    }

    Value makeIsSpaceMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "isspace",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isspace");

                return str->isspace();
            }
        );
    }

    Value makeCenterMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "center",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "center");

                if (args.size() == 1) {
                    return str->center(args[0]);
                }

                return str->center(args[0], args[1]);
            }
        );
    }

    Value makeLjustMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "ljust",

            [str](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "ljust");

                if (args.size() == 1) {
                    return str->ljust(args[0]);
                }

                return str->ljust(args[0], args[1]);
            }
        );
    }

    Value makeRjustMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "rjust",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "rjust");

                if (args.size() == 1) {
                    return str->rjust(args[0]);
                }

                return str->rjust(args[0], args[1]);
            }
        );
    }

    Value makeLstripMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "lstrip",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "lstrip");

                if (args.empty()) {
                    return str->lstrip();
                }

                return str->lstrip(
                    args[0]
                        .asString("lstrip")
                        ->toString()
                );
            }
        );
    }

    Value makeRstripMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "rstrip",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "rstrip");

                if (args.empty()) {
                    return str->rstrip();
                }

                return str->rstrip(
                    args[0]
                        .asString("rstrip")
                        ->toString()
                );
            }
        );
    }

    Value makeIsLowerMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "islower",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "islower");

                return str->islower();
            }
        );
    }

    Value makeIsUpperMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "isupper",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isupper");

                return str->isupper();
            }
        );
    }

    Value makeIsDecimalMethod(const Value& obj) {

        auto str = extract<Value::StrPtr>(obj);

        return makeBuiltin(
            "isdecimal",

            [str](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isdecimal");

                return str->isdecimal();
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
        REGISTER_METHOD("endswith", makeEndswithMethod),
        REGISTER_METHOD("find", makeFindMethod),
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("index", makeIndexMethod),
        REGISTER_METHOD("rfind", makeRfindMethod),
        REGISTER_METHOD("rindex", makeRindexMethod),
        REGISTER_METHOD("capitalize", makeCapitalizeMethod),
        REGISTER_METHOD("title", makeTitleMethod),
        REGISTER_METHOD("swapcase", makeSwapcaseMethod),
        REGISTER_METHOD("isalpha", makeIsAlphaMethod),
        REGISTER_METHOD("isdigit", makeIsDigitMethod),
        REGISTER_METHOD("isalnum", makeIsAlnumMethod),
        REGISTER_METHOD("isspace", makeIsSpaceMethod),
        REGISTER_METHOD("center", makeCenterMethod),
        REGISTER_METHOD("ljust", makeLjustMethod),
        REGISTER_METHOD("rjust", makeRjustMethod),
        REGISTER_METHOD("lstrip", makeLstripMethod),
        REGISTER_METHOD("rstrip", makeRstripMethod),
        REGISTER_METHOD("islower", makeIsLowerMethod),
        REGISTER_METHOD("isupper", makeIsUpperMethod),
        REGISTER_METHOD("isdecimal", makeIsDecimalMethod)
    };
}

Value getStrAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, STR_METHODS, "str");
}
