//
// Created by semyo on 06.07.2026.
//

#include "PassNode.h"

[[nodiscard]] Value PassNode::eval(std::shared_ptr<Environment>) const {
    return {}; // None
}

[[nodiscard]] QString PassNode::toString() const {
    return "pass";
}

[[nodiscard]] bool PassNode::shouldPrint() const { return false; }