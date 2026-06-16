//
// Created by semyo on 01.06.2026.
//
#include <BytesValue.h>

#include "IteratorValue.h"
#include "ListValue.h"
#include "StrValue.h"
#include "TupleValue.h"

#include <iomanip>
#include <sstream>

#include "DictValue.h"
#include "../runtime/ProtocolHelpers.h"

QString BytesValue::repr() const {

    const bool containsSingle = data.contains('\'');
    const bool containsDouble = data.contains('"');

    QChar quote = '\'';

    if (containsSingle && !containsDouble) {
        quote = '"';
    }

    QString result = "b";
    result += quote;

    for (const unsigned char c : data) {

        switch (c) {

            case '\n':
                result += "\\n";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\t':
                result += "\\t";
                break;

            case '\\':
                result += "\\\\";
                break;

            case '\'':

                if (quote == '\'')
                    result += "\\'";
                else
                    result += '\'';

                break;

            case '"':

                if (quote == '"')
                    result += "\\\"";
                else
                    result += '"';

                break;

            default:

                if (c >= 32 && c <= 126) {
                    result += QChar(c);
                } else {
                    result += QString("\\x%1")
                        .arg(c, 2, 16, QLatin1Char('0'));
                }

                break;
        }
    }

    result += quote;

    return result;
}

static void normalizeSliceIndices(int& start, int& end, const int size) {

    if (start < 0) {
        start += size;
    }

    if (end < 0) {
        end += size;
    }

    start = std::clamp(start, 0, size);
    end   = std::clamp(end,   0, size);

    if (end < start) {
        end = start;
    }
}

static std::pair<int, int> getSliceBounds(
    const QByteArray& data,
    const std::optional<Value>& start,
    const std::optional<Value>& end) {
    int startIdx = 0;
    int endIdx = static_cast<int>(data.size());

    if (start.has_value()) {
        startIdx = static_cast<int>(start->toBigInt());
    }

    if (end.has_value()) {
        endIdx = static_cast<int>(end->toBigInt());
    }

    normalizeSliceIndices(
        startIdx,
        endIdx,
        static_cast<int>(data.size())
    );

    return {startIdx, endIdx};
}

static bool isAsciiWhitespace(const unsigned char c) {
    switch (c) {

        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case '\f':
        case '\v':
            return true;

        default:
            return false;
    }
}

static bool isAsciiAlpha(unsigned char ch) {

    return
        (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z');
}

Value BytesValue::getItem(const Value& indexValue) const {

    if (indexValue.isSlice()) {

        const auto sliceObj = indexValue.asSlice();

        QByteArray result;

        iterateSlice(
            normalizeSlice(*sliceObj, data.size()),
            [&](const long long i) {
                result.append(
                    data[static_cast<int>(i)]
                );
            }
        );

        return Value(
            std::make_shared<BytesValue>(result)
        );
    }

    int index = indexValue.asBigInt("__getitem__").convert_to<int>();

    if (data.isEmpty()) {
        throw std::runtime_error(
            "IndexError: index out of range"
        );
    }

    if (index < 0) {
        index += data.size();
    }

    if (index < 0 || index >= data.size()) {
        throw std::runtime_error(
            "IndexError: index out of range"
        );
    }

    return Value(
        Value::BigInt(
            static_cast<unsigned char>(data[index])
        )
    );
}

std::size_t BytesValue::len() const {
    return data.size();
}

Value BytesValue::add(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: can't concat bytes to non-bytes"
        );
    }

    QByteArray result = data;
    result += other.asBytes("__add__")->bytes();

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::multiply(const Value &other) const {

    if (!other.isNumeric() || other.isBigFloat()) {
        throw std::runtime_error(
            "TypeError: can't multiply sequence by non-int"
        );
    }

    const Value::BigInt count = other.toBigInt();

    if (count <= 0) {
        return Value(std::make_shared<BytesValue>(QByteArray()));
    }

    if (count > std::numeric_limits<qsizetype>::max()) {
        throw std::runtime_error("String repetition too large");
    }

    QByteArray result;

    for (Value::BigInt i = 0; i < count; ++i) {
        result += data;
    }

    return Value(std::make_shared<BytesValue>(result));
}

bool BytesValue::equal(const Value& other) const {

    if (!other.isBytes()) {
        return false;
    }

    return data == other.asBytes("__eq__")->bytes();
}

bool BytesValue::notEqual(const Value& other) const {

    if (!other.isBytes()) {
        return true;
    }

    return data != other.asBytes("__ne__")->bytes();
}

bool BytesValue::less(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: '<' not supported between instances of 'bytes' and '" +
            other.toString().toStdString() + "'"
        );
    }

    return data < other.asBytes("__lt__")->bytes();
}

bool BytesValue::lessOrEqual(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: bytes can only be compared with bytes"
        );
    }

    return data <= other.asBytes("__le__")->bytes();
}

bool BytesValue::greater(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: bytes can only be compared with bytes"
        );
    }

    return data > other.asBytes("__gt__")->bytes();
}

bool BytesValue::greaterOrEqual(const Value& other) const {

    if (!other.isBytes()) {
        throw std::runtime_error(
            "TypeError: bytes can only be compared with bytes"
        );
    }

    return data >= other.asBytes("__ge__")->bytes();
}

bool BytesValue::contains(const Value& other) const {

    if (other.isNumeric()) {

        const auto byte = other.toBigInt();

        if (byte < 0 || byte > 255) {
            throw std::runtime_error(
                "ValueError: byte must be in range(0, 256)"
            );
        }

        return data.contains(static_cast<unsigned char>(byte));
    }

    if (other.isBytes()) {

        return data.contains(
            other.asBytes()->bytes()
        );
    }

    throw std::runtime_error(
        "TypeError: a bytes-like object is required"
    );
}

Value BytesValue::find(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    if (!sub.isBytes()) {
        throw std::runtime_error("find() argument must be bytes");
    }

    auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const auto& needle = sub.asBytes()->bytes();

    const QByteArray slice = data.mid(startIdx, endIdx - startIdx);

    const int pos = slice.indexOf(needle);

    if (pos == -1) {
        return Value(Value::BigInt(-1));
    }

    return Value(Value::BigInt(pos + startIdx));
}

