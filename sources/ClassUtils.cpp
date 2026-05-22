//
// Created by semyo on 03.05.2026.
//
#include "ClassUtils.h"

#include "CallRuntime.h"
#include "ClassValue.h"
#include "DescriptorUtils.h"
#include "DictItemsIterator.h"
#include "DictItemsView.h"
#include "DictKeysIterator.h"
#include "DictKeysView.h"
#include "DictValue.h"
#include "DictValuesIterator.h"
#include "DictValuesView.h"
#include "InstanceValue.h"
#include "ListIterator.h"
#include "ListValue.h"
#include "SetValue.h"
#include "StopIterationException.h"
#include "SuperValue.h"
#include "TupleIterator.h"
#include "TupleValue.h"

bool hasAttr(const Value::ClassPtr& cls, const QString& attr) {
    try {
        findAttrInHierarchy(cls, attr);
        return true;
    } catch (...) {
        return false;
    }
}

Value genericGetAttr(const Value& obj, const QString& attr) {

    // instance
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        auto instance = std::get<Value::InstancePtr>(obj.data);
        auto cls = instance->klass;

        //  1. data descriptor (hasSet)
        try {
            Value val = findAttrInHierarchy(cls, attr);

            if (DescriptorUtils::hasGet(val) && DescriptorUtils::hasSet(val)) {
                return DescriptorUtils::callGet(val, Value(instance), cls);
            }
        } catch (const std::runtime_error& e) {

            const std::string msg = e.what();

            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
        }

        // 2. instance fields
        if (instance->fields.contains(attr)) {
            return instance->fields[attr];
        }

        // 3. non-data descriptor | class attribute
        try {
            Value val = findAttrInHierarchy(cls, attr);

            if (DescriptorUtils::hasGet(val)) {
                return DescriptorUtils::callGet(val, Value(instance), cls);
            }

            return val;
        } catch (const std::runtime_error& e) {

            const std::string msg = e.what();

            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
        }
    }

    // super
    if (std::holds_alternative<Value::SuperPtr>(obj.data)) {
        auto super = std::get<Value::SuperPtr>(obj.data);
        return getAttrFromSuper(super, attr);
    }

    // class
    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        auto cls = std::get<Value::ClassPtr>(obj.data);

        Value val = findAttrInHierarchy(cls, attr);

        if (DescriptorUtils::hasGet(val)) {
            return DescriptorUtils::callGet(val, Value(), cls);
        }

        return val;
    }

    if (std::holds_alternative<Value::ListPtr>(obj.data)) {

        auto list = std::get<Value::ListPtr>(obj.data);

        if (attr == "__getitem__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__getitem__",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("__getitem__ expects 1 arg");
                        }

                        return list->getItem(args[0]);
                    }
                )
            );
        }

        if (attr == "__setitem__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__setitem__",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                          -> Value {

                        if (args.size() != 2) {
                            throw std::runtime_error("__setitem__ expects 2 args");
                        }

                        list->setItem(args[0], args[1]);

                        return {};
                    }
                )
            );
        }

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [list](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(std::make_shared<ListIterator>(list));
                    }
                )
            );
        }

        if (attr == "append") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "append",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                          -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("append expects 1 argument");
                        }

                        list->append(args[0]);

                        return {};
                    }
                )
            );
        }

        if (attr == "pop") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "pop",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.empty()) {
                            return list->pop();
                        }

                        if (args.size() == 1) {
                            return list->pop(args[0]);
                        }

                        throw std::runtime_error("pop expects at most 1 arg");
                    }
                )
            );
        }

        if (attr == "__len__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__len__",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__len__ expects 0 args");
                        }

                        return Value(Value::BigInt(list->len()));
                    }
                )
            );
        }

        if (attr == "extend") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "extend",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("extend expects 1 arg");
                        }

                        list->extend(args[0]);

                        return {};
                    }
                )
            );
        }

        if (attr == "insert") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "insert",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 2) {
                            throw std::runtime_error("insert expects 2 args");
                        }

                        list->insert(args[0], args[1]);

                        return {};
                    }
                )
            );
        }

        if (attr == "remove") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "remove",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("remove expects 1 arg");
                        }

                        list->remove(args[0]);

                        return {};
                    }
                )
            );
        }

        if (attr == "clear") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "clear",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("clear expects 0 args");
                        }

                        list->clear();

                        return {};
                    }
                )
            );
        }

        if (attr == "count") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "count",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("count expects 1 arg");
                        }

                        return list->count(args[0]);
                    }
                )
            );
        }

        if (attr == "index") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "index",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.empty() || args.size() > 3) {
                            throw std::runtime_error("index expects 1-3 args");
                        }

                        if (args.size() == 1) {
                            return list->index(args[0]);
                        }

                        if (args.size() == 2) {
                            return list->index(
                                args[0],
                                args[1]
                            );
                        }

                        return list->index(
                            args[0],
                            args[1],
                            args[2]
                        );
                    }
                )
            );
        }

        if (attr == "reverse") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "reverse",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("reverse expects 0 args");
                        }

                        list->reverse();

                        return {};
                    }
                )
            );
        }

        if (attr == "copy") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "copy",

                    [list](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("copy expects 0 args");
                        }

                        return list->copy();
                    }
                )
            );
        }

        if (attr == "sort") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "sort",

                    [list](const std::vector<Value>& args,
                                const Kwargs& kwargs,
                                const std::shared_ptr<Environment>& env)
                           -> Value {

                        std::optional<Value> key = std::nullopt;
                        bool reverse = false;

                        for (const auto& [name, value] : kwargs) {

                            if (name == "key") {
                                key = value;
                            }
                            else if (name == "reverse") {
                                reverse = value.toBool();
                            }
                            else {
                                throw std::runtime_error("Unknown keyword argument: "+ name.toStdString());
                            }
                        }

                        if (args.size() > 2) {
                            throw std::runtime_error("sort expects at most 2 args");
                        }

                        // sort(key)
                        // sort(reverse)
                        if (!args.empty()) {

                            // если callable, то это key
                            if (args[0].isCallable()) {
                                key = args[0];
                            }
                            else {
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
                )
            );
        }
    }

    if (std::holds_alternative<Value::DictPtr>(obj.data)) {

        auto dict = std::get<Value::DictPtr>(obj.data);

        if (attr == "__getitem__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__getitem__",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                    -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("__getitem__ expects 1 arg");
                        }

                        return dict->getItem(args[0]);
                    }
                )
            );
        }

        if (attr == "__setitem__") {
            return Value(
                std::make_shared<BuiltinFunction>(
                    "__setitem__",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                    -> Value {

                        if (args.size() != 2) {
                            throw std::runtime_error("__setitem__ expects 2 args");
                        }

                        dict->setItem(args[0], args[1]);

                        return {};
                    }
                )
            );
        }

        if (attr == "__len__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__len__",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                    -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__len__ expects 0 args");
                        }

                        return Value(Value::BigInt(dict->len()));
                    }
                )
            );
        }

        if (attr == "get") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "get",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                    -> Value {

                        if (args.empty() || args.size() > 2) {
                            throw std::runtime_error("get expects 1 or 2 args");
                        }

                        if (args.size() == 1) {
                            return dict->get(args[0]);
                        }

                        return dict->get(args[0], args[1]);
                    }
                )
            );
        }

        if (attr == "clear") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "clear",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                    -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("clear expects 0 args");
                        }

                        dict->clear();

                        return {};
                    }
                )
            );
        }

        if (attr == "copy") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "copy",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("copy expects 0 args");
                        }

                        return dict->copy();
                    }
                )
            );
        }

        if (attr == "pop") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "pop",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.empty() || args.size() > 2) {
                            throw std::runtime_error("pop expects 1 or 2 args");
                        }

                        const Value& key = args[0];

                        if (args.size() == 2) {
                            return dict->pop(key, &args[1]);
                        }

                        return dict->pop(key);
                    }
                )
            );
        }

        if (attr == "update") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "update",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("update expects 1 arg");
                        }

                        if (!args[0].isDict()) {
                            throw std::runtime_error("update expects dict");
                        }

                        dict->update(args[0].asDict());

                        return {};
                    }
                )
            );
        }

        if (attr == "setdefault") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "setdefault",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (args.size() != 1 && args.size() != 2) {
                            throw std::runtime_error("setdefault expects 1 or 2 args");
                        }

                        const Value& key = args[0];

                        Value defaultValue;

                        if (args.size() == 2) {
                            defaultValue = args[1];
                        }

                        return dict->setdefault(key, defaultValue);
                    }
                )
            );
        }

        if (attr == "popitem") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "popitem",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("popitem expects 0 args");
                        }

                        return dict->popitem();
                    }
                )
            );
        }

        if (attr == "keys") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "keys",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("keys expects 0 args");
                        }

                        return DictValue::keys(dict);
                    }
                )
            );
        }

        if (attr == "values") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "values",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("values expects 0 args");
                        }

                        return DictValue::values(dict);
                    }
                )
            );
        }

        if (attr == "items") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "items",

                    [dict](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("items expects 0 args");
                        }

                        return DictValue::items(dict);
                    }
                )
            );
        }

        if (attr == "fromkeys") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "fromkeys",

                    [](const std::vector<Value>& args,
                       const Kwargs&,
                       const std::shared_ptr<Environment>& env)
                       -> Value {

                        if (args.empty() || args.size() > 2) {
                            throw std::runtime_error("fromkeys expects 1 or 2 args");
                        }

                        std::optional<Value> defaultValue;

                        if (args.size() == 2) {
                            defaultValue = args[1];
                        }

                        return DictValue::fromKeys(args[0], defaultValue);
                    }
                )
            );
        }
    }

    if (std::holds_alternative<Value::DictKeysViewPtr>(obj.data)) {

        auto keysView = std::get<Value::DictKeysViewPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [keysView](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(std::make_shared<DictKeysIterator>(keysView->getDict()));
                    }
                )
            );
        }

    }

    if (std::holds_alternative<Value::DictValuesViewPtr>(obj.data)) {

        auto valuesView = std::get<Value::DictValuesViewPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [valuesView](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(std::make_shared<DictValuesIterator>(valuesView->getDict()));
                    }
                )
            );
        }

    }

    if (std::holds_alternative<Value::DictItemsViewPtr>(obj.data)) {

        auto itemsView = std::get<Value::DictItemsViewPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [itemsView](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(std::make_shared<DictItemsIterator>(itemsView->getDict()));
                    }
                )
            );
        }

    }

    if (std::holds_alternative<Value::TuplePtr>(obj.data)) {

        auto tuple = std::get<Value::TuplePtr>(obj.data);

        if (attr == "__getitem__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__getitem__",

                    [tuple](const std::vector<Value>& args,
                            const Kwargs&,
                            const std::shared_ptr<Environment>&)
                            -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("__getitem__ expects 1 arg");
                        }

                        return tuple->getItem(args[0]);
                    }
                )
            );
        }

        if (attr == "__setitem__") {
            throw std::runtime_error("TypeError: 'tuple' object does not support item assignment");
        }

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [tuple](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(std::make_shared<TupleIterator>(tuple));
                    }
                )
            );
        }

        if (attr == "count") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "count",

                    [tuple](const std::vector<Value>& args,
                            const Kwargs&,
                            const std::shared_ptr<Environment>&)
                            -> Value {

                        if (args.size() != 1) {
                            throw std::runtime_error("count expects 1 arg");
                        }

                        return tuple->count(args[0]);
                    }
                )
            );
        }

        if (attr == "index") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "index",

                    [tuple](const std::vector<Value>& args,
                            const Kwargs&,
                            const std::shared_ptr<Environment>&)
                            -> Value {

                        if (args.empty() || args.size() > 3) {
                            throw std::runtime_error("index expects 1-3 args");
                        }

                        std::optional<Value> start;
                        std::optional<Value> end;

                        if (args.size() >= 2) {
                            start = args[1];
                        }

                        if (args.size() >= 3) {
                            end = args[2];
                        }

                        return tuple->index(
                            args[0],
                            start,
                            end
                        );
                    }
                )
            );
        }

        if (attr == "__len__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__len__",

                    [tuple](const std::vector<Value>& args,
                            const Kwargs&,
                            const std::shared_ptr<Environment>&)
                            -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__len__ expects 0 args");
                        }

                        return Value(
                            Value::BigInt(tuple->len())
                        );
                    }
                )
            );
        }

    }

    if (obj.isSet()) {

        auto set = std::get<Value::SetPtr>(obj.data);

        if (attr == "add") {

            return Value(std::make_shared<BuiltinFunction>(
                "add",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (args.size() != 1) {
                        throw std::runtime_error("set.add() takes exactly one argument");
                    }

                    set->add(args[0]);

                    return {};
                }
            ));
        }

        if (attr == "remove") {

            return Value(std::make_shared<BuiltinFunction>(
                "remove",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (args.size() != 1) {
                        throw std::runtime_error("set.remove() takes exactly one argument");
                    }

                    set->remove(args[0]);

                    return {};
                }
            ));
        }

        if (attr == "discard") {

            return Value(std::make_shared<BuiltinFunction>(
                "discard",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (args.size() != 1) {
                        throw std::runtime_error("set.discard() takes exactly one argument");
                    }

                    set->discard(args[0]);

                    return {};
                }
            ));
        }

        if (attr == "union") {

            return Value(std::make_shared<BuiltinFunction>(
                "union",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    auto result = set->copy();

                    for (const auto &arg: args) {

                        const auto other = arg.asSet();

                        if (!other) {
                            throw std::runtime_error("union() argument must be set");
                        }

                        result = result->unionWith(other);
                    }

                    return Value(result);
                }
            ));
        }

        if (attr == "intersection") {

            return Value(std::make_shared<BuiltinFunction>(
                "intersection",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    auto result = set->copy();

                    for (const auto &arg: args) {

                        const auto other = arg.asSet();

                        if (!other) {
                            throw std::runtime_error("intersection() argument must be set");
                        }

                        result = result->intersectionWith(other);
                    }

                    return Value(result);
                }
            ));
        }

        if (attr == "difference") {

            return Value(std::make_shared<BuiltinFunction>(
                "difference",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    auto result = set->copy();

                    for (const auto &arg: args) {

                        const auto other = arg.asSet();

                        if (!other) {
                            throw std::runtime_error("difference() argument must be set");
                        }

                        result = result->differenceWith(other);
                    }

                    return Value(result);
                }
            ));
        }

        if (attr == "symmetric_difference") {

            return Value(std::make_shared<BuiltinFunction>(

            "symmetric_difference",
            [set](const std::vector<Value> &args,
                  const Kwargs &,
                  const std::shared_ptr<Environment> &) -> Value {

                    if (args.size() != 1) {
                        throw std::runtime_error("set.symmetric_difference() takes exactly one argument");
                    }

                    const auto other = args[0].asSet();

                    if (!other) {
                        throw std::runtime_error("symmetric_difference() argument must be set");
                    }

                    return Value(set->symmetricDifferenceWith(other));
                }
            ));
        }

        if (attr == "issubset") {

            return Value(std::make_shared<BuiltinFunction>(
                "issubset",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (args.size() != 1) {
                        throw std::runtime_error("set.issubset() takes exactly one argument");
                    }

                    const auto other = args[0].asSet();

                    if (!other) {
                        throw std::runtime_error("issubset() argument must be set");
                    }

                    return Value(set->isSubsetOf(other));
                }
            ));
        }

        if (attr == "issuperset") {

            return Value(std::make_shared<BuiltinFunction>(
                "issuperset",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (args.size() != 1) {
                        throw std::runtime_error("set.issuperset() takes exactly one argument");
                    }

                    const auto other = args[0].asSet();

                    if (!other) {
                        throw std::runtime_error("issuperset() argument must be set");
                    }

                    return Value(set->isSupersetOf(other));
                }
            ));
        }

        if (attr == "isdisjoint") {

            return Value(std::make_shared<BuiltinFunction>(
                "isdisjoint",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (args.size() != 1) {
                        throw std::runtime_error("set.isdisjoint() takes exactly one argument");
                    }

                    const auto other = args[0].asSet();

                    if (!other) {
                        throw std::runtime_error("isdisjoint() argument must be set");
                    }

                    return Value(set->isDisjointWith(other));
                }
            ));
        }

        if (attr == "copy") {

            return Value(std::make_shared<BuiltinFunction>(
                "copy",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (!args.empty()) {
                        throw std::runtime_error("set.copy() takes no arguments");
                    }

                    return Value(set->copy());
                }
            ));
        }

        if (attr == "clear") {

            return Value(std::make_shared<BuiltinFunction>(
                "clear",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (!args.empty()) {
                        throw std::runtime_error("set.clear() takes no arguments");
                    }

                    set->clear();

                    return {};
                }
            ));
        }

        if (attr == "pop") {

            return Value(std::make_shared<BuiltinFunction>(
                "pop",
                [set](const std::vector<Value> &args,
                      const Kwargs &,
                      const std::shared_ptr<Environment> &) -> Value {

                    if (!args.empty()) {
                        throw std::runtime_error("set.pop() takes no arguments");
                    }

                    return set->pop();
                }
            ));
        }

    }


    if (std::holds_alternative<Value::ListIteratorPtr>(obj.data)) {

        auto iter = std::get<Value::ListIteratorPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [iter](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(iter);
                    }
                )
            );
        }

        if (attr == "__next__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__next__",

                    [iter](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__next__ expects 0 args");
                        }

                        if (iter->index >= iter->list->elements.size()) {
                            throw StopIterationException();
                        }

                        return iter->list->elements[iter->index++];
                    }
                )
            );
        }
    }

    if (std::holds_alternative<Value::TupleIteratorPtr>(obj.data)) {

        auto iter = std::get<Value::TupleIteratorPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [iter](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(iter);
                    }
                )
            );
        }

        if (attr == "__next__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__next__",

                    [iter](const std::vector<Value>& args,
                           const Kwargs&,
                           const std::shared_ptr<Environment>&)
                           -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__next__ expects 0 args");
                        }

                        if (iter->index >= iter->tuple->items.size()) {
                            throw StopIterationException();
                        }

                        return iter->tuple->items[iter->index++];
                    }
                )
            );
        }
    }

    if (std::holds_alternative<Value::DictKeysIteratorPtr>(obj.data)) {

        auto keysIter = std::get<Value::DictKeysIteratorPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [keysIter](const std::vector<Value>& args,
                               const Kwargs&,
                               const std::shared_ptr<Environment>&)
                               -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(keysIter);
                    }
                )
            );
        }

        if (attr == "__next__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__next__",

                    [keysIter](const std::vector<Value>& args,
                               const Kwargs&,
                               const std::shared_ptr<Environment>&)
                               -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__next__ expects 0 args");
                        }

                        return keysIter->next();
                    }
                )
            );
        }
    }

    if (std::holds_alternative<Value::DictValuesIteratorPtr>(obj.data)) {

        auto valuesIter = std::get<Value::DictValuesIteratorPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [valuesIter](const std::vector<Value>& args,
                                 const Kwargs&,
                                 const std::shared_ptr<Environment>&)
                                 -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(valuesIter);
                    }
                )
            );
        }

        if (attr == "__next__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__next__",

                    [valuesIter](const std::vector<Value>& args,
                                 const Kwargs&,
                                 const std::shared_ptr<Environment>&)
                                 -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__next__ expects 0 args");
                        }

                        return valuesIter->next();
                    }
                )
            );
        }
    }

    if (std::holds_alternative<Value::DictItemsIteratorPtr>(obj.data)) {

        auto itemsIter = std::get<Value::DictItemsIteratorPtr>(obj.data);

        if (attr == "__iter__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__iter__",

                    [itemsIter](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                                -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__iter__ expects 0 args");
                        }

                        return Value(itemsIter);
                    }
                )
            );
        }

        if (attr == "__next__") {

            return Value(
                std::make_shared<BuiltinFunction>(
                    "__next__",

                    [itemsIter](const std::vector<Value>& args,
                                const Kwargs&,
                                const std::shared_ptr<Environment>&)
                                -> Value {

                        if (!args.empty()) {
                            throw std::runtime_error("__next__ expects 0 args");
                        }

                        return itemsIter->next();
                    }
                )
            );
        }

    }

    throw std::runtime_error("AttributeError: object has no attribute '" +
                            attr.toStdString() + "'");
}

