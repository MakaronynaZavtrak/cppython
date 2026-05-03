#ifndef VALUE_H
#define VALUE_H

#include <QHash>
#include <QString>

#include "FunctionValue.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include "BoundMethod.h"
#include "ClassValue.h"
#include "InstanceValue.h"

class ASTNode;

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
class Value {
public:
    using List = std::vector<Value>;
    using Dict = QHash<QString, Value>;

    using ListPtr = std::shared_ptr<List>;
    using DictPtr = std::shared_ptr<Dict>;
    using FunctionPtr = std::shared_ptr<FunctionValue>;

    using BigInt = boost::multiprecision::cpp_int;
    using BigFloat = boost::multiprecision::cpp_dec_float_50;

    using ClassPtr = std::shared_ptr<ClassValue>;
    using InstancePtr = std::shared_ptr<InstanceValue>;
    using BoundMethodPtr = std::shared_ptr<BoundMethod>;

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
        std::monostate
        //В будущем здесь появятся еще типы (наверное)>;
    > data;

    Value() : data(std::monostate{}) {}

    explicit Value(BigInt integer) : data(integer) {}
    explicit Value(BigFloat number) : data(number) {}
    explicit Value(bool boolean) : data(boolean) {}
    explicit Value(const QString& str) : data(str) {}
    explicit Value(const char* str) : data(QString(str)) {}

    explicit Value(const List& list) : data(std::make_shared<List>(list)) {}
    explicit Value(List&& list) : data(std::make_shared<List>(std::move(list))) {}

    explicit Value(const Dict& dict) : data(std::make_shared<Dict>(dict)) {}
    explicit Value(Dict&& dict) : data(std::make_shared<Dict>(std::move(dict))) {}

    explicit Value(FunctionPtr& func) : data(std::move(func)) {}

    explicit Value(const ClassPtr& cls) : data(cls) {}
    explicit Value(const InstancePtr& cls) : data(cls) {}
    explicit Value(const BoundMethodPtr& cls) : data(cls) {}

    [[nodiscard]] QString toString() const;
    [[nodiscard]] bool toBool() const;
    [[nodiscard]] bool isNone() const;
    [[nodiscard]] BigFloat toBigFloat() const;
    [[nodiscard]] BigInt toBigInt() const;
};
#endif //VALUE_H
