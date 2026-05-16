//
// Created by semyo on 16.05.2026.
//
#include "DictValue.h"
#include <Value.h>

DictValue::DictValue(QHash<QString, Value> values)
      : items(std::move(values)) {}

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
