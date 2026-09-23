#include "Environment.h"
#include "Interpreter.h"

#include <fstream>

#include "Lexer.h"
#include "Parser.h"
#include "BuiltinFunction.h"
#include <iostream>
#include <sstream>

#include "../exception/PythonException.h"
#include "../runtime/Runtime.h"
#include "../runtime/exceptions/RegisterExceptionClasses.h"
#include <isocline.h>

#ifdef _WIN32
  #include <io.h>
  #define CPP_ISATTY(fd) _isatty(fd)
  #define CPP_FILENO(f)  _fileno(f)
#else
  #include <unistd.h>
  #define CPP_ISATTY(fd) isatty(fd)
  #define CPP_FILENO(f)  fileno(f)
#endif

namespace {

    constexpr const char* ANSI_RESET      = "\x1b[0m";
    constexpr const char* ANSI_RED        = "\x1b[91m";
    constexpr const char* ANSI_TILDE = "\x1b[38;2;215;0;0m";
    constexpr const char* ANSI_GREEN      = "\x1b[92m";
    constexpr const char* ANSI_DARK_GREEN = "\x1b[32m";
    constexpr const char* ANSI_WHITE      = "\x1b[97m";

    bool colorsEnabled() {
        static const bool enabled = (CPP_ISATTY(CPP_FILENO(stdout)) != 0);
        return enabled;
    }

    std::string paint(const char* color, const std::string& text) {
        if (!colorsEnabled()) return text;
        return std::string(color) + text + ANSI_RESET;
    }

    std::string renderCaretLine(const QString& caretLine) {

        if (!colorsEnabled()) return caretLine.toStdString();

        auto colorFor = [](const char c) -> const char* {
            if (c == '~') return ANSI_TILDE;
            if (c == '^') return ANSI_RED;
            return nullptr;
        };

        std::string out;
        char cur = 0;

        for (const QChar& qc : caretLine) {

            const char c = qc.toLatin1();

            if (c != cur) {
                if (colorFor(cur) != nullptr) out += ANSI_RESET;
                if (const char* col = colorFor(c)) out += col;
                cur = c;
            }

            out += c;
        }

        if (colorFor(cur) != nullptr) out += ANSI_RESET;

        return out;
    }

    std::string renderSourceLine(const QString& srcLine,
                             int startColumn, int endColumn,
                             bool hasAnchor,
                             int anchorStartColumn, int anchorEndColumn) {

        if (!colorsEnabled()) return srcLine.toStdString();

        const int len = srcLine.length();
        const int startIdx = std::max(0, startColumn - 1);
        const int endIdx   = std::min(len, endColumn - 1);
        if (startIdx >= endIdx) return srcLine.toStdString();

        int anchorStart = -1, anchorEnd = -1;
        if (hasAnchor) {
            anchorStart = std::max(0, anchorStartColumn - 1);
            anchorEnd   = std::min(len, anchorEndColumn - 1);
        }

        auto colorFor = [&](int i) -> const char* {
            if (i < startIdx || i >= endIdx) return nullptr;
            if (hasAnchor)
                return (i >= anchorStart && i < anchorEnd) ? ANSI_RED
                                                           : ANSI_TILDE;
            return ANSI_RED;
        };

        std::string out;
        const char* cur = nullptr;
        int runStart = 0;

        auto flush = [&](int from, int to) {
            if (to > from) {
                const QByteArray ba = srcLine.mid(from, to - from).toUtf8();
                out.append(ba.constData(), ba.size());
            }
        };

        for (int i = 0; i < len; ++i) {
            const char* col = colorFor(i);
            if (col != cur) {
                flush(runStart, i);
                if (cur != nullptr) out += ANSI_RESET;
                if (col != nullptr) out += col;
                cur = col;
                runStart = i;
            }
        }
        flush(runStart, len);
        if (cur != nullptr) out += ANSI_RESET;

        return out;
    }

}

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
    const std::shared_ptr<Environment>& env,
    bool echo) {

    if (!Runtime::callStack.empty()) {

        auto& frame = Runtime::callStack.back();

        frame.currentLine = node->line;
        frame.sourceId = node->sourceId;
        frame.columnCaptured = false;
    }

    const Value result = node->eval(env);

    if (echo && node->shouldPrint() && !result.isNone()) {
        std::cout << result.display().toStdString() << "\n";
    }

    return result;
}

