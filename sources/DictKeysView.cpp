//
// Created by semyo on 19.05.2026.
//
#include "DictKeysView.h"

#include "DictValue.h"

DictKeysView::DictKeysView(std::shared_ptr<DictValue> dict)
    : dict(std::move(dict)) {}

QString DictKeysView::toString() const {

    QString out = "dict_keys([";

    bool first = true;

    for (const auto& key : dict->getOrder()) {

        if (!first) {
            out += ", ";
        }

        first = false;

        out += key.toString();
    }

    out += "])";

    return out;
}

QString DictKeysView::repr() const {

    QString out = "dict_keys([";

    bool first = true;

    for (const auto& key : dict->getOrder()) {

        if (!first) {
            out += ", ";
        }

        first = false;

        out += key.repr();
    }

    out += "])";

    return out;
}

std::shared_ptr<DictValue>DictKeysView::getDict() const {
    return dict;
}
