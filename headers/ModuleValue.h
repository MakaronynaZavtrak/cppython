//
// Created by semyo on 03.10.2026.
//

#ifndef CPPYTHON_MODULEVALUE_H
#define CPPYTHON_MODULEVALUE_H
#include <QMap>
#include <QString>
#include "Value.h"

class ModuleValue {
public:
    QString name;
    QMap<QString, Value> members;

    explicit ModuleValue(QString name) : name(std::move(name)) {}

    [[nodiscard]] QString toString() const;
};
#endif //CPPYTHON_MODULEVALUE_H