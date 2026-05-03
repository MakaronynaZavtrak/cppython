//
// Created by semyo on 01.05.2026.
//

#ifndef CPPYTHON_CLASSVALUE_H
#define CPPYTHON_CLASSVALUE_H
#include <qmap.h>
#include <QString>

#include "Value.h"

class ClassValue {
public:
    QString name;
    QMap<QString, Value> attributes;
    std::vector<std::shared_ptr<ClassValue>> bases;

    explicit ClassValue(QString name)
       : name(std::move(name)) {}

    QString toString() const {
        return QString("<class '%1.%2'>")
            .arg("__main__", name);
    }
};
#endif //CPPYTHON_CLASSVALUE_H