void Interpreter::printSyntaxError(const SyntaxErrorException& e) {

    if (e.hasPosition) {

        // строка File — метка и номер строки тёмно-зелёным (в синтаксисе нет ", in <...>")
        std::cout << "  File "
                  << paint(ANSI_DARK_GREEN, "\"" + Runtime::getSourceLabel(e.sourceId).toStdString() + "\"")
                  << ", line "
                  << paint(ANSI_DARK_GREEN, std::to_string(e.line))
                  << "\n";

        const QString srcLine = Runtime::getSourceLine(e.sourceId, e.line);

        // диапазон виновника [startCol, endCol); гарантируем хотя бы один символ
        const int startCol = e.startColumn;
        const int endCol   = std::max(e.endColumn, e.startColumn + 1);

        // исходная строка с подсветкой виновника (тускло-красный, как ^)
        std::cout << "    "
                  << renderSourceLine(srcLine, startCol, endCol,
                                      false, 0, 0)
                  << "\n";

        // каретная строка ^^^ под виновником
        QString caretLine(srcLine.length() + 1, ' ');
        const int startIdx = std::max(0, startCol - 1);
        const int endIdx   = std::min(static_cast<int>(caretLine.length()), endCol - 1);
        for (int i = startIdx; i < endIdx; ++i) {
            caretLine[i] = '^';
        }
        std::cout << "    " << renderCaretLine(caretLine) << "\n";
    }

    // строка "SyntaxError: ..." — та же раскраска, что у обычных исключений
    const std::string full = e.what();
    const std::size_t sep  = full.find(": ");
    if (sep != std::string::npos) {
        std::cout << paint(ANSI_GREEN,      full.substr(0, sep))
                  << paint(ANSI_WHITE,      ":") << " "
                  << paint(ANSI_DARK_GREEN, full.substr(sep + 2)) << "\n";
    } else {
        std::cout << paint(ANSI_GREEN, full) << "\n";
    }
}

