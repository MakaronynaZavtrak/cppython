//
// Created by semyo on 19.05.2026.
//

#include "DictValue.h"
#include "DictValuesView.h"

DictValuesView::DictValuesView(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

QString DictValuesView::toString() const {

    QString out = "dict_values([";

    bool first = true;

    for (const auto& key : dict->getOrder()) {

        if (!first) {
            out += ", ";
        }

        first = false;

        out += dict->getItems()[key].toString();
    }

    out += "])";

    return out;
}

std::shared_ptr<DictValue> DictValuesView::getDict() const {
    return dict;
}