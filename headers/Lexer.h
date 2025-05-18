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
 * Ключевые слова языка, такие как if, else, def, while, for
 * 
 * - TOKEN_OP
 * Операторы и разделители (+, -, *, /, =, ==, (, ), [ ], { })
 * 
 * - TOKEN_NEWLINE
 * Символ новой строки (\n), используется для разделения инструкций
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
    TOKEN_KEYWORD,
    TOKEN_OP,
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,
    TOKEN_EOF
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
    Token(const TokenType type, QString value, const int line) : type(type), value(std::move(value)), line(line) {}
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