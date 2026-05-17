#ifndef VALUE_H
#define VALUE_H

#include <QHash>
#include <QString>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include "BuiltinFunction.h"
#include "ReprMixin.h"

class DictValue;
class ListValue;
class ClassMethodValue;
class StaticMethodValue;
class SuperValue;
class InstanceValue;
class ClassValue;
class PropertyValue;
class BoundMethod;
class FunctionValue;
    /**
 * @class Value
 * @brief Представляет полиморфный контейнер данных, способный хранить различные типы данных, включая примитивные
 * типы, сложные структуры и указатели на функции или другие типы.
 *
 * Класс Value спроектирован для обеспечения гибкого контейнера для хранения и управления множеством типов значений.
 * Он поддерживает различные типы данных, включая целые числа, числа с плавающей точкой, логические значения, строки,
 * списки, словари и функции. Данные хранятся с использованием `std::variant` для эффективного управления типами
 * и обеспечения типобезопасности.
 *
 * Класс предоставляет конструкторы для инициализации экземпляра `Value` различными типами,
 * автоматически управляя выделением памяти для сложных типов с использованием умных указателей.
 */
class Value : public ReprMixin {
public:

    using ListPtr = std::shared_ptr<ListValue>;
    using DictPtr = std::shared_ptr<DictValue>;

    using FunctionPtr = std::shared_ptr<FunctionValue>;

    using BigInt = boost::multiprecision::cpp_int;
    using BigFloat = boost::multiprecision::cpp_dec_float_50;

    using ClassPtr = std::shared_ptr<ClassValue>;
    using InstancePtr = std::shared_ptr<InstanceValue>;
    using BoundMethodPtr = std::shared_ptr<BoundMethod>;

    using SuperPtr = std::shared_ptr<SuperValue>;
    using BuiltinFunctionPtr = std::shared_ptr<BuiltinFunction>;

    using PropertyPtr = std::shared_ptr<PropertyValue>;
    using StaticMethodPtr = std::shared_ptr<StaticMethodValue>;
    using ClassMethodPtr = std::shared_ptr<ClassMethodValue>;

    std::variant<
        BigInt,
        BigFloat,
        bool,
        QString,
        ListPtr,
        DictPtr,
        FunctionPtr,
        ClassPtr,
        InstancePtr,
        BoundMethodPtr,
        SuperPtr,
        BuiltinFunctionPtr,
        PropertyPtr,
        StaticMethodPtr,
        ClassMethodPtr,
        std::monostate
        //В будущем здесь появятся еще типы (наверное)>;
    > data;

    Value() : data(std::monostate{}) {}

    explicit Value(BigInt integer) : data(integer) {}
    explicit Value(BigFloat number) : data(number) {}
    explicit Value(bool boolean) : data(boolean) {}
    explicit Value(const QString& str) : data(str) {}
    explicit Value(const char* str) : data(QString(str)) {}

    explicit Value(const ListPtr& list) : data(list) {};
    explicit Value(const DictPtr& dict) : data(dict) {}

    explicit Value(const FunctionPtr& func) : data(func) {}
    explicit Value(FunctionPtr&& func) : data(std::move(func)) {}

    explicit Value(const ClassPtr& cls) : data(cls) {}
    explicit Value(const InstancePtr& cls) : data(cls) {}
    explicit Value(const BoundMethodPtr& cls) : data(cls) {}
    explicit Value(const BoundMethodPtr&& cls) : data(cls) {}

    explicit Value(const BuiltinFunctionPtr& func) : data(func) {}
    explicit Value(const SuperPtr& superValue) : data(superValue) {}

    explicit Value(const PropertyPtr& propertyValue) : data(propertyValue) {}
    explicit Value(const StaticMethodPtr& staticMethodValue) : data(staticMethodValue) {}
    explicit Value(const ClassMethodPtr& classMethodValue) : data(classMethodValue) {}

    [[nodiscard]] QString toString() const override;
    [[nodiscard]] QString asString() const;
    [[nodiscard]] bool toBool() const;
    [[nodiscard]] bool isNone() const;
    [[nodiscard]] BigFloat toBigFloat() const;
    [[nodiscard]] BigInt toBigInt() const;
    [[nodiscard]] bool isNumeric() const;
    [[nodiscard]] bool isCallable() const;

    [[nodiscard]] bool isDict() const;
    [[nodiscard]] DictPtr asDict() const;

    [[nodiscard]] bool isString() const;

    [[nodiscard]] bool operator==(const Value&) const;
    [[nodiscard]] bool operator<(const Value&) const;
};
#endif //VALUE_H
