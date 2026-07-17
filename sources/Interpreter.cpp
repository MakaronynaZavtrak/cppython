#include "Environment.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include "BuiltinFunction.h"
#include <iostream>
#include <sstream>

#include "../exception/PythonException.h"
#include "../runtime/Runtime.h"
#include "../runtime/exceptions/RegisterExceptionClasses.h"
#include <isocline.h>


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

    if (!Runtime::callStack.empty()) {

        auto& frame = Runtime::callStack.back();

        frame.currentLine = node->line;
        frame.sourceId = node->sourceId;
        frame.columnCaptured = false;
    }

    const Value result = node->eval(env);

    if (node->shouldPrint() && !result.isNone()) {
        std::cout << result.display().toStdString() << "\n";
    }

    return result;
}

void Interpreter::printSyntaxError(const SyntaxErrorException& e) {

    if (e.hasPosition) {
        std::cout << "  File \"" << Runtime::getSourceLabel(e.sourceId).toStdString()
                   << "\", line " << e.line << "\n";

        QString srcLine = Runtime::getSourceLine(e.sourceId, e.line);
        std::cout << "    " << srcLine.toStdString() << "\n";

        QString caretLine(srcLine.length() + 1, ' ');
        const int caretIdx = std::max(0, e.startColumn - 1);
        if (caretIdx < caretLine.length()) {
            caretLine[caretIdx] = '^';
        }
        std::cout << "    " << caretLine.toStdString() << "\n";
    }

    std::cout << e.what() << "\n";
}

bool Interpreter::hasUnclosedBrackets(const std::vector<std::string>& lines) {

    int depth = 0;
    char stringChar = 0;   // 0 = не в строке, иначе ' или "

    for (const auto& line : lines) {

        for (size_t i = 0; i < line.size(); ++i) {

            const char c = line[i];

            if (stringChar != 0) {
                // внутри строки — ждём закрывающую кавычку, уважая экранирование
                if (c == '\\') {
                    ++i; // пропускаем следующий символ
                    continue;
                }
                if (c == stringChar) {
                    stringChar = 0;
                }
                continue;
            }

            if (c == '"' || c == '\'') {
                stringChar = c;
                continue;
            }

            if (c == '#') {
                break; // комментарий — остаток строки игнорируем
            }

            if (c == '(' || c == '[' || c == '{') {
                ++depth;
            }
            else if (c == ')' || c == ']' || c == '}') {
                if (depth > 0) --depth;
            }
        }

        // строковый литерал не закрылся к концу строки —
        // (для простых '...'/"..." это ошибка, но тройные кавычки
        //  потребовали бы отдельной логики; для MVP считаем, что
        //  одиночные строки закрываются в пределах строки)
        stringChar = 0;
    }

    return depth > 0;
}

bool Interpreter::hasDefiniteSyntaxError(const std::string& code, Lexer& lexer) {

    QString normalized = QString::fromStdString(code);
    normalized.replace('\t', "    ");

    try {
        const QVector<Token> tokens = lexer.tokenize(normalized);
        Parser parser(tokens);
        (void)parser.parse();
        return false;
    }
    catch (const SyntaxErrorException& e) {
        return !e.incompleteInput;
    }
    catch (...) {
        return false;   // не синтаксис — разберёмся при реальном выполнении
    }
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

    QString normalizedCode = QString::fromStdString(code);
    normalizedCode.replace('\t', "    ");

    const int srcId = Runtime::registerSource(normalizedCode);

    CallStackGuard moduleGuard("<module>", srcId);

    try {
        const QVector<Token> tokens = lexer.tokenize(normalizedCode);
        Parser parser(tokens);
        const std::shared_ptr<ASTNode> ast = parser.parse();

        executeNode(ast, env);

    } catch (const SyntaxErrorException& e) {
        printSyntaxError(e);
    }
    catch (const PythonException& e) {
        printTraceback(e);
    }
}

void Interpreter::printTraceback(const PythonException& e) {

    if (!e.hasTraceback || e.traceback.empty()) {
        std::cout << e.what() << "\n";
        return;
    }

    std::cout << "Traceback (most recent call last):\n";

    for (const auto& frame : e.traceback) {

        std::cout << "  File \"" << Runtime::getSourceLabel(frame.sourceId).toStdString()
                   << "\", line " << frame.currentLine
                   << ", in " << frame.functionName.toStdString() << "\n";

        QString srcLine = Runtime::getSourceLine(frame.sourceId, frame.currentLine);

        std::cout << "    " << srcLine.toStdString() << "\n";

        if (frame.columnCaptured) {

            QString caretLine(srcLine.length(), ' ');

            const int startIdx = std::max(0, frame.currentStartColumn - 1);
            const int endIdx = std::min(static_cast<int>(srcLine.length()), frame.currentEndColumn - 1);

            if (frame.hasAnchor) {

                const int anchorStart = std::max(0, frame.anchorStartColumn - 1);
                const int anchorEnd = std::min(static_cast<int>(srcLine.length()), frame.anchorEndColumn - 1);

                for (int i = startIdx; i < endIdx; ++i) {

                    if (i >= anchorStart && i < anchorEnd) {
                        caretLine[i] = '^';
                    } else {
                        caretLine[i] = '~';
                    }
                }
            } else {
                for (int i = startIdx; i < endIdx; ++i) {
                    caretLine[i] = '^';
                }
            }

            std::cout << "    " << caretLine.toStdString() << "\n";
        }
    }

    std::cout << e.what() << "\n";
}

extern "C" bool cppython_is_input_complete(const char* input) {

    if (input == nullptr) return true;

    const std::string code(input);

    if (code.empty()) return true;

    std::vector<std::string> lines;
    std::string cur;
    for (const char ch : code) {
        if (ch == '\n') { lines.push_back(cur); cur.clear(); }
        else cur += ch;
    }
    lines.push_back(cur);

    // незакрытые скобки — ждём
    if (Interpreter::hasUnclosedBrackets(lines)) {
        return false;
    }

    const std::string& last = lines.back();

    if (!last.empty() && last.back() == ':') {
        return false;
    }

    // декоратор — ждём def
    if (!last.empty() && last[0] == '@') {
        return false;
    }

    if (lines.size() > 1) {

        return std::all_of(
            last.begin(),
            last.end(),
            [](const char ch) { return ch == ' ' || ch == '\t'; });
    }

    return true;
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
    Runtime::initialize(globalEnv);
    registerExceptionClasses(globalEnv);

    Lexer lexer;

    ic_set_prompt_marker(">>> ", "... ");
    ic_enable_multiline(true);
    ic_set_is_complete_fun(&cppython_is_input_complete);
    ic_set_history(nullptr, -1);
    ic_enable_auto_tab(true);
    ic_set_default_completer(nullptr, nullptr);
    ic_enable_completion_preview(false);
    ic_enable_hint(false);

    std::string buffer;

    while (true) {

        char* raw = ic_readline("");
        if (raw == nullptr) break;

        std::string line(raw);
        ic_free(raw);

        if (buffer.empty()) {
            if (line.empty()) continue;
            if (isExitCommand(line)) break;
            buffer = line;
        }
        else {
            buffer += "\n";
            buffer += line;
        }

        if (!cppython_is_input_complete(buffer.c_str())) {
            continue;
        }

        executeCode(buffer, lexer, globalEnv);
        buffer.clear();
    }
}