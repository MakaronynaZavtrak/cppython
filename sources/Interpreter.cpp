#include "Environment.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include "BuiltinFunction.h"
#include <iostream>
#include <sstream>

#include "Runtime.h"
#include "../runtime/builtins/bytearray/ByteArrayMethods.h"
#include "../runtime/builtins/bytes/BytesMethods.h"
#include "../runtime/builtins/str/StrMethods.h"


/**
 * Проверяет, является ли введенная команда одной из предопределенных команд выхода.
 * Если введенная строка совпадает с одной из строк в массиве EXIT_COMMANDS,
 * метод возвращает true, иначе false.
 *
 * @param input Введенная строка, которая проверяется на совпадение с командами выхода.
 * @return true, если введенная строка соответствует одной из строк в EXIT_COMMANDS,
 *         иначе false.
 */
bool Interpreter::isExitCommand(const std::string& input) {
    return std::any_of(EXIT_COMMANDS.begin(), EXIT_COMMANDS.end(),
                       [&input](const char* cmd) { return input == cmd; });
}

/**
 * Объединяет строки из вектора в одну строку, разделяя их символом новой строки.
 * Если вектор пуст, возвращается пустая строка.
 *
 * @param lines Вектор строк, которые нужно объединить.
 * @return Строка, содержащая все строки из вектора, разделенные символами новой строки.
 */
std::string Interpreter::assembleCode(const std::vector<std::string>& lines) {
    std::ostringstream oss;
    for (size_t i = 0; i < lines.size(); ++i) {
        oss << lines[i];
        if (i + 1 < lines.size()) oss << '\n';
    }
    return oss.str();
}

Value Interpreter::executeNode(
    const std::shared_ptr<ASTNode>& node,
    const std::shared_ptr<Environment>& env) {

    const Value result = node->eval(env);

    if (node->shouldPrint() && !result.isNone()) {
        std::cout << result.display().toStdString() << "\n";
    }

    return result;
}

/**
 * Выполняет интерпретацию кода, переданного в виде строки. Разбивает код на токены
 * с помощью лексера, создает абстрактное синтаксическое дерево (AST) с помощью парсера
 * и вычисляет выражение дерева в заданной среде. Если результат выполнения выражения
 * не является присваиванием или условным оператором, выводит результат вычисления.
 * В случае ошибки выводит сообщение об ошибке.
 *
 * @param code Исходный код в виде строки, который нужно интерпретировать.
 * @param lexer Лексер, используемый для токенизации переданного кода.
 * @param env Среда, содержащая переменные и их значения, используемые во время интерпретации.
 */
void Interpreter::executeCode(
    const std::string& code, Lexer& lexer,
    const std::shared_ptr<Environment> &env) {

    try {

        const QVector<Token> tokens = lexer.tokenize(QString::fromStdString(code));
        Parser parser(tokens);
        const std::shared_ptr<ASTNode> ast = parser.parse();

        executeNode(ast, env);

    } catch (const std::runtime_error& e) {
        std::cout << e.what() << "\n";
    }
}


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

    const auto globalEnv = std::make_shared<Environment>();
    BuiltinFunction::registerBuiltins(globalEnv);

    Runtime::objectClass = std::make_shared<ClassValue>("object");

    Runtime::objectClass->name = "object";

    globalEnv->set("object", Value(Runtime::objectClass));

    Runtime::objectClass->attributes["__getattribute__"] =
    globalEnv->get("__object_getattribute__");

    Runtime::objectClass->attributes["__setattr__"] =
    globalEnv->get("__object_setattr__");



    Runtime::strClass = std::make_shared<ClassValue>("str");
    Runtime::strClass->name = "str";
    Runtime::strClass->bases.push_back(Runtime::objectClass);

    auto builtin = std::get<Value::BuiltinFunctionPtr>(makeMakeTransStrClassBuiltin().data);

    Runtime::strClass->attributes["maketrans"] = makeMakeTransStrClassBuiltin();

    globalEnv->set("str", Value(Runtime::strClass));

    Runtime::strClass->attributes["__call__"] = globalEnv->get("__str_call__");

    globalEnv->set("__str_type__", Value(Runtime::strClass));



    Runtime::bytesClass = std::make_shared<ClassValue>("bytes");
    Runtime::bytesClass->name = "bytes";
    Runtime::bytesClass->bases.push_back(Runtime::objectClass);

    Runtime::bytesClass->attributes["fromhex"] = makeFromHexClassBuiltin();
    Runtime::bytesClass->attributes["maketrans"] = makeMakeTransBytesClassBuiltin();
    Runtime::bytesClass->attributes["__bytes__"] = make__bytes__ClassBuiltin();

    globalEnv->set("bytes", Value(Runtime::bytesClass));
    Runtime::bytesClass->attributes["__call__"] = globalEnv->get("__bytes_call__");
    globalEnv->set("__bytes_type__", Value(Runtime::bytesClass));


    Runtime::bytearrayClass = std::make_shared<ClassValue>("bytearray");
    Runtime::bytearrayClass->name = "bytearray";
    Runtime::bytearrayClass->bases.push_back(Runtime::objectClass);

    globalEnv->set("bytearray", Value(Runtime::bytearrayClass));
    Runtime::bytearrayClass->attributes["__call__"] = globalEnv->get("__bytearray_call__");
    globalEnv->set("__bytearray_type__", Value(Runtime::bytearrayClass));
    Runtime::bytearrayClass->attributes["__bytes__"] = make__byteArray__ClassBuiltin();

    Lexer lexer;
    std::vector<std::string> buffer;
    bool isInBlock = false;

    while (true) {
        std::cout << (isInBlock ? CONTINUATION_PROMPT : MAIN_PROMPT);

        std::string line;
        if (!std::getline(std::cin, line)) break;

        if (!isInBlock && isExitCommand(line)) break;

        if (isInBlock) {
            if (line.empty()) {
                isInBlock = false;
            } else {
                buffer.push_back(line);
                continue;
            }
        } else {
            if (line.empty()) {
                continue;
            }
            buffer.push_back(line);
            if (line.back() == ':') {
                isInBlock = true;
                continue;
            }

            // если предыдущая строка была декоратором
            if (!buffer.empty()) {
                const std::string& prev = buffer.back();

                if (!prev.empty() && prev[0] == '@') {
                    isInBlock = true;
                    continue;
                }
            }
        }

        std::string code = assembleCode(buffer);
        executeCode(code, lexer, globalEnv);
        buffer.clear();
    }
}