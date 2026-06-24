//
// Created by semyo on 25.05.2026.
//

#include "StrValue.h"

#include <QRegularExpression>

#include "ClassUtils.h"
#include "DictValue.h"
#include "IteratorValue.h"
#include "ListValue.h"
#include "TupleValue.h"
#include "Value.h"
#include "../runtime/ProtocolHelpers.h"

namespace {

    std::vector<Value> toValueVector(const QStringList& list) {

        std::vector<Value> result;
        result.reserve(list.size());

        for (const auto& item : list) {
            result.emplace_back(item);
        }

        return result;
    }

}

QString StrValue::toString() const {
    return value;
}

QString StrValue::repr() const {

    QChar quote;

    if (!value.contains('\'')) {
        quote = '\'';
    }
    else if (!value.contains('"')) {
        quote = '"';
    }
    else {
        quote = '\'';
    }

    QString escaped;

    for (const QChar ch : value) {

        switch (ch.unicode()) {

            case '\n':
                escaped += "\\n";
                break;

            case '\t':
                escaped += "\\t";
                break;

            case '\r':
                escaped += "\\r";
                break;

            case '\\':
                escaped += "\\\\";
                break;

            default:

                if (ch == quote) {
                    escaped += '\\';
                    escaped += ch;
                } else {
                    escaped += ch;
                }

                break;
        }
    }

    return QString(quote) + escaped + QString(quote);
}

std::size_t StrValue::len() const {
    return value.size();
}


Value StrValue::getItem(const Value& index) const {

    if (index.isSlice()) {

        const auto sliceObj = index.asSlice();

        QString result;

        iterateSlice(
            normalizeSlice(*sliceObj, value.size()),
            [&](const long long i) {
                result.append(
                    value[static_cast<int>(i)]
                );
            }
        );

        return Value(
            std::make_shared<StrValue>(result));
    }


    if (!index.isBigInt()) {
        throw std::runtime_error("TypeError: string indices must be integers");
    }

    auto i = static_cast<long long>(index.toBigInt());

    if (i < 0) {
        i += value.size();
    }

    if (i < 0 || i >= value.size()) {
        throw std::runtime_error("IndexError: string index out of range");
    }

    return Value(
        std::make_shared<StrValue>(
            QString(value[static_cast<int>(i)])
        )
    );
}

Value StrValue::upper() const {
    return Value(value.toUpper());
}

Value StrValue::lower() const {
    return Value(value.toLower());
}

Value StrValue::strip(const std::optional<QString>& chars) const {

    QString result = value;

    // default whitespace strip
    if (!chars.has_value()) {
        return Value(result.trimmed());
    }

    const QString& trimChars = *chars;

    qsizetype start = 0;
    qsizetype end = result.size() - 1;

    while (start < result.size() && trimChars.contains(result[start])) {
        ++start;
    }

    while (end >= start && trimChars.contains(result[end])) {
        --end;
    }

    return Value(
        result.mid(start, end - start + 1)
    );
}

Value StrValue::split(const std::optional<QString>& sep,
    const std::optional<qsizetype> maxSplit) const {

    QStringList parts;

    // whitespace split
    if (!sep.has_value()) {

        parts = value.split(
            QRegularExpression("\\s+"),
            Qt::SkipEmptyParts
        );

    } else {

        if (sep->isEmpty()) {
            throw std::runtime_error("empty separator");
        }

        if (maxSplit.has_value()) {

            QString remaining = value;
            qsizetype splits = 0;

            while (splits < *maxSplit) {

                const qsizetype idx = remaining.indexOf(*sep);

                if (idx == -1)
                    break;

                parts.append(remaining.left(idx));

                remaining = remaining.mid(idx + sep->size());

                ++splits;
            }

            parts.append(remaining);

        } else {

            parts = value.split(*sep);
        }
    }

    return Value(
        std::make_shared<ListValue>(
            toValueVector(parts)
        )
    );
}

Value StrValue::join(const Value& iterable) const {

    const auto it = iterable.getIterator();

    QStringList parts;

    while (it->hasNext()) {

        Value item = it->next();

        parts.append(item.asString("join iterable")->toString());
    }

    return Value(parts.join(value));
}

Value StrValue::replace(
    const Value& oldValue,
    const Value& newValue,
    const std::optional<Value>& count) const {

    const QString oldStr = oldValue.asString("replace")->toString();
    const QString newStr = newValue.asString("replace")->toString();

    QString result = value;

    // replace all
    if (!count.has_value()) {

        result.replace(oldStr, newStr);

        return Value(result);
    }

    const long long limit = static_cast<long long>(count->asBigInt());

    if (limit < 0) {

        result.replace(oldStr, newStr);

        return Value(result);
    }

    qsizetype pos = 0;
    long long replaced = 0;

    while ((pos = result.indexOf(oldStr, pos)) != -1 && replaced < limit) {

        result.replace(pos, oldStr.size(), newStr);

        pos += newStr.size();

        ++replaced;
    }

    return Value(result);
}

Value StrValue::startswith(
    const Value& prefix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QString prefixStr =
        prefix.asString("startwith")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->asBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->asBigInt());
    }

    // clamp
    begin = std::max<qsizetype>(0, begin);
    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    return Value(sliced.startsWith(prefixStr));
}

Value StrValue::endswith(
    const Value& suffix,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    QString suffixStr =
        suffix.asString("endswith")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->toBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->toBigInt());
    }

    // clamp
    begin = std::max<qsizetype>(0, begin);

    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    return Value(sliced.endsWith(suffixStr));
}

Value StrValue::find(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    const QString subStr = sub.asString("find")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->toBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->toBigInt());
    }

    // clamp
    begin = std::max<qsizetype>(0, begin);

    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    const qsizetype idx = sliced.indexOf(subStr);

    if (idx == -1) {
        return Value(Value::BigInt(-1));
    }

    // важно:
    // возвращаем индекс относительно исходной строки
    return Value(Value::BigInt(begin + idx));
}

Value StrValue::count(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    const QString subStr = sub.asString("count")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->toBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->toBigInt() );
    }

    begin = std::max<qsizetype>(0, begin);

    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    // особый случай:
    // Python: "".count("") == 1
    if (subStr.isEmpty()) {
        return Value(Value::BigInt(sliced.size() + 1));
    }

    qsizetype occurrences = 0;
    qsizetype pos = 0;

    while (true) {

        pos = sliced.indexOf(subStr, pos);

        if (pos == -1) {
            break;
        }

        ++occurrences;

        // не пересекающиеся вхождения
        pos += subStr.size();
    }

    return Value(Value::BigInt(occurrences));
}

