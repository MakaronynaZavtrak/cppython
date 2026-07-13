//
// Created by semyo on 13.07.2026.
//

#include "GenExprNode.h"

#include "../forNode/ForNode.h"
#include "../ifNode/IfNode.h"
#include "../yieldNode/YieldNode.h"
#include "../varNode/VarNode.h"
#include "Environment.h"
#include "GeneratorValue.h"
#include "FunctionValue.h"

Value GenExprNode::eval(const EnvPtr env) const {

    // 1. первый iterable вычисляется НЕМЕДЛЕННО, как в настоящем Python
    Value firstIterableValue = clauses[0].iterable->eval(env);

    const QString hiddenVarName = "__genexpr_source__";

    const auto genEnv = std::make_shared<Environment>(env);
    genEnv->set(hiddenVarName, firstIterableValue);

    // 2. собираем тело изнутри наружу: innermost — YieldNode
    std::vector<std::shared_ptr<ASTNode>> body = { std::make_shared<YieldNode>(resultExpr) };

    for (size_t i = clauses.size(); i-- > 0; ) {

        const auto& clause = clauses[i];

        // оборачиваем условия if (если есть) вложенными IfNode
        for (auto it = clause.conditions.rbegin(); it != clause.conditions.rend(); ++it) {
            body = { std::make_shared<IfNode>(
                *it, body,
                std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>>{},
                std::vector<std::shared_ptr<ASTNode>>{}
            ) };
        }

        // iterable: у самой первой clause подставляем VarNode на уже вычисленное значение,
        // у остальных — сохраняем оригинальное выражение (вычисляется лениво, видит предыдущие loop vars)
        std::shared_ptr<ASTNode> iterableNode = (i == 0)
            ? std::static_pointer_cast<ASTNode>(std::make_shared<VarNode>(hiddenVarName))
            : clause.iterable;

        body = { std::make_shared<ForNode>(clause.varName, iterableNode, body) };
    }

    // 3. синтетическая generator-функция без параметров
    const auto func = std::make_shared<FunctionValue>(
        std::vector<Param>{}, body, env, "<genexpr>"
    );
    func->isGenerator = true;

    const auto gen = std::make_shared<GeneratorValue>();
    gen->func = func;
    gen->env = genEnv;

    return Value(std::static_pointer_cast<IteratorValue>(gen));
}

QString GenExprNode::toString() const {
    return "GenExprNode(...)";
}