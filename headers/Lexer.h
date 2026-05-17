#ifndef LEXER_H
#define LEXER_H

#include <QVector>
#include <QRegularExpression>
#include <utility>

/**
 * @enum TokenType
 * @brief Представляет различные типы токенов, которые лексер может идентифицировать.
 *
 * Каждый тип токена соответствует элементу обрабатываемого исходного кода.
 * Это позволяет лексеру классифицировать и разделять различные конструкции во входных данных.
 * 
 * - TOKEN_ID
 * Идентификаторы, используемые для имен переменных, функций и других именованных элементов
 * 
 * - TOKEN_NUMBER
 * Числовые литералы, включая целые числа и числа с плавающей точкой (например: 10, 3.14)
 * 
 * - TOKEN_STRING
 * Строковые литералы, заключенные в кавычки (например: "hello", 'world')
 * 
 * - TOKEN_BOOL
 * Логические значения True и False
 * 
 * - TOKEN_KEYWORD
 * Ключевые слова языка, такие как if, elif, else, def, while, for, break, continue
 * 
 * - TOKEN_OP
 * Операторы и разделители (+, -, *, /, =, ==, (, ), [ ], { })
 * 
 * - TOKEN_NEWLINE
 * Символ новой строки (\\n), используется для разделения инструкций
 * 
 * - TOKEN_INDENT
 * Увеличение уровня отступа, определяющее начало нового блока кода
 * 
 * - TOKEN_DEDENT
 * Уменьшение уровня отступа, определяющее конец текущего блока кода
 * 
 * - TOKEN_EOF
 * Маркер конца файла, сигнализирующий о завершении входного потока
 */
enum TokenType {
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_BOOL,
    TOKEN_NONE,
    TOKEN_KEYWORD,
    TOKEN_OP,
    TOKEN_AT,
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_EOF
};

enum class Keyword {
    IF,
    ELIF,
    ELSE,
    DEF,
    WHILE,
    BREAK,
    CONTINUE,
    RETURN,
    PASS,
    GLOBAL,
    NONLOCAL,
    CLASS,
    LAMBDA
};

static const std::unordered_map<QString, Keyword> keywords = {
    {"if", Keyword::IF},
    {"elif", Keyword::ELIF},
    {"else", Keyword::ELSE},
    {"def", Keyword::DEF},
    {"while", Keyword::WHILE},
    {"break", Keyword::BREAK},
    {"continue", Keyword::CONTINUE},
    {"return", Keyword::RETURN},
    {"pass", Keyword::PASS},
    {"global", Keyword::GLOBAL},
    {"nonlocal", Keyword::NONLOCAL},
    {"class", Keyword::CLASS},
    {"lambda", Keyword::LAMBDA}
};

/**
 * @struct Token
 * @brief Представляет элементарную единицу лексического анализа.
 *
 * Хранит информацию о типе токена, его значении и строке,
 * где данный токен начинается в обрабатываемом тексте.
 */
struct Token {
    TokenType type;
    QString value;
    int line; //Строка, где начинается токен
    // int column; //Столбец, где начинается токен
    Token(const TokenType type,
        QString value,
        const int line,
        const std::optional<Keyword> keyword = std::nullopt)
    : type(type), value(std::move(value)), line(line), keyword(keyword) {}

    std::optional<Keyword> keyword;
};

/**
 * @class Lexer
 * @brief Отвечает за разбор входного кода на отдельные токены.
 *
 * Класс Lexer выполняет обработку строкового исходного кода, разбивая его на токены.
 * Он поддерживает различные типы токенов, такие как идентификаторы, числа,
 * строки, логические значения, операторы и ключевые слова. Основным методом работы
 * является метод tokenize(), результатом работы которого является последовательность токенов.
 */
class Lexer {
public:
    QVector<Token> tokenize(const QString& code); //Главный метод

private:
    int pos = 0; //текущая позиция в коде
    int line = 1; //текущая строка
    int column = 1; //текущий столбец
    QVector<int> indentStack;

    /**
     * @brief Извлекает следующий токен из входного кода
     * @param code Исходный код для обработки
     * @return Token Следующий найденный токен
     */
    Token nextToken(const QString& code);

        /**
     * @brief Читает и возвращает числовой токен из входного кода
     * @param code Исходный код для обработки
     * @return Token Токен, содержащий числовое значение
     */
    Token readNumber(const QString &code);

    /**
     * @brief Читает и возвращает строковый токен из входного кода
     * @param code Исходный код для обработки
     * @return Token Токен, содержащий строковое значение
     */
    Token readString(const QString &code);

    /**
     * @brief Читает идентификатор, ключевое слово или булево значение
     * @param code Исходный код для обработки
     * @return Token Токен типа TOKEN_ID, TOKEN_KEYWORD или TOKEN_BOOL
     */
    Token readIdentifierOrBool(const QString &code);

    /**
     * @brief Читает и возвращает токен оператора
     * @param code Исходный код для обработки
     * @return Token Токен, содержащий оператор
     */
    Token readOperator(const QString &code);

    /**
     * @brief Пропускает пробельные символы (кроме символа новой строки)
     * @param code Исходный код для обработки
     */
    void skipWhitespace(const QString &code);

    /**
     * @brief Пропускает однострочные комментарии, начинающиеся с #
     * @param code Исходный код для обработки
     */
    void skipComment(const QString &code);
};
#endif // LEXER_H