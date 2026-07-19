#include "Lexer.h"

#include "../exception/SyntaxErrorException.h"
#include "../exception/ValueErrorException.h"

/**
 * Разбивает заданный исходный код на QVector токенов. Этот метод
 * обрабатывает входной код и создаёт коллекцию токенов,
 * представляющих лексические элементы, такие как идентификаторы, числа, строки,
 * ключевые слова, операторы и другие, сохраняя при этом метаданные, такие как номера строк.
 *
 * @param code Исходный код для токенизации, представленный в виде QString.
 *
 * @return QVector, содержащий элементы Token, каждый из которых представляет
 *         токенизированный компонент входного исходного кода.
 */
QVector<Token> Lexer::tokenize(const QString& code) {

    QVector<Token> tokens;
    pos = 0;
    line = 1;
    lineStartPos = 0;
    indentStack.clear();
    indentStack.push_back(0);

    int bracketDepth = 0;

    while (pos < code.length()) {

        if (!tolerant) {
            int p = pos;
            while (p < code.length() && code[p] != '\n' && code[p].isSpace()) p++;

            if (p < code.length() && code[p] == '#') {
                // пробелы + комментарий до конца строки
                pos = p;
                while (pos < code.length() && code[pos] != '\n') pos++;
            }
            else if (p >= code.length() || code[p] == '\n') {
                // строка из одних пробелов — отдаём '\n'/EOF основному циклу
                pos = p;
            }
            // иначе впереди код — pos не трогаем, отступ прочитает nextToken
        }

        if (pos >= code.length()) break;

        if (QChar ch = code[pos]; ch == '\n') {

            // implicit line joining
            if (bracketDepth > 0) {
                pos++;
                line++;
                lineStartPos = pos;
                continue;
            }

            const int lineStart = pos + 1;

            // заглядываем вперёд: пустая ли следующая строка?
            int spaceCount = 0, tmpPos = pos + 1;

            while (tmpPos < code.length() &&
                (code[tmpPos] == ' ' || code[tmpPos] == '\t')) {

                if (code[tmpPos] == ' ')
                    spaceCount++;
                else
                    spaceCount += 4;

                tmpPos++;
            }

            const bool blankLine = (tmpPos >= code.length() || code[tmpPos] == '\n');

            if (tokens.isEmpty() || tokens.last().type != TOKEN_NEWLINE) {
                tokens.push_back(Token(TOKEN_NEWLINE, "", line));
            }

            pos = tmpPos;
            line++;
            lineStartPos = lineStart;

            // пустая строка не участвует в расчёте отступов
            if (blankLine) {
                continue;
            }

            if (spaceCount > indentStack.last()) {
                indentStack.append(spaceCount);
                tokens.push_back(Token(TOKEN_INDENT, "", line));
            } else while (spaceCount < indentStack.last()) {
                indentStack.pop_back();
                tokens.push_back(Token(TOKEN_DEDENT, "", line));
            }

            continue;
        }

        Token token = nextToken(code);

        if (token.type == TOKEN_EOF) {
            break;
        }

        // отслеживаем глубину скобок
        if (token.type == TOKEN_OP) {
            if (token.value == "(" || token.value == "[" || token.value == "{") {
                ++bracketDepth;
            }
            else if (token.value == ")" || token.value == "]" || token.value == "}") {
                if (bracketDepth > 0) --bracketDepth;
            }
        }

        tokens.append(token);
    }

    while (indentStack.size() > 1) {
        indentStack.pop_back();
        tokens.push_back(Token(TOKEN_DEDENT, "", line));
    }

    Token eofTok(TOKEN_EOF, "", line);
    eofTok.startColumn = currentColumn();
    eofTok.endColumn = currentColumn();
    tokens.push_back(eofTok);

    return tokens;
}

void Lexer::setTolerant(const bool t) {
    tolerant = t;
}

int Lexer::currentColumn() const {
    return pos - lineStartPos + 1;
}

/**
 * Извлекает следующий токен из заданного исходного кода. Этот метод анализирует
 * текст с текущей позиции, пропуская пробелы и комментарии, и идентифицирует
 * лексический элемент, например число, строку, идентификатор или оператор.
 *
 * @param code Исходный код, представленный в виде QString, из которого
 *             извлекается следующий токен.
 *
 * @return Объект Token, представляющий следующий токен, обнаруженный в коде.
 *         Если достигнут конец кода, возвращается токен типа TOKEN_EOF.
 */
