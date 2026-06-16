//
// Created by semyo on 11.06.2026.
//
#include "ByteArrayValue.h"
#include "StrValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ProtocolHelpers.h"

namespace {

    Value makeGetItemMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [byteArray](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__getitem__");

                return byteArray->getItem(args[0]);
            }
        );
    }

    Value makeSetItemMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__setitem__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 2, "__setitem__");

                return byteArray->setItem(args[0], args[1]);
            }
        );
    }

    Value makeAddMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__add__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__add__");

                return byteArray->add(args[0]);
            }
        );
    }

    Value makeMultiplyMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__mul__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__mul__");

                return byteArray->multiply(args[0]);
            }
        );
    }

    Value makeIAddMethod(const Value& obj) {

        return makeBuiltin(
            "__iadd__",

            [obj](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__iadd__");

                return obj.asByteArray("__iadd__")->iadd(args[0]);
            }
        );
    }

    Value makeContainsMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__contains__",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__contains__");

                return Value(
                    byteArray->contains(args[0])
                );
            }
        );
    }

    Value makeEqMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__eq__",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(
                    byteArray->equal(args[0])
                );
            }
        );
    }

    Value makeNeMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__ne__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(
                    byteArray->notEqual(args[0])
                );
            }
        );
    }

    Value makeLtMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__lt__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__lt__");

                return Value(
                    byteArray->less(args[0])
                );
            }
        );
    }

    Value makeLeMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__le__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__le__");

                return Value(
                    byteArray->lessOrEqual(args[0])
                );
            }
        );
    }

    Value makeGtMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__gt__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__gt__");

                return Value(
                    byteArray->greater(args[0])
                );
            }
        );
    }

    Value makeGeMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__ge__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__ge__");

                return Value(
                    byteArray->greaterOrEqual(args[0])
                );
            }
        );
    }

    Value makeFindMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "find",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "find");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->find(args[0], start, end);
            }
        );
    }

    Value makeRfindMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rfind",

            [byteArray](
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

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->rfind(args[0], start, end);
            }
        );
    }

    Value makeIndexMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "index",

            [byteArray](
                const std::vector<Value>& args,
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

                return byteArray->index(args[0], start, end);
            }
        );
    }

    Value makeCountMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "count",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
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

                return byteArray->count(args[0], start, end);
            }
        );
    }

    Value makeStartsWithMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "startswith",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "startswith");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->startsWith(args[0], start, end);
            }
        );
    }

    Value makeEndsWithMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "endswith",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 3, "endswith");

                std::optional<Value> start;
                std::optional<Value> end;

                if (args.size() >= 2) {
                    start = args[1];
                }

                if (args.size() >= 3) {
                    end = args[2];
                }

                return byteArray->endsWith(
                    args[0],
                    start,
                    end
                );
            }
        );
    }

    Value makeLStripMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "lstrip",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
                -> Value {

                expectArgsRange(args, 0, 1, "bytearray.lstrip");

                if (args.empty()) {
                    return byteArray->lstrip();
                }

                return byteArray->lstrip(args[0]);
            }
        );
    }

    Value makeRStripMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rstrip",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&
            ) -> Value {

                expectArgsRange(args, 0, 1, "rstrip");

                if (args.empty()) {
                    return byteArray->rstrip();
                }

                return byteArray->rstrip(args[0]);
            }
        );
    }

    Value makeStripMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "strip",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
                -> Value {

                expectArgsRange(args, 0, 1, "strip");

                if (args.empty()) {
                    return byteArray->strip();
                }

                return byteArray->strip(args[0]);
            }
        );
    }

    Value makeRemovePrefixMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "removeprefix",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "removeprefix");

                return byteArray->removeprefix(args[0]);
            }
        );
    }

    Value makeRemoveSuffixMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "removesuffix",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "removesuffix");

                return byteArray->removesuffix(args[0]);
            }
        );
    }

    Value makeReplaceMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "replace",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 2, 3, "replace");

                if (args.size() == 2) {

                    return byteArray->replace(args[0], args[1]);
                }

                return byteArray->replace(
                    args[0],
                    args[1],
                    args[2].toBigInt()
                );
            }
        );
    }

    Value makeSplitMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "split",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "split");

                if (args.size() == 1) {

                    return byteArray->split(args[0]);
                }

                return byteArray->split(args[0], args[1].toBigInt());
            }
        );
    }

    Value makeRSplitMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rsplit",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "rsplit");

                if (args.size() == 1) {

                    return byteArray->rsplit(args[0]);
                }

                return byteArray->rsplit(args[0], args[1].toBigInt());
            }
        );
    }

    Value makePartitionMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "partition",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "partition");

                return byteArray->partition(args[0]);
            }
        );
    }

    Value makeRPartitionMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rpartition",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "rpartition");

                return byteArray->rpartition(args[0]);
            }
        );
    }

    Value makeCenterMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "center",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "center");

                std::optional<Value> fill;

                if (args.size() == 2) {
                    fill = args[1];
                }

                return byteArray->center(
                    args[0].toBigInt(),
                    fill
                );
            }
        );
    }

    Value makeLJustMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "ljust",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "ljust");

                std::optional<Value> fill;

                if (args.size() == 2) {
                    fill = args[1];
                }

                return byteArray->ljust(
                    args[0].toBigInt(),
                    fill
                );
            }
        );
    }

    Value makeRJustMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "rjust",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "rjust");

                std::optional<Value> fill;

                if (args.size() == 2) {
                    fill = args[1];
                }

                return byteArray->rjust(
                    args[0].toBigInt(),
                    fill
                );
            }
        );
    }

    Value makeZFillMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "zfill",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "zfill");

                return byteArray->zfill(args[0].toBigInt());
            }
        );
    }

    Value makeLowerMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "lower",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "bytearray.lower");

                return byteArray->lower();
            }
        );
    }

    Value makeUpperMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "upper",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "bytearray.upper");

                return byteArray->upper();
            }
        );
    }

    Value makeSwapCaseMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "swapcase",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "swapcase");

                return byteArray->swapcase();
            }
        );
    }

    Value makeCapitalizeMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "capitalize",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "capitalize");

                return byteArray->capitalize();
            }
        );
    }

    Value makeTitleMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "title",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "title");

                return byteArray->title();
            }
        );
    }

    Value makeIsLowerMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "islower",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "islower");

                return byteArray->isLower();
            }
        );
    }

    Value makeIsUpperMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "isupper",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isupper");

                return byteArray->isUpper();
            }
        );
    }

    Value makeIsTitleMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "istitle",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "istitle");

                return byteArray->isTitle();
            }
        );
    }

    Value makeIsAsciiMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "isascii",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isascii");

                return byteArray->isAscii();
            }
        );
    }

    Value makeIsAlphaMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "isalpha",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isalpha");

                return byteArray->isAlpha();
            }
        );
    }

    Value makeIsDigitMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "isdigit",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isdigit");

                return byteArray->isDigit();
            }
        );
    }

    Value makeIsAlnumMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "isalnum",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "isalnum");

                return byteArray->isAlnum();
            }
        );
    }

    Value makeIsSpaceMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "isspace",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "bytearray.isspace");

                return byteArray->isSpace();
            }
        );
    }

    Value makeExpandTabsMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "expandtabs",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "bytearray.expandtabs");

                Value::BigInt tabSize = 8;

                if (!args.empty()) {
                    tabSize = args[0].toBigInt();
                }

                return byteArray->expandTabs(tabSize);
            }
        );
    }

    Value makeSplitLinesMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "splitlines",

            [byteArray](const std::vector<Value>& args,
                        const Kwargs&,
                        const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "bytearray.splitlines");

                bool keepEnds = false;

                if (!args.empty()) {
                    keepEnds = args[0].toBool();
                }

                return byteArray->splitLines(keepEnds);
            }
        );
    }

    Value makeJoinMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "join",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "join");

                return byteArray->join(args[0]);
            }
        );
    }

    Value makeAppendMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "append",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "append");

                return byteArray->append(args[0]);
            }
        );
    }

    Value makeExtendMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "extend",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "extend");

                return byteArray->extend(args[0]);
            }
        );
    }

    Value makeInsertMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "insert",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 2, "insert");

                return byteArray->insert(args[0].toBigInt(), args[1]);
            }
        );
    }

    Value makePopMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "pop",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 1, "pop");

                if (args.empty()) {

                    return byteArray->pop();
                }

                return byteArray->pop(
                    args[0].toBigInt()
                );
            }
        );
    }

    Value makeRemoveMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "remove",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "remove");

                return byteArray->remove(args[0]);
            }
        );
    }

    Value makeClearMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "clear",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "clear");

                return byteArray->clear();
            }
        );
    }

    Value makeCopyMethod(const Value& obj) {

        auto byteArray =
            extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "copy",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "copy");

                return byteArray->copy();
            }
        );
    }

    Value makeReverseMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "reverse",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "reverse");

                return byteArray->reverse();
            }
        );
    }

    Value makeHexMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "hex",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 2, "hex");

                if (args.empty()) {
                    return byteArray->hex();
                }

                const QString sep =
                    args[0]
                    .asString("hex")
                    ->toString();

                if (sep.size() != 1) {

                    throw std::runtime_error(
                        "ValueError: sep must be length 1"
                    );
                }

                if (args.size() == 1) {
                    return byteArray->hex(sep, std::nullopt);
                }

                return byteArray->hex(sep, args[1].toBigInt());
            }
        );
    }

    Value makeFromHexMethod(const Value&) {

        return makeBuiltin(
            "fromhex",

            [](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "fromhex");

                return ByteArrayValue::makeTrans(args);
            }
        );
    }

    Value makeDecodeMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "decode",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 0, 2, "decode");

                QString encoding = "utf-8";

                QString errors = "strict";

                if (args.size() >= 1) {

                    encoding =
                        args[0]
                        .asString("decode")
                        ->toString();
                }

                if (args.size() >= 2) {

                    errors =
                        args[1]
                        .asString("decode")
                        ->toString();
                }

                return byteArray->decode(encoding, errors);
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

                expectArgs(args, 2, "maketrans");

                return ByteArrayValue::makeTrans(args);
            }
        );
    }

    Value makeTranslateMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "translate",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "translate");

                std::optional<Value> deleteBytes;

                if (args.size() == 2) {
                    deleteBytes = args[1];
                }

                return byteArray->translate(args[0], deleteBytes);
            }
        );
    }

    Value makeDelItemMethod(const Value& obj) {

        auto byteArray = extract<Value::ByteArrayPtr>(obj);

        return makeBuiltin(
            "__delitem__",

            [byteArray](
                const std::vector<Value>& args,
                const Kwargs&,
                const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__delitem__");

                return byteArray->delItem(args[0]);
            }
        );
    }

    const MethodMap BYTEARRAY_METHODS = {
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::ByteArrayPtr>),
        REGISTER_METHOD("__getitem__", makeGetItemMethod),
        REGISTER_METHOD("__setitem__", makeSetItemMethod),
        REGISTER_METHOD("__delitem__", makeDelItemMethod),
        REGISTER_METHOD("__iter__", makeIterMethod),
        REGISTER_METHOD("__add__", makeAddMethod),
        REGISTER_METHOD("__mul__", makeMultiplyMethod),
        REGISTER_METHOD("__iadd__", makeIAddMethod),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__eq__", makeEqMethod),
        REGISTER_METHOD("__ne__", makeNeMethod),
        REGISTER_METHOD("__lt__", makeLtMethod),
        REGISTER_METHOD("__le__", makeLeMethod),
        REGISTER_METHOD("__gt__", makeGtMethod),
        REGISTER_METHOD("__ge__", makeGeMethod),
        REGISTER_METHOD("find", makeFindMethod),
        REGISTER_METHOD("rfind", makeRfindMethod),
        REGISTER_METHOD("index", makeIndexMethod),
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("startswith", makeStartsWithMethod),
        REGISTER_METHOD("endswith", makeEndsWithMethod),
        REGISTER_METHOD("lstrip", makeLStripMethod),
        REGISTER_METHOD("rstrip", makeRStripMethod),
        REGISTER_METHOD("strip", makeStripMethod),
        REGISTER_METHOD("removeprefix", makeRemovePrefixMethod),
        REGISTER_METHOD("removesuffix",makeRemoveSuffixMethod),
        REGISTER_METHOD("replace", makeReplaceMethod),
        REGISTER_METHOD("split", makeSplitMethod),
        REGISTER_METHOD("rsplit", makeRSplitMethod),
        REGISTER_METHOD("partition", makePartitionMethod),
        REGISTER_METHOD("rpartition", makeRPartitionMethod),
        REGISTER_METHOD("center", makeCenterMethod),
        REGISTER_METHOD("ljust", makeLJustMethod),
        REGISTER_METHOD("rjust", makeRJustMethod),
        REGISTER_METHOD("zfill", makeZFillMethod),
        REGISTER_METHOD("lower", makeLowerMethod),
        REGISTER_METHOD("upper", makeUpperMethod),
        REGISTER_METHOD("swapcase", makeSwapCaseMethod),
        REGISTER_METHOD("capitalize", makeCapitalizeMethod),
        REGISTER_METHOD("title", makeTitleMethod),
        REGISTER_METHOD("islower", makeIsLowerMethod),
        REGISTER_METHOD("isupper", makeIsUpperMethod),
        REGISTER_METHOD("istitle", makeIsTitleMethod),
        REGISTER_METHOD("isascii", makeIsAsciiMethod),
        REGISTER_METHOD("isalpha", makeIsAlphaMethod),
        REGISTER_METHOD("isdigit", makeIsDigitMethod),
        REGISTER_METHOD("isalnum", makeIsAlnumMethod),
        REGISTER_METHOD("isspace", makeIsSpaceMethod),
        REGISTER_METHOD("expandtabs", makeExpandTabsMethod),
        REGISTER_METHOD("splitlines", makeSplitLinesMethod),
        REGISTER_METHOD("join", makeJoinMethod),
        REGISTER_METHOD("append", makeAppendMethod),
        REGISTER_METHOD("extend", makeExtendMethod),
        REGISTER_METHOD("insert", makeInsertMethod),
        REGISTER_METHOD("pop", makePopMethod),
        REGISTER_METHOD("remove", makeRemoveMethod),
        REGISTER_METHOD("clear", makeClearMethod),
        REGISTER_METHOD("copy", makeCopyMethod),
        REGISTER_METHOD("reverse", makeReverseMethod),
        REGISTER_METHOD("hex", makeHexMethod),
        REGISTER_METHOD("fromhex", makeFromHexMethod),
        REGISTER_METHOD("decode", makeDecodeMethod),
        REGISTER_METHOD("maketrans", makeMakeTransMethod),
        REGISTER_METHOD("translate", makeTranslateMethod)
    };

}

Value getByteArrayAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, BYTEARRAY_METHODS, "bytes");
}

Value make_byteArray_ClassBuiltin() {

    return makeBuiltin(
        "__bytes__",

        [](const std::vector<Value>& args,
           const Kwargs&,
           const std::shared_ptr<Environment>&)
        -> Value {

            expectArgs(args, 1, "bytearray.__bytes__");

            return args[0].asByteArray("__bytes__")->_bytes_();
        }
    );

}

Value makeByteArrayFromHexBuiltin() {

    return makeBuiltin(
        "fromhex",

        [](
            const std::vector<Value>& args,
            const Kwargs&,
            const std::shared_ptr<Environment>&)
        -> Value {

            expectArgs(args, 1, "fromhex");

            return ByteArrayValue::fromHex(args);
        }
    );
}

Value makeByteArrayMakeTransBuiltin() {

    return makeBuiltin(
        "maketrans",

        [](
            const std::vector<Value>& args,
            const Kwargs&,
            const std::shared_ptr<Environment>&)
        -> Value {

            expectArgs(args, 2, "maketrans");

            return ByteArrayValue::makeTrans(args);
        }
    );
}