Value StrValue::index(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    Value result = find(sub, start, end);

    if (result.toBigInt() == -1) {
        throw std::runtime_error("ValueError: substring not found");
    }

    return result;
}

Value StrValue::rfind(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    const QString subStr = sub.asString("rfind")->toString();

    qsizetype begin = 0;
    qsizetype finish = value.size();

    if (start.has_value()) {
        begin = static_cast<qsizetype>(start->toBigInt());
    }

    if (end.has_value()) {
        finish = static_cast<qsizetype>(end->toBigInt());
    }

    begin = std::max<qsizetype>(0, begin);

    finish = std::min<qsizetype>(finish, value.size());

    if (begin > finish) {
        begin = finish;
    }

    const QString sliced = value.mid(begin, finish - begin);

    const qsizetype pos = sliced.lastIndexOf(subStr);

    if (pos == -1) {
        return Value(Value::BigInt(-1));
    }

    return Value(Value::BigInt(begin + pos));
}

Value StrValue::rindex(
    const Value& sub,
    const std::optional<Value>& start,
    const std::optional<Value>& end) const {

    Value result = rfind(sub, start, end);

    if (result.toBigInt() == -1) {
        throw std::runtime_error("ValueError: substring not found");
    }

    return result;
}

Value StrValue::capitalize() const {

    if (value.isEmpty()) {
        return Value(std::make_shared<StrValue>(""));
    }

    QString result = value.toLower();

    result[0] = result[0].toUpper();

    return Value(std::make_shared<StrValue>(result));
}

Value StrValue::title() const {

    QString result;

    bool newWord = true;

    for (const QChar ch : value) {

        if (!ch.isLetterOrNumber()) {

            result += ch;
            newWord = true;

            continue;
        }

        if (newWord) {
            result += ch.toUpper();
            newWord = false;
        }
        else {
            result += ch.toLower();
        }
    }

    return Value(std::make_shared<StrValue>(result));
}

Value StrValue::swapcase() const {

    QString result;

    for (const QChar ch : value) {

        if (ch.isUpper()) {
            result += ch.toLower();
        }

        else if (ch.isLower()) {
            result += ch.toUpper();
        }

        else {
            result += ch;
        }
    }

    return Value(result);
}

