#include "Environment.h"

/**
 * Устанавливает переменную в окружении с указанным именем и значением.
 * Если переменная уже существует, её значение будет обновлено.
 *
 * @param name Имя переменной для установки или обновления.
 * @param value Значение, которое нужно связать с указанным именем переменной.
 */
void Environment::set(const QString& name, const Value& value)
{
    variables[name] = value;
}

/**
 * Возвращает ссылку на значение переменной с указанным именем из окружения.
 * Если переменная с данным именем не существует, выбрасывается исключение std::runtime_error.
 *
 * @param name Имя переменной, значение которой требуется получить.
 * @return Ссылка на значение переменной с указанным именем.
 * @throws std::runtime_error Если переменная с указанным именем не найдена.
 */
Value& Environment::get(const QString& name)
{
    const auto it = variables.find(name);
    if (it == variables.end())
        throw std::runtime_error("Undefined variable: " + name.toStdString());
    return it->second;
}