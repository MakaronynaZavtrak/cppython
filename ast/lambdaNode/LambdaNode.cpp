//
// Created by semyo on 06.07.2026.
//

#include "LambdaNode.h"

#include "FunctionValue.h"
#include "../returnNode/ReturnNode.h"

LambdaNode::LambdaNode(std::vector<Param> params,
                       std::shared_ptr<ASTNode> body)
    : params(std::move(params)),
      body(std::move(body)) {}

Value LambdaNode::eval(EnvPtr env) const {

    std::vector<std::shared_ptr<ASTNode>> functionBody;

    functionBody.push_back(std::make_shared<ReturnNode>(body));

    const auto fn = std::make_shared<FunctionValue>(params, functionBody, env, "<lambda>");

    return Value(fn);
}

[[nodiscard]] QString LambdaNode::toString() const {
    return "<lambda>";
}

[[nodiscard]] bool LambdaNode::shouldPrint() const {
    return false;
}