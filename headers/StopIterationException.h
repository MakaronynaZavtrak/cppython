//
// Created by semyo on 18.05.2026.
//

#ifndef CPPYTHON_STOPITERATIONEXCEPTION_H
#define CPPYTHON_STOPITERATIONEXCEPTION_H
#include <exception>
class StopIterationException final : public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "StopIteration";
    }
};
#endif //CPPYTHON_STOPITERATIONEXCEPTION_H