//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTITEMSVIEW_H
#define CPPYTHON_DICTITEMSVIEW_H
#include <memory>

#include "ReprMixin.h"

class DictValue;

class DictItemsView : public ReprMixin {
public:

    explicit DictItemsView(std::shared_ptr<DictValue> dict);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] std::shared_ptr<DictValue> getDict() const;

private:

    std::shared_ptr<DictValue> dict;
};
#endif //CPPYTHON_DICTITEMSVIEW_H