Value getAttrValue(const Value& obj, const QString& attr) {

    // super bypasses __getattribute__
    if (std::holds_alternative<Value::SuperPtr>(obj.data)) {
        return genericGetAttr(obj, attr);
    }

    // instance/class custom __getattribute__
    try {

        Value getattribute = genericGetAttr(obj, "__getattribute__");

        // builtin object.__getattribute__

        bool isDefault = false;

        if (std::holds_alternative<Value::BuiltinFunctionPtr>(getattribute.data)) {

            const auto builtin =
                std::get<Value::BuiltinFunctionPtr>(getattribute.data);

            isDefault = (builtin->name == "__object_getattribute__");
        }

        if (!isDefault) {
            return call(getattribute, { Value(attr) }, {}, nullptr);
        }

    } catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        if (msg.find("AttributeError") == std::string::npos) {
            throw;
        }
    }

    // default lookup
    try {
        return genericGetAttr(obj, attr);
    }
    catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        // только если attr реально не найден
        if (msg.find("AttributeError") == std::string::npos) {
            throw;
        }
    }

    // __getattr__
    try {

        Value getattr = genericGetAttr(obj, "__getattr__");

        return call(getattr, { Value(attr) }, {}, nullptr);

    } catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        if (msg.find("AttributeError") == std::string::npos) {
            throw;
        }
    }

    throw std::runtime_error(
        "AttributeError: object has no attribute '" +
        attr.toStdString() + "'"
    );
}

