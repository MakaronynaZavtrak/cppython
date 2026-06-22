//
// Created by semyo on 16.05.2026.
//
#include "DictValue.h"
#include <Value.h>

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "DictItemsView.h"
#include "DictKeysView.h"
#include "DictValuesView.h"
#include "StopIterationException.h"
#include "TupleValue.h"

DictValue:: DictValue(const QHash<Value, Value>& items,
                      const QVector<Value>& order)
        : elements(items),
          order(order) {}

QString DictValue::toString() const {

      QString out = "{";

      bool first = true;

      for (const auto& key : order) {

            if (!first) {
                  out += ", ";
            }

            first = false;

            out += key.repr();
            out += ": ";
            out += elements[key].repr();
      }

      out += "}";

      return out;
}

QString DictValue::repr() const {
      return toString();
}

Value DictValue::getItem(const Value& key) const {

      if (!key.isHashable()) {
            throw std::runtime_error("TypeError: unhashable type");
      }

      if (!elements.contains(key)) {
            throw std::runtime_error("KeyError: " + key.toString().toStdString());
      }

      return elements[key];
}

void DictValue::setItem(const Value &key, const Value &value) {

      if (!key.isHashable()) {
            throw std::runtime_error("TypeError: unhashable type");
      }

      if (!elements.contains(key))
            order.push_back(key);

      elements[key] = value;
}

Value DictValue::get(const Value &key, const Value &defaultValue) const {

      if (!key.isHashable()) {
            throw std::runtime_error("TypeError: unhashable type");
      }

      if (!elements.contains(key)) {
            return defaultValue;
      }

      return elements[key];
}

std::size_t DictValue::len() const {
      return elements.size();
}

void DictValue::clear() {
      elements.clear();
      order.clear();
}

Value DictValue::copy() const {
      return Value(std::make_shared<DictValue>(elements, order));
}

Value DictValue::pop(const Value& key, const Value* defaultValue) {

      if (elements.contains(key)) {

            Value result = elements[key];
            elements.remove(key);
            order.removeAll(key);

            return result;
      }

      if (defaultValue) {
            return *defaultValue;
      }

      throw std::runtime_error("KeyError: " + key.toString().toStdString());
}

void DictValue::update(const std::shared_ptr<DictValue>& other) {
      for (const auto& key : other->order) {

            if (!elements.contains(key)) {
                  order.push_back(key);
            }

            elements[key] = other->elements[key];
      }
}

Value DictValue::setdefault(const Value& key, const Value& defaultValue) {

      if (elements.contains(key)) {
            return elements[key];
      }

      elements[key] = defaultValue;
      order.push_back(key);

      return defaultValue;
}

Value DictValue::popitem() {

      if (order.empty()) {
            throw std::runtime_error("KeyError: 'popitem(): dictionary is empty'");
      }

      const Value key = order.back();
      order.pop_back();

      const Value value = elements[key];

      elements.remove(key);

      std::vector tupleItems = {
            Value(key),
            value
        };

      return Value(std::make_shared<TupleValue>(tupleItems));
}

QVector<Value> DictValue::getOrder() const {
      return order;
}

QHash<Value, Value> DictValue::getElements() const {
      return elements;
}

const QHash<Value, Value>& DictValue::getElementsRef() const {
      return elements;
}

Value DictValue::keys(const std::shared_ptr<DictValue>& self) {
      return Value(std::make_shared<DictKeysView>(self));
}

Value DictValue::values(const std::shared_ptr<DictValue>& self) {
      return Value(std::make_shared<DictValuesView>(self));
}

Value DictValue::items(const std::shared_ptr<DictValue>& self) {
      return Value(std::make_shared<DictItemsView>(self));
}

Value DictValue::fromKeys(
    const Value& iterable,
    const std::optional<Value>& defaultValue
) {

      auto result = std::make_shared<DictValue>();

      Value value = defaultValue.has_value()
          ? *defaultValue
          : Value();

      Value iterMethod = getAttrValue(iterable, "__iter__");

      Value iterator = call(iterMethod, {}, {}, nullptr);

      while (true) {

            try {

                  Value nextMethod = getAttrValue(
                      iterator,
                      "__next__"
                  );

                  Value key = call(
                      nextMethod,
                      {},
                      {},
                      nullptr
                  );

                  result->setItem(key, value);

            } catch (const StopIterationException&) {
                  break;
            }
      }

      return Value(result);
}

Value DictValue::bitOr(const Value& other) const {

      if (!other.isDict()) {
            throw std::runtime_error(
                "TypeError: unsupported operand type(s) for |"
            );
      }

      const auto result =
          std::make_shared<DictValue>(
              elements,
              order
          );

      result->update(other.asDict());

      return Value(result);
}

Value DictValue::ior(const Value& other) {

      if (!other.isDict()) {
            throw std::runtime_error(
                "TypeError: unsupported operand type(s) for |="
            );
      }

      update(other.asDict());

      return Value(
          shared_from_this()
      );
}

bool DictValue::equal(const Value& other) const {

      if (!other.isDict()) {
            return false;
      }

      const auto rhs = other.asDict();

      if (elements.size() != rhs->elements.size()) {
            return false;
      }

      for (const auto& key : order) {

            if (!rhs->elements.contains(key)) {
                  return false;
            }

            if (elements[key] != rhs->elements[key]) {
                  return false;
            }
      }

      return true;
}

bool DictValue::notEqual(const Value& other) const {
      return !equal(other);
}

Value DictValue::ror(const Value& other) const {

      if (!other.isDict()) {
            throw std::runtime_error(
                "TypeError: unsupported operand type(s) for |"
            );
      }

      return other.asDict()->bitOr(
          Value(
              std::const_pointer_cast<DictValue>(
                  shared_from_this()
              )
          )
      );
}

bool DictValue::contains(const Value& key) const {

      if (!key.isHashable()) {
            throw std::runtime_error("TypeError: unhashable type");
      }

      return elements.contains(key);
}
