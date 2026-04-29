#include "Environment.h"

/**
 * Устанавливает переменную в окружении с указанным именем и значением.
 * Если переменная уже существует, её значение будет обновлено.
 *
 * @param name Имя переменной для установки или обновления.
 * @param value Значение, которое нужно связать с указанным именем переменной.
 */
void Environment::set(const QString& name, const Value& value) {
    if (globalVars.count(name)) {
        auto global = this;
        while (global->parent) {
            global = global->parent.get();
        }
        global->variables[name] = value;
        return;
    }

    if (nonlocalVars.count(name)) {
        auto env = parent;
        while (env) {
            if (env->variables.count(name)) {
                env->variables[name] = value;
                return;
            }
            env = env->parent;
        }
        throw std::runtime_error("No binding for nonlocal " + name.toStdString());
    }

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
Value& Environment::get(const QString& name) {
    if (const auto it = variables.find(name); it != variables.end())
        return it->second;

    if (parent)
        return parent->get(name);

    throw std::runtime_error("Undefined variable: " + name.toStdString());
}