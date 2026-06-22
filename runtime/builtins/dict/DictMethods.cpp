//
// Created by semyo on 24.05.2026.
//

#include "CallRuntime.h"
#include "DictValue.h"
#include "../BuiltinAttrLookup.h"
#include "../BuiltinMethodRegistry.h"
#include "../../ArgValidation.h"
#include "../../ProtocolHelpers.h"
#include "../../RuntimeUtils.h"

namespace {

    Value make_getitem_Method(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__getitem__",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 1, "__getitem__");

                return dict->getItem(args[0]);
            }
        );
    }

    Value make_setitem_Method(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__setitem__",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 2, "__setitem__");

                dict->setItem(args[0], args[1]);

                return {};
            }
        );
    }

    Value makeContainsMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__contains__",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__contains__");

                return Value(
                    dict->contains(args[0])
                );
            }
        );
    }

    Value makeOrMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__or__",

            [dict](const std::vector<Value>& args,
                    const Kwargs&,
                    const std::shared_ptr<Environment>&)
                -> Value {

                expectArgs(args, 1, "__or__");

                return dict->bitOr(args[0]);
            }
        );
    }

    Value makeIOrMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__ior__",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__ior__");

                return dict->ior(args[0]);
            }
        );
    }

    Value makeROrMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__ror__",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__ror__");

                return dict->ror(args[0]);
            }
        );
    }

    Value makeEqualMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__eq__",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__eq__");

                return Value(
                    dict->equal(args[0])
                );
            }
        );
    }

    Value makeNotEqualMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "__ne__",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&) -> Value {

                expectArgs(args, 1, "__ne__");

                return Value(
                    dict->notEqual(args[0])
                );
            }
        );
    }

    Value makeGetMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "get",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
            -> Value {

                expectArgsRange(args, 1, 2, "get");

                if (args.size() == 1) {
                    return dict->get(args[0]);
                }

                return dict->get(args[0], args[1]);
            }
        );
    }

    Value makeClearMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "clear",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
            -> Value {

                expectArgs(args, 0, "clear");

                dict->clear();

                return {};
            }
        );
    }

    Value makeCopyMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "copy",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 0, "copy");

                return dict->copy();
            }
        );
    }

    Value makePopMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "pop",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgsRange(args, 1, 2, "pop");

                const Value& key = args[0];

                if (args.size() == 2) {
                    return dict->pop(key, &args[1]);
                }

                return dict->pop(key);
            }
        );
    }

    Value makeUpdateMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "update",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 1, "update");

                dict->update(args[0].asDict("update"));

                return {};
            }
        );
    }

    Value makeSetdefaultMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "setdefault",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgsRange(args, 1, 2, "setdefault");

                const Value& key = args[0];

                Value defaultValue;

                if (args.size() == 2) {
                    defaultValue = args[1];
                }

                return dict->setdefault(key, defaultValue);
            }
        );
    }

    Value makePopItemMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "popitem",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 0, "popitem");

                return dict->popitem();
            }
        );
    }

    Value makeKeysMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "keys",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 0, "keys");

                return DictValue::keys(dict);
            }
        );
    }

    Value makeValuesMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "values",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 0, "values");

                return DictValue::values(dict);
            }
        );
    }

    Value makeItemsMethod(const Value& obj) {

        auto dict = extract<Value::DictPtr>(obj);

        return makeBuiltin(
            "items",

            [dict](const std::vector<Value>& args,
                   const Kwargs&,
                   const std::shared_ptr<Environment>&)
                   -> Value {

                expectArgs(args, 0, "items");

                return DictValue::items(dict);
            }
        );
    }

    Value makeFromKeysMethod(const Value& obj) {

        return makeBuiltin(
            "fromkeys",

            [](const std::vector<Value>& args,
               const Kwargs&,
               const std::shared_ptr<Environment>&)
               -> Value {

                expectArgsRange(args, 1, 2, "fromKeys");

                std::optional<Value> defaultValue;

                if (args.size() == 2) {
                    defaultValue = args[1];
                }

                return DictValue::fromKeys(args[0], defaultValue);
            }
        );
    }

    const MethodMap DICT_METHODS = {
        REGISTER_METHOD("__iter__", makeIterMethodBuiltin),
        REGISTER_METHOD("__getitem__", make_getitem_Method),
        REGISTER_METHOD("__setitem__", make_setitem_Method),
        REGISTER_METHOD("__contains__", makeContainsMethod),
        REGISTER_METHOD("__len__", makeLenMethodBuiltin<Value::DictPtr>),
        REGISTER_METHOD("__or__", makeOrMethod),
        REGISTER_METHOD("__ior__", makeIOrMethod),
        REGISTER_METHOD("__ror__", makeROrMethod),
        REGISTER_METHOD("__eq__", makeEqualMethod),
        REGISTER_METHOD("__ne__", makeNotEqualMethod),
        REGISTER_METHOD("get", makeGetMethod),
        REGISTER_METHOD("pop", makePopMethod),
        REGISTER_METHOD("update", makeUpdateMethod),
        REGISTER_METHOD("setdefault", makeSetdefaultMethod),
        REGISTER_METHOD("popitem", makePopItemMethod),
        REGISTER_METHOD("clear", makeClearMethod),
        REGISTER_METHOD("keys", makeKeysMethod),
        REGISTER_METHOD("values", makeValuesMethod),
        REGISTER_METHOD("items", makeItemsMethod),
        REGISTER_METHOD("copy", makeCopyMethod),
        REGISTER_METHOD("fromkeys", makeFromKeysMethod)
    };

}

Value getDictAttr(const Value& obj, const QString& attr) {

    return getBuiltinAttr(obj, attr, DICT_METHODS, "dict");
}
