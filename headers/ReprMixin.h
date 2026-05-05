//
// Created by semyo on 05.05.2026.
//

#ifndef CPPYTHON_REPRMIXIN_H
#define CPPYTHON_REPRMIXIN_H
#include <QString>

class ReprMixin {
public:
    virtual QString toString() const = 0;

    virtual QString repr() const {
        return "'" + toString() + "'";
    }

    virtual ~ReprMixin() = default;
};
#endif //CPPYTHON_REPRMIXIN_H