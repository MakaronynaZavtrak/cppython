//
// Created by semyo on 25.05.2026.
//

#include "StrValue.h"

#include <QRegularExpression>

#include "DictValue.h"
#include "IteratorValue.h"
#include "ListValue.h"
#include "TupleValue.h"
#include "Value.h"

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
    result.replace(QChar(0x00DF), "ss"); // ß
    result.replace(QChar(0x1E9E), "ss"); // ẞ

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
