//
// Created by semyo on 16.05.2026.
//
#include "DictValue.h"
#include <Value.h>

#include "DictItemsView.h"
#include "DictKeysView.h"
#include "DictValuesView.h"
#include "TupleValue.h"

DictValue:: DictValue(const QHash<QString, Value>& items,
                      const QVector<QString>& order)
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

            out += "'" + key + "'";
            out += ": ";
            out += elements[key].toString();
      }

      out += "}";

      return out;
}

QString DictValue::repr() const {
      return toString();
}

Value DictValue::getItem(const Value& key) const {
      // TODO: пока что поддерживаются только строки в качестве ключей
      if (!std::holds_alternative<QString>(key.data)) {
            throw std::runtime_error("Dict key must be string");
      }

      const QString strKey = std::get<QString>(key.data);

      if (!elements.contains(strKey)) {
            throw std::runtime_error("KeyError: " + strKey.toStdString());
      }

      return elements[strKey];
}

void DictValue::setItem(const Value &key, const Value &value) {

      //TODO: пока только строки
      if (!std::holds_alternative<QString>(key.data)) {
            throw std::runtime_error("Dict key must be string");
      }

      const auto strKey = std::get<QString>(key.data);

      if (!elements.contains(strKey))
            order.push_back(strKey);

      elements[strKey] = value;
}

Value DictValue::get(const Value &key, const Value &defaultValue) const {
      //TODO: пока только строки
      if (!std::holds_alternative<QString>(key.data)) {
            throw std::runtime_error("Dict key must be string");
      }

      const QString strKey = std::get<QString>(key.data);

      if (!elements.contains(strKey)) {
            return defaultValue;
      }

      return elements[strKey];
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

Value DictValue::pop(const QString& key, const Value* defaultValue) {
      if (elements.contains(key)) {
            Value result = elements[key];
            elements.remove(key);
            order.removeAll(key);

            return result;
      }

      if (defaultValue) {
            return *defaultValue;
      }

      throw std::runtime_error(
          QString("KeyError: '%1'")
              .arg(key)
              .toStdString()
      );
}

void DictValue::update(const std::shared_ptr<DictValue>& other) {
      for (const auto& key : other->order) {

            if (!elements.contains(key)) {
                  order.push_back(key);
            }

            elements[key] = other->elements[key];
      }
}

Value DictValue::setdefault(const QString& key, const Value& defaultValue) {

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

      const QString key = order.back();
      order.pop_back();

      const Value value = elements[key];

      elements.remove(key);

      std::vector tupleItems = {
            Value(key),
            value
        };

      return Value(std::make_shared<TupleValue>(tupleItems));
}

QVector<QString> DictValue::getOrder() const {
      return order;
}

QHash<QString, Value> DictValue::getElements() const {
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