bool Interpreter::hasUnclosedBrackets(const std::vector<std::string>& lines) {

    int depth = 0;
    char stringChar = 0;

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

// Interpreter.cpp
void Interpreter::runFile(
    const std::string& path, Lexer& lexer,
    const std::shared_ptr<Environment>& env) {

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "cppython: can't open file '" << path
                  << "': [Errno 2] No such file or directory\n";
        return;
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    executeProgram(ss.str(), lexer, env, path);
}

void Interpreter::executeProgram(
    const std::string& code, Lexer& lexer,
    const std::shared_ptr<Environment>& env,
    const std::string& label) {

    QString normalizedCode = QString::fromStdString(code);
    normalizedCode.replace("\r\n", "\n");
    normalizedCode.replace('\r',  "\n");
    normalizedCode.replace('\t', "    ");

    const int srcId = Runtime::registerSource(normalizedCode,
                                              QString::fromStdString(label));

    CallStackGuard moduleGuard("<module>", srcId);

    try {
        const QVector<Token> tokens = lexer.tokenize(normalizedCode);
        Parser parser(tokens);

        const std::vector<std::shared_ptr<ASTNode>> program = parser.parseProgram();

        for (const auto& stmt : program) {
            if (stmt) executeNode(stmt, env, /*echo=*/false);
        }
    }
    catch (const SyntaxErrorException& e) {
        printSyntaxError(e);
    }
    catch (const PythonException& e) {
        printTraceback(e);
    }
}

bool Interpreter::isSyntacticallyIncomplete(const std::string& code) {

    QString normalized = QString::fromStdString(code);
    normalized.replace('\t', "    ");

    try {
        Lexer lexer;
        const QVector<Token> tokens = lexer.tokenize(normalized);
        Parser parser(tokens);
        (void)parser.parse();
        return false;
    }
    catch (const SyntaxErrorException& e) {
        return e.incompleteInput;
    }
    catch (...) {
        return false;
    }
}

void Interpreter::printExceptionLine(const PythonException& e) {

    const std::string type = e.getTypeName().toStdString();
    const std::string msg  = e.getMessage().toStdString();

    std::cout << paint(ANSI_GREEN, type);

    // пустое сообщение — CPython печатает только имя, без двоеточия
    if (!msg.empty()) {
        std::cout << paint(ANSI_WHITE, ":") << " " << paint(ANSI_DARK_GREEN, msg);
    }

    std::cout << "\n";
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
        if (ast == nullptr) return;

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
        printExceptionLine(e);
        return;
    }

    std::cout << "Traceback (most recent call last):\n";

    for (const auto& frame : e.traceback) {

        std::cout << "  File "
                  << paint(ANSI_DARK_GREEN, "\"" + Runtime::getSourceLabel(frame.sourceId).toStdString() + "\"")
                  << ", line "
                  << paint(ANSI_DARK_GREEN, std::to_string(frame.currentLine))
                  << ", in "
                  << paint(ANSI_DARK_GREEN, frame.functionName.toStdString())
                  << "\n";

        QString srcLine = Runtime::getSourceLine(frame.sourceId, frame.currentLine);

        if (frame.columnCaptured) {

            std::cout << "    "
                      << renderSourceLine(srcLine,
                                          frame.currentStartColumn, frame.currentEndColumn,
                                          frame.hasAnchor,
                                          frame.anchorStartColumn, frame.anchorEndColumn)
                      << "\n";

            // ~~^~~
            QString caretLine(srcLine.length(), ' ');

            const int startIdx = std::max(0, frame.currentStartColumn - 1);
            const int endIdx   = std::min(static_cast<int>(srcLine.length()), frame.currentEndColumn - 1);

            if (frame.hasAnchor) {

                const int anchorStart = std::max(0, frame.anchorStartColumn - 1);
                const int anchorEnd   = std::min(static_cast<int>(srcLine.length()), frame.anchorEndColumn - 1);

                for (int i = startIdx; i < endIdx; ++i) {
                    caretLine[i] = (i >= anchorStart && i < anchorEnd) ? '^' : '~';
                }
            } else {
                for (int i = startIdx; i < endIdx; ++i) {
                    caretLine[i] = '^';
                }
            }

            std::cout << "    " << renderCaretLine(caretLine) << "\n";

        } else {
            std::cout << "    " << srcLine.toStdString() << "\n";
        }
    }

    printExceptionLine(e);
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

    Lexer probe;
    if (Interpreter::hasDefiniteSyntaxError(code, probe)) {
        return true;
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

        const bool lastIsBlank = std::all_of(
            last.begin(), last.end(),
            [](const char ch) { return ch == ' ' || ch == '\t'; });

        if (!lastIsBlank) return false;

        return !Interpreter::isSyntacticallyIncomplete(code);
    }

    return true;
}

namespace {

    std::vector<int> buildUtf8Offsets(const QString& s) {

        std::vector<int> offsets(static_cast<size_t>(s.length()) + 1, 0);
        int byteOff = 0;

        for (int i = 0; i < s.length(); ) {

            offsets[static_cast<size_t>(i)] = byteOff;

            if (s[i].isHighSurrogate() && i + 1 < s.length() && s[i+1].isLowSurrogate()) {
                offsets[static_cast<size_t>(i) + 1] = byteOff;
                byteOff += 4;
                i += 2;
            }
            else {
                byteOff += QString(s[i]).toUtf8().size();
                i += 1;
            }
        }

        offsets[static_cast<size_t>(s.length())] = byteOff;
        return offsets;
    }