Value StrValue::isalpha() const {

    if (value.isEmpty()) {
        return Value(false);
    }

    for (const QChar& ch : value) {

        if (!ch.isLetter()) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::isdigit() const {

    if (value.isEmpty()) {
        return Value(false);
    }

    for (const QChar& ch : value) {

        if (!ch.isDigit()) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::isalnum() const {

    if (value.isEmpty()) {
        return Value(false);
    }

    for (const QChar& ch : value) {

        if (!ch.isLetterOrNumber()) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::isspace() const {

    if (value.isEmpty()) {
        return Value(false);
    }

    for (const QChar& ch : value) {

        if (!ch.isSpace()) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::add(const Value& other) const {

    if (!other.isString()) {
        throw std::runtime_error(
            "TypeError: can only concatenate str to str"
        );
    }

    QString result = value;
    result += other.asString("__add__")->value;

    return Value(
        std::make_shared<StrValue>(
            std::move(result)
        )
    );
}

Value StrValue::multiply(const Value& other) const {

    if (!other.isNumeric() || other.isBigFloat()) {
        throw std::runtime_error(
            "TypeError: can't multiply sequence by non-int"
        );
    }

    const Value::BigInt numVal = other.toBigInt();

    if (numVal <= 0)
        return Value("");

    if (numVal > std::numeric_limits<qsizetype>::max()) {
        throw std::runtime_error("String repetition too large");
    }

    return Value(value.repeated(static_cast<qsizetype>(numVal)));

}

bool StrValue::equal(const Value &other) const {

    if (!other.isString())
        return false;

    return value == other.asString("__eq__")->value;
}

bool StrValue::notEqual(const Value &other) const {

    if (!other.isString())
        return true;

    return value != other.asString("__ne__")->value;
}

bool StrValue::lessOrEqual(const Value &other) const {

    if (!other.isString()) {
        throw std::runtime_error(
            "TypeError: '<=' not supported between instances of 'str' and other type"
        );
    }

    return value <= other.asString("__le__")->value;
}

bool StrValue::less(const Value &other) const {

    if (!other.isString()) {
        throw std::runtime_error(
            "TypeError: '<' not supported between instances of 'str' and other type"
        );
    }

    return value < other.asString("__lt__")->value;
}

bool StrValue::greaterOrEqual(const Value &other) const {

    if (!other.isString()) {
        throw std::runtime_error(
            "TypeError: '>=' not supported between instances of 'str' and other type"
        );
    }

    return value >= other.asString("__ge__")->value;
}

bool StrValue::greater(const Value &other) const {

    if (!other.isString()) {
        throw std::runtime_error(
            "TypeError: '>' not supported between instances of 'str' and other type"
        );
    }

    return value > other.asString("__gt__")->value;
}

QString StrValue::escapeString(const QString &str) {

    QString result;

    for (const QChar ch : str) {

        switch (ch.unicode()) {

            case '\n':
                result += "\\n";
                break;

            case '\t':
                result += "\\t";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\\':
                result += "\\\\";
                break;

            case '\'':
                result += "\\'";
                break;

            case '\"':
                result += "\\\"";
                break;

            default:
                result += ch;
                break;
        }
    }

    return result;
}

Value StrValue::center(
    const Value& widthValue,
    const std::optional<Value>& fillCharValue) const {

    const auto width = static_cast<qsizetype>(widthValue.toBigInt());

    QString fillChar = " ";

    if (fillCharValue.has_value()) {

        fillChar = fillCharValue->asString("fillchar")->toString();

        if (fillChar.size() != 1) {
            throw std::runtime_error(
                "TypeError: The fill character must be exactly one character long");
        }
    }

    if (width <= value.size()) {
        return Value(value);
    }

    const qsizetype marg = width - value.size();

    const qsizetype left = marg / 2 + (marg & width & 1);

    const qsizetype right = marg - left;

    return Value(
        QString(left, fillChar[0]) +
        value +
        QString(right, fillChar[0])
    );
}

Value StrValue::ljust(
    const Value& widthValue,
    const std::optional<Value>& fillCharValue) const {

    const auto width = static_cast<qsizetype>(widthValue.toBigInt());

    QString fillChar = " ";

    if (fillCharValue.has_value()) {

        fillChar = fillCharValue->asString("fillchar")->toString();

        if (fillChar.size() != 1) {
            throw std::runtime_error(
                "TypeError: The fill character must be exactly one character long"
            );
        }
    }

    if (width <= value.size()) {
        return Value(value);
    }

    const qsizetype padding = width - value.size();

    return Value(value + QString(padding, fillChar[0]));
}

Value StrValue::rjust(
    const Value& widthValue,
    const std::optional<Value>& fillCharValue) const {

    const auto width = static_cast<qsizetype>(widthValue.toBigInt());

    QString fillChar = " ";

    if (fillCharValue.has_value()) {

        fillChar = fillCharValue->asString("fillchar")->toString();

        if (fillChar.size() != 1) {
            throw std::runtime_error(
                "TypeError: The fill character must be exactly one character long"
            );
        }
    }

    if (width <= value.size()) {
        return Value(value);
    }

    const qsizetype padding = width - value.size();

    return Value(QString(padding, fillChar[0]) + value);
}

Value StrValue::lstrip(
    const std::optional<QString>& chars) const {

    if (value.isEmpty()) {
        return Value(value);
    }

    qsizetype start = 0;

    if (!chars.has_value()) {

        while (start < value.size() && value[start].isSpace()) {
            ++start;
        }

    } else {

        const QString& stripChars = *chars;

        while (start < value.size() && stripChars.contains(value[start])) {
            ++start;
        }
    }

    return Value(value.mid(start));
}

Value StrValue::rstrip(const std::optional<QString>& chars) const {

    if (value.isEmpty()) {
        return Value(value);
    }

    qsizetype end = value.size() - 1;

    if (!chars.has_value()) {

        while (
            end >= 0 &&
            value[end].isSpace()
        ) {
            --end;
        }

    } else {

        const QString& stripChars = *chars;

        while (
            end >= 0 &&
            stripChars.contains(value[end])
        ) {
            --end;
        }
    }

    return Value(value.left(end + 1));
}

Value StrValue::islower() const {

    bool hasLetter = false;

    for (const QChar ch : value) {

        if (!ch.isLetter()) {
            continue;
        }

        hasLetter = true;

        if (!ch.isLower()) {
            return Value(false);
        }
    }

    return Value(hasLetter);
}

Value StrValue::isupper() const {

    bool hasLetter = false;

    for (const QChar ch : value) {

        if (!ch.isLetter()) {
            continue;
        }

        hasLetter = true;

        if (!ch.isUpper()) {
            return Value(false);
        }
    }

    return Value(hasLetter);
}

Value StrValue::isdecimal() const {

    if (value.isEmpty()) {
        return Value(false);
    }

    for (const QChar ch : value) {

        if (!ch.isDigit()) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::isnumeric() const {

    if (value.isEmpty()) {
        return Value(false);
    }

    for (const QChar ch : value) {

        if (!ch.isDigit()) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::istitle() const {

    bool hasLetter = false;
    bool newWord = true;

    for (const QChar ch : value) {

        if (!ch.isLetter()) {
            newWord = true;
            continue;
        }

        hasLetter = true;

        if (newWord) {

            if (!ch.isUpper()) {
                return Value(false);
            }

            newWord = false;
        }
        else {

            if (!ch.isLower()) {
                return Value(false);
            }
        }
    }

    return Value(hasLetter);
}

Value StrValue::isASCII() const {

    for (const QChar ch : value) {

        if (ch.unicode() > 127) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::isidentifier() const {

    if (value.isEmpty()) {
        return Value(false);
    }

    const QChar first = value[0];

    if (!(first == '_' || first.isLetter())) {
        return Value(false);
    }

    for (qsizetype i = 1; i < value.size(); ++i) {

        const QChar ch = value[i];

        if (!(ch == '_' || ch.isLetterOrNumber())) {
            return Value(false);
        }
    }

    return Value(true);
}

Value StrValue::isprintable() const {

    for (const QChar ch : value) {

        switch (ch.category()) {

            case QChar::Other_Control:
            case QChar::Other_Format:
            case QChar::Other_Surrogate:
            case QChar::Other_PrivateUse:
            case QChar::Other_NotAssigned:

                return Value(false);

            default:
                break;
        }
    }

    return Value(true);
}

Value StrValue::partition(const Value& sepValue) const {

    const QString sep =
        sepValue
            .asString("partition")
            ->toString();

    if (sep.isEmpty()) {
        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const qsizetype pos = value.indexOf(sep);

    if (pos == -1) {

        return Value(
            std::make_shared<TupleValue>(
                std::vector<Value>{
                    Value(value),
                    Value(""),
                    Value("")
                }
            )
        );
    }

    return Value(
        std::make_shared<TupleValue>(
            std::vector{
                Value(value.left(pos)),
                Value(sep),
                Value(value.mid(pos + sep.size()))
            }
        )
    );
}

Value StrValue::rpartition(const Value& sepValue) const {

    const QString sep =
        sepValue
            .asString("rpartition")
            ->toString();

    if (sep.isEmpty()) {
        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const qsizetype pos = value.lastIndexOf(sep);

    if (pos == -1) {

        return Value(
            std::make_shared<TupleValue>(
                std::vector{
                    Value(""),
                    Value(""),
                    Value(value)
                }
            )
        );
    }

    return Value(
        std::make_shared<TupleValue>(
            std::vector{
                Value(value.left(pos)),
                Value(sep),
                Value(value.mid(pos + sep.size()))
            }
        )
    );
}

Value StrValue::splitlines(
    const std::optional<Value>& keepEnds) const {

    const bool keep = keepEnds.has_value()
            ? keepEnds->toBool()
            : false;

    std::vector<Value> result;

    qsizetype start = 0;

    while (start < value.size()) {

        qsizetype end = start;

        while (
            end < value.size()
            && value[end] != '\n'
            && value[end] != '\r')
        {
            ++end;
        }

        if (end == value.size()) {
            result.emplace_back(value.mid(start));
            break;
        }

        const qsizetype lineEnd = end;

        if (value[end] == '\r'
            && end + 1 < value.size()
            && value[end + 1] == '\n')
        {
            end += 2;
        }
        else {
            ++end;
        }

        if (keep) {

            result.emplace_back(value.mid(start, end - start));
        }
        else {

            result.emplace_back(value.mid(start, lineEnd - start));
        }

        start = end;
    }

    return Value(std::make_shared<ListValue>(result));
}

Value StrValue::zfill(const Value& widthValue) const {

    const auto width = static_cast<qsizetype>(widthValue.toBigInt());

    if (width <= value.size()) {
        return Value(value);
    }

    const qsizetype zeros = width - value.size();

    if (!value.isEmpty()
        && (value[0] == '+' || value[0] == '-'))
    {
        return Value(
            QString(value[0]) +
            QString(zeros, '0') +
            value.mid(1)
        );
    }

    return Value(QString(zeros, '0') + value);
}

Value StrValue::expandtabs(
    const std::optional<Value>& tabsizeValue) const {

    const qsizetype tabsize =
        tabsizeValue.has_value()
            ? static_cast<qsizetype>(tabsizeValue->toBigInt())
            : 8;

    QString result;

    qsizetype column = 0;

    for (const QChar ch : value) {

        if (ch == '\t') {

            const qsizetype spaces =
                tabsize <= 0
                    ? 0
                    : tabsize - (column % tabsize);

            result += QString(spaces, ' ');
            column += spaces;

            continue;
        }

        result += ch;

        if (ch == '\n' || ch == '\r') {
            column = 0;
        }
        else {
            ++column;
        }
    }

    return Value(result);
}

Value StrValue::rsplit(
    const std::optional<QString>& sep,
    const std::optional<qsizetype> maxSplit) const {

    // rsplit(None)
    if (!sep.has_value()) {

        const qsizetype limit =
            maxSplit.has_value()
                ? *maxSplit
                : -1;

        // без ограничения
        if (limit < 0) {

            const QStringList parts =
                value.split(
                    QRegularExpression("\\s+"),
                    Qt::SkipEmptyParts
                );

            return Value(
                std::make_shared<ListValue>(
                    toValueVector(parts)
                )
            );
        }

        QString remaining = value.trimmed();

        QStringList result;

        qsizetype splits = 0;

        while (splits < limit) {

            qsizetype pos = remaining.size() - 1;

            while (pos >= 0 && remaining[pos].isSpace()) {
                --pos;
            }

            if (pos < 0) {
                break;
            }

            const qsizetype wordEnd = pos;

            while (pos >= 0 && !remaining[pos].isSpace()) {
                --pos;
            }

            result.prepend(remaining.mid(pos + 1, wordEnd - pos));

            remaining = remaining.left(pos + 1);

            ++splits;
        }

        remaining = remaining.trimmed();

        if (!remaining.isEmpty()) {
            result.prepend(remaining);
        }

        return Value(
            std::make_shared<ListValue>(
                toValueVector(result)
            )
        );
    }

    const QString& separator = *sep;

    if (separator.isEmpty()) {
        throw std::runtime_error(
            "ValueError: empty separator"
        );
    }

    const qsizetype limit =
        maxSplit.has_value()
            ? *maxSplit
            : -1;

    if (limit < 0) {

        const QStringList parts = value.split(separator);

        return Value(
            std::make_shared<ListValue>(
                toValueVector(parts)
            )
        );
    }

    QString remaining = value;

    QStringList reversed;

    qsizetype splits = 0;

    while (splits < limit) {

        const qsizetype pos = remaining.lastIndexOf(separator);

        if (pos == -1) {
            break;
        }

        reversed.prepend(remaining.mid(pos + separator.size()));

        remaining = remaining.left(pos);

        ++splits;
    }

    reversed.prepend(remaining);

    return Value(
        std::make_shared<ListValue>(
            toValueVector(reversed)
        )
    );
}

//TODO: метод пока костыльный, не поддерживает полностью unicode.
Value StrValue::casefold() const {

    QString result = value.toCaseFolded();

    //TODO: костыль
    // TODO: Full Unicode Case Folding (CaseFolding.txt)
    result.replace(QChar(0x00DF), "ss"); // ß
    result.replace(QChar(0x1E9E), "ss"); // ẞ
    result.replace(QString(QChar(0x0130)), QString("i\u0307")); // İ


    return Value(result);
}

Value StrValue::maketrans(const std::vector<Value>& args) {

    if (args.size() == 1) {

        const auto dict = args[0].asDict("maketrans");

        const auto result = std::make_shared<DictValue>();

        for (const auto& key : dict->getOrder()) {

            Value convertedKey;

            if (key.isString()) {

                const QString str = key.asString("maketrans")->toString();

                if (str.size() != 1) {
                    throw std::runtime_error(
                        "ValueError: string keys in translate table must be length 1"
                    );
                }

                convertedKey = Value(Value::BigInt(str[0].unicode()));
            }
            else if (key.isBigInt()) {

                convertedKey = key;
            }
            else {

                throw std::runtime_error(
                    "TypeError: keys in translate table must be integers or single-character strings"
                );
            }

            result->setItem(convertedKey, dict->getItem(key));
        }

        return Value(result);
    }

    if (args.size() == 2 || args.size() == 3) {

        const QString from = args[0].asString("maketrans")->toString();

        const QString to = args[1].asString("maketrans")->toString();

        if (from.size() != to.size()) {
            throw std::runtime_error(
                "ValueError: the first two maketrans arguments must have equal length"
            );
        }

        const auto result = std::make_shared<DictValue>();

        for (qsizetype i = 0; i < from.size(); ++i) {

            result->setItem(
                Value(Value::BigInt(from[i].unicode())),
                Value(Value::BigInt(to[i].unicode()))
            );
        }

        if (args.size() == 3) {

            const QString remove = args[2].asString("maketrans")->toString();

            for (const auto ch : remove) {

                result->setItem(
                    Value(Value::BigInt(ch.unicode())),
                    Value()
                );
            }
        }

        return Value(result);
    }

    throw std::runtime_error(
        "maketrans expected 1, 2 or 3 arguments"
    );
}

Value StrValue::translate(const Value& table) const {

    const auto dict = table.asDict("translate");

    QString result;

    for (const auto ch : value) {

        const Value key(
            Value::BigInt(ch.unicode())
        );

        Value replacement;

        try {
            replacement = dict->getItem(key);
        }
        catch (...) {

            result += ch;
            continue;
        }

        if (replacement.isNone()) {
            continue;
        }

        if (replacement.isString()) {

            result += replacement.asString("translate")->toString();
            continue;
        }

        if (replacement.isBigInt()) {

            const auto code =
                static_cast<char32_t>(
                    replacement.asBigInt("translate")
                );

            result += QChar(static_cast<char16_t>(code));

            continue;
        }

        throw std::runtime_error(
            "TypeError: character mapping must return integer, None or str"
        );
    }

    return Value(result);
}

Value StrValue::formatMap(const Value& mapping) const {

    const auto dict = mapping.asDict("format_map");

    QString result;

    qsizetype pos = 0;

    while (pos < value.size()) {

        if (value[pos] == '{' &&
            pos + 1 < value.size() &&
            value[pos + 1] == '{')
        {
            result += '{';
            pos += 2;
            continue;
        }

        if (value[pos] == '}' &&
            pos + 1 < value.size() &&
            value[pos + 1] == '}')
        {
            result += '}';
            pos += 2;
            continue;
        }

        if (value[pos] != '{') {
            result += value[pos++];
            continue;
        }

        const qsizetype end = value.indexOf('}', pos);

        if (end == -1) {

            throw std::runtime_error("ValueError: unmatched '{'");
        }

        QString field = value.mid(pos + 1, end - pos - 1);

        QString formatSpec;

        const qsizetype colon = field.indexOf(':');

        if (colon != -1) {

            formatSpec = field.mid(colon + 1);

            field = field.left(colon);
        }

        QString key = field;

        QChar conversion;

        const qsizetype bang = field.indexOf('!');

        if (bang != -1) {

            key = field.left(bang);

            if (bang + 1 >= field.size()) {

                throw std::runtime_error("ValueError: expected conversion");
            }

            conversion = field[bang + 1];
        }

        Value replacement;

        try {
            if (key.isEmpty()) {
                replacement = resolveEmptyFormatField(dict);
            } else {
                replacement = resolveFormatField(dict, key);
            }
        }
        catch (...) {

            throw std::runtime_error(
                "KeyError: '" + key.toStdString() + "'"
            );
        }

        if (conversion == 'r') {

            result += replacement.repr();
        }
        else if (conversion == 's') {

            result += applyFormatSpec(replacement, formatSpec);
        }
        else if (!conversion.isNull()) {

            throw std::runtime_error(
                "ValueError: unknown conversion"
            );
        }
        else {

            result += applyFormatSpec(replacement, formatSpec);
        }

        pos = end + 1;
    }

    return Value(result);
}

Value StrValue::resolveFormatField(
    const std::shared_ptr<DictValue>& dict,
    const QString& field) {

    qsizetype pos = 0;

    QString root;

    while (pos < field.size()
           && field[pos] != '.'
           && field[pos] != '[')
    {
        root += field[pos++];
    }

    Value current = dict->getItem(Value(root));

    while (pos < field.size()) {

        // .attr
        if (field[pos] == '.') {

            ++pos;

            QString attr;

            while (pos < field.size()
                   && field[pos] != '.'
                   && field[pos] != '[')
            {
                attr += field[pos++];
            }

            current = getAttrValue(current, attr);

            continue;
        }

        // [ ... ]
        if (field[pos] == '[') {

            ++pos;

            QString token;

            while (pos < field.size()
                   && field[pos] != ']')
            {
                token += field[pos++];
            }

            if (pos >= field.size()) {
                throw std::runtime_error(
                    "ValueError: unmatched '[' in format field"
                );
            }

            ++pos; // пропускаем ]

            // ['name'] или ["name"]
            if ((token.startsWith('\'') && token.endsWith('\'')) ||
                (token.startsWith('"') && token.endsWith('"')))
            {
                token = token.mid(1, token.size() - 2);

                if (current.isDict()) {

                    current =
                        current.asDict("format_map")
                               ->getItem(Value(token));
                }
                else {

                    current = getItemValue(current, Value(token));
                }

                continue;
            }

            // [123]
            bool ok = false;

            const qlonglong index = token.toLongLong(&ok);

            if (ok) {

                current =
                    getItemValue(
                        current,
                        Value(Value::BigInt(index))
                    );

                continue;
            }

            // [name]
            if (current.isDict()) {

                current =
                    current.asDict("format_map")
                           ->getItem(Value(token));

                continue;
            }

            throw std::runtime_error(
                "ValueError: invalid index in format field"
            );
        }

        throw std::runtime_error(
            "ValueError: invalid format field"
        );
    }

    return current;
}

Value StrValue::getItemValue(const Value& obj, const Value& key) {

    if (obj.isString()) {
        return obj.asString("getitem")->getItem(key);
    }

    if (obj.isList()) {
        return obj.asList("getitem")->getItem(key);
    }

    if (obj.isTuple()) {
        return obj.asTuple("getitem")->getItem(key);
    }

    if (obj.isDict()) {
        return obj.asDict("getitem")->getItem(key);
    }

    throw std::runtime_error(
        "TypeError: object is not subscriptable"
    );
}

Value StrValue::format(
    const std::vector<Value>& args,
    const Kwargs& kwargs) const {
    const auto mapping = std::make_shared<DictValue>();

    // позиционные аргументы
    for (size_t i = 0; i < args.size(); ++i) {

        mapping->setItem(Value(QString::number(i)), args[i]);
    }

    // именованные аргументы
    for (const auto& [name, val] : kwargs) {

        mapping->setItem(Value(name), val);
    }

    return formatMap(Value(mapping));
}

Value StrValue::resolveEmptyFormatField(const std::shared_ptr<DictValue>& dict) {

    if (dict->hasKey(Value("_positional_0"))) {
        return dict->getItem(Value("_positional_0"));
    }

    qlonglong idx = 0;

    while (true) {

        Value key = Value(QString::number(idx));

        if (dict->hasKey(key)) {
            return dict->getItem(key);
        }

        idx++;

        if (idx > 100) break;
    }

    throw std::runtime_error("KeyError: ''");
}

QString StrValue::applyFormatSpec(const Value& value, const QString& spec) {

    QString text = value.toString();

    if (spec.isEmpty()) {
        return text;
    }

    // >10
    if (spec.startsWith('>')) {

        const int width = spec.mid(1).toInt();

        return text.rightJustified(width, ' ');
    }

    // <10
    if (spec.startsWith('<')) {

        const int width =
            spec.mid(1).toInt();

        return text.leftJustified(width, ' ');
    }

    // ^10
    if (spec.startsWith('^')) {

        const int width = spec.mid(1).toInt();

        if (width <= text.length()) {
            return text;
        }

        const int totalSpaces = width - text.length();
        const int leftSpaces = totalSpaces / 2;
        const int rightSpaces = totalSpaces - leftSpaces;

        return QString(leftSpaces, ' ') + text + QString(rightSpaces, ' ');
    }

    // 04
    // 08
    if (spec.size() >= 2 && spec[0] == '0') {

        const int width = spec.toInt();

        return text.rightJustified(width, '0');
    }

    if (spec == "%") {

        if (!value.isBigFloat() && !value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: percent format requires number"
            );
        }

        double d;

        if (value.isBigInt()) {
            d = value.toBigInt().convert_to<double>();
        } else {
            d = value.toBigFloat().convert_to<double>();
        }

        return QString::number(d * 100.0, 'f', 6) + "%";
    }

    if (spec == ",") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: comma format requires integer"
            );
        }

        QString txt = value.toString();

        int insertPos = txt.length() - 3;

        while (insertPos > 0) {

            txt.insert(insertPos, ',');

            insertPos -= 3;
        }

        return txt;
    }

    if (spec == "+d") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: +d format requires integer"
            );
        }

        const auto n = value.toBigInt();

        if (n >= 0) {
            return "+" + value.toString();
        }

        return value.toString();
    }

    if (spec == " d") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: space-sign format requires integer"
            );
        }

        auto n = value.toBigInt();

        if (n >= 0) {
            return " " + value.toString();
        }

        return value.toString();
    }

    if (spec == "d") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: d format requires integer"
            );
        }

        return value.toString();
    }

    if (spec == "x") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: x format requires integer"
            );
        }

        const auto num = value.toBigInt().convert_to<long long>();

        return QString::number(num, 16);
    }

    if (spec == "X") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: X format requires integer"
            );
        }

        const auto num = value.toBigInt().convert_to<long long>();

        return QString::number(num, 16).toUpper();
    }

    if (spec == "o") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: o format requires integer"
            );
        }

        const auto n = value.toBigInt().convert_to<long long>();

        return QString::number(n, 8);
    }

    if (spec == "b") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: b format requires integer"
            );
        }

        long long n =
            value.toBigInt().convert_to<long long>();

        if (n == 0) {
            return "0";
        }

        QString result;

        while (n > 0) {

            result.prepend(
                n & 1 ? '1' : '0'
            );

            n >>= 1;
        }

        return result;
    }

    if (spec.startsWith("+.") && spec.endsWith('f'))
    {
        if (!value.isBigFloat() && !value.isBigInt()) {
            throw std::runtime_error(
                "ValueError: float format requires number"
            );
        }

        const int precision = spec.mid(2, spec.size() - 3).toInt();

        double d;

        if (value.isBigInt()) {
            d = value.toBigInt().convert_to<double>();
        } else {
            d = value.toBigFloat().convert_to<double>();
        }

        QString result = QString::number(d, 'f', precision);

        if (d >= 0) {
            result.prepend('+');
        }

        return result;
    }

    // .2f
    if (spec.startsWith('.') && spec.endsWith('f')) {

        if (!value.isBigFloat() && !value.isBigInt()) {

            throw std::runtime_error("ValueError: float format requires number");
        }

        const int precision = spec.mid(1, spec.size() - 2).toInt();

        double d;

        if (value.isBigInt()) {
            d = value.toBigInt().convert_to<double>();
        } else {
            d = value.toBigFloat().convert_to<double>();
        }

        return QString::number(d, 'f', precision);
    }

    throw std::runtime_error(
        "Unsupported format specifier: "
        + spec.toStdString()
    );
}