Token Lexer::nextToken(const QString& code) {

    while (true) {

        const int oldPos = pos;

        skipWhitespace(code);

        if (tolerant && pos < code.length() && code[pos] == '#') break;

        skipComment(code);

        if (pos == oldPos)
            break;
    }

    if (pos >= code.length()) {
        Token tok{TOKEN_EOF, "", line};
        tok.startColumn = currentColumn();
        tok.endColumn = currentColumn();
        return tok;
    }

    const int startCol = currentColumn();
    const int startPosition = pos;

    const QChar ch = code[pos];

    Token token = [&]() -> Token {

        if (tolerant && ch == '#') return readComment(code);

        if (ch.isDigit()) {
            return readNumber(code);
        }

        if ((ch == 'b' || ch == 'B') &&
            pos + 1 < code.length() &&
            (code[pos + 1] == '"' || code[pos + 1] == '\'')) {
            return readBytes(code);
            }

        if (ch == '\"' || ch == '\'') {
            return readString(code);
        }

        if (ch.isLetter() || ch == '_') {
            return readIdentifierOrBool(code);
        }

        return readOperator(code);
    }();

    token.startColumn = startCol;
    token.endColumn = currentColumn();
    token.startPos = startPosition;
    token.endPos = pos;

    return token;
}

/**
 * Читает числовой литерал из исходного кода и возвращает соответствующий токен.
 * Этот метод поддерживает как целые числа, так и числа с плавающей запятой.
 *
 * @param code Строка, представляющая входной исходный код.
 *
 * @return Token с типом TOKEN_NUMBER, содержащий числовое значение и информацию
 *         о строке, в которой находится число.
 */
Token Lexer::readNumber(const QString& code) {

    const int start = pos;
    bool hasDot = false;
    bool hasExp = false;

    while (pos < code.length()) {
        QChar ch = code[pos];

        if (ch.isDigit() || ch == '_') {
            pos++;
        }
        else if (ch == '.' && !hasDot && !hasExp) {
            hasDot = true;
            pos++;
        }
        else if ((ch == 'e' || ch == 'E') && !hasExp) {
            hasExp = true;
            pos++;

            // после e может быть + или -
            if (pos < code.length() && (code[pos] == '+' || code[pos] == '-')) {
                pos++;
            }
        }
        else {
            break;
        }
    }

    QString num = code.mid(start, pos - start);

    if (!tolerant && (num.endsWith('e') || num.endsWith('E'))) {
        throw SyntaxErrorException("Invalid number format");
    }

    return {TOKEN_NUMBER, num, line};
}

/**
 * Считывает строковый литерал из указанного исходного кода. Этот метод идентифицирует
 * и возвращает строку, заключённую в кавычках (одинарные или двойные).
 * Если строка не закрыта, генерируется ошибка.
 *
 * @param code Исходный код, представленный в виде QString, из которого будет считан строковый литерал.
 *
 * @return Token, представляющий строковый литерал, содержащий его тип, значение и номер строки.
 */
Token Lexer::readString(const QString& code) {

    const QChar quote = code[pos++];
    QString result;

    while (pos < code.length()) {

        QChar ch = code[pos++];

        // конец строки
        if (ch == quote) {
            return {TOKEN_STRING, result, line};
        }

        // escape sequence
        if (ch == '\\') {

            if (pos >= code.length()) {
                if (tolerant) return {TOKEN_STRING, result, line};
                throw SyntaxErrorException("Invalid escape sequence");
            }

            QChar next = code[pos++];

            switch (next.unicode()) {

                case 'n':
                    result += '\n';
                    break;

                case 't':
                    result += '\t';
                    break;

                case 'r':
                    result += '\r';
                    break;

                case '\\':
                    result += '\\';
                    break;

                case '\'':
                    result += '\'';
                    break;

                case 'v':
                    result += '\v';
                    break;

                case 'f':
                    result += '\f';
                    break;

                case '"':
                    result += '"';
                    break;
                case 'x': {

                    if (pos + 1 >= code.length()) {
                        if (tolerant) return {TOKEN_STRING, result, line};
                        throw SyntaxErrorException("Invalid hex escape");
                    }

                    QString hex;

                    hex += code[pos++];
                    hex += code[pos++];

                    bool ok = false;

                    const int value = hex.toInt(&ok, 16);

                    if (!ok) {
                        if (tolerant) { result += hex; continue; }
                        throw SyntaxErrorException("Invalid hex escape");
                    }

                    result += QChar(value);

                    continue;
                }

                default:
                    result += next;
                    break;
            }

            continue;
        }

        result += ch;
    }

    if (tolerant) {
        return {TOKEN_STRING, result, line};
    }

    throw SyntaxErrorException("Unterminated string literal");
}

