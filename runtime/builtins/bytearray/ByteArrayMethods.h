//
// Created by semyo on 11.06.2026.
//

#ifndef CPPYTHON_BYTEARRAYMETHODS_H
#define CPPYTHON_BYTEARRAYMETHODS_H
#include "Value.h"

Value getByteArrayAttr(const Value& obj, const QString& attr);

Value make_byteArray_ClassBuiltin();

Value makeByteArrayFromHexBuiltin();

Value makeByteArrayMakeTransBuiltin();
#endif //CPPYTHON_BYTEARRAYMETHODS_H