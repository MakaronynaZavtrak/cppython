//
// Created by semyo on 06.07.2026.
//

#ifndef CPPYTHON_CALLNODE_H
#define CPPYTHON_CALLNODE_H

#include "../ASTNode.h"

struct KeywordArg {
    QString name;
    std::shared_ptr<ASTNode> value;
};

struct ParsedCallArgs {
    std::vector<std::shared_ptr<ASTNode>> positional;
    std::vector<KeywordArg> keyword;
};

class CallNode final : public ASTNode {
public:
    std::shared_ptr<ASTNode> callee;
    std::vector<std::shared_ptr<ASTNode>> args;
    std::vector<KeywordArg> kwargs;

    int calleeEndColumn = 0;

    CallNode(std::shared_ptr<ASTNode> callee,
        std::vector<std::shared_ptr<ASTNode>> args,
        std::vector<KeywordArg> kwargs);

    [[nodiscard]] Value eval(EnvPtr env) const override;

    [[nodiscard]] QString toString() const override;
};
#endif //CPPYTHON_CALLNODE_H