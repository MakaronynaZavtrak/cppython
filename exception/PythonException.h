//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_PYTHONEXCEPTION_H
#define CPPYTHON_PYTHONEXCEPTION_H
#include <exception>

#include "Value.h"

class PythonException : public std::exception {

protected:
    Value::InstancePtr instance;
    std::string cachedWhat;

public:

    explicit PythonException(Value::InstancePtr  instance);

    explicit PythonException(
    const Value::ClassPtr& klass,
    const QString& message)
    : PythonException(
        makeInstance(klass, message)) {}

    [[nodiscard]] const char* what() const noexcept override;

    [[nodiscard]] const Value::ClassPtr& getClass() const;

    [[nodiscard]] const Value::InstancePtr& getInstance() const;

    [[nodiscard]] QString getTypeName() const;

    [[nodiscard]] QString getMessage() const;

    [[nodiscard]] virtual bool isCatchable() const { return true; }

    [[nodiscard]] static bool isSubclass(
        const Value::ClassPtr& child,
        const Value::ClassPtr& parent);

    static Value::InstancePtr makeInstance(
    const Value::ClassPtr& klass,
    const QString& message);
};
#endif //CPPYTHON_PYTHONEXCEPTION_H