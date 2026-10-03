#include "Value.h"

#include "BoundMethod.h"
#include "ByteArrayValue.h"
#include "BytesIterator.h"
#include "BytesValue.h"
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
#include "SliceValue.h"
#include "StaticMethodValue.h"
#include "StringIterator.h"
#include "StrValue.h"
#include "SuperValue.h"
#include "TupleIterator.h"
#include "TupleValue.h"
#include "ByteArrayIterator.h"

#include <QDebug>

#include "ClassUtils.h"
#include "FrozenSetIterator.h"
#include "FrozenSetValue.h"
#include "RangeIterator.h"
#include "RangeValue.h"
#include "../exception/ArithmeticErrorException.h"
#include "../exception/AttributeErrorException.h"
#include "../exception/TypeErrorException.h"
#include "../exception/ZeroDivisionErrorException.h"
#include "ModuleValue.h"
#include "PartialValue.h"

Value::Value(const QString& str) : data(std::make_shared<StrValue>(str)) {}

Value::Value(const char *str) : data(std::make_shared<StrValue>(str)) {}

template<typename Op>
static Value applyBitwise(const Value& l, const Value& r,
                          const char* opName, Op operation) {

    const bool bothIntLike =
        (l.isBigInt() || l.isBool()) &&
        (r.isBigInt() || r.isBool());

    if (!bothIntLike) {
        throw TypeErrorException(
            QString("unsupported operand type(s) for ") + opName + ": " +
            l.toString() + " " + r.toString()
        );
    }

    return Value(operation(l.toBigInt(), r.toBigInt()));
}

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

    return std::visit(
        overloaded{

            [](const BigInt& v) {
                return QString::fromStdString(v.convert_to<std::string>());
            },

            [](const BigFloat& v) {
                return formatFloat(v);
            },

            [](const bool v) {
                return QString(v ? "True" : "False");
            },

            [](const StrPtr& p) {
                return p->toString();
            },

            [](const BytesPtr& p) {
                return p->toString();
            },

            [](const ListPtr& p) {
                return p->toString();
            },

            [](const DictPtr& p) {
                return p->toString();
            },

            [](const TuplePtr& p) {
                return p->toString();
            },

            [](const FunctionPtr& p) {
                return p->toString();
            },

            [](const ClassPtr& p) {
                return p->toString();
            },

            [](const InstancePtr& p) {
                return p->toString();
            },

            [](const BoundMethodPtr& p) {
                return p->toString();
            },

            [](const SuperPtr& p) {
                return p->toString();
            },

            [](const BuiltinFunctionPtr& p) {
                return p->toString();
            },

            [](const PropertyPtr& p) {
                return p->toString();
            },

            [](const StaticMethodPtr& p) {
                return p->toString();
            },

            [](const ClassMethodPtr& p) {
                return p->toString();
            },

            [](const DictKeysViewPtr& p) {
                return p->toString();
            },

            [](const DictValuesViewPtr& p) {
                return p->toString();
            },

            [](const DictItemsViewPtr& p) {
                return p->toString();
            },

            [](const SetPtr& p) {
                return p->toString();
            },

            [](const IteratorPtr& p) {
                return p->toString();
            },

            [](const SlicePtr& p) {
                return p->toString();
            },

            [](const ByteArrayPtr& p) {
                return p->toString();
            },

            [](const ObjectPtr& p) {
                return p->toString();
            },

            [](const FrozenSetPtr &p) {
                return p->toString();
            },

            [](const RangePtr &p) {
                return p->toString();
            },

            [](const ModulePtr &p) {
                return p->toString();
            },

            [](const PartialPtr &p) {
                return p->toString();
            },

            [](std::monostate) {
                return QString("None");
            }

        },
        data
    );
}

