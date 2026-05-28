#include "Value.h"

#include "BoundMethod.h"
#include "ClassMethodValue.h"
#include "DictItemsIterator.h"
#include "DictItemsView.h"
#include "DictKeysIterator.h"
#include "DictKeysView.h"
#include "DictValue.h"
#include "DictValuesIterator.h"
#include "DictValuesView.h"
#include "FunctionValue.h"
#include "ListIterator.h"
#include "ListValue.h"
#include "PropertyValue.h"
#include "SetIterator.h"
#include "SetValue.h"
#include "StaticMethodValue.h"
#include "StringIterator.h"
#include "StrValue.h"
#include "TupleIterator.h"
#include "TupleValue.h"

Value::Value(const QString& str) : data(std::make_shared<StrValue>(str)) {}

Value::Value(const char *str) : data(std::make_shared<StrValue>(str)) {}

/**
 * Преобразует экземпляр `Value` в его строковое представление в зависимости от его типа.
 *
 * Этот метод обрабатывает следующие типы: `int`, `double`, `bool`, `QString`,
 * `ListPtr`, `DictPtr` и `FunctionPtr`. Для неподдерживаемых или неизвестных типов
 * возвращает "Unknown unsupported type".
 *
 * - Для `int`: возвращает целое число в виде строки.
 * - Для `double`: возвращает число с плавающей точкой в виде строки.
 * - Для `bool`: возвращает "True" или "False".
 * - Для `QString`: возвращает строку, заключенную в одинарные кавычки.
 * - Для `ListPtr`: возвращает "[...]".
 * - Для `DictPtr`: возвращает "{...}".
 * - Для `FunctionPtr`: возвращает "<function>".
 *
 * @return Строковое представление экземпляра `Value`.
 */
QString Value::toString() const {

    if (std::holds_alternative<BigInt>(data)) {
        return QString::fromStdString(std::get<BigInt>(data).convert_to<std::string>());
    }

    if (std::holds_alternative<BigFloat>(data)) {
        const auto& num = std::get<BigFloat>(data);

        std::string s = num.str(15);

        if (s.find('.') == std::string::npos &&
            s.find('e') == std::string::npos &&
            s.find('E') == std::string::npos)
        {
            s += ".0";
        }

        return QString::fromStdString(s);
    }

    if (std::holds_alternative<bool>(data)) {
        return std::get<bool>(data) ? "True" : "False";
    }

    if (isString()) {
        return std::get<StrPtr>(data)->toString();
    }

    if (isList()) {
        return std::get<ListPtr>(data)->toString();
    }

    if (isDict()) {
        return std::get<DictPtr>(data)->toString();
    }

    if (isTuple()) {
        return std::get<TuplePtr>(data)->toString();
    }

    if (std::holds_alternative<FunctionPtr>(data)) {
        return std::get<FunctionPtr>(data)->toString();
    }

    if (std::holds_alternative<ClassPtr>(data)) {
        return std::get<ClassPtr>(data)->toString();
    }

    if (std::holds_alternative<InstancePtr>(data)) {
        return std::get<InstancePtr>(data)->toString();
    }

    if (std::holds_alternative<BoundMethodPtr>(data)) {
        return std::get<BoundMethodPtr>(data)->toString();
    }

    if (std::holds_alternative<StaticMethodPtr>(data)) {
        if (std::holds_alternative<StaticMethodPtr>(data)) {
            return "<staticmethod>";
        }
        return std::get<StaticMethodPtr>(data)->toString();
    }

    if (std::holds_alternative<ClassMethodPtr>(data)) {
        return std::get<ClassMethodPtr>(data)->toString();
    }

    if (isDictKeysView()) {
        return std::get<DictKeysViewPtr>(data)->toString();
    }

    if (isDictValuesView()) {
        return std::get<DictValuesViewPtr>(data)->toString();
    }

    if (isDictItemsView()) {
        return std::get<DictItemsViewPtr>(data)->toString();
    }

    if (isSet()) {
        return std::get<SetPtr>(data)->toString();
    }

    if (std::holds_alternative<IteratorPtr>(data)) {
        return std::get<IteratorPtr>(data)->toString();
    }

    if (std::holds_alternative<std::monostate>(data)) {
        return "None";
    }

    return "Unknown unsupported type";
}

