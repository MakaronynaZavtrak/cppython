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