#include "Value.h"

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
QString Value::toString() const
{
    if (std::holds_alternative<int>(data))
    {
        return QString::number(std::get<int>(data));
    }
    if (std::holds_alternative<double>(data))
    {
        double d = std::get<double>(data);
        QString s = QString::number(d, 'g', 15);
        if (!s.contains('.') && !s.contains('e') && !s.contains('E')) {
            s += ".0";
        }
        return s;
    }
    if (std::holds_alternative<bool>(data))
    {
        return std::get<bool>(data) ? "True" : "False";
    }
    if (std::holds_alternative<QString>(data))
    {
        return QString("\'" + std::get<QString>(data) + "\'");
    }
    if (std::holds_alternative<ListPtr>(data))
    {
        return "[...]";
    }
    if (std::holds_alternative<DictPtr>(data))
    {
        return "{...}";
    }
    if (std::holds_alternative<FunctionPtr>(data))
    {
        return "<function>";
    }

    return "Unknown unsupported type";
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
    if (std::holds_alternative<int>(data))
    {
        return std::get<int>(data) != 0;
    }
    if (std::holds_alternative<double>(data))
    {
        return std::get<double>(data) != 0.0;
    }
    if (std::holds_alternative<bool>(data))
    {
        return std::get<bool>(data);
    }
    if (std::holds_alternative<QString>(data))
    {
        return !std::get<QString>(data).isEmpty();
    }
    if (std::holds_alternative<ListPtr>(data))
    {
        return std::get<ListPtr>(data) != nullptr;
    }
    if (std::holds_alternative<DictPtr>(data))
    {
        return std::get<DictPtr>(data) != nullptr;
    }
    if (std::holds_alternative<FunctionPtr>(data))
    {
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
double Value::toDouble() const {
    if (std::holds_alternative<int>(data)) {
        return std::get<int>(data);
    }
    if (std::holds_alternative<double>(data)) {
        return std::get<double>(data);
    }
    if (std::holds_alternative<bool>(data)) {
        return std::get<bool>(data) ? 1.0 : 0.0;
    }
    throw std::runtime_error("Cannot convert to double");
}
