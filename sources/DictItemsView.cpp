//
// Created by semyo on 19.05.2026.
//

#include "DictItemsView.h"

#include "DictValue.h"

DictItemsView::DictItemsView(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

QString DictItemsView::toString() const {

    QString out = "dict_items([";

    bool first = true;

    for (const auto& key : dict->getOrder()) {

        if (!first) {
            out += ", ";
        }

        first = false;

        out += "('";
        out += key;
        out += "', ";
        out += dict->getItems()[key].toString();
        out += ")";
    }

    out += "])";

    return out;
}

std::shared_ptr<DictValue> DictItemsView::getDict() const {
    return dict;
}