QString Value::repr() const {

    if (isString()) {
        return std::get<StrPtr>(data)->repr();
    }

    if (isList()) {
        return std::get<ListPtr>(data)->repr();
    }

    if (isDict()) {
        return std::get<DictPtr>(data)->repr();
    }

    if (isTuple()) {
        return std::get<TuplePtr>(data)->repr();
    }

    if (isDictKeysView()) {
        return std::get<DictKeysViewPtr>(data)->repr();
    }

    if (isDictValuesView()) {
        return std::get<DictValuesViewPtr>(data)->repr();
    }

    if (isDictItemsView()) {
        return std::get<DictItemsViewPtr>(data)->repr();
    }

    if (isSet()) {
        return std::get<SetPtr>(data)->repr();
    }

    return toString();
}

QString Value::replString() const {
    return repr();
}

/**
 * Преобразует экземпляр `Value` в булево значение в зависимости от его типа.
 *
 * Этот метод обрабатывает следующие типы: `int`, `double`, `bool`, `QString`,
 * `ListPtr`, `DictPtr` и `FunctionPtr`. Для неподдерживаемых типов выбрасывается исключение.
 *
 * - Для `int`: возвращает `true`, если значение не равно 0.
 * - Для `double`: возвращает `true`, если значение не равно 0.0.
 * - Для `bool`: возвращает значение булевой переменной.
 * - Для `QString`: возвращает `true`, если строка не пуста.
 * - Для `ListPtr`: возвращает `true`, если указатель на список не является `nullptr`.
 * - Для `DictPtr`: возвращает `true`, если указатель на словарь не является `nullptr`.
 * - Для `FunctionPtr`: возвращает `true`, если указатель на функцию не является `nullptr`.
 *
 * @return Булево представление экземпляра `Value`.
 * @throws std::runtime_error Если тип данных не поддерживается.
 */
bool Value::toBool() const {
    if (std::holds_alternative<BigInt>(data)) {
        return std::get<BigInt>(data) != 0;
    }

    if (std::holds_alternative<BigFloat>(data)) {
        return std::get<BigFloat>(data) != 0.0;
    }

    if (std::holds_alternative<bool>(data)) {
        return std::get<bool>(data);
    }

    if (std::holds_alternative<StrPtr>(data)) {
        return std::get<StrPtr>(data)->len() == 0;
    }

    if (std::holds_alternative<ListPtr>(data)) {
        return !std::get<ListPtr>(data)->elements.empty();
    }

    if (std::holds_alternative<DictPtr>(data)) {
        return std::get<DictPtr>(data) != nullptr;
    }

    if (std::holds_alternative<FunctionPtr>(data)) {
        return std::get<FunctionPtr>(data) != nullptr;
    }

    if (std::holds_alternative<ClassPtr>(data)) {
        return std::get<ClassPtr>(data) != nullptr;
    }

    if (std::holds_alternative<InstancePtr>(data)) {
        return std::get<InstancePtr>(data) != nullptr;
    }

    if (std::holds_alternative<BoundMethodPtr>(data)) {
        return std::get<BoundMethodPtr>(data) != nullptr;
    }

    if (std::holds_alternative<SuperPtr>(data)) {
        return std::get<SuperPtr>(data) != nullptr;
    }

    if (std::holds_alternative<BuiltinFunctionPtr>(data)) {
        return std::get<BuiltinFunctionPtr>(data) != nullptr;
    }

    if (std::holds_alternative<PropertyPtr>(data)) {
        return std::get<PropertyPtr>(data) != nullptr;
    }

    if (std::holds_alternative<StaticMethodPtr>(data)) {
        return std::get<StaticMethodPtr>(data) != nullptr;
    }

    if (std::holds_alternative<ClassMethodPtr>(data)) {
        return std::get<ClassMethodPtr>(data) != nullptr;
    }

    throw std::runtime_error("Unsupported type");
}

/**
 * Преобразует экземпляр `Value` в число с плавающей точкой (`double`), если это возможно.
 *
 * Этот метод обрабатывает следующие типы:
 * - `int`: возвращает значение целого числа в виде `double`.
 * - `double`: возвращает само значение.
 * - `bool`: возвращает `1.0` для `true` и `0.0` для `false`.
 *
 * Если `Value` содержит неподдерживаемый тип, выбрасывает исключение `std::runtime_error`.
 *
 * @return Числовое значение типа `double`.
 * @throws std::runtime_error если преобразование невозможно.
 */
Value::BigFloat Value::toBigFloat() const {
    if (std::holds_alternative<BigInt>(data)) {
        return BigFloat(std::get<BigInt>(data));
    }

    if (std::holds_alternative<BigFloat>(data)) {
        return std::get<BigFloat>(data);
    }

    if (std::holds_alternative<bool>(data)) {
        return std::get<bool>(data) ? BigFloat(1.0) : BigFloat(0.0);
    }

    throw std::runtime_error("Cannot convert to double");
}

