#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <memory>

#include "ComprehensionClause.h"
#include "../ast/callNode/CallNode.h"

/**
 * @class Parser
 * @brief Выполняет разбор последовательности токенов в абстрактное синтаксическое дерево (AST).
 *
 * Parser организует разбор выражений на основании грамматики с учетом приоритетов операций.
 * Класс принимает поток токенов на вход и предоставляет соответствующий AST в качестве результата.
 * Используется в интерпретаторах, компиляторах и других системах анализа кода.
 *
 * @details
 * Основной метод `parse` возвращает корневой узел AST, представляющего собой анализируемое выражение.
 * Парсинг происходит в несколько этапов, начиная с самых низкоприоритетных операций до высокоприоритетных.
 * Класс включает методы, которые обеспечивают парсинг конкретных конструкций, таких как выражения со скобками,
 * операции унарного минуса, математические операции, сравнения и присваивания.
 * Если входные токены содержат синтаксические ошибки, генерируются исключения.
 */
class Parser {
public:
    explicit Parser(const QVector<Token> &tokens);

    std::shared_ptr<ASTNode> parse(); //Главный метод

private:
    //Здесь методы разделены для анализа выражения согласно приоритету
    /**
     * @brief Разбирает операции присваивания (=)
     * @return Узел присваивания или выражение более высокого приоритета
     */
    std::shared_ptr<ASTNode> parseExpression();

    std::shared_ptr<ASTNode> parseExpressionNoAssign();

    std::shared_ptr<ASTNode> parseStarredExpression();

    std::shared_ptr<ASTNode> parseDoubleStarredExpression();

    /**
     * @brief Разбирает операции сравнения (==, !=, <, <=, >, >=)
     * @return Узел сравнения или выражение более высокого приоритета
     */
    std::shared_ptr<ASTNode> parseComparison();

    /**
     * @brief Разбирает операции сложения и вычитания (+, -)
     * @return Узел арифметической операции или выражение более высокого приоритета
     */
    std::shared_ptr<ASTNode> parseAdditionAndSubtraction();
    /**
    * @brief Разбирает операции умножения, деления и остатка (*, /, //, %)
    * @return Узел арифметической операции или выражение более высокого приоритета
    */
    std::shared_ptr<ASTNode> parseTerm();

    /**
     * @brief Разбирает операцию возведения в степень (**)
     * @return Узел возведения в степень или первичное выражение
     */
    std::shared_ptr<ASTNode> parsePower();

    /**
     * @brief Разбирает первичные выражения (числа, строки, переменные, выражения в скобках)
     * @return Узел первичного выражения
     */
    std::shared_ptr<ASTNode> parsePrimary();

    /**
     * @brief Разбирает числовой токен
     * @return Узел числового значения
     */
    std::shared_ptr<ASTNode> parseNumberToken();

    /**
     * @brief Разбирает строковый токен
     * @return Узел строкового значения
     */
    std::shared_ptr<ASTNode> parseStringToken();

    std::shared_ptr<ASTNode> parseBytesToken();

    /**
     * @brief Разбирает логический токен
     * @return Узел логического значения
     */
    std::shared_ptr<ASTNode> parseBoolToken();

    std::shared_ptr<ASTNode> parseNoneToken();

    /**
     * @brief Разбирает токен идентификатора
     * @return Узел переменной
     */
    std::shared_ptr<ASTNode> parseIdentifierToken();

    /**
     * @brief Разбирает выражение в скобках
     * @return Узел выражения внутри скобок
     */
    std::shared_ptr<ASTNode> parseParenthesizedExpression();

    /**
     * @brief Возвращает текущий токен без продвижения
     * @return Текущий токен
     */
    [[nodiscard]] Token peek() const;

    /**
    * @brief Возвращает текущий токен и переходит к следующему
    * @return Текущий токен
    */
    Token advance();

    /**
     * @brief Выбрасывает ошибку о неожиданном токене
     * @param token Неожиданный токен
     */
    static void throwUnexpectedTokenError(const Token &token);

