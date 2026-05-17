#include "Lexer.h"

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
    pos = 0; //текущая позиция в коде
    line = 1;
    column = 1;
    indentStack.clear();
    indentStack.push_back(0);

    while (pos < code.length()) {
        if (QChar ch = code[pos]; ch == '\n') {
            tokens.push_back(Token(TOKEN_NEWLINE, "", line));
            pos++;
            line++;
            column = 1;

            int spaceCount = 0, tmpPos = pos;
            while (tmpPos < code.length() && (code[tmpPos] == ' ' || code[tmpPos] == '\t')) {
                if (code[tmpPos] == ' ')
                    spaceCount++;
                else if (code[tmpPos] == '\t')
                    spaceCount +=4;
                tmpPos++;
            }
            if (spaceCount > indentStack.last()) {
                indentStack.append(spaceCount);
                tokens.push_back(Token(TOKEN_INDENT, "", line));
            } else while (spaceCount < indentStack.last()) {
                    indentStack.pop_back();
                    tokens.push_back(Token(TOKEN_DEDENT, "", line));
                }
            pos = tmpPos;
            continue;
        }

        Token token = nextToken(code);
        if (token.type == TOKEN_EOF) {
            break;
        }

        tokens.append(token);
    }
    while (indentStack.size() > 1) {
        indentStack.pop_back();
        tokens.push_back(Token(TOKEN_DEDENT, "", line));
    }

    tokens.push_back(Token(TOKEN_EOF, "", line));

    return tokens;
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
        skipComment(code);

        if (pos == oldPos)
            break;
    }

    if (pos >= code.length()) {
        return {TOKEN_EOF, "", line};
    }

    const QChar ch = code[pos];

    if (ch.isDigit()) {
        return readNumber(code);
    }
    if (ch == '\"' || ch == '\'') {
        return readString(code);
    }
    if (ch.isLetter() || ch == '_') {
        return readIdentifierOrBool(code);
    }
    return readOperator(code);
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
Token Lexer::readNumber(const QString& code)
{
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

    if (num.endsWith('e') || num.endsWith('E')) {
        throw std::runtime_error("Invalid number format");
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
    const QChar quote = code[pos];
    pos++;
    column++;
    const int start = pos;

    while (pos < code.length() && code[pos] != quote) {
        if (code[pos] == '\n') {
            line++;
            column = 1;
        }
        pos++;
        column++;
    }

    if (pos >= code.length()) {
        throw std::runtime_error("Unterminated string literal");
    }

    QString str = code.mid(start, pos - start);
    pos++;
    column++;
    return {TOKEN_STRING, str, line};
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
    const QChar op = code[pos++];

    // Декораторы
    if (op == '@') {
        return {TOKEN_AT, "@", line};
    }

    // Двухсимвольные операторы
    if (pos < code.length()) {
        const QChar next = code[pos];
        if (QString combined = QString(op) + next; combined == "==" || combined == "+=" || combined == "!=" ||
                                                   combined == "-=" || combined == "//" || combined == "**" ||
                                                   combined == "<=" || combined == ">=" || combined == "->") {
            pos++;
            return {TOKEN_OP, combined, line};
        }
    }

    // Обычный оператор
    return {TOKEN_OP, QString(op), line};
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
        column++;
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
    if (pos < code.length() && code[pos] == '#') {
        while (pos < code.length() && code[pos] != '\n') {
            pos++;
        }
        column = 1;
        line++;
        pos++;
    }
}