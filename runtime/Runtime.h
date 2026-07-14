//
// Created by semyo on 08.05.2026.
//

#ifndef CPPYTHON_RUNTIME_H
#define CPPYTHON_RUNTIME_H
#include <memory>

#include "Value.h"

class Environment;
class ClassValue;

struct TracebackFrame {
    QString functionName;
    int sourceId = 0;
    int currentLine = 0;
    int currentStartColumn = 0;
    int currentEndColumn = 0;
    bool columnCaptured = false;
};

class Runtime {

public:

    static void initialize(const std::shared_ptr<Environment>& env);

    static std::shared_ptr<ClassValue> objectClass;

    static std::shared_ptr<ClassValue> strClass;

    static std::shared_ptr<ClassValue> bytesClass;

    static std::shared_ptr<ClassValue> bytearrayClass;

    static std::vector<Value::InstancePtr> exceptionStack;

    static std::shared_ptr<ClassValue> baseExceptionClass;

    static std::shared_ptr<ClassValue> exceptionClass;

    static std::shared_ptr<ClassValue> arithmeticErrorClass;

    static std::shared_ptr<ClassValue> attributeErrorClass;

    static std::shared_ptr<ClassValue> indexErrorClass;

    static std::shared_ptr<ClassValue> keyErrorClass;

    static std::shared_ptr<ClassValue> lookupErrorClass;

    static std::shared_ptr<ClassValue> nameErrorClass;

    static std::shared_ptr<ClassValue> overflowErrorClass;

    static std::shared_ptr<ClassValue> runtimeErrorClass;

    static std::shared_ptr<ClassValue> stopIterationClass;

    static std::shared_ptr<ClassValue> syntaxErrorClass;

    static std::shared_ptr<ClassValue> typeErrorClass;

    static std::shared_ptr<ClassValue> unicodeDecodeClass;

    static std::shared_ptr<ClassValue> valueErrorClass;

    static std::shared_ptr<ClassValue> generatorExitClass;

    static std::shared_ptr<ClassValue> rangeClass;

    static int currentSourceId;
    static QHash<int, QStringList> sourceRegistry;

    static std::vector<TracebackFrame> callStack;

    static int registerSource(const QString& code);
    static QString getSourceLine(int sourceId, int line);
    static QString getSourceLabel(int sourceId);
};
#endif //CPPYTHON_RUNTIME_H