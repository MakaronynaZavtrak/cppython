#ifndef INTERPRETER_H
#define INTERPRETER_H
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
};
#endif //INTERPRETER_H