    /**
     * @brief Разбирает конструкцию условного оператора (`if`) и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий конструкцию условного оператора.
     * Возвращенный узел содержит информацию о главном условии, теле, а также необязательных блоках `elif` и `else`.
     */
    std::shared_ptr<ASTNode> parseIfStatement();

    /**
     * @brief Разбирает блок кода и возвращает список узлов AST, представляющих инструкции внутри блока.
     * @return Список узлов AST, представляющих проанализированные инструкции внутри блока кода.
     */
    std::vector<std::shared_ptr<ASTNode>> parseBlock();

    /**
     * @brief Разбирает конструкцию цикла `while` и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий конструкцию цикла `while`.
     */
    std::shared_ptr<ASTNode> parseWhileStatement();

    /**
     * @brief Разбирает инструкцию `break` и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий инструкцию `break`.
     */
    std::shared_ptr<ASTNode> parseBreakStatement();

    /**
     * @brief Разбирает инструкцию `continue` и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий инструкцию `continue`.
     */
    std::shared_ptr<ASTNode> parseContinueStatement();

    std::shared_ptr<ASTNode> parseFunctionDef(const std::vector<std::shared_ptr<ASTNode>>& decorators = {});

    std::shared_ptr<ASTNode> parseReturn();

    std::shared_ptr<ASTNode> parsePass();

    std::shared_ptr<ASTNode> parseGlobalStatement();

    std::shared_ptr<ASTNode> parseNonlocalStatement();

    std::shared_ptr<ASTNode> parseYieldStatement();

    std::shared_ptr<ASTNode> parseRightHandSide();

    std::shared_ptr<ASTNode> parseClassDef(const std::vector<std::shared_ptr<ASTNode>>& decorators = {});

    std::shared_ptr<ASTNode> parsePostfix(std::shared_ptr<ASTNode>);

    std::shared_ptr<ASTNode> parseDecorated();

    std::shared_ptr<ASTNode> parseList();

    std::shared_ptr<ASTNode> parseLambda();

    ParsedCallArgs parseCallArguments();

    std::shared_ptr<ASTNode> parseDict();

    std::shared_ptr<ASTNode> parseSet();

    bool isDictLiteral();

    std::shared_ptr<ASTNode> parseForStatement();

    std::shared_ptr<ASTNode> parseDictOrSet();

    std::shared_ptr<ASTNode> parseDictComp();

    std::shared_ptr<ASTNode> parseSetComp();

    std::shared_ptr<ASTNode> parseIndexOrSlice();

    QString consume(TokenType type, const QString &value);

    [[nodiscard]] bool match(TokenType type, const QString& value) const;

    bool matchAndAdvance(TokenType type, const QString& value);

    [[nodiscard]] bool matchAny(TokenType type, const std::vector<QString>& values) const;

    bool matchAnyAndAdvance(TokenType type, const std::vector<QString>& values);

    [[nodiscard]] bool isComparisonOperator() const;

    QString parseComparisonOperator();

    std::shared_ptr<ASTNode> parseUnary();

    std::shared_ptr<ASTNode> parseNot();

    std::shared_ptr<ASTNode> parseAnd();

    std::shared_ptr<ASTNode> parseOr();

    std::shared_ptr<ASTNode> parseBitOr();

    std::shared_ptr<ASTNode> parseBitXor();

    std::shared_ptr<ASTNode> parseBitAnd();

    std::shared_ptr<ASTNode> parseShift();

    std::shared_ptr<ASTNode> parseDelStatement();

    std::shared_ptr<ASTNode> parseTryStatement();

    std::shared_ptr<ASTNode> parseRaiseStatement();

    std::shared_ptr<ASTNode> parseExpressionStatement();

    std::shared_ptr<ASTNode> parseAssignmentTail(std::shared_ptr<ASTNode> left);

    std::vector<ComprehensionClause> parseComprehensionClauses();

    enum class BraceKind { Dict, DictComp, Set, SetComp };

    BraceKind classifyBraces();

    QVector<Token> tokens;
    int current = 0;
};
#endif //PARSER_H
