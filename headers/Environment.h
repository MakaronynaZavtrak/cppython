#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
#include "Value.h"
#include <QSet>
#include <QHash>
#include <utility>

/**
 * @class Environment
 * @brief Управляет коллекцией именованных значений и обеспечивает доступ к ним.
 *
 * Класс Environment служит хранилищем для переменных в заданной области видимости.
 * Он позволяет хранить и получать именованные значения, что делает его полезным для управления
 * переменными и связанными с ними данными в рамках определенного контекста.
 */
class Environment : public std::enable_shared_from_this<Environment> {
public:
    QSet<QString> globalVars;
    QSet<QString> nonlocalVars;
    QHash<QString, Value> variables;
    std::shared_ptr<Environment> parent;

    void set(const QString& name, const Value& value);
    Value& get(const QString& name);

    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
       : parent(std::move(std::move(parent))) {}
};
#endif //ENVIRONMENT_H
