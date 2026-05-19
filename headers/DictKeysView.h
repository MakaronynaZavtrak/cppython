//
// Created by semyo on 19.05.2026.
//

#ifndef CPPYTHON_DICTKEYSVIEW_H
#define CPPYTHON_DICTKEYSVIEW_H
#include <memory>

#include "ReprMixin.h"

class DictValue;

class DictKeysView : public ReprMixin {
public:

    explicit DictKeysView(std::shared_ptr<DictValue> dict);

    [[nodiscard]] QString toString() const override;

    [[nodiscard]] std::shared_ptr<DictValue> getDict() const;

private:
    std::shared_ptr<DictValue> dict;
};
#endif //CPPYTHON_DICTKEYSVIEW_H