QString StrValue::asciiRepr(const Value& value) {

    QString text = value.repr();

    QString result;

    for (QChar ch : text) {

        if (ch.unicode() < 128) {

            result += ch;
        }
        else {

            result +=
                QString("\\u%1").arg(
                    static_cast<int>(ch.unicode()),
                    4,
                    16,
                    QChar('0')
                );
        }
    }

    return result;
}

QString StrValue::applyPercentSpec(
    const Value& value,
    const QString& spec,
     const int precision) {

    if (spec == "s") {
        return value.toString();
    }

    if (spec == "r") {
        return value.repr();
    }

    if (spec == "a") {

        return asciiRepr(value);
    }

    if (spec == "d" ||
    spec == "i" ||
    spec == "u") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "%d requires integer"
            );
        }

        return value.toString();
    }

    if (spec == "x") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "%x requires integer"
            );
        }

        return QString::number(
            value.toBigInt().convert_to<long long>(),
            16
        );
    }

    if (spec == "X") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "%X requires integer"
            );
        }

        return QString::number(
            value.toBigInt().convert_to<long long>(),
            16
        ).toUpper();
    }

    if (spec == "o") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "%o requires integer"
            );
        }

        return QString::number(
            value.toBigInt().convert_to<long long>(),
            8
        );
    }

    if (spec == "b") {

        if (!value.isBigInt()) {
            throw std::runtime_error(
                "%b requires integer"
            );
        }

        auto n = value.toBigInt().convert_to<long long>();

        if (n == 0) {
            return "0";
        }

        QString result;

        while (n > 0) {

            result.prepend(
                (n & 1) ? '1' : '0'
            );

            n >>= 1;
        }

        return result;
    }

    if (spec == "c") {

        if (value.isBigInt()) {

            const auto code = value.toBigInt().convert_to<long long>();

            if (code < 0 || code > 0x10FFFF) {

                throw std::runtime_error(
                    "%c arg not in range(0x110000)"
                );
            }

            const auto cp = static_cast<char32_t>(code);

            return QString::fromUcs4(&cp, 1);
        }

        if (value.isString()) {

            const QString s =
                value.toString();

            if (s.size() != 1) {

                throw std::runtime_error(
                    "%c requires a character"
                );
            }

            return s;
        }

        throw std::runtime_error(
            "%c requires int or char"
        );
    }

    if (spec == "f") {

        double d;

        if (value.isBigInt()) {
            d = value.toBigInt().convert_to<double>();
        }
        else if (value.isBigFloat()) {
            d = value.toBigFloat().convert_to<double>();
        }
        else {
            throw std::runtime_error(
                "%f requires number"
            );
        }

        const int digits = precision >= 0
        ? precision
        : 6;

        return QString::number(
            d,
            'f',
            digits
        );
    }

    if (spec == "e") {

        double d;

        if (value.isBigInt()) {

            d = value.toBigInt().convert_to<double>();

        } else if (value.isBigFloat()) {

            d = value.toBigFloat().convert_to<double>();

        } else {

            throw std::runtime_error(
                "%e requires number"
            );
        }

        const int digits = precision >= 0 ? precision : 6;

        return QString::number(
            d,
            'e',
            digits
        );
    }

    if (spec == "E") {

        double d;

        if (value.isBigInt()) {

            d = value.toBigInt().convert_to<double>();

        } else if (value.isBigFloat()) {

            d = value.toBigFloat().convert_to<double>();

        } else {

            throw std::runtime_error(
                "%E requires number"
            );
        }

        const int digits = precision >= 0 ? precision : 6;

        return QString::number(
            d,
            'e',
            digits
        ).toUpper();
    }

    if (spec == "g") {

        double d;

        if (value.isBigInt()) {

            d = value.toBigInt().convert_to<double>();

        } else if (value.isBigFloat()) {

            d = value.toBigFloat().convert_to<double>();

        } else {

            throw std::runtime_error(
                "%g requires number"
            );
        }

        const int digits = precision >= 0 ? precision : 6;

        return QString::number(d, 'g', digits);
    }

    if (spec == "G") {

        double d;

        if (value.isBigInt()) {

            d = value.toBigInt().convert_to<double>();

        } else if (value.isBigFloat()) {

            d = value.toBigFloat().convert_to<double>();

        } else {

            throw std::runtime_error(
                "%G requires number"
            );
        }

        const int digits = precision >= 0 ? precision : 6;

        return QString::number(d, 'g', digits).toUpper();
    }

    throw std::runtime_error("unsupported format");
}

