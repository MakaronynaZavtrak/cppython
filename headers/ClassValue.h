//
// Created by semyo on 01.05.2026.
//

#ifndef CPPYTHON_CLASSVALUE_H
#define CPPYTHON_CLASSVALUE_H
#include <qmap.h>
#include <QString>

class Value;

class ClassValue {
public:
    QString name;
    QMap<QString, Value> attributes;

    explicit ClassValue(QString name)
       : name(std::move(name)) {}
};
#endif //CPPYTHON_CLASSVALUE_H