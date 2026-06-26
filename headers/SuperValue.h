//
// Created by semyo on 04.05.2026.
//

#ifndef CPPYTHON_SUPERVALUE_H
#define CPPYTHON_SUPERVALUE_H
#include <utility>

#include "ClassValue.h"

class ClassValue;
class InstanceValue;

class SuperValue : public ReprMixin {
public:
    std::shared_ptr<ClassValue> currentClass;
    Value receiver; // self или cls
    std::shared_ptr<ClassValue> originClass; // где super вызван

    SuperValue(std::shared_ptr<ClassValue> cls, Value receiver, std::shared_ptr<ClassValue>orig)
        : currentClass(std::move(cls)), receiver(std::move(receiver)), originClass(std::move(orig)) {}

    [[nodiscard]] QString toString() const override {
        return "<super: " + (currentClass ? currentClass->name : "unknown") + ">";
    }
};
#endif //CPPYTHON_SUPERVALUE_H
