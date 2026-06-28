//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_PYTHONEXCEPTION_H
#define CPPYTHON_PYTHONEXCEPTION_H
#include <exception>
#include <QString>

class PythonException : public std::exception {

protected:
    QString typeName;
    QString message;
    std::string cachedWhat;

public:

    explicit PythonException(QString typeName, QString message);

    [[nodiscard]] const char* what() const noexcept override;

    [[nodiscard]] const QString& getTypeName() const;

    [[nodiscard]] const QString& getMessage() const;

    virtual bool isCatchable() const { return true; }
};
#endif //CPPYTHON_PYTHONEXCEPTION_H