Token Lexer::readBytes(const QString& code) {

    pos++; // skip b

    Token str = readString(code);

    return {TOKEN_BYTES, str.value, str.line};
}


/**
 * Читает идентификатор, ключевое слово или значение типа булево из кода.
 * Этот метод анализирует последовательность символов, начиная с текущей позиции,
 * чтобы определить, является ли она идентификатором (например, именем переменной),
 * ключевым словом (например, "if", "else", "def") или булевым значением ("True", "False").
 *
 * @param code Исходный код, представленный в виде QString, из которого будет производиться чтение.
 *
 * @return Объект типа Token, содержащий тип токена (TOKEN_ID, TOKEN_KEYWORD или TOKEN_BOOL),
 *         строковое значение токена и номер строки, в которой токен находится.
 */
Token Lexer::readIdentifierOrBool(const QString& code) {
    const int start = pos;
    pos++;

    while (pos < code.length() && (code[pos].isLetterOrNumber() || code[pos] == '_')) {
        pos++;
    }
    QString id = code.mid(start, pos - start);

    if (keywords.count(id) == 1) {
        return {TOKEN_KEYWORD, id, line, keywords.at(id)};
    }

    if (id == "True" || id == "False") {
        return {TOKEN_BOOL, id, line};
    }

    if (id == "None") {
        return {TOKEN_NONE, id, line};
    }

    return {TOKEN_ID, id, line};
}

/**
* Считывает оператор из переданной строки кода. Этот метод идентифицирует
* одиночные и составные операторы, такие как "==", "+=", "!=" и другие.
* В случае составного оператора происходит дополнительное смещение позиции.
*
* @param code Строка кода, из которой производится чтение оператора.
*
* @return Token, представляющий оператор, содержащий его тип, значение и
*         строку, где он был обнаружен.
*/
Token Lexer::readOperator(const QString& code) {

    if (pos + 2 < code.length()) {

        QString three = code.mid(pos, 3);

        if (three == "//=" || three == "**=") {
            pos += 3;
            return {TOKEN_OP, three, line};
        }
    }

    if (pos + 1 < code.length()) {

        QString two = code.mid(pos, 2);

        if (two == "==" ||
            two == "!=" ||
            two == "<=" ||
            two == ">=" ||
            two == "+=" ||
            two == "-=" ||
            two == "*=" ||
            two == "/=" ||
            two == "%=" ||
            two == "//" ||
            two == "**" ||
            two == "->" ||
            two == "|=" ||
            two == "&=" ||
            two == "^=") {

            pos += 2;
            return {TOKEN_OP, two, line};
            }
    }

    const QChar op = code[pos++];

    if (op == '@') {
        return {TOKEN_AT, "@", line};
    }

    return {TOKEN_OP, QString(op), line};
}

Token Lexer::readComment(const QString& code) {
    const int start = pos;
    while (pos < code.length() && code[pos] != '\n') pos++;
    return {TOKEN_COMMENT, code.mid(start, pos - start), line};
}

/**
 * Пропускает пробельные символы в коде, исключая символ новой строки. Этот метод
 * обновляет текущую позицию в коде, а также отслеживает положение в колонке.
 *
 * @param code Исходный код, представленный в виде строки QString, который
 *             анализируется текущим лексером.
 */
void Lexer::skipWhitespace(const QString& code) {
    while (pos < code.length() && code[pos].isSpace() && code[pos] != '\n') {
        pos++;
    }
}

/**
 * Пропускает комментарии в заданном исходном коде. Этот метод проверяет,
 * начинается ли текущая позиция с символа комментария ('#'), и пропускает
 * весь текст до конца строки. После этого обновляет текущую строку и позицию.
 *
 * @param code Исходный код, представленный в виде QString, который будет обработан
 *             для игнорирования комментариев.
 */
void Lexer::skipComment(const QString& code) {

    if (pos >= code.length() || code[pos] != '#') return;

    while (pos < code.length() && code[pos] != '\n') {
        pos++;
    }

    while (pos < code.length() && code[pos] == '\n') {

        pos++;
        line++;
        lineStartPos = pos;

        int look = pos;
        while (look < code.length() && (code[look] == ' ' || code[look] == '\t')) {
            look++;
        }

        if (look >= code.length() || code[look] != '\n') {
            break;
        }

        pos = look;
    }
}
