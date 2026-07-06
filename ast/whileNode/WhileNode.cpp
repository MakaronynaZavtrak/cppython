//
// Created by semyo on 06.07.2026.
//

#include "WhileNode.h"

#include "Interpreter.h"
#include "../../exception/BreakException.h"
#include "../../exception/ContinueException.h"

WhileNode::WhileNode(std::shared_ptr<ASTNode> condition,
                     std::vector<std::shared_ptr<ASTNode>> body,
                     std::vector<std::shared_ptr<ASTNode>> elseBody)
            : condition(std::move(condition)),
            body(std::move(body)),
            elseBody(std::move(elseBody)) {}

Value WhileNode::eval(const EnvPtr env) const {

    Value last;
    bool broken = false;

    while (condition->eval(env).toBool()) {

        try {
            for (auto& stmt : body) {
                last = Interpreter::executeNode(stmt, env);
            }
        }
        catch ([[maybe_unused]] const ContinueException& e) {}
        catch ([[maybe_unused]] const BreakException& e) {
            broken = true;
            break;
        }
    }

    if (!broken) {
        for (auto& stmt : elseBody) {
            last = Interpreter::executeNode(stmt, env);
        }
    }
    return last;
}

QString WhileNode::toString() const {

    QString out = "while " + condition->toString() + ":\n";

    for (auto& stmt : body) {
        out += "\t" + stmt->toString() + "\n";
    }

    if (!elseBody.empty()) {

        out += "else:\n";

        for (auto& stmt : elseBody) {
            out += "\t" + stmt->toString() + "\n";
        }
    }

    return out;
}

bool WhileNode::shouldPrint() const { return false; }
