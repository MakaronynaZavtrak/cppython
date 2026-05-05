//
// Created by semyo on 04.05.2026.
//

#ifndef CPPYTHON_SUPERVALUE_H
#define CPPYTHON_SUPERVALUE_H

class ClassValue;
class InstanceValue;

class SuperValue {
public:
    std::shared_ptr<ClassValue> currentClass;
    std::shared_ptr<InstanceValue> instance; // self
    std::shared_ptr<ClassValue> originClass; // где super вызван

    SuperValue(std::shared_ptr<ClassValue> cls, std::shared_ptr<InstanceValue> inst, std::shared_ptr<ClassValue>orig)
        : currentClass(std::move(cls)), instance(std::move(inst)), originClass(std::move(orig)) {}
};
#endif //CPPYTHON_SUPERVALUE_H
