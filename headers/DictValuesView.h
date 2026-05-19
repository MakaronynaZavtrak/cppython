//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTVALUESVIEW_H
#define CPPYTHON_DICTVALUESVIEW_H

#include <memory>

#include "ReprMixin.h"

class DictValue;

class DictValuesView : public ReprMixin {
public:

    explicit DictValuesView(std::shared_ptr<DictValue> dict);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] std::shared_ptr<DictValue> getDict() const;

private:

    std::shared_ptr<DictValue> dict;
};
#endif //CPPYTHON_DICTVALUESVIEW_H