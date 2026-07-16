//
// Created by semyo on 06.07.2026.
//

#include "ClassDefNode.h"

#include "CallRuntime.h"
#include "ClassMethodValue.h"
#include "ClassValue.h"
#include "FunctionValue.h"
#include "StaticMethodValue.h"
#include "../../exception/TypeErrorException.h"

ClassDefNode::ClassDefNode(QString name,
                           std::vector<std::shared_ptr<ASTNode>> bases,
                           QVector<std::shared_ptr<ASTNode>> body,
                           std::vector<std::shared_ptr<ASTNode>> decorators)
        : name(std::move(name)),
        baseExprs(std::move(bases)),
        body(std::move(body)),
        decorators(std::move(decorators)) {}


Value ClassDefNode::eval(EnvPtr env) const {
        std::vector<Value::ClassPtr> bases;

        for (auto& baseExpr : baseExprs) {
            Value baseVal = baseExpr->eval(env);

            if (!baseVal.isClass()) {
                throw TypeErrorException("Base must be a class");
            }

            bases.push_back(baseVal.asClass());
        }

        // наследование по умолчанию
        if (bases.empty()) {
            bases.push_back(Runtime::objectClass);
        }

        const auto cls = std::make_shared<ClassValue>(name);
        cls->bases = bases;

        // создаём временное окружение
        const auto classEnv = std::make_shared<Environment>(env);

        // выполняем тело класса
        for (const auto& stmt : body) {
            [[maybe_unused]] const auto _ = stmt->eval(classEnv);
        }

        // копируем всё из classEnv в attributes
        for (auto it = classEnv->variables.cbegin();
             it != classEnv->variables.cend();
             ++it) {

            const QString& key = it.key();
            const Value& val = it.value();

            Value newVal = val;

            // обычная функция
            if (newVal.isFunction()) {

                auto func = newVal.asFunction();
                func->ownerClass = cls;
            }

            // staticmethod
            else if (newVal.isStaticMethod()) {

                auto sm = newVal.asStaticMethod();
                sm->func->ownerClass = cls;
            }

            // classmethod
            else if (newVal.isClassMethod()) {

                auto cm = newVal.asClassMethod();
                cm->func->ownerClass = cls;
            }

            cls->attributes.insert(key, val);
        }

        Value classValue(cls);

        // применяем декораторы снизу вверх
        for (auto it = decorators.rbegin(); it != decorators.rend(); ++it) {
            Value decorator = (*it)->eval(env);

            classValue = call(decorator, { classValue }, {}, env);
        }

        env->set(name, classValue);

        return classValue;
    }

QString ClassDefNode::toString() const { return "class " + name; }

bool ClassDefNode::shouldPrint() const { return false; }