Value BytesValue::rfind(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    if (!sub.isBytes()) {
        throw std::runtime_error(
            "rfind() argument must be bytes"
        );
    }

    int startIdx = 0;
    int endIdx = data.size();

    if (start.has_value()) {
        startIdx =
            static_cast<int>(
                start->toBigInt()
            );
    }

    if (end.has_value()) {
        endIdx =
            static_cast<int>(
                end->toBigInt()
            );
    }

    normalizeSliceIndices(startIdx, endIdx, data.size());

    const QByteArray needle = sub.asBytes()->bytes();

    const QByteArray haystack =
        data.mid(
            startIdx,
            endIdx - startIdx
        );

    const int pos = haystack.lastIndexOf(needle);

    if (pos == -1) {

        return Value(
            Value::BigInt(-1)
        );
    }

    return Value(
        Value::BigInt(
            pos + startIdx
        )
    );
}

Value BytesValue::index(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    Value result = find(sub, start, end);

    if (result.toBigInt() == Value::BigInt(-1)) {
        throw std::runtime_error("ValueError: subsection not found");
    }

    return result;
}

Value BytesValue::rindex(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    Value result = rfind(sub, start, end);

    if (result.toBigInt() == Value::BigInt(-1)) {

        throw std::runtime_error(
            "ValueError: subsection not found"
        );
    }

    return result;
}

Value BytesValue::count(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    if (!sub.isBytes()) {
        throw std::runtime_error("count() argument must be bytes");
    }

    auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const QByteArray haystack = data.mid(startIdx, endIdx - startIdx);

    const QByteArray needle = sub.asBytes()->bytes();


    if (needle.isEmpty()) {
        return Value(
            Value::BigInt(haystack.size() + 1)
        );
    }

    int count = 0;
    int pos = 0;

    while (true) {

        pos = haystack.indexOf(needle, pos);

        if (pos == -1) {
            break;
        }

        ++count;

        // неперекрывающиеся вхождения
        pos += needle.size();
    }

    return Value(Value::BigInt(count));
}

Value BytesValue::startsWith(
    const Value& prefix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    if (!prefix.isBytes()) {
        throw std::runtime_error(
            "startswith first arg must be bytes"
        );
    }

    auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const QByteArray slice = data.mid(startIdx, endIdx - startIdx);

    const QByteArray& needle = prefix.asBytes()->bytes();

    return Value(slice.startsWith(needle));
}

Value BytesValue::endsWith(
    const Value& suffix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    const auto [startIdx, endIdx] = getSliceBounds(data, start, end);

    const QByteArray slice = data.mid(startIdx, endIdx - startIdx);

    return Value(
        slice.endsWith(
            suffix.asBytes("endswith")->bytes()
        )
    );
}

Value BytesValue::split(
    const std::optional<Value>& sep,
    const Value::BigInt &maxsplit) const {

    std::vector<Value> result;

    if (!sep.has_value()) {

        qsizetype i = 0;
        Value::BigInt splits = 0;

        while (i < data.size()) {

            while (i < data.size() && isAsciiWhitespace(data[i])) {
                ++i;
            }

            if (i >= data.size()) {
                break;
            }

            if (maxsplit >= 0 && splits >= maxsplit) {

                result.emplace_back(
                    std::make_shared<BytesValue>(
                        data.mid(i)
                    )
                );

                break;
            }

            const qsizetype start = i;

            while (i < data.size() && !isAsciiWhitespace(data[i])) {
                ++i;
            }

            result.emplace_back(
                std::make_shared<BytesValue>(
                    data.mid(start, i - start)
                )
            );

            ++splits;
        }

        return Value(
            std::make_shared<ListValue>(
                std::move(result)
            )
        );
    }

    if (!sep->isBytes()) {
        throw std::runtime_error(
            "split() separator must be bytes"
        );
    }

    const QByteArray delimiter = sep->asBytes()->bytes();

    if (delimiter.isEmpty()) {
        throw std::runtime_error("ValueError: empty separator");
    }

    qsizetype start = 0;
    Value::BigInt splits = 0;

    while (true) {

        if (maxsplit >= 0 && splits >= maxsplit) {
            break;
        }

        const qsizetype pos = data.indexOf(delimiter, start);

        if (pos == -1) {
            break;
        }

        result.emplace_back(
            std::make_shared<BytesValue>(
                data.mid(start, pos - start)
            )
        );

        start = pos + delimiter.size();

        ++splits;
    }

    result.emplace_back(
        std::make_shared<BytesValue>(
            data.mid(start)
        )
    );

    return Value(
        std::make_shared<ListValue>(
            std::move(result)
        )
    );

}

