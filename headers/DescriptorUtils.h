//
// Created by semyo on 12.05.2026.
//

#ifndef CPPYTHON_DESCRIPTORUTILS_H
#define CPPYTHON_DESCRIPTORUTILS_H
#include <memory>

class Value;
class ClassValue;

namespace DescriptorUtils {
    bool hasGet(const Value& descriptor);

    bool hasSet(const Value& descriptor);

    Value callGet(const Value& descriptor,
                  const Value& instance,
                  const std::shared_ptr<ClassValue>& owner);

    void callSet(const Value& descriptor,
                 const Value& instance,
                 const std::shared_ptr<ClassValue>& owner,
                 const Value& value);
}

#endif //CPPYTHON_DESCRIPTORUTILS_H