QString Value::repr() const {

    if (isString()) {
        return std::get<StrPtr>(data)->repr();
    }

    if (isBytes()) {
        return std::get<BytesPtr>(data)->repr();
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

    if (isByteArray()) {
        return std::get<ByteArrayPtr>(data)->repr();
    }

    return toString();
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

    if (std::holds_alternative<bool>(data)) {
        return std::get<bool>(data);
    }

    if (isBigInt())   return std::get<BigInt>(data) != 0;
    if (isBigFloat()) return std::get<BigFloat>(data) != 0;

    if (isNone()) return false;

    if (isString())    return std::get<StrPtr>(data)->len() != 0;
    if (isBytes())     return !std::get<BytesPtr>(data)->bytes().isEmpty();
    if (isByteArray()) return !std::get<ByteArrayPtr>(data)->bytes().isEmpty();
    if (isList())      return !std::get<ListPtr>(data)->elements.empty();
    if (isDict())      return std::get<DictPtr>(data)->len() != 0;
    if (isTuple())     return !std::get<TuplePtr>(data)->items.empty();
    if (isSet())       return std::get<SetPtr>(data)->len() != 0;
    if (isFrozenSet()) return std::get<FrozenSetPtr>(data)->len() != 0;
    if (isRange())     return std::get<RangePtr>(data)->len() != 0;

    // пользовательские классы: __bool__, потом __len__, иначе True
    if (isInstance()) {

        try {
            const Value method = getAttrValue(*this, "__bool__");
            const Value result = call(method, {}, {}, nullptr);

            if (!std::holds_alternative<bool>(result.data)) {
                throw TypeErrorException(
                    "__bool__ should return bool, returned " + result.getTypeName()
                );
            }

            return std::get<bool>(result.data);
        }
        catch (const AttributeErrorException&) {}

        try {
            const Value method = getAttrValue(*this, "__len__");
            const Value result = call(method, {}, {}, nullptr);
            return result.toBigInt() != 0;
        }
        catch (const AttributeErrorException&) {}

        return true;
    }

    // всё остальное (функции, классы, методы, итераторы, слайсы...) — истинно
    return true;
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

    throw TypeErrorException("Cannot convert to double");
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

template<typename Op>
    static bool applyComparison(const Value &l, const Value &r, const bool isFloat, Op operation) {
    return isFloat
    ? operation(l.toBigFloat(), r.toBigFloat())
    : operation(l.toBigInt(), r.toBigInt());
}

template<typename Op>
    static Value applyCalculation(const Value &l, const Value &r, const bool isFloat, Op operation) {
    return isFloat
    ? Value(operation(l.toBigFloat(), r.toBigFloat()))
    : Value(operation(l.toBigInt(), r.toBigInt()));
}

bool Value::operator==(const Value& other) const {

    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyComparison(*this, other, isFloat, std::equal_to<>());
    }

    if (isNone() && other.isNone()) {
        return true;
    }

    if (isObject()) {
        try {
            return asObject()->equal(other);
        }
        catch (...) {}
    }

    return false;
}

bool Value::operator<(const Value& other) const {

    // numeric comparison
    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyComparison(*this, other, isFloat, std::less<>());
    }

    if (isBytes() && other.isBytes()) {

        return std::get<BytesPtr>(data)->bytes() <
               std::get<BytesPtr>(other.data)->bytes();
    }

    if (isObject()) {
        try {
            return asObject()->less(other);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported comparison: " +
        toString() + " " + other.toString());
}

bool Value::operator!=(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyComparison(*this, other, isFloat, std::not_equal_to<>());
    }

    if (isObject()) {
        try {
            return asObject()->notEqual(other);
        }
        catch (...) {}
    }

    throw TypeErrorException("unsupported operand type(s) for !=: "
        + toString() + " " + other.toString());
}

bool Value::operator<=(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyComparison(*this, other, isFloat, std::less_equal<>());
    }

    if (isObject()) {
        try {
            return asObject()->lessOrEqual(other);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for <=: " + toString() + " " + other.toString()
    );
}

bool Value::operator>(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyComparison(*this, other, isFloat, std::greater<>());
    }

    if (isObject()) {
        try {
            return asObject()->greater(other);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for >: " + toString() + " " + other.toString()
    );
}