Value BytesValue::rsplit(const std::optional<Value> &sep, const Value::BigInt &maxsplit) const {

    if (!sep.has_value()) {

        std::vector<QByteArray> parts;

        qsizetype i = data.size() - 1;
        Value::BigInt splits = 0;

        while (i >= 0) {

            while (
                i >= 0 &&
                isAsciiWhitespace(
                    static_cast<unsigned char>(data[i])
                )
            ) {
                --i;
            }

            if (i < 0) {
                break;
            }

            const qsizetype end = i + 1;

            while (
                i >= 0 &&
                !isAsciiWhitespace(
                    static_cast<unsigned char>(data[i])
                )
            ) {
                --i;
            }

            const qsizetype start = i + 1;

            parts.push_back(
                data.mid(start, end - start)
            );

            ++splits;

            if (
                maxsplit >= 0 &&
                splits >= maxsplit
            ) {

                while (
                    i >= 0 &&
                    isAsciiWhitespace(
                        static_cast<unsigned char>(data[i])
                    )
                ) {
                    --i;
                }

                if (i >= 0) {
                    parts.push_back(
                        data.left(i + 1)
                    );
                }

                break;
            }
        }

        std::reverse(parts.begin(), parts.end());

        std::vector<Value> result;

        for (const auto& part : parts) {

            result.emplace_back(
                std::make_shared<BytesValue>(
                    part
                )
            );
        }

        return Value(
            std::make_shared<ListValue>(
                std::move(result)
            )
        );
    }

    const QByteArray delimiter = sep->asBytes("rsplit")->bytes();

    if (delimiter.isEmpty()) {
        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    std::vector<QByteArray> parts;

    qsizetype end = data.size();
    Value::BigInt splits = 0;

    while (
        maxsplit < 0 ||
        splits < maxsplit
    ) {

        const qsizetype searchFrom = end - delimiter.size();

        if (searchFrom < 0) {
            break;
        }

        const qsizetype pos =
            data.lastIndexOf(
                delimiter,
                searchFrom
            );

        if (pos == -1) {
            break;
        }

        parts.push_back(
            data.mid(
                pos + delimiter.size(),
                end - pos - delimiter.size()
            )
        );

        end = pos;

        ++splits;
    }

    parts.push_back(data.left(end));

    std::reverse(parts.begin(), parts.end());

    std::vector<Value> result;

    for (const auto& part : parts) {

        result.emplace_back(
            std::make_shared<BytesValue>(
                part
            )
        );
    }

    return Value(
        std::make_shared<ListValue>(
            std::move(result)
        )
    );
}

Value BytesValue::join(const Value& iterable) const {

    if (!iterable.isIterable()) {
        throw std::runtime_error(
            "TypeError: can only join an iterable"
        );
    }

    QByteArray result;

    const auto iter = iterable.getIterator();

    bool first = true;

    while (iter->hasNext()) {

        const Value item = iter->next();

        if (!item.isBytes()) {
            throw std::runtime_error(
                "TypeError: sequence item is not bytes"
            );
        }

        if (!first) {
            result += data;
        }

        result += item.asBytes()->bytes();

        first = false;
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::replace(
    const Value& oldValue,
    const Value& newValue,
    const Value::BigInt& count) const {

    if (!oldValue.isBytes()) {
        throw std::runtime_error(
            "replace() argument 1 must be bytes"
        );
    }

    if (!newValue.isBytes()) {
        throw std::runtime_error(
            "replace() argument 2 must be bytes"
        );
    }

    const QByteArray oldBytes = oldValue.asBytes()->bytes();

    const QByteArray newBytes = newValue.asBytes()->bytes();

    QByteArray result = data;

    if (count < 0) {

        result.replace(oldBytes, newBytes);

        return Value(
            std::make_shared<BytesValue>(
                std::move(result)
            )
        );
    }

    if (oldBytes.isEmpty()) {

        QByteArray out;

        Value::BigInt replacements = 0;

        for (qsizetype i = 0; i <= data.size(); ++i) {

            if (replacements < count) {
                out += newBytes;
                ++replacements;
            }

            if (i < data.size()) {
                out += data[i];
            }
        }

        return Value(
            std::make_shared<BytesValue>(
                std::move(out)
            )
        );
    }

    Value::BigInt replacements = 0;
    int pos = 0;

    while (replacements < count) {

        pos = result.indexOf(oldBytes, pos);

        if (pos == -1) {
            break;
        }

        result.replace(pos, oldBytes.size(), newBytes);

        pos += newBytes.size();

        ++replacements;
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::isAscii() const {

    for (const unsigned char byte : data) {

        if (byte > 127) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isAlpha() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (const unsigned char byte : data) {

        if (!isAsciiAlpha(byte)) {
            return Value(false);
        }
    }

    return Value(true);
}

static bool isAsciiDigit(unsigned char ch) {

    return ch >= '0' && ch <= '9';
}

Value BytesValue::isDigit() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (unsigned char byte : data) {

        if (!isAsciiDigit(byte)) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isAlnum() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (unsigned char byte : data) {

        if (
            !isAsciiAlpha(byte) &&
            !isAsciiDigit(byte)
        ) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isSpace() const {

    if (data.isEmpty()) {
        return Value(false);
    }

    for (const unsigned char byte : data) {

        if (!isAsciiWhitespace(byte)) {
            return Value(false);
        }
    }

    return Value(true);
}

Value BytesValue::isLower() const {

    bool hasCased = false;

    for (const unsigned char c : data) {

        if (c >= 'a' && c <= 'z') {
            hasCased = true;
            continue;
        }

        if (c >= 'A' && c <= 'Z') {
            return Value(false);
        }
    }

    return Value(hasCased);
}

Value BytesValue::isUpper() const {

    bool hasCased = false;

    for (const unsigned char c : data) {

        if (c >= 'A' && c <= 'Z') {
            hasCased = true;
            continue;
        }

        if (c >= 'a' && c <= 'z') {
            return Value(false);
        }
    }

    return Value(hasCased);
}

Value BytesValue::lower() const {

    QByteArray result = data;

    for (char& c : result) {

        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::upper() const {

    QByteArray result = data;

    for (char& c : result) {

        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::swapcase() const {

    QByteArray result = data;

    for (char& c : result) {

        if (c >= 'a' && c <= 'z') {

            c = static_cast<char>(
                c - 'a' + 'A'
            );
        }

        else if (c >= 'A' && c <= 'Z') {

            c = static_cast<char>(
                c - 'A' + 'a'
            );
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::capitalize() const {

    QByteArray result = data;

    if (result.isEmpty()) {
        return Value(
            std::make_shared<BytesValue>(result)
        );
    }

    const auto first =
        static_cast<unsigned char>(result[0]);

    if (first >= 'a' && first <= 'z') {
        result[0] =
            static_cast<char>(first - ('a' - 'A'));
    }

    for (int i = 1; i < result.size(); ++i) {

        const auto c =
            static_cast<unsigned char>(result[i]);

        if (c >= 'A' && c <= 'Z') {

            result[i] =
                static_cast<char>(c + ('a' - 'A'));
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::title() const {

    QByteArray result = data;

    bool newWord = true;

    for (char& c : result) {

        const unsigned char uc =
            static_cast<unsigned char>(c);

        if (uc >= 'A' && uc <= 'Z') {

            if (newWord) {
                // оставить заглавной
            } else {
                c = static_cast<char>(uc - 'A' + 'a');
            }

            newWord = false;
        }

        else if (uc >= 'a' && uc <= 'z') {

            if (newWord) {
                c = static_cast<char>(uc - 'a' + 'A');
            }

            newWord = false;
        }

        else {

            newWord = true;
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::isTitle() const {

    bool hasCased = false;
    bool newWord = true;

    for (const unsigned char c : data) {

        if (c >= 'A' && c <= 'Z') {

            if (!newWord) {
                return Value(false);
            }

            hasCased = true;
            newWord = false;
        }

        else if (c >= 'a' && c <= 'z') {

            if (newWord) {
                return Value(false);
            }

            hasCased = true;
            newWord = false;
        }

        else {

            newWord = true;
        }
    }

    return Value(hasCased);
}

Value BytesValue::lstrip(const std::optional<Value>& chars) const {

    qsizetype pos = 0;

    if (!chars.has_value()) {

        while (
            pos < data.size() &&
            isAsciiWhitespace(
                static_cast<unsigned char>(data[pos])
            )
        ) {
            ++pos;
        }
    }

    else {

        if (!chars->isBytes()) {
            throw std::runtime_error(
                "lstrip arg must be bytes"
            );
        }

        const QByteArray stripChars =
            chars->asBytes()->bytes();

        while (
            pos < data.size() &&
            stripChars.contains(data[pos])
        ) {
            ++pos;
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            data.mid(pos)
        )
    );
}

Value BytesValue::rstrip(const std::optional<Value>& chars) const {

    qsizetype end = data.size();

    if (!chars.has_value()) {

        while (
            end > 0 &&
            isAsciiWhitespace(
                static_cast<unsigned char>(data[end - 1])
            )
        ) {
            --end;
        }
    }

    else {

        if (!chars->isBytes()) {
            throw std::runtime_error(
                "rstrip arg must be bytes"
            );
        }

        const QByteArray stripChars =
            chars->asBytes()->bytes();

        while (
            end > 0 &&
            stripChars.contains(data[end - 1])
        ) {
            --end;
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            data.left(end)
        )
    );
}

Value BytesValue::strip(const std::optional<Value>& chars) const {

    const auto leftStripped =
        lstrip(chars)
            .asBytes();

    return leftStripped->rstrip(chars);
}

Value BytesValue::center(
    const Value::BigInt& width,
    const std::optional<Value>& fillchar) const {

    QByteArray fill = " ";

    if (fillchar.has_value()) {

        if (!fillchar->isBytes()) {
            throw std::runtime_error(
                "center() argument 2 must be bytes"
            );
        }

        fill = fillchar->asBytes()->bytes();

        if (fill.size() != 1) {
            throw std::runtime_error(
                "TypeError: center() argument 2 must be a byte string of length 1"
            );
        }
    }

    const qsizetype targetWidth =
        static_cast<qsizetype>(width);

    if (targetWidth <= data.size()) {

        return Value(
            std::make_shared<BytesValue>(data)
        );
    }

    const qsizetype padding =
        targetWidth - data.size();

    const qsizetype left =
        padding / 2;

    const qsizetype right =
        padding - left;

    QByteArray result;

    result.reserve(targetWidth);

    result.append(left, fill[0]);
    result += data;
    result.append(right, fill[0]);

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::ljust(
    const Value::BigInt& width,
    const std::optional<Value>& fillchar) const {

    QByteArray fill = " ";

    if (fillchar.has_value()) {

        if (!fillchar->isBytes()) {
            throw std::runtime_error(
                "ljust() argument 2 must be bytes"
            );
        }

        fill = fillchar->asBytes()->bytes();

        if (fill.size() != 1) {
            throw std::runtime_error(
                "TypeError: ljust() argument 2 must be a byte string of length 1"
            );
        }
    }

    const qsizetype targetWidth =
        static_cast<qsizetype>(width);

    if (targetWidth <= data.size()) {

        return Value(
            std::make_shared<BytesValue>(data)
        );
    }

    QByteArray result = data;

    result.append(
        targetWidth - data.size(),
        fill[0]
    );

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::rjust(
    const Value::BigInt& width,
    const std::optional<Value>& fillchar) const {

    QByteArray fill = " ";

    if (fillchar.has_value()) {

        if (!fillchar->isBytes()) {
            throw std::runtime_error(
                "rjust() argument 2 must be bytes"
            );
        }

        fill = fillchar->asBytes()->bytes();

        if (fill.size() != 1) {
            throw std::runtime_error(
                "TypeError: rjust() argument 2 must be a byte string of length 1"
            );
        }
    }

    const qsizetype targetWidth = static_cast<qsizetype>(width);

    if (targetWidth <= data.size()) {

        return Value(std::make_shared<BytesValue>(data));
    }

    QByteArray result;

    result.reserve(targetWidth);

    result.append(targetWidth - data.size(), fill[0]);

    result += data;

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::zfill(const Value::BigInt& width) const {

    const qsizetype targetWidth = static_cast<qsizetype>(width);

    if (targetWidth <= data.size()) {

        return Value(std::make_shared<BytesValue>(data));
    }

    const qsizetype fillCount =
        targetWidth - data.size();

    QByteArray result;

    result.reserve(targetWidth);

    if (
        !data.isEmpty() &&
        (data[0] == '+' || data[0] == '-')
    ) {

        result.append(data[0]);

        result.append(fillCount, '0');

        result += data.mid(1);
    }
    else {

        result.append(fillCount, '0');

        result += data;
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::removeprefix(const Value& prefix) const {

    if (!prefix.isBytes()) {
        throw std::runtime_error(
            "removeprefix() argument must be bytes"
        );
    }

    const QByteArray& needle = prefix.asBytes()->bytes();

    if (needle.isEmpty()) {

        return Value(
            std::make_shared<BytesValue>(data)
        );
    }

    if (!data.startsWith(needle)) {

        return Value(
            std::make_shared<BytesValue>(data)
        );
    }

    return Value(
        std::make_shared<BytesValue>(
            data.mid(needle.size())
        )
    );
}

Value BytesValue::removeSuffix(const Value& suffix) const {

    if (!suffix.isBytes()) {
        throw std::runtime_error(
            "removesuffix() argument must be bytes"
        );
    }

    const QByteArray& suffixBytes = suffix.asBytes()->bytes();

    if (!suffixBytes.isEmpty() && data.endsWith(suffixBytes)) {

        return Value(
            std::make_shared<BytesValue>(
                data.mid(
                    0,
                    data.size() - suffixBytes.size()
                )
            )
        );
    }

    return Value(std::make_shared<BytesValue>(data));
}

Value BytesValue::partition(const Value& sep) const {

    if (!sep.isBytes()) {
        throw std::runtime_error(
            "partition() argument must be bytes"
        );
    }

    const QByteArray& separator =
        sep.asBytes()->bytes();

    if (separator.isEmpty()) {
        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const int pos = data.indexOf(separator);

    std::vector<Value> items;

    if (pos == -1) {

        items.emplace_back(
            std::make_shared<BytesValue>(data)
        );

        items.emplace_back(
            std::make_shared<BytesValue>(QByteArray())
        );

        items.emplace_back(
            std::make_shared<BytesValue>(QByteArray())
        );

    } else {

        items.emplace_back(
            std::make_shared<BytesValue>(
                data.left(pos)
            )
        );

        items.emplace_back(
            std::make_shared<BytesValue>(
                separator
            )
        );

        items.emplace_back(
            std::make_shared<BytesValue>(
                data.mid(pos + separator.size())
            )
        );
    }

    return Value(
        std::make_shared<TupleValue>(
            std::move(items)
        )
    );
}

Value BytesValue::rpartition(const Value& sep) const {

    if (!sep.isBytes()) {
        throw std::runtime_error(
            "rpartition() argument must be bytes"
        );
    }

    const QByteArray& separator = sep.asBytes()->bytes();

    if (separator.isEmpty()) {
        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const int pos = data.lastIndexOf(separator);

    std::vector<Value> items;

    if (pos == -1) {

        items.emplace_back(
            std::make_shared<BytesValue>(
                QByteArray()
            )
        );

        items.emplace_back(
            std::make_shared<BytesValue>(
                QByteArray()
            )
        );

        items.emplace_back(
            std::make_shared<BytesValue>(
                data
            )
        );

    } else {

        items.emplace_back(
            std::make_shared<BytesValue>(
                data.mid(0, pos)
            )
        );

        items.emplace_back(
            std::make_shared<BytesValue>(
                separator
            )
        );

        items.emplace_back(
            std::make_shared<BytesValue>(
                data.mid(
                    pos + separator.size()
                )
            )
        );
    }

    return Value(
        std::make_shared<TupleValue>(
            std::move(items)
        )
    );
}

Value BytesValue::splitlines(const bool keepends) const {

    std::vector<Value> result;

    qsizetype start = 0;
    qsizetype i = 0;

    while (i < data.size()) {

        const qsizetype lineEnd = i;
        qsizetype separatorLength = 0;

        const unsigned char ch = data[i];

        if (ch == '\n') {

            separatorLength = 1;

        } else if (ch == '\r') {

            if (
                i + 1 < data.size() &&
                data[i + 1] == '\n'
            ) {

                separatorLength = 2;

            } else {

                separatorLength = 1;
            }

        }

        if (separatorLength != 0) {

            const qsizetype length =
                keepends
                ? (lineEnd - start + separatorLength)
                : (lineEnd - start);

            result.emplace_back(
                std::make_shared<BytesValue>(
                    data.mid(start, length)
                )
            );

            i += separatorLength;
            start = i;

            continue;
        }

        ++i;
    }

    if (start < data.size()) {

        result.emplace_back(
            std::make_shared<BytesValue>(
                data.mid(start)
            )
        );
    }

    return Value(
        std::make_shared<ListValue>(
            std::move(result)
        )
    );
}

Value BytesValue::expandTabs(const int tabsize) const {

    QByteArray result;

    int column = 0;

    for (unsigned char ch : data) {

        if (ch == '\t') {

            const int spaces =
                tabsize - (column % tabsize);

            result.append(
                QByteArray(spaces, ' ')
            );

            column += spaces;

            continue;
        }

        result.append(ch);

        if (ch == '\n' || ch == '\r') {

            column = 0;

        } else {

            ++column;
        }
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::hex() const {

    return Value(
        QString::fromLatin1(
            data.toHex()
        )
    );
}

Value BytesValue::fromHex(const QString& text) {

    QString cleaned;

    for (const QChar ch : text) {

        if (!ch.isSpace()) {
            cleaned += ch;
        }
    }

    if (cleaned.size() % 2 != 0) {

        throw std::runtime_error(
            "ValueError: non-hexadecimal number found in fromhex() arg"
        );
    }

    QByteArray result;

    result.reserve(cleaned.size() / 2);

    for (int i = 0; i < cleaned.size(); i += 2) {

        bool ok = false;

        const QString pair = cleaned.mid(i, 2);

        const int value = pair.toInt(&ok, 16);

        if (!ok) {

            throw std::runtime_error(
                "ValueError: non-hexadecimal number found in fromhex() arg"
            );
        }

        result.append(static_cast<char>(value));
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

Value BytesValue::decode(
    const QString& encoding,
    const QString& errors
) const {

    QString result;

    if (encoding == "utf-8" || encoding == "utf8") {

        QStringDecoder decoder(QStringDecoder::Utf8);

        result = decoder.decode(data);

        if (decoder.hasError()) {

            if (errors == "ignore") {

                QStringDecoder ignoreDecoder(
                    QStringDecoder::Utf8,
                    QStringConverter::Flag::ConvertInvalidToNull
                );

                result = ignoreDecoder.decode(data);

            } else {

                throw std::runtime_error(
                    "UnicodeDecodeError: invalid utf-8 sequence"
                );
            }
        }

    } else if (encoding == "ascii") {

        for (const unsigned char ch : data) {

            if (ch > 127) {

                if (errors == "ignore") {
                    continue;
                }

                throw std::runtime_error(
                    "UnicodeDecodeError: ordinal not in range(128)"
                );
            }

            result += QChar(ch);
        }

    } else if (encoding == "latin-1" ||
               encoding == "latin1") {

        for (unsigned char ch : data) {
            result += QChar(ch);
        }

    } else {

        throw std::runtime_error(
            QString(
                "LookupError: unknown encoding '%1'"
            ).arg(encoding).toStdString()
        );
    }

    return Value(
        std::make_shared<StrValue>(result)
    );
}

Value BytesValue::maketrans(const std::vector<Value>& args) {

    if (args.size() != 2) {
        throw std::runtime_error(
            "maketrans expected 2 arguments"
        );
    }

    if (
        !args[0].isBytes() ||
        !args[1].isBytes()
    ) {
        throw std::runtime_error(
            "maketrans arguments must be bytes"
        );
    }

    const QByteArray from = args[0].asBytes()->bytes();

    const QByteArray to = args[1].asBytes()->bytes();

    if (from.size() != to.size()) {
        throw std::runtime_error(
            "ValueError: maketrans arguments must have equal length"
        );
    }

    QByteArray table(256, '\0');

    for (int i = 0; i < 256; ++i) {
        table[i] = static_cast<char>(i);
    }

    for (int i = 0; i < from.size(); ++i) {

        const auto src = static_cast<unsigned char>(from[i]);
        table[src] = to[i];
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(table)
        )
    );
}

Value BytesValue::translate(
    const Value& table,
    const std::optional<Value>& deleteBytes) const {

    if (!table.isBytes()) {
        throw std::runtime_error(
            "translate table must be bytes"
        );
    }

    const QByteArray mapping = table.asBytes()->bytes();

    if (mapping.size() != 256) {
        throw std::runtime_error(
            "translation table must be 256 characters long"
        );
    }

    QByteArray deleteSet;

    if (deleteBytes.has_value()) {

        if (!deleteBytes->isBytes()) {
            throw std::runtime_error(
                "delete argument must be bytes"
            );
        }

        deleteSet = deleteBytes->asBytes()->bytes();
    }

    QByteArray result;
    result.reserve(data.size());

    for (const unsigned char byte : data) {

        if (deleteSet.contains(
                static_cast<char>(byte)
            )) {
            continue;
            }

        result.append(mapping[byte]);
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

struct BytesFormatSpec {

    bool leftAlign = false;
    bool zeroPad = false;
    bool showSign = false;
    bool spaceSign = false;
    bool alternate = false;
    bool hasPrecision = false;
    bool widthFromArg = false;
    bool precisionFromArg = false;

    QString mappingKey;
    bool hasMappingKey = false;

    int width = 0;
    int precision = 0;
    char type = '\0';
};

static BytesFormatSpec parseBytesFormat(
    const QByteArray& format,
    qsizetype& pos) {

    BytesFormatSpec spec;

    while (pos < format.size()) {

        if (format[pos] == '-') {

            spec.leftAlign = true;
            ++pos;

        } else if (format[pos] == '+') {

            spec.showSign = true;
            ++pos;

        } else if (format[pos] == '0') {

            spec.zeroPad = true;
            ++pos;

        } else if (format[pos] == '#') {

            spec.alternate = true;
            ++pos;

        } else if (format[pos] == ' ') {

            spec.spaceSign = true;
            ++pos;

        } else {

            break;
        }
    }

    if (pos < format.size() && format[pos] == '*') {

        spec.widthFromArg = true;
        ++pos;
    } else {
        while (
        pos < format.size() &&
            std::isdigit(
                static_cast<unsigned char>(
                    format[pos]
                )
            )
        ) {
            spec.width = spec.width * 10 + (format[pos] - '0');
            ++pos;
        }
    }

    while (
        pos < format.size() &&
        std::isdigit(
            static_cast<unsigned char>(
                format[pos]
            )
        )
    ) {

        spec.width = spec.width * 10 + (format[pos] - '0');
        ++pos;
    }

    if (pos < format.size() && format[pos] == '.') {

        spec.hasPrecision = true;

        ++pos;

        if (pos < format.size() && format[pos] == '*') {

            spec.precisionFromArg = true;
            ++pos;

        } else {

            while (
                pos < format.size() &&
                std::isdigit(
                    static_cast<unsigned char>(
                        format[pos]
                    )
                )
            ) {

                spec.precision = spec.precision * 10 + (format[pos] - '0');
                ++pos;
            }
        }
    }

    if (pos >= format.size()) {

        throw std::runtime_error(
            "incomplete format"
        );
    }

    spec.type = format[pos];

    return spec;
}

static QByteArray applyWidth(
    const QByteArray& value,
    const BytesFormatSpec& spec) {

    if (spec.width <= value.size()) {
        return value;
    }

    const int padding = spec.width - value.size();

    if (spec.leftAlign) {
        return value + QByteArray(padding, ' ');
    }

    const char fillChar = spec.zeroPad ? '0' : ' ';

    if (fillChar == '0') {

        if (
        value.startsWith("-") ||
        value.startsWith("+") ||
        value.startsWith(" ")
    ) {

        return value.left(1)
            + QByteArray(padding, '0')
            + value.mid(1);
    }

    if (
        value.startsWith("0x") ||
        value.startsWith("0X") ||
        value.startsWith("0o")
    ) {

        return value.left(2)
            + QByteArray(padding, '0')
            + value.mid(2);
    }
}


    return QByteArray(padding, fillChar) + value;
}

static void applySign(
    QByteArray& result,
    const BytesFormatSpec& spec,
    const Value::BigInt& number) {

    if (number < 0) {
        return;
    }

    if (spec.showSign) {

        result.prepend('+');
        return;
    }

    if (spec.spaceSign) {

        result.prepend(' ');
    }
}

static QByteArray applyPrecision(
    QByteArray value,
    const BytesFormatSpec& spec) {

    if (!spec.hasPrecision) {
        return value;
    }

    switch (spec.type) {

        case 's':
        case 'b':
        case 'r':
        case 'a':

            if (value.size() > spec.precision) {

                value = value.left(spec.precision);
            }

            return value;

        case 'd':
        case 'i':
        case 'u':
        case 'x':
        case 'X':
        case 'o': {
            QByteArray prefix;

            if (
                value.startsWith("0x") ||
                value.startsWith("0X") ||
                value.startsWith("0o")
            ) {
                prefix = value.left(2);
                value = value.mid(2);
            }
            else if (
                !value.isEmpty() &&
                (
                    value[0] == '+' ||
                    value[0] == '-' ||
                    value[0] == ' '
                )
            ) {
                prefix = value.left(1);
                value = value.mid(1);
            }

            while (value.size() < spec.precision) {

                value.prepend('0');
            }

            return prefix + value;
        }

        default:
            return value;
    }
}

QByteArray stripTrailingZeros(QByteArray value) {

    qsizetype expPos = value.indexOf('e');

    if (expPos == -1)
        expPos = value.indexOf('E');

    QByteArray exponent;

    if (expPos != -1) {

        exponent = value.mid(expPos);
        value = value.left(expPos);
    }

    while (
        value.contains('.') &&
        value.endsWith('0')
    ) {
        value.chop(1);
    }

    if (value.endsWith('.')) {
        value.chop(1);
    }

    return value + exponent;
}

int computeExponent(const Value::BigFloat & number) {

    std::ostringstream oss;

    oss << std::scientific
        << std::setprecision(1)
        << number;


    const int exponent = std::stoi(
        oss.str().substr(
            oss.str().find('e') + 1
        )
    );

    return exponent;
}

static QByteArray bigIntToBase(
    Value::BigInt number,
    int base,
    bool upper = false
) {

    if (number == 0)
        return "0";

    const char* digitsLower = "0123456789abcdef";

    const char* digitsUpper = "0123456789ABCDEF";

    const char* digits = upper
        ? digitsUpper
        : digitsLower;

    const bool negative = number < 0;

    if (negative)
        number = -number;

    QByteArray result;

    while (number > 0) {

        const int digit = (number % base).convert_to<int>();

        result.prepend(digits[digit]);

        number /= base;
    }

    if (negative)
        result.prepend('-');

    return result;
}

static QByteArray formatBytesArgument(
    const BytesFormatSpec& specifier,
    const Value& value) {

    QByteArray result;

    switch (specifier.type) {

        case 's':
        case 'b':

            if (!value.isBytes()) {
                throw std::runtime_error(
                    "%s/%b requires bytes"
                );
            }

            result = value.asBytes()->bytes();
            break;

        case 'd':
        case 'i':
        case 'u': {

            const auto number = value.toBigInt();

            result = QByteArray::fromStdString(number.str());

            applySign(result, specifier, number);

            break;
        }

        case 'x': {

            const auto number = value.toBigInt();

            result = bigIntToBase(number, 16);

            if (specifier.alternate) {
                result.prepend("0x");
            }

            applySign(result, specifier, number);

            break;
        }

        case 'X': {

            const auto number = value.toBigInt();

            result = bigIntToBase(number, 16, true);

            if (specifier.alternate) {
                result.prepend("0X");
            }

            applySign(result, specifier, number);

            break;
        }

        case 'o': {
            const auto number = value.toBigInt();

            result = bigIntToBase(number, 8);

            if (specifier.alternate) {
                result.prepend("0o");
            }

            applySign(result, specifier, number);

            break;
        }

        case 'c': {

            const auto code = value.toBigInt();

            if (code < 0 || code > 255) {

                throw std::runtime_error(
                    "%c arg not in range(256)"
                );
            }

            result = QByteArray(
                1,
                static_cast<char>(code)
            );

            break;

        }

        case 'r': {

            result = value.repr().toUtf8();

            break;
        }

        case 'a': {

            result = value.ascii().toUtf8();

            break;
        }

        case 'f':
        case 'F': {

            const auto number = value.toBigFloat();

            int precision =
                specifier.hasPrecision
                ? specifier.precision
                : 6;

            std::ostringstream oss;

            oss << std::fixed
                << std::setprecision(precision)
                << number;

            result = QByteArray::fromStdString(
                oss.str()
            );

            if (number >= 0) {

                if (specifier.showSign) {
                    result.prepend('+');
                }
                else if (specifier.spaceSign) {
                    result.prepend(' ');
                }
            }

            break;
        }

        case 'e': {

            const auto number = value.toBigFloat();

            const int precision =
                specifier.hasPrecision
                ? specifier.precision
                : 6;

            std::ostringstream oss;

            if (precision == 0) {

                Value::BigFloat mantissa = number;
                int exponent = 0;

                if (mantissa != 0) {

                    while (abs(mantissa) >= 10) {
                        mantissa /= 10;
                        ++exponent;
                    }

                    while (abs(mantissa) < 1) {
                        mantissa *= 10;
                        --exponent;
                    }
                }

                oss << static_cast<long long>(mantissa);

                oss << 'e';

                oss << (exponent >= 0 ? '+' : '-');

                oss << std::setw(2)
                    << std::setfill('0')
                    << std::abs(exponent);

                result = QByteArray::fromStdString(
                    oss.str()
                );

            } else {

                oss << std::scientific
                    << std::setprecision(precision)
                    << number;

                result = QByteArray::fromStdString(oss.str());
            }

            if (number >= 0) {

                if (specifier.showSign) {
                    result.prepend('+');
                }
                else if (specifier.spaceSign) {
                    result.prepend(' ');
                }
            }

            break;
        }

        case 'E': {

            const auto number = value.toBigFloat();

            const int precision =
                specifier.hasPrecision
                ? specifier.precision
                : 6;

            std::ostringstream oss;

            if (precision == 0) {

                Value::BigFloat mantissa = number;
                int exponent = 0;

                if (mantissa != 0) {

                    while (abs(mantissa) >= 10) {
                        mantissa /= 10;
                        ++exponent;
                    }

                    while (abs(mantissa) < 1) {
                        mantissa *= 10;
                        --exponent;
                    }
                }

                oss << static_cast<long long>(mantissa);

                oss << 'E';

                oss << (exponent >= 0 ? '+' : '-');

                oss << std::setw(2)
                    << std::setfill('0')
                    << std::abs(exponent);

                result = QByteArray::fromStdString(
                    oss.str()
                );

            } else {

                oss << std::scientific
                    << std::setprecision(precision)
                    << number;

                result = QByteArray::fromStdString(oss.str());
            }

            result.replace('e', 'E');

            if (number >= 0) {

                if (specifier.showSign) {
                    result.prepend('+');
                }
                else if (specifier.spaceSign) {
                    result.prepend(' ');
                }
            }

            break;
        }

        case 'g':
        case 'G': {

            const auto number = value.toBigFloat();

            int precision =
                specifier.hasPrecision
                ? specifier.precision
                : 6;

            if (precision == 0)
                precision = 1;

            int exponent = computeExponent(number);

            std::ostringstream oss;

            bool useScientific =
                exponent < -4 ||
                exponent >= precision;

            if (useScientific) {

                oss << std::scientific
                    << std::setprecision(
                        precision - 1
                    );

            } else {

                oss << std::fixed
                    << std::setprecision(
                        precision - exponent - 1
                    );
            }

            oss << number;

            result =
                QByteArray::fromStdString(
                    oss.str()
                );

            result =
                stripTrailingZeros(result);

            if (specifier.type == 'G') {
                result.replace('e', 'E');
            }

            if (number >= 0) {

                if (specifier.showSign)
                    result.prepend('+');
                else if (specifier.spaceSign)
                    result.prepend(' ');
            }

            break;
        }

        default: throw std::runtime_error(
        QString("unsupported format character '%1'")
            .arg(specifier.type)
            .toStdString());
    }

    result = applyPrecision(result, specifier);

    return applyWidth(result, specifier);
}

//TODO: не реализован пока флаг %q
// Когда появится нормальный механизм исключений, надо будет проверить:
// b'%d' % b'abc'
// b'%c' % 1000
// b'%s' % 42
// b'%(x)s' % {}
Value BytesValue::mod(const Value& rhs) const {

    std::vector<Value> arguments;

    if (rhs.isTuple()) {

        arguments = rhs.asTuple()->items;

    } else if (!rhs.isDict()) {

        arguments.push_back(rhs);
    }

    std::size_t argIndex = 0;

    QByteArray result;

    for (qsizetype i = 0; i < data.size(); ++i) {

        if (data[i] != '%') {

            result.append(data[i]);
            continue;
        }

        if (i + 1 >= data.size()) {

            throw std::runtime_error("incomplete format");
        }

        ++i;

        QString mappingKey;
        bool hasMappingKey = false;

        if (i < data.size() && data[i] == '(') {

            hasMappingKey = true;

            ++i;

            while (i < data.size() && data[i] != ')') {

                mappingKey += QChar(data[i]);
                ++i;
            }

            if (i >= data.size() || data[i] != ')') {

                throw std::runtime_error("incomplete format key");
            }

            ++i;
        }

        BytesFormatSpec spec = parseBytesFormat(data, i);

        spec.hasMappingKey = hasMappingKey;
        spec.mappingKey = mappingKey;

        if (spec.type == '%') {

            result.append('%');
            continue;
        }

        BytesFormatSpec effectiveSpec = spec;

        if (effectiveSpec.widthFromArg) {

            if (argIndex >= arguments.size()) {

                throw std::runtime_error(
                    "not enough arguments for format string"
                );
            }

            effectiveSpec.width =
                static_cast<int>(
                    arguments[argIndex++].toBigInt()
                );

            if (effectiveSpec.width < 0) {

                effectiveSpec.leftAlign = true;
                effectiveSpec.width = -effectiveSpec.width;
            }
        }

        if (effectiveSpec.precisionFromArg) {

            if (argIndex >= arguments.size()) {

                throw std::runtime_error(
                    "not enough arguments for format string"
                );
            }

            effectiveSpec.precision =
                static_cast<int>(
                    arguments[argIndex++].toBigInt()
                );

            if (effectiveSpec.precision < 0) {

                effectiveSpec.hasPrecision = true;
                effectiveSpec.precision = 0;
            }
        }

        Value currentArg;

        if (effectiveSpec.hasMappingKey) {

            if (!rhs.isDict()) {

                throw std::runtime_error(
                    "format requires a mapping"
                );
            }

            auto dict = rhs.asDict();

            const auto& elements = dict->getElementsRef();

            auto bytesKey = Value(
                std::make_shared<BytesValue>(
                    effectiveSpec.mappingKey.toUtf8()
                )
            );

            auto it = elements.find(bytesKey);

            if (it == elements.end()) {

                throw std::runtime_error(
                    "missing key in mapping"
                );
            }

            currentArg = it.value();

        } else {

            if (argIndex >= arguments.size()) {

                throw std::runtime_error(
                    "not enough arguments for format string"
                );
            }

            currentArg = arguments[argIndex++];
        }

        result += formatBytesArgument(
            effectiveSpec,
            currentArg
        );
    }

    if (argIndex != arguments.size()) {

        throw std::runtime_error(
            "not all arguments converted during bytes formatting"
        );
    }

    return Value(
        std::make_shared<BytesValue>(
            std::move(result)
        )
    );
}

BytesValue::BytesValue(QByteArray data) : data(std::move(data)) {}

const QByteArray& BytesValue::bytes() const {
    return data;
}

QString BytesValue::toString() const {
    return repr();
}

Value BytesValue::_bytes_() const {

    return Value(
        std::make_shared<BytesValue>(data)
    );
}

Value BytesValue::rmul(const Value& other) const {
    return multiply(other);
}
