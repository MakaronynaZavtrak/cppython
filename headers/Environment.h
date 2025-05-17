#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "Value.h"
#include <unordered_map>

/**
 * @class Environment
 * @brief Управляет коллекцией именованных значений и обеспечивает доступ к ним.
 *
 * Класс Environment служит хранилищем для переменных в заданной области видимости.
 * Он позволяет хранить и получать именованные значения, что делает его полезным для управления
 * переменными и связанными с ними данными в рамках определенного контекста.
 */
class Environment {
public:
    void set(const QString& name, const Value& value);
    Value& get(const QString& name);
private:
    std::unordered_map<QString, Value> variables;
};
#endif //ENVIRONMENT_H
