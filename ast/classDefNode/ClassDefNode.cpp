//
// Created by semyo on 06.07.2026.
//

#include "ClassDefNode.h"

#include "CallRuntime.h"
#include "ClassMethodValue.h"
#include "ClassUtils.h"
#include "ClassValue.h"
#include "DictValue.h"
#include "FunctionValue.h"
#include "StaticMethodValue.h"
#include "TupleValue.h"
#include "../../exception/AttributeErrorException.h"
#include "../../exception/NameErrorException.h"
#include "../../exception/PythonException.h"
#include "../../exception/TypeErrorException.h"

ClassDefNode::ClassDefNode(QString name,
                           std::vector<std::shared_ptr<ASTNode>> bases,
                           QVector<std::shared_ptr<ASTNode>> body,
                           std::vector<std::shared_ptr<ASTNode>> decorators,
                           std::vector<std::pair<QString, std::shared_ptr<ASTNode>>> keywords)
        : name(std::move(name)),
        baseExprs(std::move(bases)),
        body(std::move(body)),
        decorators(std::move(decorators)),
        keywords(std::move(keywords)) {}


Value ClassDefNode::eval(EnvPtr env) const {

    // __module__ = текущий __name__ (как globals()['__name__'] в CPython)
    Value moduleName;
    try {
        moduleName = env->get("__name__");
    } catch (const NameErrorException&) {
        moduleName = Value(QStringLiteral("__main__"));
    }

    // --- базовые классы ---
    std::vector<Value::ClassPtr> bases;

    for (auto& baseExpr : baseExprs) {
        Value baseVal = baseExpr->eval(env);

        if (!baseVal.isClass()) {
            throw TypeErrorException("Base must be a class");
        }

        bases.push_back(baseVal.asClass());
    }

    // --- keyword-аргументы заголовка: metaclass=... и остальные ---
    Value::ClassPtr explicitMeta;
    Kwargs extraKwargs;

    for (const auto& [kw, expr] : keywords) {

        if (kw == "metaclass") {

            Value mv = expr->eval(env);

            if (!mv.isClass()) {
                throw TypeErrorException("metaclass must be a class");
            }

            explicitMeta = mv.asClass();
        }
        else {
            extraKwargs.push_back({ kw, expr->eval(env) });
        }
    }

    // --- самый производный метакласс среди баз (+ детект конфликта) ---
    auto metaOf = [](const Value::ClassPtr& b) -> Value::ClassPtr {
        return b->metaclass ? b->metaclass : Runtime::typeClass;
    };

    Value::ClassPtr winner = Runtime::typeClass;

    for (const auto& b : bases) {

        Value::ClassPtr bm = metaOf(b);

        if (PythonException::isSubclass(bm, winner)) {
            winner = bm;
        }
        else if (!PythonException::isSubclass(winner, bm)) {
            throw TypeErrorException(
                "metaclass conflict: the metaclass of a derived class must be "
                "a (non-strict) subclass of the metaclasses of all its bases");
        }
    }

    Value::ClassPtr metaclass = winner;

    if (explicitMeta) {

        if (!PythonException::isSubclass(explicitMeta, winner)) {
            throw TypeErrorException(
                "metaclass conflict: the metaclass of a derived class must be "
                "a (non-strict) subclass of the metaclasses of all its bases");
        }

        metaclass = explicitMeta;
    }

    // Отклоняемся от штатного пути только при реальном кастомном метаклассе
    // (или наличии прочих keyword-аргументов). Дефолтный `type` без kwargs —
    // ровно прежний быстрый путь, чтобы не задеть существующее поведение.
    const bool customMeta =
        (metaclass != Runtime::typeClass) || !extraKwargs.empty();

    Value classValue;

    if (!customMeta) {

        // --- быстрый путь: дефолтный type, как и было ---
        std::vector<Value::ClassPtr> effBases = bases;

        if (effBases.empty()) {
            effBases.push_back(Runtime::objectClass);
        }

        const auto cls = std::make_shared<ClassValue>(name);
        cls->bases = effBases;

        const auto classEnv = std::make_shared<Environment>(env);

        for (const auto& stmt : body) {
            [[maybe_unused]] const auto _ = stmt->eval(classEnv);
        }

        for (auto it = classEnv->variables.cbegin();
             it != classEnv->variables.cend();
             ++it) {

            const QString& key = it.key();
            const Value& val = it.value();

            Value newVal = val;

            if (newVal.isFunction()) {
                newVal.asFunction()->ownerClass = cls;
            }
            else if (newVal.isStaticMethod()) {
                newVal.asStaticMethod()->func->ownerClass = cls;
            }
            else if (newVal.isClassMethod()) {
                newVal.asClassMethod()->func->ownerClass = cls;
            }

            cls->attributes.insert(key, val);
        }

        // __module__ / __qualname__ (как в CPython), если не заданы телом
        if (!cls->attributes.contains("__module__")) {
            cls->attributes.insert("__module__", moduleName);
        }
        if (!cls->attributes.contains("__qualname__")) {
            cls->attributes.insert("__qualname__", Value(name));
        }

        classValue = Value(cls);
    }
    else {

        // --- путь метакласса ---

        // кортеж баз (нужен и для __prepare__, и для вызова метакласса)
        std::vector<Value> baseValues;
        baseValues.reserve(bases.size());

        for (const auto& b : bases) {
            baseValues.push_back(Value(b));
        }

        const auto basesTuple = std::make_shared<TupleValue>(baseValues);

        // namespace: из metaclass.__prepare__(name, bases, **kwds), иначе пустой dict
        std::shared_ptr<DictValue> ns;

        Value prepareMethod;
        bool hasPrepare = false;

        try {
            prepareMethod = getAttrValue(Value(metaclass), "__prepare__");
            hasPrepare = true;
        } catch (const AttributeErrorException&) {}

        if (hasPrepare) {

            Value prepared = call(prepareMethod,
                                  { Value(name), Value(basesTuple) },
                                  extraKwargs, env);

            if (!prepared.isDict()) {
                throw TypeErrorException(
                    "__prepare__() must return a mapping, not "
                    + prepared.getTypeName());
            }

            ns = prepared.asDict();
        }
        else {
            ns = std::make_shared<DictValue>();
        }

        // __module__ / __qualname__ кладутся в namespace до тела (идут первыми),
        // как в CPython
        if (!ns->hasKey(Value(QStringLiteral("__module__")))) {
            ns->setItem(Value(QStringLiteral("__module__")), moduleName);
        }
        if (!ns->hasKey(Value(QStringLiteral("__qualname__")))) {
            ns->setItem(Value(QStringLiteral("__qualname__")), Value(name));
        }

        // тело класса исполняется В namespace: имена пишутся в ns по порядку
        const auto classEnv = std::make_shared<Environment>(env);
        classEnv->nsCapture = ns;

        for (const auto& stmt : body) {
            [[maybe_unused]] const auto _ = stmt->eval(classEnv);
        }

        classValue = call(
            Value(metaclass),
            { Value(name), Value(basesTuple), Value(ns) },
            extraKwargs,
            env);
    }

    // --- декораторы снизу вверх ---
    for (auto it = decorators.rbegin(); it != decorators.rend(); ++it) {
        Value decorator = (*it)->eval(env);

        classValue = call(decorator, { classValue }, {}, env);
    }

    env->set(name, classValue);

    return classValue;
}

QString ClassDefNode::toString() const { return "class " + name; }

bool ClassDefNode::shouldPrint() const { return false; }