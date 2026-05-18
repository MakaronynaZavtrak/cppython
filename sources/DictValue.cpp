//
// Created by semyo on 16.05.2026.
//
#include "DictValue.h"
#include <Value.h>

#include "TupleValue.h"

DictValue:: DictValue(const QHash<QString, Value>& items,
                      const QVector<QString>& order)
        : items(items),
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
            out += items[key].toString();
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

      if (!items.contains(strKey)) {
            throw std::runtime_error("KeyError: " + strKey.toStdString());
      }

      return items[strKey];
}

void DictValue::setItem(const Value &key, const Value &value) {

      //TODO: пока только строки
      if (!std::holds_alternative<QString>(key.data)) {
            throw std::runtime_error("Dict key must be string");
      }

      const auto strKey = std::get<QString>(key.data);

      if (!items.contains(strKey))
            order.push_back(strKey);

      items[strKey] = value;
}

Value DictValue::get(const Value &key, const Value &defaultValue) const {
      //TODO: пока только строки
      if (!std::holds_alternative<QString>(key.data)) {
            throw std::runtime_error("Dict key must be string");
      }

      const QString strKey = std::get<QString>(key.data);

      if (!items.contains(strKey)) {
            return defaultValue;
      }

      return items[strKey];
}

std::size_t DictValue::len() const {
      return items.size();
}

void DictValue::clear() {
      items.clear();
      order.clear();
}

Value DictValue::copy() const {
      return Value(std::make_shared<DictValue>(items, order));
}

Value DictValue::pop(const QString& key, const Value* defaultValue) {
      if (items.contains(key)) {
            Value result = items[key];
            items.remove(key);
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

            if (!items.contains(key)) {
                  order.push_back(key);
            }

            items[key] = other->items[key];
      }
}

Value DictValue::setdefault(const QString& key, const Value& defaultValue) {

      if (items.contains(key)) {
            return items[key];
      }

      items[key] = defaultValue;
      order.push_back(key);

      return defaultValue;
}

Value DictValue::popitem() {

      if (order.empty()) {
            throw std::runtime_error("KeyError: 'popitem(): dictionary is empty'");
      }

      const QString key = order.back();
      order.pop_back();

      const Value value = items[key];

      items.remove(key);

      std::vector tupleItems = {
            Value(key),
            value
        };

      return Value(std::make_shared<TupleValue>(tupleItems));
}

