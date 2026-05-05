#include "Value.h"

#include "BoundMethod.h"
#include "FunctionValue.h"
#include "PropertyValue.h"

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

    if (std::holds_alternative<QString>(data)) {
        return QString("\'" + std::get<QString>(data) + "\'");
    }

    if (std::holds_alternative<ListPtr>(data)) {
        return "[...]";
    }

    if (std::holds_alternative<DictPtr>(data)) {
        return "{...}";
    }

    if (std::holds_alternative<FunctionPtr>(data)) {
        return std::get<FunctionPtr>(data)->toString();
    }

    if (std::holds_alternative<ClassPtr>(data)) {
        return "<class>";
    }

    if (std::holds_alternative<InstancePtr>(data)) {
        return std::get<InstancePtr>(data)->toString();
    }

    if (std::holds_alternative<BoundMethodPtr>(data)) {
        return std::get<BoundMethodPtr>(data)->toString();
    }

    if (std::holds_alternative<std::monostate>(data)) {
        return "None";
    }

    return "Unknown unsupported type";
}

QString Value::asString() const {
    if (!std::holds_alternative<QString>(data))
        throw std::runtime_error("Not a string");

    return std::get<QString>(data);
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

    if (std::holds_alternative<QString>(data)) {
        return !std::get<QString>(data).isEmpty();
    }

    if (std::holds_alternative<ListPtr>(data)) {
        return std::get<ListPtr>(data) != nullptr;
    }

    if (std::holds_alternative<DictPtr>(data)) {
        return std::get<DictPtr>(data) != nullptr;
    }

    if (std::holds_alternative<FunctionPtr>(data)) {
        return std::get<FunctionPtr>(data) != nullptr;
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

bool Value::hasGet() const {
    return std::holds_alternative<FunctionPtr>(data) ||
           std::holds_alternative<PropertyPtr>(data);;
}

Value Value::callGet(const InstancePtr& instance,
                    const ClassPtr& owner) const {

    if (const auto f = std::get_if<FunctionPtr>(&data)) {
        return (*f)->get(instance, owner);
    }

    // property
    if (const auto p = std::get_if<PropertyPtr>(&data)) {
        return (*p)->get(instance, owner);
    }


    return *this;
}
