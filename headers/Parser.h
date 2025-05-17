#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <memory>
#include <cmath>
#include <utility>

/**
 * @class ASTNode
 * @brief Представляет узел абстрактного синтаксического дерева (AST) в интерпретаторе языка программирования.
 *
 * ASTNode служит базовым классом для всех типов узлов в абстрактном синтаксическом дереве.
 * Производные классы реализуют конкретные типы узлов и определяют их поведение для вычисления
 * и строкового представления. Этот класс спроектирован для расширения и не может быть создан напрямую.
 *
 * @details
 * Абстрактное синтаксическое дерево представляет иерархическую структуру исходного кода, где каждый
 * узел соответствует синтаксической конструкции, такой как переменная, оператор или функция.
 * От производных классов ожидается предоставление конкретных реализаций методов `eval` и `toString`.
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    [[nodiscard]] virtual double eval() const = 0;
    [[nodiscard]] virtual QString toString() const = 0;
};


/**
 * @class NumberNode
 * @brief Представляет узел числового значения в абстрактном синтаксическом дереве (AST) интерпретатора языка программирования.
 *
 * NumberNode — это конкретная реализация класса ASTNode, инкапсулирующая одно числовое значение.
 * Этот класс используется для представления константных чисел в синтаксическом дереве.
 *
 * @details
 * Данный узел предоставляет реализации методов вычисления значения и строкового представления,
 * которые непосредственно соответствуют хранящемуся числовому значению. Метод `eval`
 * возвращает числовое значение узла, а метод `toString` преобразует значение
 * в его строковый эквивалент.
 *
 * Этот класс объявлен как final, то есть не может иметь наследников.
 */
class NumberNode final : public ASTNode {
public:
    double value;
    [[nodiscard]] QString toString() const override { return QString::number(value); }
    [[nodiscard]] double eval() const override { return value; }
};

/**
 * @class BinOpNode
 * @brief Представляет узел бинарной операции в абстрактном синтаксическом дереве (AST) интерпретатора языка программирования.
 *
 * BinOpNode обрабатывает бинарные операции, такие как арифметические выражения, сравнения или конкатенация строк.
 * Он принимает два дочерних узла (левый и правый) и оператор, и вычисляет их для получения результата.
 *
 * @details
 * Класс поддерживает вычисление (`eval`) и строковое представление (`toString`) бинарных операций.
 * Поддерживаемые операции включают арифметические (`+`, `-`, `*`, `/`, `%`, `**`, `//`), сравнения (`==`, `!=`, `>`, `>=`, `<`, `<=`),
 * и строковые операции. Узел вычисляется путем рекурсивного вычисления левого и правого дочерних узлов
 * и применения указанного оператора. Если встречается неподдерживаемая операция или комбинация типов операндов,
 * генерируются соответствующие ошибки.
 *
 * Метод `eval` обеспечивает правильную обработку типов (например, числовых, строковых или смешанных) и делегирует конкретные
 * поведения вспомогательным методам, таким как `evalTwoNumbers`, `evalTwoStrings` и `evalNumAndString`.
 */
class BinOpNode final : public ASTNode {
public:
    BinOpNode(std::shared_ptr<ASTNode> left, QString  op, std::shared_ptr<ASTNode> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    [[nodiscard]] QString toString() const override {
        return "(" + left->toString() + " " + op + " " + right->toString() + ")";
    }

    [[nodiscard]] double eval() const override {
        const double l = left->eval();
        const double r = right->eval();

        if (op == "+") return l + r;
        if (op == "-") return l - r;
        if (op == "*") return l * r;
        if (op == "**") return pow(l, r);
        if (op == "/") return r != 0 ? l / r : throw std::runtime_error("Division by zero");
        if (op == "%") return r != 0 ? static_cast<int>(l) % static_cast<int>(r) : throw std::runtime_error("Division by zero");
        if (op == "//") return r != 0 ? static_cast<int>(l / r) : throw std::runtime_error("Division by zero");
        if (op == "==") return l == r;
        if (op == "!=") return l != r;

        throw std::runtime_error("Unknown operator: " + op.toStdString());
    }

    std::shared_ptr<ASTNode> left;
    QString op; // "+", "-", "=", "/", "%", "*", "**", "//", "=="
    std::shared_ptr<ASTNode> right;
};

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
    QVector<Token> tokens;
    int current = 0;

    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseTerm();
    std::shared_ptr<ASTNode> parsePower();
    std::shared_ptr<ASTNode> parseFactor();
    [[nodiscard]] Token peek() const;
    Token advance();
};
#endif //PARSER_H
