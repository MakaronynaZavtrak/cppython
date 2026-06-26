//
// Created by semyo on 01.06.2026.
//

#ifndef CPPYTHON_BYTESMETHODS_H
#define CPPYTHON_BYTESMETHODS_H
#include "Value.h"

Value getBytesAttr(const Value& obj, const QString& attr);

Value makeFromHexClassBuiltin();

Value makeMakeTransBytesClassBuiltin();

Value make__bytes__ClassBuiltin();
#endif //CPPYTHON_BYTESMETHODS_H