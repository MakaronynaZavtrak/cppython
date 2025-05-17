#include "Environment.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include <iostream>

/**
 * Запускает основной цикл интерпретатора Python. Этот метод непрерывно принимает
 * пользовательский ввод, обрабатывает его с помощью лексера и парсера, вычисляет результат
 * и выводит результат вычисления или сообщение об ошибке. Цикл завершается,
 * когда пользователь вводит команды выхода, такие как "exit", "quit", "q" или "Q".
 *
 * @param argc Количество аргументов командной строки, переданных программе.
 * @param argv Массив строк аргументов командной строки.
 */
void Interpreter::run(int argc, char* argv[]) {
    std::cout << "Hello and welcome to my minimal Python interpreter!\n"
                 "Made by Semenov Oleg, with care from MathMech. Let's code!\n";

    std::string input;
    Environment env;
    Lexer lexer;
    while (true) {
        std::cout << MAIN_PROMPT;
        std::getline(std::cin, input);

        if (input == "exit" || input == "quit" || input == "q" || input == "Q") break;

        if (input.empty()) {
            continue;
        }

        try {
            QVector<Token> tokens = lexer.tokenize(QString::fromStdString(input));
            Parser parser(tokens);
            const std::shared_ptr<ASTNode> ast = parser.parse();

            const auto result = ast->eval(env);
            if (!dynamic_cast<AssignNode*>(ast.get())) {
                std::cout << result.toString().toStdString() << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}