Value::BigInt Value::toBigInt() const {
    if (std::holds_alternative<BigInt>(data))
        return std::get<BigInt>(data);

    if (std::holds_alternative<bool>(data))
        return std::get<bool>(data) ? BigInt(1) : BigInt(0);

    // сюда лучше не попадать, но на всякий:
    return BigInt(std::get<BigFloat>(data));
}

bool Value::isNone() const {
    return std::holds_alternative<std::monostate>(data);;
}

bool Value::operator==(const Value& other) const {

    // числовое сравнение
    if (isNumeric() && other.isNumeric()) {
        return toBigFloat() == other.toBigFloat();
    }

    // string
    if (isString() && other.isString()) {
        return std::get<StrPtr>(data)->toString() ==
            std::get<StrPtr>(other.data)->toString();
    }

    // list
    if (isList() && other.isList()) {

        const auto& a = std::get<ListPtr>(data)->elements;
        const auto& b = std::get<ListPtr>(other.data)->elements;

        if (a.size() != b.size()) {
            return false;
        }

        for (size_t i = 0; i < a.size(); ++i) {
            if (!(a[i] == b[i])) {
                return false;
            }
        }

        return true;
    }

    // tuple
    if (isTuple() && other.isTuple()) {

        const auto& a = std::get<TuplePtr>(data)->items;
        const auto& b = std::get<TuplePtr>(other.data)->items;

        if (a.size() != b.size()) {
            return false;
        }

        for (size_t i = 0; i < a.size(); ++i) {
            if (!(a[i] == b[i])) {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool Value::operator<(const Value& other) const {
    // numeric comparison
    if (isNumeric() && other.isNumeric()) {
        return toBigFloat() < other.toBigFloat();
    }

    // string
    if (isString() && other.isString()) {
        return std::get<StrPtr>(data)->toString()
        < std::get<StrPtr>(other.data)->toString();
    }

    // list
    if (isList() && other.isList()) {

        const auto& a = std::get<ListPtr>(data)->elements;
        const auto& b = std::get<ListPtr>(other.data)->elements;

        const size_t minSize = std::min(a.size(), b.size());

        for (size_t i = 0; i < minSize; ++i) {

            if (a[i] == b[i]) {
                continue;
            }

            return a[i] < b[i];
        }

        return a.size() < b.size();
    }

    //tuple
    if (isTuple() && other.isTuple()) {

        const auto& a = std::get<TuplePtr>(data)->items;
        const auto& b = std::get<TuplePtr>(other.data)->items;

        const size_t minSize = std::min(a.size(), b.size());

        for (size_t i = 0; i < minSize; ++i) {

            if (a[i] == b[i]) {
                continue;
            }

            return a[i] < b[i];
        }

        return a.size() < b.size();
        }

    throw std::runtime_error("TypeError: unsupported comparison: " + toString().toStdString() + " " + " " + other.toString().toStdString());
}

size_t qHash(const Value &value, const size_t seed) {
    return value.hash() ^ seed;
}

bool Value::isNumeric() const {
    return std::holds_alternative<BigInt>(data) ||
           std::holds_alternative<BigFloat>(data) ||
           std::holds_alternative<bool>(data);
}

bool Value::isCallable() const {

    return
        std::holds_alternative<FunctionPtr>(data) ||
        std::holds_alternative<BuiltinFunctionPtr>(data) ||
        std::holds_alternative<ClassPtr>(data) ||
        std::holds_alternative<BoundMethodPtr>(data) ||
        std::holds_alternative<StaticMethodPtr>(data) ||
        std::holds_alternative<ClassMethodPtr>(data);
}

bool Value::isBigInt() const {
    return std::holds_alternative<BigInt>(data);
}

Value::BigInt Value::asBigInt(const QString& where) const {

    if (!isBigInt()) {
        throw std::runtime_error(
        (where + " argument must be int")
           .toStdString()
       );
    }

    return std::get<BigInt>(data);
}

bool Value::isList() const {
    return std::holds_alternative<ListPtr>(data);
}

Value::ListPtr Value::asList(const QString& where) const {
    if (!isList()) {
        throw std::runtime_error(
        (where + " argument must be list")
           .toStdString()
       );
    }

    return std::get<ListPtr>(data);
}

bool Value::isDict() const {
    return std::holds_alternative<DictPtr>(data);
}

Value::DictPtr Value::asDict(const QString& where) const {

    if (!isDict()) {
        throw std::runtime_error(
        (where + " argument must be dict")
           .toStdString()
       );
    }

    return std::get<DictPtr>(data);
}

bool Value::isTuple() const {
    return std::holds_alternative<TuplePtr>(data);
}

Value::TuplePtr Value::asTuple(const QString& where) const {

    if (!isTuple()) {
        throw std::runtime_error(
        (where + " argument must be tuple")
           .toStdString()
       );
    }

    return std::get<TuplePtr>(data);
}

bool Value::isString() const {
    return std::holds_alternative<StrPtr>(data);
}

Value::StrPtr Value::asString(const QString& where) const {

    if (!isString()) {
        throw std::runtime_error(
        (where + " argument must be str")
           .toStdString()
       );
    }

    return std::get<StrPtr>(data);
}

bool Value::isDictKeysView() const {
    return std::holds_alternative<DictKeysViewPtr>(data);
}

Value::DictKeysViewPtr Value::asDictKeysView() const {
    if (!isDictKeysView()) {
        throw std::runtime_error("Value is not a dict keys view");
    }

    return std::get<DictKeysViewPtr>(data);
}

bool Value::isDictValuesView() const {
    return std::holds_alternative<DictValuesViewPtr>(data);
}

Value::DictValuesViewPtr Value::asDictValuesView() const {
    if (!isDictValuesView()) {
        throw std::runtime_error("Value is not a dict values view");
    }

    return std::get<DictValuesViewPtr>(data);
}

bool Value::isDictItemsView() const {
    return std::holds_alternative<DictItemsViewPtr>(data);
}

Value::DictItemsViewPtr Value::asDictItemsView() const {
    if (!isDictItemsView()) {
        throw std::runtime_error("Value is not a dict items view");
    }

    return std::get<DictItemsViewPtr>(data);
}

bool Value::isSet() const {
    return std::holds_alternative<SetPtr>(data);
}

Value::SetPtr Value::asSet(const QString& where) const {

    if (!isSet()) {
        throw std::runtime_error(
        (where + " argument must be set")
           .toStdString()
       );
    }

    return std::get<SetPtr>(data);
}

bool Value::isHashable() const {

    if (isNumeric())
        return true;

    // str
    if (std::holds_alternative<StrPtr>(data)) {
        return true;
    }

    // tuple
    if (std::holds_alternative<TuplePtr>(data)) {

        const auto& items =
            std::get<TuplePtr>(data)->items;

        for (const auto& item : items) {

            if (!item.isHashable()) {
                return false;
            }
        }

        return true;
    }

    return false;
}

std::size_t Value::hash() const {

    if (!isHashable()) {
        throw std::runtime_error("TypeError: unhashable type");
    }

    // numeric
    if (isNumeric()) {

        const auto value = toBigFloat();

        // 1 == 1.0 == True
        if (floor(value) == value) {

            return std::hash<long long>{}(
                value.convert_to<long long>()
            );
        }

        return std::hash<long double>{}(
            value.convert_to<long double>()
        );
    }

    // str
    if (isString()) {

        return qHash(
            std::get<StrPtr>(data)->toString()
        );
    }

    // tuple
    if (isTuple()) {

        const auto& items = std::get<TuplePtr>(data)->items;

        std::size_t seed = 0;

        for (const auto& item : items) {

            seed ^= item.hash()
                + 0x9e3779b9
                + (seed << 6)
                + (seed >> 2);
        }

        return seed;
    }

    throw std::runtime_error("TypeError: unhashable type");
}

bool Value::isIterable() const {
    return isList() ||
           isTuple() ||
           isString() ||
           isSet() ||
           isDict() ||
           isDictKeysView() ||
           isDictValuesView() ||
           isDictItemsView();
}

Value::IteratorPtr Value::getIterator() const {

    if (isSet()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<SetIterator>(std::get<SetPtr>(data)));
    }

    if (isList()) {
        return std::static_pointer_cast<IteratorValue>(
                std::make_shared<ListIterator>(std::get<ListPtr>(data)));
    }

    if (isTuple()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<TupleIterator>(std::get<TuplePtr>(data)));
    }

    if (isDictKeysView()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictKeysIterator>(
                std::get<DictKeysViewPtr>(data)->getDict()
            )
        );
    }

    if (isDictItemsView()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictItemsIterator>(
                std::get<DictItemsViewPtr>(data)->getDict()
            )
        );
    }

    if (isDictValuesView()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictValuesIterator>(
                std::get<DictValuesViewPtr>(data)->getDict()
            )
        );
    }

    if (isString()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<StringIterator>(
                std::get<StrPtr>(data)->toString()
            )
        );
    }

    if (isDict()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictKeysIterator>(
                std::get<DictPtr>(data)
            )
        );
    }

    throw std::runtime_error("Object is not iterable");
}