bool Value::operator>=(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {
        const bool isFloat = isBigFloat() || other.isBigFloat();
        return applyComparison(*this, other, isFloat, std::greater_equal<>());
    }

    if (isObject()) {
        try {
            return asObject()->greaterOrEqual(other);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for >=: " + toString() + " " + other.toString()
    );
}

Value Value::operator+(const Value& other) const {

    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyCalculation(*this, other, isFloat, std::plus<>());
    }

    if (isObject()) {
        try {
            return asObject()->add(other);
        }
        catch (...) {}
    }

    if (other.isObject()) {

        try {
            return other.asObject()->radd(*this);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for +: " + toString() + " " + other.toString()
    );

}

Value Value::operator-(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyCalculation(*this, other, isFloat, std::minus<>());
    }

    if (isObject()) {
        try {
            return asObject()->sub(other);
        }
        catch (...) {}
    }

    if (other.isObject()) {

        try {
            return other.asObject()->rsub(*this);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for -: " + toString() + " " + other.toString()
    );
}

bool Value::isObject() const {

    return isString()
        || isBytes()
        || isByteArray()
        || isList()
        || isTuple()
        || isDict()
        || isSet()
        || isFrozenSet()
        || isRange();
}

Value::ObjectPtr Value::asObject() const {

    if (isString()) {
        return std::static_pointer_cast<ObjectValue>(asString());
    }

    if (isBytes()) {
        return std::static_pointer_cast<ObjectValue>(asBytes());
    }

    if (isByteArray()) {
        return std::static_pointer_cast<ObjectValue>(asByteArray());
    }

    if (isList()) {
        return std::static_pointer_cast<ObjectValue>(asList());
    }

    if (isTuple()) {
        return std::static_pointer_cast<ObjectValue>(asTuple());
    }

    if (isDict()) {
        return std::static_pointer_cast<ObjectValue>(asDict());
    }

    if (isSet()) {
        return std::static_pointer_cast<ObjectValue>(asSet());
    }

    if (isFrozenSet()) {
        return std::static_pointer_cast<ObjectValue>(asFrozenSet());
    }

    if (isRange()) {
        return std::static_pointer_cast<ObjectValue>(asRange());
    }

    throw TypeErrorException("Value is not an object");
}

bool Value::isFrozenSet() const {
    return std::holds_alternative<FrozenSetPtr>(data);
}

Value::FrozenSetPtr Value::asFrozenSet(const QString &) const {

    if (!isFrozenSet()) {
        throw TypeErrorException("Value is not a bytes");
    }

    return std::get<FrozenSetPtr>(data);

}

bool Value::isRange() const {
    return std::holds_alternative<RangePtr>(data);
}

Value::RangePtr Value::asRange(const QString& where) const {
    if (!isRange()) {
        throw TypeErrorException(where + " argument must be a range");
    }
    return std::get<RangePtr>(data);
}

bool Value::isModule() const {
    return std::holds_alternative<ModulePtr>(data);
}

Value::ModulePtr Value::asModule() const {
    if (!isModule()) {
        throw TypeErrorException("Value is not a module");
    }
    return std::get<ModulePtr>(data);
}

bool Value::isPartial() const {
    return std::holds_alternative<PartialPtr>(data);
}

Value::PartialPtr Value::asPartial() const {
    return std::get<PartialPtr>(data);
}

Value Value::operator*(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {

        const bool isFloat = isBigFloat() || other.isBigFloat();

        return applyCalculation(*this, other, isFloat, std::multiplies<>());
    }


    if (isObject()) {
        try {
            return asObject()->multiply(other);
        }
        catch (...) {}
    }

    if (other.isObject()) {

        try {
            return other.asObject()->rmul(*this);
        }
        catch (...) {}
    }

    throw TypeErrorException("unsupported operand type(s) for *: "
        + toString() + " " + other.toString());
}

Value Value::operator/(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {

        const BigFloat r = other.toBigFloat();

        if (r == 0) {
            throw ZeroDivisionErrorException("division by zero");
        }

        return Value(toBigFloat() / r);
    }

    throw TypeErrorException(
        "unsupported operand type(s) for /: " + toString() + " " + other.toString()
    );
}

Value Value::operator%(const Value &other) const {

    if (isNumeric() && other.isNumeric()) {

        const auto rf = other.toBigFloat();

        if (rf == 0) {
            throw ZeroDivisionErrorException("division by zero");
        }

        if (!isBigFloat() && !other.isBigFloat()) {
            return Value(toBigInt() % other.toBigInt());
        }

        const BigFloat lf = toBigFloat();

        const BigFloat quotient = floor(lf / rf);
        const BigFloat remainder = lf - rf * quotient;

        return Value(remainder);
    }

    if (isObject()) {
        try {
            return asObject()->mod(other);
        }
        catch (...) {}
    }

    if (other.isObject()) {

        try {
            return other.asObject()->rmod(*this);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for %: " + toString() + " " + other.toString()
    );
}

Value Value::power(const Value& other) const {

    if (isNumeric() && other.isNumeric()) {

        if (isBigFloat() || other.isBigFloat()) {
            return Value(pow(toBigFloat(), other.toBigFloat()));
        }

        const auto base = toBigInt();
        const auto exp  = other.toBigInt();

        if (exp < 0) {
            return Value(pow(toBigFloat(), other.toBigFloat()));
        }

        BigInt result = 1;

        for (BigInt i = 0; i < exp; ++i)
            result *= base;

        return Value(result);
    }

    throw TypeErrorException(
        "unsupported operand type(s) for **: " + toString() + " " + " " + other.toString()
    );
}

Value Value::intDivide(const Value& other) const {

    if (isNumeric() && other.isNumeric()) {

        const BigFloat lf = toBigFloat();
        const BigFloat rf = other.toBigFloat();

        if (rf == 0) {
            throw ZeroDivisionErrorException("division by zero");
        }

        const BigFloat result = floor(lf / rf);

        return isBigFloat() || other.isBigFloat()
        ? Value(result)
        : Value(BigInt(result));

    }

    throw TypeErrorException(
        "unsupported operand type(s) for //: " + toString() + " " + other.toString()
    );
}

Value& Value::operator+=(const Value &other) {

    if (isByteArray()) {

        asByteArray()->iadd(other);

        return *this;
    }

    *this = *this + other;
    return *this;
}

Value& Value::operator-=(const Value &other) {

    *this = *this - other;
    return *this;
}

Value& Value::operator*=(const Value &other) {

    if (isByteArray()) {

        asByteArray()->imul(other);

        return *this;
    }

    *this = *this * other;
    return *this;
}

Value& Value::operator/=(const Value &other) {

    *this = *this / other;
    return *this;
}

Value& Value::operator%=(const Value &other) {

    *this = *this % other;
    return *this;
}

Value& Value::intDivideEqual(const Value &other) {

    *this = intDivide(other);
    return *this;
}

Value & Value::powerEqual(const Value &other) {

    *this = power(other);
    return *this;
}

bool Value::is(const Value& other) const {

    if (data.index() != other.data.index())
        return false;

    if (isBigInt())
        return toBigInt() == other.toBigInt();

    if (isBigFloat())
        return toBigFloat() == other.toBigFloat();

    if (isBool())
        return toBool() == other.toBool();

    if (isString()) {
        return asString() == other.asString();
    }

    if (isBytes())
        return asBytes() == other.asBytes();

    if (isList())
        return asList().get() == other.asList().get();

    if (isDict())
        return asDict().get() == other.asDict().get();

    if (isTuple()) {
        return asTuple().get() == other.asTuple().get();
    }

    if (isFrozenSet()) {
        return asFrozenSet().get() == other.asFrozenSet().get();
    }

    if (isFunction()) {
        return asFunction().get() == other.asFunction().get();
    }

    if (isClass()) {
        return asClass().get() == other.asClass().get();
    }

    if (isInstance()) {
        return asInstance().get() == other.asInstance().get();
    }

    if (isBoundMethod()) {
        return asBoundMethod().get() == other.asBoundMethod().get();
    }

    if (isSuper()) {
        return asSuper().get() == other.asSuper().get();
    }

    if (isBuiltinFunction()) {
        return asBuiltinFunction().get() == other.asBuiltinFunction().get();
    }

    if (isProperty()) {
        return asProperty().get() == other.asProperty().get();
    }

    if (isStaticMethod()) {
        return asStaticMethod().get() == other.asStaticMethod().get();
    }

    if (isClassMethod()) {
        return asClassMethod().get() == other.asClassMethod().get();
    }

    if (isDictKeysView()) {
        return asDictKeysView().get() == other.asDictKeysView().get();
    }

    if (isDictValuesView()) {
        return asDictValuesView().get() == other.asDictValuesView().get();
    }

    if (isDictItemsView()) {
        return asDictItemsView().get() == other.asDictItemsView().get();
    }

    if (isSet()) {
        return asSet().get() == other.asSet().get();
    }

    if (isIterable()) {
        return getIterator() == other.getIterator();
    }

    if (isNone())
        return true;

    throw TypeErrorException(
        "is not supported between instances of '" + toString() + "' and '" + other.toString() + "'"
    );
}

Value Value::operator+() const {

    if (isBool()) {
        return Value(toBigInt());
    }

    if (isNumeric()) {
        return *this;
    }

    throw TypeErrorException("bad operand type for unary +");
}

Value Value::operator-() const {

    if (isBigInt()) {
        return Value(-toBigInt());
    }

    if (isBigFloat()) {
        return Value(-toBigFloat());
    }

    if (isBool()) {
        return Value(-toBigInt());
    }

    throw TypeErrorException("bad operand type for unary -");
}

bool Value::contains(const Value &value) const {

    if (isObject()) {
        try {
            return asObject()->contains(value);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "argument of type '" + toString() + "' is not iterable"
    );
}

Value Value::operator|(const Value& other) const {

    if (isNumeric() && other.isNumeric()) {
        return applyBitwise(*this, other, "|", std::bit_or<>());
    }

    if (isObject()) {
        try {
            return asObject()->bitOr(other);
        }
        catch (...) {}
    }

    if (other.isObject()) {

        try {
            return other.asObject()->ror(*this);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for |: " + toString() + " " + other.toString()
    );
}

Value Value::operator&(const Value& other) const {

    if (isNumeric() && other.isNumeric()) {
        return applyBitwise(*this, other, "&", std::bit_and<>());
    }

    if (isObject()) {
        try {
            return asObject()->bitAnd(other);
        }
        catch (...) {}
    }

    if (other.isObject()) {

        try {
            return other.asObject()->rand(*this);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for &: "
        + toString() + " " + other.toString());
}

Value Value::operator^(const Value& other) const {

    if (isNumeric() && other.isNumeric()) {
        return applyBitwise(*this, other, "^", std::bit_xor<>());
    }

    if (isObject()) {
        try {
            return asObject()->bitXor(other);
        }
        catch (...) {}
    }

    if (other.isObject()) {

        try {
            return other.asObject()->rxor(*this);
        }
        catch (...) {}
    }

    throw TypeErrorException(
        "unsupported operand type(s) for ^: " + toString() + " " + other.toString()
    );
}

Value& Value::operator|=(const Value& other) {

    if (isSet()) {

        asSet()->ior(other);

        return *this;
    }

    *this = *this | other;
    return *this;
}

Value & Value::operator&=(const Value& other) {

    if (isSet()) {

        asSet()->iand(other);

        return *this;
    }

    *this = *this & other;
    return *this;
}

Value & Value::operator^=(const Value& other) {

    if (isSet()) {

        asSet()->ixor(other);

        return *this;
    }

    *this = *this ^ other;
    return *this;
}

QString Value::ascii() const {

    QString reprString = repr();

    QString result;

    for (const QChar ch : reprString) {

        if (ch.unicode() < 128) {

            result += ch;
        }
        else {

            result += QString("\\u%1")
                    .arg(
                        static_cast<unsigned int>(ch.unicode()),
                        4,
                        16,
                        QChar('0')
                    );
        }
    }

    return result;
}


QString Value::formatFloat(const BigFloat& num) {
    std::string s = num.str(15);

    if (s.find('.') == std::string::npos &&
        s.find('e') == std::string::npos &&
        s.find('E') == std::string::npos) {
        s += ".0";
    }

    return QString::fromStdString(s);
}

QString Value::getTypeName() const {

    if (isBool())            return "bool";
    if (isBigInt())          return "int";
    if (isBigFloat())        return "float";
    if (isString())          return "str";
    if (isBytes())           return "bytes";
    if (isByteArray())       return "bytearray";
    if (isList())            return "list";
    if (isDict())            return "dict";
    if (isTuple())           return "tuple";
    if (isSet())             return "set";
    if (isFrozenSet())       return "frozenset";
    if (isRange())           return "range";
    if (isNone())            return "NoneType";
    if (isFunction())        return "function";
    if (isBuiltinFunction()) return "builtin_function_or_method";
    if (isClass())           return "type";
    if (isInstance())        return asInstance()->klass->name;
    if (isBoundMethod())     return "method";
    if (isSuper())           return "super";
    if (isProperty())        return "property";
    if (isStaticMethod())    return "staticmethod";
    if (isClassMethod())     return "classmethod";
    if (isSlice())           return "slice";
    if (isDictKeysView())    return "dict_keys";
    if (isDictValuesView())  return "dict_values";
    if (isDictItemsView())   return "dict_items";
    if (isModule())          return "module";
    if (isPartial())         return "partial";

    if (std::holds_alternative<IteratorPtr>(data)) {
        return std::get<IteratorPtr>(data)->getTypeName();
    }

    return "object";
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
        std::holds_alternative<ClassMethodPtr>(data) ||
        std::holds_alternative<PartialPtr>(data);
}

bool Value::isBigInt() const {
    return std::holds_alternative<BigInt>(data);
}

Value::BigInt Value::asBigInt(const QString& where) const {

    if (!isBigInt()) {
        throw TypeErrorException(where + " argument must be int");
    }

    return std::get<BigInt>(data);
}

bool Value::isBigFloat() const {
    return std::holds_alternative<BigFloat>(data);
}

Value::BigFloat Value::asBigFloat(const QString &) const {

    if (!isBigFloat()) {
        throw TypeErrorException("Value is not a float");
    }

    return std::get<BigFloat>(data);
}

bool Value::isList() const {
    return std::holds_alternative<ListPtr>(data);
}

Value::ListPtr Value::asList(const QString& where) const {
    if (!isList()) {
        throw TypeErrorException(
            where + " argument must be list"
        );
    }

    return std::get<ListPtr>(data);
}

bool Value::isDict() const {
    return std::holds_alternative<DictPtr>(data);
}

Value::DictPtr Value::asDict(const QString& where) const {

    if (!isDict()) {
        throw TypeErrorException(
            where + " argument must be dict"
        );
    }

    return std::get<DictPtr>(data);
}

bool Value::isTuple() const {
    return std::holds_alternative<TuplePtr>(data);
}

Value::TuplePtr Value::asTuple(const QString& where) const {

    if (!isTuple()) {
        throw TypeErrorException(
            where + " argument must be tuple"
        );
    }

    return std::get<TuplePtr>(data);
}

bool Value::isString() const {
    return std::holds_alternative<StrPtr>(data);
}

Value::StrPtr Value::asString(const QString& where) const {

    if (!isString()) {
        throw TypeErrorException(
            where + " argument must be str");
    }

    return std::get<StrPtr>(data);
}

QString Value::display() const {
    return repr();
}

bool Value::isBool() const {
    return std::holds_alternative<bool>(data);
}

bool Value::isDictKeysView() const {
    return std::holds_alternative<DictKeysViewPtr>(data);
}

Value::DictKeysViewPtr Value::asDictKeysView() const {
    if (!isDictKeysView()) {
        throw TypeErrorException("Value is not a dict keys view");
    }

    return std::get<DictKeysViewPtr>(data);
}

bool Value::isDictValuesView() const {
    return std::holds_alternative<DictValuesViewPtr>(data);
}

Value::DictValuesViewPtr Value::asDictValuesView() const {

    if (!isDictValuesView()) {
        throw TypeErrorException("Value is not a dict values view");
    }

    return std::get<DictValuesViewPtr>(data);
}

bool Value::isDictItemsView() const {
    return std::holds_alternative<DictItemsViewPtr>(data);
}

Value::DictItemsViewPtr Value::asDictItemsView() const {

    if (!isDictItemsView()) {
        throw TypeErrorException("Value is not a dict items view");
    }

    return std::get<DictItemsViewPtr>(data);
}

bool Value::isFunction() const {
    return std::holds_alternative<FunctionPtr>(data);
}

Value::FunctionPtr Value::asFunction() const {

    if (!isFunction()) {
        throw TypeErrorException("Value is not a function");
    }

    return std::get<FunctionPtr>(data);

}

bool Value::isClass() const {
    return std::holds_alternative<ClassPtr>(data);
}

Value::ClassPtr Value::asClass() const {

    if (!isClass()) {
        throw TypeErrorException("Value is not a class");
    }

    return std::get<ClassPtr>(data);
}

bool Value::isInstance() const {
    return std::holds_alternative<InstancePtr>(data);
}

Value::InstancePtr Value::asInstance(const QString& where) const {

    if (!isInstance()) {
        throw TypeErrorException(where + " is not an instance");
    }

    return std::get<InstancePtr>(data);
}

bool Value::isBoundMethod() const {
    return std::holds_alternative<BoundMethodPtr>(data);
}

Value::BoundMethodPtr Value::asBoundMethod() const {

    if (!isBoundMethod()) {
        throw TypeErrorException("Value is not a bound method");
    }

    return std::get<BoundMethodPtr>(data);
}

bool Value::isSuper() const {
    return std::holds_alternative<SuperPtr>(data);
}

Value::SuperPtr Value::asSuper() const {

    if (!isSuper()) {
        throw TypeErrorException("Value is not a super");
    }

    return std::get<SuperPtr>(data);
}

bool Value::isBuiltinFunction() const {
    return std::holds_alternative<BuiltinFunctionPtr>(data);
}

Value::BuiltinFunctionPtr Value::asBuiltinFunction() const {

    if (!isBuiltinFunction()) {
        throw TypeErrorException("Value is not a builtin function");
    }

    return std::get<BuiltinFunctionPtr>(data);
}

bool Value::isProperty() const {
    return std::holds_alternative<PropertyPtr>(data);
}

Value::PropertyPtr Value::asProperty() const {

    if (!isProperty()) {
        throw TypeErrorException("Value is not a property");
    }

    return std::get<PropertyPtr>(data);
}

bool Value::isStaticMethod() const {
    return std::holds_alternative<StaticMethodPtr>(data);
}

Value::StaticMethodPtr Value::asStaticMethod() const {

    if (!isStaticMethod()) {
        throw TypeErrorException("Value is not a static method");
    }

    return std::get<StaticMethodPtr>(data);
}

bool Value::isClassMethod() const {
    return std::holds_alternative<ClassMethodPtr>(data);
}

Value::ClassMethodPtr Value::asClassMethod() const {

    if (!isClassMethod()) {
        throw TypeErrorException("Value is not a class method");
    }

    return std::get<ClassMethodPtr>(data);
}

bool Value::isSet() const {
    return std::holds_alternative<SetPtr>(data);
}

Value::SetPtr Value::asSet(const QString& where) const {

    if (!isSet()) {
        throw TypeErrorException(where + " argument must be set");
    }

    return std::get<SetPtr>(data);
}

bool Value::isHashable() const {

    if (isNumeric())
        return true;

    if (isNone()) {
        return true;
    }

    if (isString()) {
        return true;
    }

    if (isBytes()) {
        return true;
    }

    // tuple
    if (isTuple()) {

        const auto& items = asTuple()->items;

        return std::all_of(
            items.begin(),
            items.end(),
            [&](const Value& item) { return item.isHashable(); }
        );
    }

    if (isFrozenSet()) {
        return true;
    }

    return false;
}

std::size_t Value::hash() const {

    if (!isHashable()) {
        throw TypeErrorException("unhashable type");
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

    if (isNone()) {
        return 0x9e3779b97f4a7c15ULL;
    }

    // str
    if (isString()) {
        return qHash(asString()->toString());
    }

    if (isBytes()) {
        return asBytes()->hash();
    }

    if (isTuple()) {
        return asTuple()->hash();
    }

    if (isFrozenSet()) {
        return asFrozenSet()->hash();
    }

    throw TypeErrorException("unhashable type");
}

bool Value::isIterable() const {
    return isList() ||
           isTuple() ||
           isString() ||
           isBytes() ||
           isSet() ||
           isDict() ||
           isDictKeysView() ||
           isDictValuesView() ||
           isDictItemsView();
}

Value::IteratorPtr Value::getIterator() const {

    if (isSet()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<SetIterator>(asSet()));
    }

    if (isList()) {
        return std::static_pointer_cast<IteratorValue>(
                std::make_shared<ListIterator>(asList()));
    }

    if (isTuple()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<TupleIterator>(asTuple()));
    }

    if (isDictKeysView()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictKeysIterator>(
                asDictKeysView()->getDict()
            )
        );
    }

    if (isDictItemsView()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictItemsIterator>(
                asDictItemsView()->getDict()
            )
        );
    }

    if (isDictValuesView()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictValuesIterator>(
                asDictValuesView()->getDict()
            )
        );
    }

    if (isString()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<StringIterator>(
                asString()->toString()
            )
        );
    }

    if (isBytes()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<BytesIterator>(
                asBytes()
            )
        );
    }

    if (isByteArray()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<ByteArrayIterator>(
                asByteArray()->bytes()
            )
        );
    }

    if (isDict()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<DictKeysIterator>(
                asDict()
            )
        );
    }

    if (isFrozenSet()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<FrozenSetIterator>(
                asFrozenSet()
            )
        );
    }

    if (isRange()) {
        return std::static_pointer_cast<IteratorValue>(
            std::make_shared<RangeIterator>(asRange()));
    }

    if (std::holds_alternative<IteratorPtr>(data)) {
        return std::get<IteratorPtr>(data);
    }

    throw TypeErrorException("Object is not iterable");
}

bool Value::isBytes() const {
    return std::holds_alternative<BytesPtr>(data);
}

Value::BytesPtr Value::asBytes(const QString &) const {

    if (!isBytes()) {
        throw TypeErrorException("Value is not a bytes");
    }

    return std::get<BytesPtr>(data);
}

bool Value::isSlice() const {
    return std::holds_alternative<SlicePtr>(data);
}

Value::SlicePtr Value::asSlice() const {

    if (!isSlice()) {
        throw TypeErrorException("Value is not a slice");
    }

    return std::get<SlicePtr>(data);
}

bool Value::isByteArray() const {
    return std::holds_alternative<ByteArrayPtr>(data);
}

Value::ByteArrayPtr Value::asByteArray(const QString &where) const {

    if (!isByteArray()) {
        throw TypeErrorException(where + "value is not a bytearray");
    }

    return std::get<ByteArrayPtr>(data);
}