Value getAttrFromSuper(const Value::SuperPtr& super, const QString& attr) {
    std::vector<Value::ClassPtr> mro;
    buildMRO(getObjectClass(super->receiver), mro);

    bool foundOrigin = false;

    for (const auto& cls : mro) {

        if (!foundOrigin) {
            if (cls == super->originClass) {
                foundOrigin = true;
            }

            continue;
        }

        if (cls->attributes.contains(attr)) {

            Value val = cls->attributes[attr];

            if (DescriptorUtils::hasGet(val)) {
                return DescriptorUtils::callGet(val, Value(super->receiver), cls);
            }

            return val;
        }
    }

    throw std::runtime_error("AttributeError: object has no attribute '" +
                            attr.toStdString() + "'");
}

void buildMRO(const Value::ClassPtr& cls, std::vector<Value::ClassPtr>& out) {
    out.push_back(cls);

    for (const auto& base : cls->bases) {
        buildMRO(base, out);
    }
}

Value::ClassPtr getObjectClass(const Value& obj)
{
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        return std::get<Value::InstancePtr>(obj.data)->klass;
    }

    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        return std::get<Value::ClassPtr>(obj.data);
    }

    throw std::runtime_error("super(): invalid receiver");
}

Value findAttrInHierarchy(const Value::ClassPtr& cls, const QString& attr) {
    if (cls->attributes.contains(attr)) {
        return cls->attributes[attr];
    }

    for (const auto& base : cls->bases) {
        try {
            return findAttrInHierarchy(base, attr);

        } catch (const std::runtime_error& e) {

            const std::string msg = e.what();

            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
        }
    }

    throw std::runtime_error("Attribute not found: " + attr.toStdString());
}

