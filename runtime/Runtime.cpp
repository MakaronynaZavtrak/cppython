//
// Created by semyo on 08.05.2026.
//
#include "Runtime.h"

#include "classes/RegisterClasses.h"
#include "exceptions/RegisterExceptionClasses.h"

void Runtime::initialize(const std::shared_ptr<Environment> &env) {

    registerObjectClass(env);

    registerStringClass(env);

    registerBytesClass(env);

    registerByteArrayClass(env);

    registerExceptionClasses(env);

    registerRangeClass(env);

}

std::shared_ptr<ClassValue> Runtime::objectClass = nullptr;
std::shared_ptr<ClassValue> Runtime::strClass = nullptr;
std::shared_ptr<ClassValue> Runtime::bytesClass = nullptr;
std::shared_ptr<ClassValue> Runtime::bytearrayClass = nullptr;
std::vector<Value::InstancePtr> Runtime::exceptionStack;
std::shared_ptr<ClassValue> Runtime::baseExceptionClass = nullptr;
std::shared_ptr<ClassValue> Runtime::exceptionClass = nullptr;
std::shared_ptr<ClassValue> Runtime::arithmeticErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::attributeErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::indexErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::keyErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::lookupErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::nameErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::overflowErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::runtimeErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::stopIterationClass = nullptr;
std::shared_ptr<ClassValue> Runtime::syntaxErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::typeErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::unicodeDecodeClass = nullptr;
std::shared_ptr<ClassValue> Runtime::valueErrorClass = nullptr;
std::shared_ptr<ClassValue> Runtime::generatorExitClass = nullptr;
std::shared_ptr<ClassValue> Runtime::rangeClass = nullptr;
std::shared_ptr<ClassValue> Runtime::zeroDivisionErrorClass = nullptr;

int Runtime::currentSourceId = 0;
QHash<int, QStringList> Runtime::sourceRegistry;

std::vector<TracebackFrame> Runtime::callStack;

QHash<int, QString> Runtime::sourceLabels;
int Runtime::inputCounter = 0;

int Runtime::registerSource(const QString& code, const QString& label) {
    ++currentSourceId;
    sourceRegistry[currentSourceId] = code.split('\n');

    sourceLabels[currentSourceId] = label.isEmpty()
        ? QString("<python-input-%1>").arg(inputCounter++)
        : label;

    return currentSourceId;
}

QString Runtime::getSourceLine(const int sourceId, const int line) {

    const auto it = sourceRegistry.find(sourceId);

    if (it == sourceRegistry.end()) {
        return "";
    }

    const auto& lines = it.value();

    if (line < 1 || line > lines.size()) {
        return "";
    }

    return lines[line - 1];
}

QString Runtime::getSourceLabel(const int sourceId) {
    if (const auto it = sourceLabels.find(sourceId); it != sourceLabels.end()) {
        return it.value();
    }
    return QString("<python-input-%1>").arg(sourceId);   // фолбэк на всякий
}
