//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_BREAKEXCEPTION_H
#define CPPYTHON_BREAKEXCEPTION_H
#include "PythonException.h"
/**
 * @class BreakException
 * @brief Исключение, генерируемое для выхода из выполнения цикла.
 *
 * BreakException используется для имитации выполнения конструкции `break`
 * внутри циклов. Это исключение может содержать сообщение для дополнительных
 * описаний возникшей ситуации.
 *
 * @details
 * Данный класс является производным от `std::exception` и предлагает
 * возможность передачи текстового сообщения, доступного через метод `what`.
 * Это сообщение помогает идентифицировать причину прерывания выполнения.
 *
 * @note
 * Исключение `BreakException` обычно обрабатывается специально в интерпретаторе
 * или компиляторе для управления потоком выполнения.
 */
class BreakException final : public PythonException {
public:
    explicit BreakException() : PythonException("", "") {}
};
#endif //CPPYTHON_BREAKEXCEPTION_H