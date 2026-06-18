#ifndef VALUE_H
#define VALUE_H

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>

#include "BuiltinFunction.h"
#include "ReprMixin.h"

class FrozenSetValue;
class ObjectValue;
class ByteArrayValue;
class SliceValue;
class BytesValue;
class StrValue;
class IteratorValue;
class SetValue;
class DictItemsIterator;
class DictValuesIterator;
class DictKeysIterator;
class DictItemsView;
class DictValuesView;
class DictKeysView;
class TupleIterator;
class ListIterator;
class TupleValue;
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
    using TuplePtr = std::shared_ptr<TupleValue>;

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

    using DictKeysViewPtr = std::shared_ptr<DictKeysView>;

    using DictValuesViewPtr = std::shared_ptr<DictValuesView>;

    using DictItemsViewPtr = std::shared_ptr<DictItemsView>;

    using SetPtr = std::shared_ptr<SetValue>;
    using IteratorPtr = std::shared_ptr<IteratorValue>;

    using StrPtr = std::shared_ptr<StrValue>;

    using BytesPtr = std::shared_ptr<BytesValue>;

    using SlicePtr = std::shared_ptr<SliceValue>;

    using ByteArrayPtr = std::shared_ptr<ByteArrayValue>;

    using ObjectPtr = std::shared_ptr<ObjectValue>;

    using FrozenSetPtr = std::shared_ptr<FrozenSetValue>;

    std::variant<
        BigInt,
        BigFloat,
        bool,
        StrPtr,
        BytesPtr,
        ListPtr,
        DictPtr,
        TuplePtr,
        FunctionPtr,
        ClassPtr,
        InstancePtr,
        BoundMethodPtr,
        SuperPtr,
        BuiltinFunctionPtr,
        PropertyPtr,
        StaticMethodPtr,
        ClassMethodPtr,
        DictKeysViewPtr,
        DictValuesViewPtr,
        DictItemsViewPtr,
        SetPtr,
        IteratorPtr,
        SlicePtr,
        ByteArrayPtr,
        ObjectPtr,
        FrozenSetPtr,
        std::monostate
        //В будущем здесь появятся еще типы (наверное)>;
    > data;

    Value() : data(std::monostate{}) {}

    explicit Value(BigInt integer) : data(integer) {}
    explicit Value(BigFloat number) : data(number) {}
    explicit Value(bool boolean) : data(boolean) {}

    explicit Value(const StrPtr& str) : data(str) {}
    explicit Value(const QString& str);
    explicit Value(const char* str);

    explicit Value(const BytesPtr& bytes) : data(bytes) {}

    explicit Value(const ListPtr& list) : data(list) {}
    explicit Value(const DictPtr& dict) : data(dict) {}
    explicit Value(const TuplePtr& tuple) : data(tuple) {}


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


    explicit Value(const DictKeysViewPtr& dictKeys) : data(dictKeys) {}

    explicit Value(const DictValuesViewPtr& dictValues) : data(dictValues) {}

    explicit Value(const DictItemsViewPtr& dictItems) : data(dictItems) {}

    explicit Value(const SetPtr& set) : data(set) {}

    explicit Value(const IteratorPtr & iter) : data(iter) {}

    explicit Value(const SlicePtr & slice) : data(slice) {}

    explicit Value(const ByteArrayPtr& byteArray) : data(byteArray) {}

    explicit Value(const FrozenSetPtr& frozenSet): data(frozenSet) {}

    [[nodiscard]] QString toString() const override;
    [[nodiscard]] QString repr() const override;
    [[nodiscard]] QString display() const;

    [[nodiscard]] bool isBool() const;
    [[nodiscard]] bool toBool() const;

    [[nodiscard]] bool isNone() const;
    [[nodiscard]] BigFloat toBigFloat() const;
    [[nodiscard]] BigInt toBigInt() const;
    [[nodiscard]] bool isNumeric() const;
    [[nodiscard]] bool isCallable() const;

    [[nodiscard]] bool isBigInt() const;
    [[nodiscard]] BigInt asBigInt(const QString& = "") const;

    [[nodiscard]] bool isBigFloat() const;
    [[nodiscard]] BigFloat asBigFloat(const QString& = "") const;

    [[nodiscard]] bool isList() const;
    [[nodiscard]] ListPtr asList(const QString& = "") const;

    [[nodiscard]] bool isDict() const;
    [[nodiscard]] DictPtr asDict(const QString& = "") const;

    [[nodiscard]] bool isTuple() const;
    [[nodiscard]] TuplePtr asTuple(const QString& = "") const;

    [[nodiscard]] bool isString() const;
    [[nodiscard]] StrPtr asString(const QString& = "") const;

    [[nodiscard]] bool isDictKeysView() const;
    [[nodiscard]] DictKeysViewPtr asDictKeysView() const;

    [[nodiscard]] bool isDictValuesView() const;
    [[nodiscard]] DictValuesViewPtr asDictValuesView() const;

    [[nodiscard]] bool isDictItemsView() const;
    [[nodiscard]] DictItemsViewPtr asDictItemsView() const;

    [[nodiscard]] bool isFunction() const;
    [[nodiscard]] FunctionPtr asFunction() const;

    [[nodiscard]] bool isClass() const;
    [[nodiscard]] ClassPtr asClass() const;

    [[nodiscard]] bool isInstance() const;
    [[nodiscard]] InstancePtr asInstance() const;

    [[nodiscard]] bool isBoundMethod() const;
    [[nodiscard]] BoundMethodPtr asBoundMethod() const;

    [[nodiscard]] bool isSuper() const;
    [[nodiscard]] SuperPtr asSuper() const;

    [[nodiscard]] bool isBuiltinFunction() const;
    [[nodiscard]] BuiltinFunctionPtr asBuiltinFunction() const;

    [[nodiscard]] bool isProperty() const;
    [[nodiscard]] PropertyPtr asProperty() const;

    [[nodiscard]] bool isStaticMethod() const;
    [[nodiscard]] StaticMethodPtr asStaticMethod() const;

    [[nodiscard]] bool isClassMethod() const;
    [[nodiscard]] ClassMethodPtr asClassMethod() const;

    [[nodiscard]] bool isSet() const;
    [[nodiscard]] SetPtr asSet(const QString& = "") const;

    [[nodiscard]] bool isHashable() const;
    [[nodiscard]] std::size_t hash() const;

    [[nodiscard]] bool isIterable() const;
    [[nodiscard]] IteratorPtr getIterator() const;

    [[nodiscard]] bool isBytes() const;
    [[nodiscard]] BytesPtr asBytes(const QString& = "") const;

    [[nodiscard]] bool isSlice() const;
    [[nodiscard]] SlicePtr asSlice() const;

    [[nodiscard]] bool isByteArray() const;
    [[nodiscard]] ByteArrayPtr asByteArray(const QString& = "") const;

    [[nodiscard]] Value operator+(const Value&) const;

    [[nodiscard]] Value operator-(const Value&) const;

    [[nodiscard]] bool isObject() const;
    [[nodiscard]] ObjectPtr asObject() const;

    [[nodiscard]] bool isFrozenSet() const;
    [[nodiscard]] FrozenSetPtr asFrozenSet(const QString& = "") const;

    [[nodiscard]] Value operator*(const Value&) const;

    [[nodiscard]] Value operator/(const Value&) const;

    [[nodiscard]] Value operator%(const Value&) const;

    [[nodiscard]] Value power (const Value&) const;

    [[nodiscard]] Value intDivide(const Value&) const;

    [[nodiscard]] bool operator==(const Value&) const;

    [[nodiscard]] bool operator<(const Value&) const;

    [[nodiscard]] bool operator!=(const Value&) const;

    [[nodiscard]] bool operator<=(const Value&) const;

    [[nodiscard]] bool operator>(const Value&) const;

    [[nodiscard]] bool operator>=(const Value&) const;

    Value& operator+=(const Value&);

    Value& operator-=(const Value&);

    Value& operator*=(const Value&);

    Value& operator/=(const Value&);

    Value& operator%=(const Value&);

    Value& intDivideEqual(const Value&);

    Value& powerEqual(const Value&);

    [[nodiscard]] bool is(const Value&) const;

    Value operator+() const;

    Value operator-() const;

    [[nodiscard]] bool contains(const Value& value) const;

    [[nodiscard]] QString ascii() const;

    template<class... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    static QString formatFloat(const BigFloat& num);

};

size_t qHash(const Value& value, size_t seed = 0);
#endif //VALUE_H
