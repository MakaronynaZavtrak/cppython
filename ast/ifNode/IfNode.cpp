//
// Created by semyo on 06.07.2026.
//

#include "IfNode.h"

#include "Interpreter.h"

IfNode::IfNode(std::shared_ptr<ASTNode> condition,
               std::vector<std::shared_ptr<ASTNode>> body,
               std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs,
               std::vector<std::shared_ptr<ASTNode>> elseBody)
    : condition(std::move(std::move(condition))), body(std::move(body)), elifs(std::move(elifs)), elseBody(std::move(elseBody)) {}

    Value IfNode::eval(EnvPtr env) const {

        if (condition->eval(env).toBool()) {

            Value lastValue;

            for (const auto& stmt : body) {
                lastValue = Interpreter::executeNode(stmt, env);
            }

            return lastValue;
        }

        for (const auto&[fst, snd]: elifs) {

            if (fst->eval(env).toBool()) {

                Value lastValue;

                for (const auto& stmt : snd) {
                    lastValue = Interpreter::executeNode(stmt, env);
                }

                return lastValue;
            }
        }

        if (!elseBody.empty()) {

            Value lastValue;

            for (const auto& stmt : elseBody) {
                lastValue = Interpreter::executeNode(stmt, env);
            }

            return lastValue;
        }

        return {};
    }

    QString IfNode::toString() const {

        QString result = "if " + condition->toString() + ":\n";

        for (const auto& stmt : body) {
            result += "    " + stmt->toString() + "\n";
        }

        for (const auto&[fst, snd] : elifs) {

            result += "elif " + fst->toString() + ":\n";

            for (const auto& stmt : snd) {
                result += "    " + stmt->toString() + "\n";
            }
        }

        if (!elseBody.empty()) {

            result += "else:\n";

            for (const auto& stmt : elseBody) {
                result += "    " + stmt->toString() + "\n";
            }
        }

        return result;
    }

    bool IfNode::shouldPrint() const { return false; }