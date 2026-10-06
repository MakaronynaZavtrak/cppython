#include "Environment.h"

#include "DictValue.h"
#include "../exception/NameErrorException.h"
#include "../exception/SyntaxErrorException.h"

/**
 * Устанавливает переменную в окружении с указанным именем и значением.
 * Если переменная уже существует, её значение будет обновлено.
 *
 * @param name Имя переменной для установки или обновления.
 * @param value Значение, которое нужно связать с указанным именем переменной.
 */
void Environment::set(const QString& name, const Value& value) {
    if (globalVars.contains(name)) {
        auto global = this;
        while (global->parent) {
            global = global->parent.get();
        }
        global->variables[name] = value;
        return;
    }

    if (nonlocalVars.contains(name)) {
        auto env = parent;
        while (env) {
            if (env->variables.count(name)) {
                env->variables[name] = value;
                return;
            }
            env = env->parent;
        }
        throw SyntaxErrorException("No binding for nonlocal " + name);
    }

    variables[name] = value;

    // Захват в namespace (тело класса через метакласс): порядок хранит DictValue.
    if (nsCapture) {
        nsCapture->setItem(Value(name), value);
    }
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

    if (globalVars.contains(name)) {

        auto env = this;
        while (env->parent) {
            env = env->parent.get();
        }

        if (env->variables.count(name)) {
            return env->variables[name];
        }

        throw NameErrorException("Undefined variable: " + name);
    }

    if (nonlocalVars.contains(name)) {

        auto env = parent;

        while (env) {
            if (env->variables.count(name)) {
                return env->variables[name];
            }
            env = env->parent;
        }

        throw SyntaxErrorException("No binding for nonlocal " + name);
    }

    if (variables.count(name)) {
        return variables[name];
    }

    if (parent)
        return parent->get(name);

    throw NameErrorException("Undefined variable: " + name);
}