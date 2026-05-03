//
// Created by semyo on 03.05.2026.
//

#ifndef CPPYTHON_DESCRIPTORUTILS_H
#define CPPYTHON_DESCRIPTORUTILS_H
#include "Value.h"

Value applyDescriptor(const Value&,
                      const Value::InstancePtr &,
                      const Value::ClassPtr &);
#endif //CPPYTHON_DESCRIPTORUTILS_H