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
    while (true) {
        Token token = nextToken(code);
        if (token.type == TOKEN_EOF) {
            break;
        }

        if (!token.value.isEmpty()) {
            tokens.append(token);
        }
    }
    pos = 0;
    line = 0;
    column = 0;
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
    skipWhitespace(code);
    skipComment(code);

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
    while (pos < code.length() && (code[pos].isDigit() || code[pos] == '.')) {
        pos++;
    }
    QString num = code.mid(start, pos - start);
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
    while (pos < code.length() && (code[pos].isLetter() || code[pos] == '_')) {
        pos++;
    }
    QString id = code.mid(start, pos - start);
    if (id == "if" || id == "else" || id == "def") {
        return {TOKEN_KEYWORD, id, line};
    }
    if (id == "True" || id == "False") {
        return {TOKEN_BOOL, id, line};
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

    if (pos < code.length()) {
        const QChar next = code[pos];
        if (QString combined = QString(op) + next; combined == "==" || combined == "+=" || combined == "!=" ||
                                                   combined == "-=" || combined == "//" || combined == "**") {
            pos++;
            return {TOKEN_OP, combined, line};
        }
    }

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