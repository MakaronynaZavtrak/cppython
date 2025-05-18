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
        return QString::number(std::get<double>(data));
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