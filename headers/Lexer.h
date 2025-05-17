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
 */
enum TokenType {
    TOKEN_ID, //для имен переменных
    TOKEN_NUMBER, //для числовых литералов (10, 3.14)
    TOKEN_STRING, //для строковых литералов
    TOKEN_BOOL, //True, False
    TOKEN_KEYWORD, //if, else, def
    TOKEN_OP, //+, -, =, ==, ()
    TOKEN_NEWLINE, //\n
    TOKEN_INDENT, //отступы
    TOKEN_EOF //конец файла
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

    Token nextToken(const QString& code); //Следующий токен
    Token readNumber(const QString& code); //Читает число
    Token readString(const QString& code); //Читает строку
    Token readIdentifierOrBool(const QString& code); //Читает имя или ключевое слово, или булево значение (True | False)
    Token readOperator(const QString& code); //Читает оператор(+, -, =)
    void skipWhitespace(const QString& code); //Пропускает пробелы, но не \n
    void skipComment(const QString& code); //Пропускает комментарии (#...)
};
#endif // LEXER_H