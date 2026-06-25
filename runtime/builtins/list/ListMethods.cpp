//
// Created by semyo on 24.05.2026.
//

#include "CallRuntime.h"
#include "ListValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../ProtocolHelpers.h"
#include "../../RuntimeUtils.h"

namespace {

    Value make_getitem_Method(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "__getitem__");

                return list->getItem(args[0]);
            }
        );
    }

    Value make_setitem_Method(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "__setitem__",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 2, "__setitem__");

                list->setItem(args[0], args[1]);

                return {};
            }
        );
    }

    Value makeEqualMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "__eq__",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(list->equal(args[0]));
            }
        );
    }

    Value makeNotEqualMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "__ne__",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(list->notEqual(args[0]));
            }
        );
    }

    Value makeAppendMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "append",

            [list](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 1, "append");

                list->append(args[0]);

                return {};
            }
        );
    }

    Value makePopMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "pop",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                if (args.empty()) {
                    return list->pop();
                }

                if (args.size() == 1) {
                    return list->pop(args[0]);
                }

                throw std::runtime_error("pop expects at most 1 arg");
            }
        );
    }

    Value makeExtendMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "extend",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "extend");

                list->extend(args[0]);

                return {};
            }
        );
    }

    Value makeInsertMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "insert",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 2, "insert");

                list->insert(args[0], args[1]);

                return {};
            }
        );
    }

    Value makeRemoveMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "remove",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "remove");

                list->remove(args[0]);

                return {};
            }
        );
    }

    Value makeClearMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "clear",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 0, "clear");

                list->clear();

                return {};
            }
        );
    }

    Value makeCountMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "count",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 1, "count");

                return list->count(args[0]);
            }
        );
    }

    Value makeIndexMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "index",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgsRange(args, 1, 3, "index");

                if (args.size() == 1) {
                    return list->index(args[0]);
                }

                if (args.size() == 2) {
                    return list->index(args[0], args[1]);
                }

                return list->index(args[0], args[1], args[2]);
            }
        );
    }

    Value makeReverseMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "reverse",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 0, "reverse");

                list->reverse();

                return {};
            }
        );
    }

    Value makeCopyMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "copy",

            [list](const std::vector<Value> &args,
                   const Kwargs &,
                   const std::shared_ptr<Environment> &) -> Value {

                expectArgs(args, 0, "copy");

                return list->copy();
            }
        );
    }

    Value makeSortMethod(const Value& obj) {

        auto list = extract<Value::ListPtr>(obj);

        return makeBuiltin(
            "sort",

            [list](const std::vector<Value> &args,
                   const Kwargs &kwargs,
                   const std::shared_ptr<Environment> &env) -> Value {

                std::optional<Value> key = std::nullopt;

                bool reverse = false;

                for (const auto &[name, value]: kwargs) {

                    if (name == "key") {
                        key = value;
                    } else if (name == "reverse") {
                        reverse = value.toBool();
                    } else {
                        throw std::runtime_error("Unknown keyword argument: " + name.toStdString());
                    }
                }

                expectArgsRange(args, 0, 2, "sort");

                // sort(key)
                // sort(reverse)
                if (!args.empty()) {
                    // если callable, то это key
                    if (args[0].isCallable()) {
                        key = args[0];
                    } else {
                        reverse = args[0].toBool();
                    }
                }

                // sort(key, reverse)
                if (args.size() == 2) {
                    key = args[0];
                    reverse = args[1].toBool();
                }

                list->sort(key, reverse, env);

                return {};
            }
        );
    }

    const MethodMap LIST_METHODS = {
        REGISTER_METHOD("__getitem__", make_getitem_Method),
        REGISTER_METHOD("__setitem__", make_setitem_Method),
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::ListPtr>),
        REGISTER_METHOD("__eq__", makeEqualMethod),
        REGISTER_METHOD("__ne__", makeNotEqualMethod),
        REGISTER_METHOD("append", makeAppendMethod),
        REGISTER_METHOD("pop", makePopMethod),
        REGISTER_METHOD("extend", makeExtendMethod),
        REGISTER_METHOD("insert", makeInsertMethod),
        REGISTER_METHOD("remove", makeRemoveMethod),
        REGISTER_METHOD("clear", makeClearMethod),
        REGISTER_METHOD("count", makeCountMethod),
        REGISTER_METHOD("index", makeIndexMethod),
        REGISTER_METHOD("reverse", makeReverseMethod),
        REGISTER_METHOD("copy", makeCopyMethod),
        REGISTER_METHOD("sort", makeSortMethod)
    };
}

Value getListAttr(const Value &obj, const QString &attr) {

    return getBuiltinAttr(obj, attr, LIST_METHODS, "list");
}