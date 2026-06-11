//
// Created by semyo on 11.06.2026.
//

#ifndef CPPYTHON_BYTEARRAYMETHODS_H
#define CPPYTHON_BYTEARRAYMETHODS_H
#include "Value.h"

Value getByteArrayAttr(const Value& obj, const QString& attr);

Value make__byteArray__ClassBuiltin();
#endif //CPPYTHON_BYTEARRAYMETHODS_H