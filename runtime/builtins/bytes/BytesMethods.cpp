//
// Created by semyo on 01.06.2026.
//
#include "CallRuntime.h"
#include "BytesValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../ProtocolHelpers.h"
#include "../../RuntimeUtils.h"

namespace {

    Value makeGetItemMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__getitem__");

                return bytes->getItem(args[0]);
            }
        );
    }

    Value makeAddMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__add__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__add__");

                return bytes->add(args[0]);
            }
        );
    }

    Value makeMultiplyMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__mul__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__add__");

                return bytes->multiply(args[0]);
            }
        );
    }

    Value makeModMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__mod__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__mod__");

                return bytes->mod(args[0]);
            }
        );
    }

    Value makeEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__eq__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(bytes->equal(args[0]));
            }
        );
    }

    Value makeNotEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__ne__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(
                    bytes->notEqual(args[0])
                );
            }
        );
    }

    Value makeLessMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__lt__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__lt__");

                return Value(
                    bytes->less(args[0])
                );
            }
        );
    }

    Value makeLessOrEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__le__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__le__");

                return Value(
                    bytes->lessOrEqual(args[0])
                );
            }
        );
    }

    Value makeGreaterMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__gt__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__gt__");

                return Value(
                    bytes->greater(args[0])
                );
            }
        );
    }

    Value makeGreaterOrEqualMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__ge__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ge__");

                return Value(
                    bytes->greaterOrEqual(args[0])
                );
            }
        );
    }

    Value makeContainsMethod(const Value& obj) {

        auto bytes = extract<Value::BytesPtr>(obj);

        return makeBuiltin(
            "__contains__",

            [bytes](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__contains__");

                return Value(
                    bytes->contains(args[0])
                );
            }
        );
    }

    Value makeFindMethod(const Value& obj) {

        return makeBuiltin(
            "find",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {
                expectArgsRange(args, 1, 3, "find");

                const Value& sub = args[0];

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) start = args[1];

                if (args.size() == 3) end = args[2];

                return obj.asBytes()->find(sub, start, end);
            }
        );
    }

    Value makeIndexMethod(const Value& obj) {

        return makeBuiltin(
            "index",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 1, 3, "index");

                const Value& sub = args[0];

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2)
                    start = args[1];

                if (args.size() >= 3)
                    end = args[2];

                return obj.asBytes()->index(sub, start, end);
            }
        );
    }

    Value makeCountMethod(const Value &obj) {

        return makeBuiltin(
            "count",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 1, 3, "count");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return obj.asBytes()->count(args[0], start, end);
            }
        );
    }

    Value makeStartsWithMethod(const Value &obj) {

        return makeBuiltin(
            "startswith",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 1, 3, "startswith");

                const Value& prefix = args[0];

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return obj.asBytes()->startsWith(prefix, start, end);
            }
        );
    }

    Value makeEndsWithMethod(const Value &obj) {
        return makeBuiltin(
            "endswith",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 1, 3, "endswith");

                const Value& suffix = args[0];

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() == 3) {
                    end = args[2];
                }

                return obj.asBytes()->endsWith(suffix, start, end);
            }
        );
    }

    Value makeSplitMethod(const Value &obj) {
        return makeBuiltin(
            "split",

            [obj](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &)
        -> Value {

                expectArgsRange(args, 0, 2, "split");

                std::optional<Value> sep;
                Value::BigInt maxsplit = -1;

                if (!args.empty()) {

                    if (!args[0].isNone()) {
                        sep = args[0];
                    }
                }

                if (args.size() == 2) {
                    maxsplit = args[1].toBigInt();
                }

                return obj.asBytes()->split(sep, maxsplit);
            }
        );
    }

    Value makeJoinMethod(const Value& obj) {

        return makeBuiltin(
            "join",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "join");

                return obj.asBytes()->join(args[0]);
            }
        );
    }

    Value makeReplaceMethod(const Value& obj) {

        return makeBuiltin(
            "replace",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 2, 3, "replace");

                const Value& oldValue = args[0];
                const Value& newValue = args[1];

                Value::BigInt count = -1;

                if (args.size() == 3) {
                    count = args[2].toBigInt();
                }

                return obj.asBytes()->replace(oldValue, newValue, count);
            }
        );
    }

    Value makeIsAsciiMethod(const Value& obj) {

        return makeBuiltin(
            "isascii",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isascii");

                return obj.asBytes()->isAscii();
            }
        );
    }

    Value makeIsAlphaMethod(const Value& obj) {

        return makeBuiltin(
            "isalpha",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isalpha");

                return obj.asBytes()->isAlpha();
            }
        );
    }

    Value makeIsDigitMethod(const Value& obj) {

        return makeBuiltin(
            "isdigit",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isdigit");

                return obj.asBytes()->isDigit();
            }
        );
    }

    Value makeIsAlnumMethod(const Value& obj) {

        return makeBuiltin(
            "isalnum",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isalnum");

                return obj.asBytes()->isAlnum();
            }
        );
    }

    Value makeIsSpaceMethod(const Value& obj) {

        return makeBuiltin(
            "isspace",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isspace");

                return obj.asBytes()->isSpace();
            }
        );
    }

    Value makeIsLowerMethod(const Value& obj) {

        return makeBuiltin(
            "islower",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "islower");

                return obj.asBytes()->isLower();
            }
        );
    }

    Value makeIsUpperMethod(const Value& obj) {

        return makeBuiltin(
            "isupper",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isupper");

                return obj.asBytes()->isUpper();
            }
        );
    }

    Value makeLowerMethod(const Value& obj) {

        return makeBuiltin(
            "lower",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "lower");

                return obj.asBytes()->lower();
            }
        );
    }

    Value makeUpperMethod(const Value& obj) {

        return makeBuiltin(
            "upper",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "upper");

                return obj.asBytes()->upper();
            }
        );
    }

    Value makeSwapcaseMethod(const Value& obj) {

        return makeBuiltin(
            "swapcase",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "swapcase");

                return obj.asBytes()->swapcase();
            }
        );
    }

    Value makeCapitalizeMethod(const Value& obj) {

        return makeBuiltin(
            "capitalize",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "capitalize");

                return obj.asBytes()->capitalize();
            }
        );
    }

    Value makeTitleMethod(const Value& obj) {

        return makeBuiltin(
            "title",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "title");

                return obj.asBytes()->title();
            }
        );
    }

    Value makeIsTitleMethod(const Value& obj) {

        return makeBuiltin(
            "istitle",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "istitle");

                return obj.asBytes()->isTitle();
            }
        );
    }

    Value makeLStripMethod(const Value& obj) {

        return makeBuiltin(
            "lstrip",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "lstrip");

                if (args.empty()) {
                    return obj.asBytes()->lstrip();
                }

                return obj.asBytes()->lstrip(args[0]);
            }
        );
    }

    Value makeRStripMethod(const Value& obj) {

        return makeBuiltin(
            "rstrip",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "rstrip");

                if (args.empty()) {
                    return obj.asBytes()->rstrip();
                }

                return obj.asBytes()->rstrip(args[0]);
            }
        );
    }

    Value makeStripMethod(const Value& obj) {

        return makeBuiltin(
            "strip",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "strip");

                if (args.empty()) {
                    return obj.asBytes()->strip();
                }

                return obj.asBytes()->strip(args[0]);
            }
        );
    }

    Value makeCenterMethod(const Value& obj) {

        return makeBuiltin(
            "center",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "center");

                const auto width =
                    args[0].toBigInt();

                std::optional<Value> fillchar;

                if (args.size() == 2) {
                    fillchar = args[1];
                }

                return obj.asBytes()->center(
                    width,
                    fillchar
                );
            }
        );
    }

    Value makeLJustMethod(const Value& obj) {

        return makeBuiltin(
            "ljust",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "ljust");

                const auto width =
                    args[0].toBigInt();

                std::optional<Value> fillchar;

                if (args.size() == 2) {
                    fillchar = args[1];
                }

                return obj.asBytes()->ljust(
                    width,
                    fillchar
                );
            }
        );
    }

    Value makeRJustMethod(const Value& obj) {

        return makeBuiltin(
            "rjust",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "rjust");

                const auto width =
                    args[0].toBigInt();

                std::optional<Value> fillchar;

                if (args.size() == 2) {
                    fillchar = args[1];
                }

                return obj.asBytes()->rjust(width, fillchar);
            }
        );
    }

    Value makeZfillMethod(const Value& obj) {

        return makeBuiltin(
            "zfill",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "zfill");

                return obj.asBytes()->zfill(args[0].toBigInt());
            }
        );
    }

    Value makeRemovePrefixMethod(
    const Value& obj
) {

        return makeBuiltin(
            "removeprefix",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "removeprefix"
                );

                return obj.asBytes()->removeprefix(args[0]);
            }
        );
    }

    Value makeRemoveSuffixMethod(const Value& obj) {

        return makeBuiltin(
            "removesuffix",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "removesuffix");

                return obj.asBytes()->removeSuffix(args[0]);
            }
        );
    }

    Value makePartitionMethod(const Value& obj) {

        return makeBuiltin(
            "partition",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "partition");

                return obj.asBytes()->partition(args[0]
                );
            }
        );
    }

    Value makeRPartitionMethod(const Value& obj) {

        return makeBuiltin(
            "rpartition",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "rpartition");

                return obj.asBytes()->rpartition(args[0]);
            }
        );
    }

    Value makeSplitLinesMethod(
    const Value& obj
) {

        return makeBuiltin(
            "splitlines",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "splitlines");

                bool keepends = false;

                if (!args.empty()) {
                    keepends = args[0].toBool();
                }

                return obj.asBytes()
                    ->splitlines(keepends);
            }
        );
    }

    Value makeExpandTabsMethod(const Value& obj) {

        return makeBuiltin(
            "expandtabs",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "expandtabs");

                int tabsize = 8;

                if (!args.empty()) {
                    tabsize =static_cast<int>(args[0].toBigInt());
                }

                return obj.asBytes()->expandTabs(tabsize);
            }
        );
    }

    Value makeHexMethod(const Value& obj) {

        return makeBuiltin(
            "hex",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "hex");

                return obj.asBytes()->hex();
            }
        );
    }

    Value makeFromHexMethod(const Value&) {

        return makeBuiltin(
            "fromhex",

            [](const std::vector<Value>& args,
               const Kwargs&,
               const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "fromhex");

                return BytesValue::fromHex(args[0].toString());
            }
        );
    }

    Value makeDecodeMethod(const Value& obj) {

        return makeBuiltin(
            "decode",

            [obj](const std::vector<Value>& args,
                  const Kwargs&,
                  const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 2, "decode");

                QString encoding = "utf-8";
                QString errors = "strict";

                if (!args.empty()) {
                    encoding = args[0].toString();
                }

                if (args.size() == 2) {
                    errors = args[1].toString();
                }

                return obj.asBytes()->decode(encoding, errors);
            }
        );
    }

    Value makeMakeTransMethod(const Value&) {

        return makeBuiltin(
            "maketrans",

            [](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
                -> Value {

                return BytesValue::maketrans(args);
            }
        );
    }

    Value makeTranslateMethod(const Value& obj) {

        return makeBuiltin(
            "translate",

            [obj](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&
            ) -> Value {

                expectArgsRange(args, 1, 2, "translate");

                std::optional<Value> deleteBytes;

                if (args.size() == 2) {
                    deleteBytes = args[1];
                }

                return obj.asBytes()->translate(args[0], deleteBytes);
            }
        );
    }

    Value makeRFindMethod(const Value& obj) {

        return makeBuiltin(
            "rfind",

            [obj](
                const std::vector<Value>& args,
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

                return obj.asBytes()->rfind(args[0], start, end);
            }
        );
    }

    Value makeRIndexMethod(const Value& obj) {

        return makeBuiltin(
            "rindex",

            [obj](
                const std::vector<Value>& args,
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

                return obj.asBytes()->rindex(args[0], start, end);
            }
        );
    }

    Value makeRSplitMethod(const Value& obj) {

        return makeBuiltin(
            "rsplit",

            [obj](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
                -> Value {

                expectArgsRange(args, 0, 2, "rsplit");

                std::optional<Value> sep;
                Value::BigInt maxsplit = -1;

                if (args.size() >= 1) {

                    if (!args[0].isNone()) {
                        sep = args[0];
                    }
                }

                if (args.size() == 2) {
                    maxsplit = args[1].toBigInt();
                }

                return obj.asBytes()->rsplit(sep, maxsplit);
            }
        );
    }

    Value make__bytes__Method(const Value& obj) {

        return makeBuiltin(
            "__bytes__",

            [obj](const std::vector<Value> &args,
               const Kwargs &,
               const std::shared_ptr<Environment> &)
               -> Value {

                expectArgs(args, 0, "__bytes__");

                return obj.asBytes("__bytes__")->_bytes_();
            }
        );
    }

    const MethodMap BYTES_METHODS = {
        REGISTER_METHOD("__getitem__", makeGetItemMethod),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::BytesPtr>),
        REGISTER_METHOD("__add__", makeAddMethod),
        REGISTER_METHOD("__mul__", makeMultiplyMethod),
        REGISTER_METHOD("__mod__", makeModMethod),
        REGISTER_METHOD("__eq__", makeEqualMethod),
        REGISTER_METHOD("__ne__", makeNotEqualMethod),
        REGISTER_METHOD("__lt__", makeLessMethod),
        REGISTER_METHOD("__le__", makeLessOrEqualMethod),
        REGISTER_METHOD("__gt__", makeGreaterMethod),
        REGISTER_METHOD("__ge__", makeGreaterOrEqualMethod),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__bytes__", make__bytes__Method),
        REGISTER_METHOD("find", makeFindMethod),
        REGISTER_METHOD("rfind", makeRFindMethod),
        REGISTER_METHOD("index", makeIndexMethod),
        REGISTER_METHOD("rindex", makeRIndexMethod),
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("startswith", makeStartsWithMethod),
        REGISTER_METHOD("endswith", makeEndsWithMethod),
        REGISTER_METHOD("split", makeSplitMethod),
        REGISTER_METHOD("rsplit", makeRSplitMethod),
        REGISTER_METHOD("join", makeJoinMethod),
        REGISTER_METHOD("replace", makeReplaceMethod),
        REGISTER_METHOD("isascii", makeIsAsciiMethod),
        REGISTER_METHOD("isalpha", makeIsAlphaMethod),
        REGISTER_METHOD("isdigit", makeIsDigitMethod),
        REGISTER_METHOD("isalnum", makeIsAlnumMethod),
        REGISTER_METHOD("isspace", makeIsSpaceMethod),
        REGISTER_METHOD("islower", makeIsLowerMethod),
        REGISTER_METHOD("isupper", makeIsUpperMethod),
        REGISTER_METHOD("lower", makeLowerMethod),
        REGISTER_METHOD("upper", makeUpperMethod),
        REGISTER_METHOD("swapcase", makeSwapcaseMethod),
        REGISTER_METHOD("capitalize", makeCapitalizeMethod),
        REGISTER_METHOD("title", makeTitleMethod),
        REGISTER_METHOD("istitle", makeIsTitleMethod),
        REGISTER_METHOD("lstrip", makeLStripMethod),
        REGISTER_METHOD("rstrip", makeRStripMethod),
        REGISTER_METHOD("strip", makeStripMethod),
        REGISTER_METHOD("center", makeCenterMethod),
        REGISTER_METHOD("ljust", makeLJustMethod),
        REGISTER_METHOD("rjust", makeRJustMethod),
        REGISTER_METHOD("zfill", makeZfillMethod),
        REGISTER_METHOD("removeprefix", makeRemovePrefixMethod),
        REGISTER_METHOD("removesuffix", makeRemoveSuffixMethod),
        REGISTER_METHOD("partition", makePartitionMethod),
        REGISTER_METHOD("rpartition", makeRPartitionMethod),
        REGISTER_METHOD("splitlines",makeSplitLinesMethod),
        REGISTER_METHOD("expandtabs", makeExpandTabsMethod),
        REGISTER_METHOD("hex", makeHexMethod),
        REGISTER_METHOD("fromhex", makeFromHexMethod),
        REGISTER_METHOD("decode", makeDecodeMethod),
        REGISTER_METHOD("maketrans", makeMakeTransMethod),
        REGISTER_METHOD("translate", makeTranslateMethod)
    };
}

Value getBytesAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, BYTES_METHODS, "bytes");
}

Value makeFromHexClassBuiltin() {

    return makeBuiltin(
        "fromhex",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>&)
        -> Value {

            expectArgs(args, 1, "fromhex");

            return BytesValue::fromHex(args[0].toString());
        }
    );
}

Value makeMakeTransBytesClassBuiltin() {

    return makeBuiltin(
        "maketrans",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>&)
        -> Value {

            return BytesValue::maketrans(args);
        }
    );
}

Value make__bytes__ClassBuiltin() {

    return makeBuiltin(
        "__bytes__",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>&)
        -> Value {

            expectArgs(args, 1, "bytes.__bytes__");

            return args[0].asBytes("__bytes__")->_bytes_();
        }
    );

}
