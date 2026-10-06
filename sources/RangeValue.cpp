//
// Created by semyo on 12.07.2026.
//

#include "RangeValue.h"
#include "../exception/TypeErrorException.h"
#include "../exception/ValueErrorException.h"
#include "../exception/IndexErrorException.h"
#include "../runtime/ProtocolHelpers.h"

RangeValue::RangeValue(Value::BigInt start, Value::BigInt stop, Value::BigInt step)
    : start(std::move(start)), stop(std::move(stop)), step(std::move(step)) {

    if (this->step == 0) {
        throw ValueErrorException("range() arg 3 must not be zero");
    }
}

std::size_t RangeValue::len() const {

    if (step > 0) {
        if (start >= stop) return 0;
        return static_cast<std::size_t>(((stop - start - 1) / step + 1).convert_to<long long>());
    }

    if (start <= stop) return 0;
    return static_cast<std::size_t>(((start - stop - 1) / (-step) + 1).convert_to<long long>());
}

QString RangeValue::toString() const {
    return repr();
}

QString RangeValue::repr() const {

    if (step == 1) {
        return QString("range(%1, %2)")
            .arg(QString::fromStdString(start.str()))
            .arg(QString::fromStdString(stop.str()));
    }

    return QString("range(%1, %2, %3)")
        .arg(QString::fromStdString(start.str()))
        .arg(QString::fromStdString(stop.str()))
        .arg(QString::fromStdString(step.str()));
}

Value RangeValue::getItem(const Value& index) const {

    const auto n = static_cast<long long>(len());

    if (index.isSlice()) {

        const auto sliceObj = index.asSlice();
        const auto normalized = normalizeSlice(*sliceObj, len());

        Value::BigInt newStart = start + normalized.start * step;
        Value::BigInt newStep = step * normalized.step;

        long long count = 0;
        for (long long i = normalized.start; normalized.step > 0 ? i < normalized.stop : i > normalized.stop; i += normalized.step) {
            count++;
        }

        Value::BigInt newStop = newStart + newStep * count;

        return Value(std::make_shared<RangeValue>(newStart, newStop, newStep));
    }

    if (!index.isBigInt() && !index.isBool()) {
        throw TypeErrorException("range indices must be integers or slices");
    }

    long long i = index.toBigInt().convert_to<long long>();

    if (i < 0) i += n;

    if (i < 0 || i >= n) {
        throw IndexErrorException("range object index out of range");
    }

    return Value(Value::BigInt(start + step * i));
}

bool RangeValue::equal(const Value& other) const {

    if (!other.isRange()) return false;

    const auto rhs = other.asRange();

    const auto lenSelf = len();
    const auto lenOther = rhs->len();

    if (lenSelf != lenOther) return false;
    if (lenSelf == 0) return true;

    if (start != rhs->start) return false;
    if (lenSelf == 1) return true;

    return step == rhs->step;
}

bool RangeValue::notEqual(const Value& other) const { return !equal(other); }

bool RangeValue::lessOrEqual(const Value&) const {
    throw TypeErrorException("'<=' not supported between instances of 'range'");
}
bool RangeValue::less(const Value&) const {
    throw TypeErrorException("'<' not supported between instances of 'range'");
}
bool RangeValue::greaterOrEqual(const Value&) const {
    throw TypeErrorException("'>=' not supported between instances of 'range'");
}
bool RangeValue::greater(const Value&) const {
    throw TypeErrorException("'>' not supported between instances of 'range'");
}

std::size_t RangeValue::hash() const {

    const auto n = len();

    std::size_t seed = std::hash<long long>{}(static_cast<long long>(n));

    if (n > 0) {
        seed ^= std::hash<long long>{}(start.convert_to<long long>()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    if (n > 1) {
        seed ^= std::hash<long long>{}(step.convert_to<long long>()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

bool RangeValue::contains(const Value& value) const {

    if (!value.isBigInt() && !value.isBool()) {
        return false;
    }

    const Value::BigInt v = value.toBigInt();

    if (step > 0) {
        if (v < start || v >= stop) return false;
    } else {
        if (v > start || v <= stop) return false;
    }

    return (v - start) % step == 0;
}

Value RangeValue::count(const Value& value) const {
    return Value(Value::BigInt(contains(value) ? 1 : 0));
}

Value RangeValue::index(const Value& value) const {

    if (!contains(value)) {
        throw ValueErrorException(value.repr() + " is not in range");
    }

    Value::BigInt v = value.toBigInt();
    return Value(Value::BigInt((v - start) / step));
}

bool RangeValue::toBool() const {
    return len() > 0;
}