void genericSetAttr(const Value& obj, const QString& attr, const Value& value) {
    // instance
    if (std::holds_alternative<Value::InstancePtr>(obj.data)) {
        const auto instance = std::get<Value::InstancePtr>(obj.data);
        const auto cls = instance->klass;

        // 1. проверяем descriptor в классе
        try {
            const Value descr = findAttrInHierarchy(cls, attr);

            if (DescriptorUtils::hasSet(descr)) {
                DescriptorUtils::callSet(descr, Value(instance), cls, value);
                return;
            }
        } catch (const std::runtime_error& e) {
            const std::string msg = e.what();
            if (msg.find("Attribute not found") == std::string::npos) {
                throw;
            }
        }

        // 2. обычная запись в поля
        instance->fields[attr] = value;
        return;
    }

    // class
    if (std::holds_alternative<Value::ClassPtr>(obj.data)) {
        const auto cls = std::get<Value::ClassPtr>(obj.data);
        cls->attributes[attr] = value;
        return;
    }

    throw std::runtime_error("setattr: object has no attributes");
}

void setAttrValue(const Value& obj, const QString& attr, const Value& value) {

    // super bypass
    if (std::holds_alternative<Value::SuperPtr>(obj.data)) {
        genericSetAttr(obj, attr, value);
        return;
    }

    try {

        Value setattr =
            genericGetAttr(obj, "__setattr__");

        bool isDefault = false;

        if (std::holds_alternative<Value::BuiltinFunctionPtr>(
                setattr.data)) {

            const auto builtin =
                std::get<Value::BuiltinFunctionPtr>(
                    setattr.data);

            isDefault =
                (builtin->name == "__object_setattr__");
                }

        if (!isDefault) {

            call(setattr, { Value(attr), value }, {}, nullptr);

            return;
        }

    } catch (const std::runtime_error& e) {

        const std::string msg = e.what();

        if (msg.find("AttributeError") == std::string::npos) {
            throw;
        }
    }

    genericSetAttr(obj, attr, value);
}
