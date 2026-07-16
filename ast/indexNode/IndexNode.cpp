//
// Created by semyo on 06.07.2026.
//

#include "IndexNode.h"

#include "CallRuntime.h"
#include "ClassUtils.h"
#include "../../exception/PythonException.h"

IndexNode::IndexNode(std::shared_ptr<ASTNode> object,
                     std::shared_ptr<ASTNode> index)
        : object(std::move(object)),
          index(std::move(index)) {}

Value IndexNode::eval(EnvPtr env) const {

    Value obj = object->eval(env);
    Value idx = index->eval(env);

    try {
        Value getter = genericGetAttr(obj, "__getitem__");
        return call(getter, {idx}, {}, env);
    }
    catch (PythonException& e) {
        e.setPositionIfMissing(line, startColumn, endColumn, sourceId);
        e.recordFramePosition(startColumn, endColumn, objectEndColumn, endColumn, true);
        e.captureTracebackIfMissing();
        throw;
    }
}

QString IndexNode::toString() const {
    return object->toString() + "[" + index->toString() + "]";
}