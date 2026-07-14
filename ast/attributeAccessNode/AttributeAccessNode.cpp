//
// Created by semyo on 06.07.2026.
//

#include "AttributeAccessNode.h"

#include "ClassUtils.h"
#include "../../exception/PythonException.h"

AttributeAccessNode::AttributeAccessNode(std::shared_ptr<ASTNode> object, QString attr)
        : object(std::move(object)), attr(std::move(attr)) {}

Value AttributeAccessNode::eval(const EnvPtr env) const {

    const Value objVal = object->eval(env);

    try {

        if (objVal.isSuper()) {
            const auto super = objVal.asSuper();
            return getAttrFromSuper(super, attr);
        }

        return getAttrValue(objVal, attr);

    }
    catch (PythonException& e) {
        e.setPositionIfMissing(line, startColumn, endColumn, sourceId);
        e.recordFramePosition(startColumn, endColumn);
        e.captureTracebackIfMissing();
        throw;
    }
}

QString AttributeAccessNode::toString() const {
    return object->toString() + "." + attr;
}