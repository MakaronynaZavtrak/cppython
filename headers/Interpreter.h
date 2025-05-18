#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Environment.h"
#include "Lexer.h"

/**
 * @class Interpreter
 * @brief Минимальный интерпретатор Python, предоставляющий REPL для выполнения Python-подобного кода.
 *
 * Класс Interpreter служит основной точкой входа для запуска минимального интерпретатора Python.
 * Этот интерпретатор поддерживает чтение пользовательского ввода, токенизацию, разбор в абстрактное синтаксическое дерево (AST)
 * и выполнение Python-подобных инструкций и выражений. REPL позволяет пользователю интерактивно вводить код
 * и мгновенно видеть результаты.
 */
class Interpreter {
    public:
        static void run(int argc, char *argv[]);
    private:
        static constexpr const char* MAIN_PROMPT = ">>> ";
        static constexpr const char* CONTINUATION_PROMPT = "... ";
        static constexpr std::array<const char*, 4> EXIT_COMMANDS = {"exit", "quit", "q", "Q"};

        /**
         * @brief Проверяет, является ли введенная команда командой выхода из интерпретатора
         * @param input Введенная пользователем строка
         * @return true, если команда является командой выхода; false в противном случае
         */
        static bool isExitCommand(const std::string &input);

        /**
         * @brief Объединяет несколько строк кода в единый блок
         * @param lines Вектор строк кода для объединения
         * @return Строка, содержащая объединенный код
         */
        static std::string assembleCode(const std::vector<std::string> &lines);

        /**
         * @brief Выполняет заданный блок кода
         * @param code Строка с кодом для выполнения
         * @param lexer Экземпляр лексического анализатора
         * @param env Окружение для выполнения кода
         */
        static void executeCode(const std::string& code, Lexer& lexer, Environment& env);


};
#endif //INTERPRETER_H
