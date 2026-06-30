//
// Created by semyo on 27.06.2026.
//

#ifndef CPPYTHON_CONTINUEEXCEPTION_H
#define CPPYTHON_CONTINUEEXCEPTION_H
#include "PythonException.h"
/**
 * @class ContinueException
 * @brief Исключение, используемое для реализации оператора continue.
 *
 * Эта структура исключения предназначена для управления потоком выполнения в циклических конструкциях,
 * таких как `while` или `for`. Она позволяет пропускать оставшуюся часть текущей итерации и переходить
 * к следующей итерации цикла.
 *
 * @details
 * Классу передается сообщение, описывающее причину возникновения исключения. Это сообщение
 * может быть получено с помощью метода `what`, что делает удобной диагностику и отладку
 * кода при возникновении подобных исключительных ситуаций.
 *
 * Этот класс является финальным и не может быть унаследован.
 */
class ContinueException final : public PythonException {
public:
    explicit ContinueException() : PythonException(Runtime::baseExceptionClass, "") {}
};
#endif //CPPYTHON_CONTINUEEXCEPTION_H