    const QSet<QString>& pythonBuiltins() {
        static const QSet<QString> names = {
            // функции и типы
            "print", "len", "list", "tuple", "dict", "set", "frozenset",
            "str", "int", "float", "bool", "bytes", "bytearray", "range",
            "iter", "next", "hash", "repr", "reversed", "format", "super",
            "hasattr", "getattr", "setattr", "property", "staticmethod",
            "classmethod", "object", "type", "exit", "sum", "all", "any",
            "min", "max", "sorted", "map", "filter", "zip", "enumerate",
            "callable", "abs", "round",
            // исключения
            "BaseException", "Exception", "ArithmeticError", "OverflowError",
            "LookupError", "IndexError", "KeyError", "RuntimeError", "NameError",
            "AttributeError", "SyntaxError", "TypeError", "ValueError",
            "UnicodeDecodeError", "StopIteration", "GeneratorExit",
            "ZeroDivisionError"
        };
        return names;
    }

}

extern "C" void cppython_highlighter(ic_highlight_env_t* henv, const char* input, void* arg) {

    (void)arg;
    if (input == nullptr || henv == nullptr) return;

    const QString code = QString::fromUtf8(input);
    if (code.isEmpty()) return;

    const std::vector<int> off = buildUtf8Offsets(code);

    try {

        Lexer lexer;
        lexer.setTolerant(true);

        const QVector<Token> tokens = lexer.tokenize(code);

        for (const Token& t : tokens) {

            const char* style = nullptr;

            switch (t.type) {
                case TOKEN_NUMBER:  style = "py-number";  break;
                case TOKEN_STRING:
                case TOKEN_BYTES:   style = "py-string";  break;
                case TOKEN_KEYWORD:
                case TOKEN_BOOL:
                case TOKEN_NONE:    style = "py-keyword"; break;
                case TOKEN_OP:
                case TOKEN_AT:      style = "py-op";      break;
                case TOKEN_ID:
                    if (pythonBuiltins().contains(t.value)) style = "py-builtin"; break;
                case TOKEN_COMMENT: style = "py-comment"; break;
                default: break;   // NEWLINE/INDENT/DEDENT/EOF
            }

            if (style == nullptr) continue;
            if (t.startPos < 0 || t.endPos > code.length() || t.endPos <= t.startPos) continue;

            const long p = off[static_cast<size_t>(t.startPos)];
            const long n = off[static_cast<size_t>(t.endPos)] - p;

            if (n > 0) ic_highlight(henv, p, n, style);
        }
    }
    catch (...) {
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

    const auto globalEnv = std::make_shared<Environment>();
    BuiltinFunction::registerBuiltins(globalEnv);
    Runtime::initialize(globalEnv);
    registerExceptionClasses(globalEnv);

    Lexer lexer;

    if (argc > 1) {
        runFile(argv[1], lexer, globalEnv);
        return;
    }



    std::cout << "Hello and welcome to my minimal Python interpreter!\n"
                 "Made by Semenov Oleg, with care from MathMech. Let's code!\n";

    ic_set_prompt_marker(">>> ", "... ");
    ic_enable_multiline(true);
    ic_set_is_complete_fun(&cppython_is_input_complete);
    ic_set_history(nullptr, -1);
    ic_enable_completion_preview(false);
    ic_enable_hint(false);

    ic_style_def("py-number",  "gold");
    ic_style_def("py-string",  "#4EC9B0");
    ic_style_def("py-keyword", "#2361E8");
    ic_style_def("py-builtin", "#149BDE");
    ic_style_def("py-op",      "white");
    ic_style_def("py-comment", "#D42F2F");

    ic_set_default_highlighter(&cppython_highlighter, nullptr);

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