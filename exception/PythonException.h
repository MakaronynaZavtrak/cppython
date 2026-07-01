//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_PYTHONEXCEPTION_H
#define CPPYTHON_PYTHONEXCEPTION_H
#include <exception>

#include "Value.h"

class PythonException : public std::exception {

protected:
    Value::ClassPtr klass;
    std::shared_ptr<InstanceValue> instance;
    QString message;
    std::string cachedWhat;

public:

    PythonException(const Value::ClassPtr& klass, QString message);

    [[nodiscard]] const char* what() const noexcept override;

    [[nodiscard]] const Value::ClassPtr& getClass() const;

    [[nodiscard]] const std::shared_ptr<InstanceValue>& getInstance() const;

    [[nodiscard]] const QString& getTypeName() const;

    [[nodiscard]] const QString& getMessage() const;

    [[nodiscard]] virtual bool isCatchable() const { return true; }

    [[nodiscard]] static bool isSubclass(
        const Value::ClassPtr& child,
        const Value::ClassPtr& parent);
};
#endif //CPPYTHON_PYTHONEXCEPTION_H