QString StrValue::formatPercentValue(
    const Value& val,
    const QString& spec,
    const bool leftAlign,
    const bool zeroPad,
    const bool showSign,
    const bool spaceSign,
    const bool alternateForm,
    const int width,
    const int precision) {

    QString replacement = applyPercentSpec(val, spec, precision);

    const bool integerSpec =
    spec == "d" ||
    spec == "i" ||
    spec == "u" ||
    spec == "x" ||
    spec == "X" ||
    spec == "o";

    if (integerSpec && precision > 0) {

        const bool negative = replacement.startsWith('-');

        QString digits = replacement;

        if (negative) {
            digits.remove(0, 1);
        }

        digits = digits.rightJustified(precision, '0');

        replacement = negative
        ? "-" + digits
        : digits;
    }

    QString prefix;

    if (alternateForm) {

        if (spec == "x") {

            prefix = "0x";

        } else if (spec == "X") {

            prefix = "0X";

        } else if (spec == "o") {

            prefix = "0o";
        }
    }

    const bool numericSpec =
    spec == "d" ||
    spec == "i" ||
    spec == "u" ||
    spec == "f" ||
    spec == "e" ||
    spec == "E" ||
    spec == "g" ||
    spec == "G";

    if (numericSpec && !replacement.startsWith('-')) {
        if (showSign) {
            replacement.prepend('+');
        }
        else if (spaceSign) {
            replacement.prepend(' ');
        }
    }

    if (width > 0) {

        if (leftAlign) {

            replacement = prefix + replacement;

            replacement = replacement.leftJustified(width, ' ');
        }
        else if (zeroPad) {

            const int padWidth = width - prefix.size() - replacement.size();

            if (padWidth > 0) {

                replacement =
                    prefix +
                    QString(padWidth, '0') +
                    replacement;
            }
            else {

                replacement = prefix + replacement;
            }
        }
        else {

            replacement = prefix + replacement;

            replacement =
                replacement.rightJustified(
                    width,
                    ' '
                );
        }
    }
    else {

        replacement = prefix + replacement;
    }

    return replacement;
}

