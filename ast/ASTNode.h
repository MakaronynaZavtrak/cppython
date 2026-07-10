//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_ASTNODE_H
#define CPPYTHON_ASTNODE_H
#include <memory>

#include "Value.h"
#include "../service/ResumeContext.h"

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
    using EnvPtr = std::shared_ptr<Environment>;
    virtual ~ASTNode() = default;
    [[nodiscard]] virtual Value eval(EnvPtr env) const = 0;
    [[nodiscard]] virtual QString toString() const = 0;
    [[nodiscard]] virtual bool shouldPrint() const { return true; }
    [[nodiscard]] virtual bool containsYield() const { return false; }

    [[nodiscard]] virtual Value evalResumable(const EnvPtr env, ResumeContext& ctx) const {
        return eval(env);
    }

    [[nodiscard]] virtual bool isBareYieldStatement() const { return false; }

    static void printIfNeeded(const std::shared_ptr<ASTNode>& stmt, const Value& value) {
        if (stmt->shouldPrint() && !value.isNone()) {
            std::cout << value.display().toStdString() << "\n";
        }
    }
};
#endif //CPPYTHON_ASTNODE_H