Value StrValue::modTuple(const std::shared_ptr<TupleValue>& tuple) const {

    QString result;

    qsizetype tupleIndex = 0;

    for (qsizetype pos = 0; pos < value.size();) {

        if (value[pos] != '%') {

            result += value[pos++];
            continue;
        }

        if (pos + 1 < value.size() && value[pos + 1] == '%') {

            result += '%';

            pos += 2;

            continue;
        }

        bool leftAlign = false;
        bool zeroPad = false;
        bool showSign = false;
        bool spaceSign = false;
        bool alternateForm = false;
        int width = 0;
        int precision = -1;

        qsizetype i = pos + 1;

        while (i < value.size()) {

            if (value[i] == '+') {
                showSign = true;
            }
            else if (value[i] == ' ') {
                spaceSign = true;
            }
            else if (value[i] == '#') {
                alternateForm = true;
            }
            else if (value[i] == '-') {
                leftAlign = true;
            }
            else if (value[i] == '0') {
                zeroPad = true;
            }
            else {
                break;
            }

            ++i;
        }

        if (i < value.size() && value[i] == '*') {
            if (tupleIndex >= tuple->len()) {

                throw std::runtime_error(
                    "not enough arguments for format string"
                );
            }

            Value widthValue =
                tuple->getItem(
                    Value(
                        Value::BigInt(tupleIndex++)
                    )
                );

            if (!widthValue.isBigInt()) {

                throw std::runtime_error(
                    "* wants int"
                );
            }

            width =
                static_cast<int>(
                    widthValue.toBigInt()
                    .convert_to<long long>()
                );

            if (width < 0) {

                leftAlign = true;
                width = -width;
            }

            ++i;
        }
        else {

            while (i < value.size() && value[i].isDigit()) {

                width = width * 10 + value[i].digitValue();

                ++i;
            }
        }

        if (i < value.size() && value[i] == '.') {

            ++i;

            if (i < value.size() && value[i] == '*') {

                if (tupleIndex >= tuple->len()) {

                    throw std::runtime_error(
                        "not enough arguments for format string"
                    );
                }

                Value precisionValue =
                    tuple->getItem(
                        Value(
                            Value::BigInt(tupleIndex++)
                        )
                    );

                if (!precisionValue.isBigInt()) {

                    throw std::runtime_error(
                        "* wants int"
                    );
                }

                precision =
                    static_cast<int>(
                        precisionValue.toBigInt()
                        .convert_to<long long>()
                    );

                ++i;
            }
            else {

                precision = 0;

                while (i < value.size() && value[i].isDigit()) {

                    precision = precision * 10 + value[i].digitValue();

                    ++i;
                }
            }
        }

        if (i >= value.size()) {

            throw std::runtime_error(
                "incomplete format"
            );
        }

        QString spec(value[i]);

        if (tupleIndex >= tuple->len()) {

            throw std::runtime_error(
                "TypeError: not enough arguments for format string"
            );
        }

        Value current = tuple->getItem(
            Value(
                Value::BigInt(tupleIndex++)
            )
        );

        result +=
                formatPercentValue(
                    current,
                    spec,
                    leftAlign,
                    zeroPad,
                    showSign,
                    spaceSign,
                    alternateForm,
                    width,
                    precision
                );

        pos = i + 1;

    }

    if (tupleIndex != tuple->len()) {

        throw std::runtime_error(
            "TypeError: not all arguments converted during string formatting"
        );
    }

    return Value(result);
}

//TODO: метод пока не полнцоценный
// приколы наподобие таких не поддерживаются
// "%s %s" % ("a", "b")
// "%(name)s" % {"name": "Bob"}
Value StrValue::mod(const Value& rhs) const {

    QString text = value;

    qsizetype count = 0;

    for (qsizetype i = 0; i < text.size(); ++i) {

        if (text[i] == '%') {

            if (i + 1 < text.size() &&
                text[i + 1] == '%') {
                ++i;
                continue;
            }

            ++count;
        }
    }

    if (rhs.isDict() && value.contains("%(")) {

        return modMapping(rhs.asDict());
    }

    if (count == 1 && !rhs.isTuple()) {

        return modSingle(rhs);
    }

    if (!rhs.isTuple()) {

        throw std::runtime_error(
            "TypeError: not enough arguments for format string"
        );
    }

    return modTuple(rhs.asTuple());

}

QString StrValue::applyStringFormatSpec(
    const QString& text,
    const QString& spec) {

    bool onlyDigits = true;

    for (QChar ch : spec) {

        if (!ch.isDigit()) {
            onlyDigits = false;
            break;
        }
    }

    if (onlyDigits) {

        const int width = spec.toInt();

        return text.leftJustified(
            width,
            ' '
        );
    }


    if (spec.isEmpty()) {
        return text;
    }

    // >10
    if (spec.startsWith('>')) {

        const int width =
            spec.mid(1).toInt();

        return text.rightJustified(
            width,
            ' '
        );
    }

    // <10
    if (spec.startsWith('<')) {

        const int width =
            spec.mid(1).toInt();

        return text.leftJustified(
            width,
            ' '
        );
    }

    // ^10
    if (spec.startsWith('^')) {

        const int width = spec.mid(1).toInt();

        if (width <= text.size()) {
            return text;
        }

        const int totalPadding = width - text.size();

        const int leftPadding = totalPadding / 2;

        const int rightPadding = totalPadding - leftPadding;

        return QString(leftPadding, ' ')
               + text
               + QString(rightPadding, ' ');
    }

    throw std::runtime_error(
        "Unknown format code for str"
    );


}

Value StrValue::formatSelf(const QString& spec) const {

    if (spec.isEmpty()) {
        return Value(value);
    }

    return Value(
        applyFormatSpec(
            Value(value),
            spec
        )
    );
}

Value StrValue::modSingle(const Value& rhs) const {

    QString text = value;

    const qsizetype pos = text.indexOf('%');

    if (pos == -1) {
        throw std::runtime_error(
            "no format specifier"
        );
    }

    bool leftAlign = false;
    bool zeroPad = false;
    int width = 0;
    int precision = -1;
    bool showSign = false;
    bool spaceSign = false;
    bool alternateForm = false;

    qsizetype i = pos + 1;

    while (i < value.size()) {

        if (value[i] == '+') {
            showSign = true;
        }
        else if (value[i] == ' ') {
            spaceSign = true;
        }
        else if (value[i] == '#') {
            alternateForm = true;
        }
        else if (value[i] == '-') {
            leftAlign = true;
        }
        else if (value[i] == '0') {
            zeroPad = true;
        }
        else {
            break;
        }

        ++i;
    }

    while (i < text.size() && text[i].isDigit()) {
        width = width * 10 + (text[i].digitValue());
        ++i;
    }

    if (i < text.size() && text[i] == '.') {

        ++i;

        precision = 0;

        while (i < text.size() && text[i].isDigit()) {
            precision =
                precision * 10 +
                text[i].digitValue();

            ++i;
        }
    }

    if (i >= text.size()) {
        throw std::runtime_error(
            "incomplete format"
        );
    }

    const QString spec(text[i]);

    const QString replacement =
    formatPercentValue(
        rhs,
        spec,
        leftAlign,
        zeroPad,
        showSign,
        spaceSign,
        alternateForm,
        width,
        precision
    );

    text.replace(
        pos,
        i - pos + 1,
        replacement
    );

    return Value(text);

}

Value StrValue::modMapping(const Value::DictPtr &dict) const {

    QString result;

    for (qsizetype pos = 0; pos < value.size();) {

        if (value[pos] != '%') {

            result += value[pos++];
            continue;
        }

        if (pos + 1 < value.size() && value[pos + 1] == '%') {

            result += '%';

            pos += 2;

            continue;
        }

        if (pos + 1 >= value.size() ||
            value[pos + 1] != '(')
        {
            throw std::runtime_error(
                "invalid mapping format"
            );
        }

        const qsizetype keyStart = pos + 2;

        const qsizetype keyEnd = value.indexOf(')', keyStart);

        if (keyEnd == -1) {

            throw std::runtime_error(
                "incomplete mapping key"
            );
        }

        QString key = value.mid(keyStart, keyEnd - keyStart);

        const qsizetype specPos = keyEnd + 1;

        bool leftAlign = false;
        bool zeroPad = false;
        bool showSign = false;
        bool spaceSign = false;
        bool alternateForm = false;
        int width = 0;
        int precision = -1;

        qsizetype i = specPos;

        while (i < value.size()) {

            if (value[i] == '+') {
                showSign = true;
            }
            else if (value[i] == ' ') {
                spaceSign = true;
            }
            else if (value[i] == '#') {
                alternateForm = true;
            }
            else if (value[i] == '-') {
                leftAlign = true;
            }
            else if (value[i] == '0') {
                zeroPad = true;
            }
            else {
                break;
            }

            ++i;
        }

        while (i < value.size() && value[i].isDigit()) {

            width = width * 10 + value[i].digitValue();

            ++i;
        }

        if (i < value.size() &&
            value[i] == '.')
        {
            ++i;

            precision = 0;

            while (i < value.size() &&
                   value[i].isDigit())
            {
                precision =
                    precision * 10 +
                    value[i].digitValue();

                ++i;
            }
        }

        if (i >= value.size()) {

            throw std::runtime_error(
                "missing format specifier"
            );
        }

        QString spec(value[i]);

        Value fieldValue = dict->getItem(Value(key));

        const QString replacement =
                formatPercentValue(
                    fieldValue,
                    spec,
                    leftAlign,
                    zeroPad,
                    showSign,
                    spaceSign,
                    alternateForm,
                    width,
                    precision
                );

        result += replacement;

        pos = i + 1;
    }

    return Value(result);

}

bool StrValue::contains(const Value& val) const {

    if (!val.isString()) {
        throw std::runtime_error(
            "TypeError: 'in <string>' requires string as left operand"
        );
    }

    return value.contains(
        val.toString()
    );
}

std::size_t StrValue::hash() const {
    return qHash(value);
}

Value StrValue::rmul(const Value& other) const {
    return multiply(other);
}
