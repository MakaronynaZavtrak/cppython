#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Value.h"
#include "Environment.h"
#include <memory>
#include <cmath>
#include <utility>

#include "CallRuntime.h"
#include "ClassMethodValue.h"
#include "ClassUtils.h"
#include "DictValue.h"
#include "FunctionValue.h"
#include "InstanceValue.h"
#include "Interpreter.h"
#include "IteratorValue.h"
#include "ListValue.h"
#include "Param.h"
#include "Runtime.h"
#include "SetValue.h"
#include "SliceValue.h"
#include "StaticMethodValue.h"
#include "StopIterationException.h"
#include "TupleValue.h"

/**
 * @class ASTNode
 * @brief Представляет узел абстрактного синтаксического дерева (AST) в интерпретаторе языка программирования.
 *
 * ASTNode служит базовым классом для всех типов узлов в абстрактном синтаксическом дереве.
 * Производные классы реализуют конкретные типы узлов и определяют их поведение для вычисления
 * и строкового представления. Этот класс спроектирован для расширения и не может быть создан напрямую.
 *
 * @details
 * Абстрактное синтаксическое дерево представляет иерархическую структуру исходного кода, где каждый
 * узел соответствует синтаксической конструкции, такой как переменная, оператор или функция.
 * От производных классов ожидается предоставление конкретных реализаций методов `eval` и `toString`.
 */
class ASTNode {
public:
    using EnvPtr = std::shared_ptr<Environment>;
    virtual ~ASTNode() = default;
    [[nodiscard]] virtual Value eval(EnvPtr env) const = 0;
    [[nodiscard]] virtual QString toString() const = 0;
    [[nodiscard]] virtual bool shouldPrint() const { return true; }
};


/**
 * @class ValueNode
 * @brief Представляет узел, содержащий значение, в абстрактном синтаксическом дереве.
 *
 * ValueNode хранит конкретное значение, представленное экземпляром класса `Value`.
 * Этот узел используется для представления литеральных значений, таких как числа, строки, булевы значения и другие.
 *
 * @details
 * Узел предоставляет методы для вычисления (`eval`) и строкового представления (`toString`) своего значения.
 * Метод `eval` просто возвращает сохранённое значение, в то время как `toString` преобразует его в строку
 * на основе типа данных, содержащегося в `Value`. Класс является финальным и не может быть унаследован.
 */
class ValueNode final : public ASTNode {
public:
    explicit ValueNode(Value value) : value(std::move(value)) {}

    Value value;

    [[nodiscard]] QString toString() const override {
        return value.toString();
    }

    [[nodiscard]] Value eval(EnvPtr env) const override {
        return value;
    }
};

class UnaryOpNode final : public ASTNode {

    QString op;
    std::shared_ptr<ASTNode> operand;

    enum class Operation {
        Not,
        UnaryPlus,
        UnaryMinus
    };

    static Operation parseOperation(const QString &op) {
        static const std::unordered_map<QString, Operation> opMap = {
            {"+", Operation::UnaryPlus},
            {"-", Operation::UnaryMinus},
            {"not", Operation::Not}
        };

        const auto it = opMap.find(op);
        if (it == opMap.end()) {
            throw std::runtime_error("Unsupported operation: " + op.toStdString());
        }
        return it->second;
    }

public:

    UnaryOpNode(QString  op, std::shared_ptr<ASTNode> operand)
        : op(std::move(op)), operand(std::move(operand)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        const Value val = operand->eval(env);

        switch (parseOperation(op)) {
            case Operation::Not:        return Value(!val.toBool());
            case Operation::UnaryPlus:  return +val;
            case Operation::UnaryMinus: return -val;

            default: throw std::runtime_error("Unsupported operation: " + op.toStdString());
        }


    }

    [[nodiscard]] QString toString() const override {
        return "(" + operand->toString() + op + ")";
    }
};

/**
 * @class BinOpNode
 * @brief Представляет узел бинарной операции в абстрактном синтаксическом дереве (AST) интерпретатора языка программирования.
 *
 * BinOpNode обрабатывает бинарные операции, такие как арифметические выражения, сравнения или конкатенация строк.
 * Он принимает два дочерних узла (левый и правый) и оператор, и вычисляет их для получения результата.
 *
 * @details
 * Класс поддерживает вычисление (`eval`) и строковое представление (`toString`) бинарных операций.
 * Поддерживаемые операции включают арифметические (`+`, `-`, `*`, `/`, `%`, `**`, `//`), сравнения (`==`, `!=`, `>`, `>=`, `<`, `<=`),
 * и строковые операции. Узел вычисляется путем рекурсивного вычисления левого и правого дочерних узлов
 * и применения указанного оператора. Если встречается неподдерживаемая операция или комбинация типов операндов,
 * генерируются соответствующие ошибки.
 *
 * Метод `eval` обеспечивает правильную обработку типов (например, числовых, строковых или смешанных) и делегирует конкретные
 * поведения вспомогательным методам, таким как `evalTwoNumbers`, `evalTwoStrings` и `evalNumAndString`.
 */
class BinOpNode final : public ASTNode {

    std::shared_ptr<ASTNode> left;
    QString op; // "+", "-", "=", "/", "%", "*", "**", "//", "=="
    std::shared_ptr<ASTNode> right;

public:

    BinOpNode(std::shared_ptr<ASTNode> left, QString  op, std::shared_ptr<ASTNode> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    [[nodiscard]] QString toString() const override {
        return "(" + left->toString() + " " + op + " " + right->toString() + ")";
    }

    /**
     * @brief Вычисляет бинарную операцию, представленную текущим узлом.
     *
     * Этот метод рекурсивно вычисляет левый и правый дочерние узлы для получения их значений,
     * а затем применяет бинарную операцию, связанную с этим узлом (например, арифметическую, сравнение),
     * на основе строки оператора. Метод поддерживает операции над числовыми, строковыми или смешанными типами.
     * Выбрасывает исключение, если операция не поддерживается для данных типов операндов.
     *
     * @param env Среда выполнения, содержащая привязки переменных и состояние.
     * @return Результат вычисления бинарной операции. Тип результата зависит от операции
     *         и операндов (например, числовой результат для арифметических операций, строковый результат для строковых операций).
     * @throws std::runtime_error Если операция не поддерживается для данных типов операндов.
     */
    [[nodiscard]] Value eval(const EnvPtr env) const override {

        const Value l = left->eval(env);
        const Value r = right->eval(env);

        switch (parseOperation(op)) {
            case Operation::Add:            return l + r;
            case Operation::Subtract:       return l - r;
            case Operation::Multiply:       return l * r;
            case Operation::Power:          return l.power(r);
            case Operation::Divide:         return l / r;
            case Operation::Modulo:         return l % r;
            case Operation::IntDivide:      return l.intDivide(r);
            case Operation::Is:             return Value(l.is(r));

            default: throw std::runtime_error("Unsupported operation: " + op.toStdString());
        }
    }

    /**
     * @enum Operation
     * @brief Представляет операции, которые могут быть выполнены в бинарном узле AST.
     *
     * Перечисление `Operation` определяет набор поддерживаемых бинарных операций,
     * таких как арифметические операции, операции сравнения и операции модификации.
     * Оно используется для представления и обработки этих операций в синтаксическом анализе
     * и вычислении узлов AST.
     *
     * @details
     * Операции разбиваются на несколько категорий:
     * - **Арифметические операции**: Add, Subtract, Multiply, Divide, Modulo, IntDivide, Power.
     * - **Операции сравнения**: Equal, NotEqual, Greater, GreaterEqual, Less, LessEqual.
     * Эти операции могут быть применены к числовым значениям, а также могут включать
     * дополнительную проверку на деление на ноль или другие ошибки выполнения.
     */
    enum class Operation {
        Add,
        Subtract,
        Multiply,
        Power,
        Divide,
        Modulo,
        IntDivide,
        And,
        Or,
        Is
    };

    /**
     * @brief Разбирает строковое представление операции и сопоставляет его с соответствующим значением перечисления `Operation`.
     *
     * Этот метод принимает строку, представляющую математический оператор или оператор сравнения, и,
     * используя предопределенное отображение, преобразует её в соответствующее значение перечисления `Operation`.
     * Если предоставленная строка не соответствует ни одному известному оператору, выбрасывается исключение.
     *
     * @param op Строковое представление оператора для разбора (например, "+", "-", "**").
     * @return Соответствующее значение перечисления `Operation` для указанного оператора.
     * @throws std::runtime_error Если входная строка не соответствует поддерживаемой операции.
     */
    static Operation parseOperation(const QString &op) {
        static const std::unordered_map<QString, Operation> opMap = {
            {"+", Operation::Add},
            {"-", Operation::Subtract},
            {"*", Operation::Multiply},
            {"**", Operation::Power},
            {"/", Operation::Divide},
            {"%", Operation::Modulo},
            {"//", Operation::IntDivide},
            {"and", Operation::And},
            {"or", Operation::Or},
            {"is", Operation::Is}
        };

        const auto it = opMap.find(op);
        if (it == opMap.end()) {
            throw std::runtime_error("Unsupported operation: " + op.toStdString());
        }
        return it->second;
    }
};

class LogicalOpNode : public ASTNode {
    QString op;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;

public:

    LogicalOpNode(std::shared_ptr<ASTNode> left, QString op,  std::shared_ptr<ASTNode> right) :
    op(std::move(op)), left(std::move(left)), right(std::move(right)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        Value l = left->eval(env);

        if (op == "and") {

            if (!l.toBool())
                return l;

            return right->eval(env);
        }

        if (op == "or") {

            if (l.toBool())
                return l;

            return right->eval(env);
        }

        throw std::runtime_error("Unknown logical operator");
    }

    [[nodiscard]] QString toString() const override {
        return left->toString() + " " + op + " " + right->toString();
    }
};

/**
 * @class VarNode
 * @brief Представляет узел ссылки на переменную в абстрактном синтаксическом дереве (AST) интерпретатора.
 *
 * VarNode используется для представления переменных путем хранения их имен и предоставления механизмов
 * для определения их значений во время вычисления. Этот класс является финальным и не может быть далее унаследован.
 *
 * @details
 * Класс VarNode хранит имя переменной и предоставляет реализации методов
 * `eval` и `toString`. Во время вычисления он определяет значение переменной
 * путем поиска её в предоставленной среде выполнения. Также обеспечивает
 * представление имени переменной в виде строки.
 */
class VarNode final : public ASTNode {
public:
    explicit VarNode(QString name) : name(std::move(name)) {}

    QString name;
    [[nodiscard]] QString toString() const override { return name; }
    [[nodiscard]] Value eval(const EnvPtr env) const override { return env.get()->get(name); }
};

/**
 * @class AssignNode
 * @brief Представляет операцию присваивания в абстрактном синтаксическом дереве (AST) интерпретатора языка программирования.
 *
 * Класс AssignNode отвечает за присваивание значения переменной. Он вычисляет выражение справа от знака присваивания и
 * присваивает полученное значение переменной в переданной среде выполнения.
 *
 * @details
 * AssignNode — это конкретная реализация класса ASTNode, предназначенная для операций присваивания переменным.
 * При вычислении он вычисляет значение дочернего узла выражения, обновляет окружение, связывая имя переменной
 * с этим значением, и затем возвращает это значение. Строковое представление AssignNode записывается в стандартной
 * форме присваивания, объединяя имя переменной и выражение в виде "varName = valueExpr".
 */
class AssignNode final : public ASTNode {
public:
    AssignNode(QString varName, std::shared_ptr<ASTNode> valueExpr) :
    varName(std::move(varName)), valueExpr(std::move(valueExpr)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        Value val = valueExpr->eval(env);
        env.get()->set(varName, val);
        return val;
    }

    [[nodiscard]] QString toString() const override {
        return varName + " = " + valueExpr->toString();
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }

private:
    QString varName;
    std::shared_ptr<ASTNode> valueExpr;
};

/**
 * @class IfNode
 * @brief Представляет конструкцию условного ветвления в абстрактном синтаксическом дереве (AST).
 *
 * Класс IfNode моделирует оператор `if`, включая необязательные блоки `elif` и `else`.
 * Он оценивает условие и выполняет соответствующий блок, если условие истинно.
 * В случае множества ветвей `elif` они проверяются по порядку, пока одно из условий не окажется истинным.
 * Если ни одно из условий не выполнено, в случае наличия выполняется блок `else`.
 *
 * @details
 * Этот класс инкапсулирует логику и структуру оператора `if`. Он содержит:
 * - Узел `condition` с основным условием `if`.
 * - `body` — набор операторов для выполнения, если условие `if` истинно.
 * - Необязательную последовательность условий `elif` и соответствующих им блоков.
 * - Необязательный `elseBody` для выполнения, если ни одно из условий `if` или `elif` не оказалось истинным.
 *
 * Метод `eval` выполняет соответствующий блок, оценивая условия, и возвращает результат последнего выполненного оператора.
 * Метод `toString` восстанавливает текстовое представление структуры оператора `if`.
 */
class IfNode final : public ASTNode {
public:
    IfNode(std::shared_ptr<ASTNode> condition,
        std::vector<std::shared_ptr<ASTNode>> body,
        std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs,
        std::vector<std::shared_ptr<ASTNode>> elseBody)
    : condition(std::move(std::move(condition))), body(std::move(body)), elifs(std::move(elifs)), elseBody(std::move(elseBody)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        if (condition->eval(env).toBool()) {

            Value lastValue;

            for (const auto& stmt : body) {
                lastValue = Interpreter::executeNode(stmt, env);
            }

            return lastValue;
        }

        for (const auto& elif: elifs) {

            if (elif.first->eval(env).toBool()) {

                Value lastValue;

                for (const auto& stmt : elif.second) {
                    lastValue = Interpreter::executeNode(stmt, env);
                }

                return lastValue;
            }
        }

        if (!elseBody.empty()) {

            Value lastValue;

            for (const auto& stmt : elseBody) {
                lastValue = Interpreter::executeNode(stmt, env);
            }

            return lastValue;
        }

        return {};
    }

    [[nodiscard]] QString toString() const override {

        QString result = "if " + condition->toString() + ":\n";

        for (const auto& stmt : body) {
            result += "    " + stmt->toString() + "\n";
        }

        for (const auto&[fst, snd] : elifs) {

            result += "elif " + fst->toString() + ":\n";

            for (const auto& stmt : snd) {
                result += "    " + stmt->toString() + "\n";
            }
        }

        if (!elseBody.empty()) {

            result += "else:\n";

            for (const auto& stmt : elseBody) {
                result += "    " + stmt->toString() + "\n";
            }
        }

        return result;
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }

private:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs;
    std::vector<std::shared_ptr<ASTNode>> elseBody;
};

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
class BreakException final : public std::exception {
public:
    explicit BreakException(std::string  message = "") : message(std::move(message)) {}

    [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

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
class ContinueException final : public std::exception {
public:
    explicit ContinueException(std::string message = "") : message(std::move(message)) {}

    [[nodiscard]] const char* what() const noexcept override { return message.c_str(); }

private:
    std::string message;
};

/**
 * @class BreakNode
 * @brief Представляет узел инструкции `break` в абстрактном синтаксическом дереве (AST).
 *
 * BreakNode — специализированный тип узла в AST, обозначающий выполнение
 * инструкции `break`, которая обычно используется для выхода из цикла или других
 * конструкций управления потоком. При вычислении он выбрасывает исключение BreakException,
 * чтобы сигнализировать о прерывании нормального потока выполнения.
 *
 * @details
 * Этот класс предназначен для появления в AST в тех местах исходного кода, где встречается
 * инструкция `break`. Его поведение при вычислении состоит исключительно в выбрасывании исключения,
 * а строковое представление — это слово "break". Этот узел нельзя наследовать или расширять,
 * так как он объявлен как final.
 */
class BreakNode final : public ASTNode {
public:
    [[nodiscard]] Value eval(EnvPtr env) const override { throw BreakException(); }

    [[nodiscard]] QString toString() const override { return "break"; }
};

/**
 * @class ContinueNode
 * @brief Представляет оператор "continue" в абстрактном синтаксическом дереве (AST) интерпретатора.
 *
 * ContinueNode — это специализированный тип ASTNode, соответствующий ключевому слову "continue" в исходном коде.
 * При вычислении он выбрасывает исключение ContinueException, чтобы сигнализировать о событии управления потоком,
 * пропускающем оставшиеся инструкции в текущей итерации цикла.
 *
 * @details
 * Этот узел используется для реализации поведения оператора "continue" в языках программирования.
 * Метод `eval` генерирует исключение ContinueException, которое перехватывается на соответствующем уровне для изменения
 * потока управления. Метод `toString` возвращает текстовое представление узла — строку "continue".
 */
class ContinueNode final : public ASTNode {
    [[nodiscard]] Value eval(EnvPtr env) const override { throw ContinueException(); }

    [[nodiscard]] QString toString() const override { return "continue"; }
};

/**
 * @class WhileNode
 * @brief Представляет узел цикла "while" в абстрактном синтаксическом дереве (AST).
 *
 * WhileNode моделирует цикл "while" с опциональными блоками "else" и отвечает
 * за выполнение тела цикла и опционального блока else, если условие цикла становится ложным.
 *
 * @details
 * WhileNode многократно вычисляет условие, пока оно истинно.
 * На каждой итерации выполняются инструкции из тела цикла. Особая обработка предусмотрена
 * для исключений `break` и `continue` внутри цикла:
 * - `ContinueException` приводит к пропуску оставшихся инструкций текущей итерации.
 * - `BreakException` немедленно завершает цикл.
 *
 * Если цикл завершается без прерывания с помощью "break", выполняется опциональный блок "else" (если он есть).
 * Возвращаемое значение метода `eval` — результат последнего выполненного выражения в теле цикла или теле else.
 */
class WhileNode final : public ASTNode {
public:
    WhileNode(std::shared_ptr<ASTNode> condition,
        std::vector<std::shared_ptr<ASTNode>> body,
        std::vector<std::shared_ptr<ASTNode>> elseBody)
            : condition(std::move(condition))
            , body(std::move(body))
            , elseBody(std::move(elseBody)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {

        Value last;
        bool broken = false;

        while (condition->eval(env).toBool()) {

            try {
                for (auto& stmt : body) {
                    last = Interpreter::executeNode(stmt, env);
                }
            }
            catch ([[maybe_unused]] const ContinueException& e) {}
            catch ([[maybe_unused]] const BreakException& e) {
                broken = true;
                break;
            }
        }

        if (!broken) {
            for (auto& stmt : elseBody) {
                last = Interpreter::executeNode(stmt, env);
            }
        }
        return last;
    }

    [[nodiscard]] QString toString() const override {

        QString out = "while " + condition->toString() + ":\n";

        for (auto& stmt : body) {
            out += "\t" + stmt->toString() + "\n";
        }

        if (!elseBody.empty()) {

            out += "else:\n";

            for (auto& stmt : elseBody) {
                out += "\t" + stmt->toString() + "\n";
            }
        }

        return out;
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }

private:
    std::shared_ptr<ASTNode> condition;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::vector<std::shared_ptr<ASTNode>> elseBody;
};

/**
 * @class CompareNode
 * @brief Реализует узел для операций сравнения в абстрактном синтаксическом дереве (AST).
 *
 * CompareNode представляет операцию сравнения между левым операндом и одним или несколькими
 * правыми операндами с использованием операторов, таких как `<`, `<=`, `>`, `>=`, `==` или `!=`.
 * Он последовательно вычисляет сравнения и гарантирует, что все условия выполняются для сложных выражений.
 *
 * @details
 * Этот класс используется для вычисления выражений сравнения и получения логического результата. Он поддерживает
 * как строковые сравнения, так и числовые сравнения, в зависимости от типов вычисляемых операндов.
 * Если во время вычисления обнаруживается несоответствие (например, условие оператора не выполняется), вычисление
 * останавливается и возвращает `false`. В противном случае возвращается `true`, если все сравнения успешны. Логическая
 * цепочка сравнений позволяет естественным образом обрабатывать составные выражения (например, `a < b < c`).
 *
 * Метод `toString` создаёт текстовое представление выражения сравнения в формате AST.
 *
 * Класс не может быть далее унаследован и реализует поведение для узлов сравнения в AST.
 */
class CompareNode final : public ASTNode {
    std::shared_ptr<ASTNode> left;
    std::vector<QString> ops;
    std::vector<std::shared_ptr<ASTNode>> rights;

    enum class Operation {
        Equal,
        NotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        In,
        NotIn
    };

    static Operation parseOperation(const QString &op) {
        static const std::unordered_map<QString, Operation> opMap = {
            {"==", Operation::Equal},
            {"!=", Operation::NotEqual},
            {"<", Operation::Less},
            {"<=", Operation::LessOrEqual},
            {">", Operation::Greater},
            {">=", Operation::GreaterOrEqual},
            {"in", Operation::In},
            {"not in", Operation::NotIn}
        };

        const auto it = opMap.find(op);
        if (it == opMap.end()) {
            throw std::runtime_error("Unsupported operation: " + op.toStdString());
        }
        return it->second;
    }

    static bool compare(const Value& a,
                        const Value& b,
                        const Operation op) {

        switch (op) {
            case Operation::Equal:          return a == b;
            case Operation::NotEqual:       return a != b;
            case Operation::Less:           return a < b;
            case Operation::LessOrEqual:    return a <= b;
            case Operation::Greater:        return a > b;
            case Operation::GreaterOrEqual: return a >= b;
            case Operation::In:             return b.contains(a);
            case Operation::NotIn:          return !b.contains(a);

            default: throw std::runtime_error("Invalid comparison operation");
        }
    }

public:

    CompareNode(std::shared_ptr<ASTNode> lhs,
                std::vector<QString> operators,
                std::vector<std::shared_ptr<ASTNode>> rgs)
      : left(std::move(lhs)), ops(std::move(operators)), rights(std::move(rgs)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {

        Value a = left->eval(env);

        for (size_t i = 0; i < ops.size(); ++i) {

            Value b = rights[i]->eval(env);

            if (!compare(a, b, parseOperation(ops[i]))) {
                return Value(false);
            }

            a = std::move(b);
        }

        return Value(true);
    }

    [[nodiscard]] QString toString() const override {
        QString out = "(" + left->toString() + ")";
        for (size_t i = 0; i < ops.size(); ++i) {
            out += " " + ops[i] + " " + rights[i]->toString();
        }
        return out;
    }
};

class ReturnException final : public std::exception {
public:
    explicit ReturnException(Value val) : value(std::move(val)) {}

    [[nodiscard]] const char* what() const noexcept override { return "ReturnException"; }

    [[nodiscard]] Value getValue() const { return value; }

private:
    Value value;
};

class FunctionDefNode final : public ASTNode {
public:
    QString name;
    std::vector<Param> params;
    std::vector<std::shared_ptr<ASTNode>> body;
    std::vector<std::shared_ptr<ASTNode>> decorators;

    FunctionDefNode(QString name,
                    std::vector<Param> params,
                    std::vector<std::shared_ptr<ASTNode>> body,
                    std::vector<std::shared_ptr<ASTNode>> decorators = {})
    : name(std::move(name)), params(std::move(params)), body(std::move(body)), decorators(std::move(decorators)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        const auto func = std::make_shared<FunctionValue>(params, body, env, name);

        Value v(func);

        // применяем декораторы снизу вверх
        for (auto it = decorators.rbegin(); it != decorators.rend(); ++it) {
            Value decorator = (*it)->eval(env);
            v = call(decorator, { v }, {}, env);
        }

        env->set(name, v);

        return v;
    }

    [[nodiscard]] QString toString() const override {
        QString out = "def " + name + "(";

        for (size_t i = 0; i < params.size(); ++i) {
            out += params[i].name;

            if (!params[i].type.isEmpty()) {
                out += ": " + params[i].type;
            }

            if (i + 1 < params.size())
                out += ", ";
        }
        out += "):\n";

        for (auto& stmt : body) {
            out += "\t" + stmt->toString() + "\n";
        }

        return out;
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

struct KeywordArg {
    QString name;
    std::shared_ptr<ASTNode> value;
};

struct ParsedCallArgs {
    std::vector<std::shared_ptr<ASTNode>> positional;
    std::vector<KeywordArg> keyword;
};

class CallNode final : public ASTNode {
public:
    std::shared_ptr<ASTNode> callee;
    std::vector<std::shared_ptr<ASTNode>> args;
    std::vector<KeywordArg> kwargs;

    CallNode(std::shared_ptr<ASTNode> callee,
        std::vector<std::shared_ptr<ASTNode>> args,
        std::vector<KeywordArg> kwargs)
        : callee(std::move(callee)),
        args(std::move(args)),
        kwargs(std::move(kwargs)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        const Value calleeVal = callee->eval(env);

        // positional
        std::vector<Value> evaluatedArgs;
        evaluatedArgs.reserve(args.size());

        for (const auto& arg : args)
            evaluatedArgs.push_back(arg->eval(env));

        // keyword
        std::vector<std::pair<QString, Value>> evaluatedKwargs;
        evaluatedKwargs.reserve(kwargs.size());

        for (const auto&[name, value] : kwargs) {

            evaluatedKwargs.emplace_back(
                name,
                value->eval(env)
            );
        }

        return call(calleeVal, evaluatedArgs, evaluatedKwargs, env);
    }

    [[nodiscard]] QString toString() const override {
        return callee->toString() + "(...)";
    }
};

class ReturnNode final : public ASTNode {
public:
    explicit ReturnNode(std::shared_ptr<ASTNode> expr) : expr(std::move(expr)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        const Value val = expr ? expr->eval(env) : Value();
        throw ReturnException(val);
    }

    [[nodiscard]] QString toString() const override {
        return "return ...";
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }

private:
    std::shared_ptr<ASTNode> expr;
};

class PassNode : public ASTNode {
public:
    [[nodiscard]] Value eval(std::shared_ptr<Environment> env) const override {
        return {}; // None
    }

    [[nodiscard]] QString toString() const override {
        return "pass";
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

class GlobalNode : public ASTNode {
public:
    std::vector<QString> names;

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        for (const auto& name : names) {
            env->globalVars.insert(name);
        }
        return {};
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

class NonlocalNode : public ASTNode {
public:
    std::vector<QString> names;

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        for (const auto& name : names) {
            env->nonlocalVars.insert(name);
        }
        return {};
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

class ClassDefNode final : public ASTNode {
public:
    QString name;
    std::vector<std::shared_ptr<ASTNode>> baseExprs;
    QVector<std::shared_ptr<ASTNode>> body;
    std::vector<std::shared_ptr<ASTNode>> decorators;

    ClassDefNode(QString name,
                 std::vector<std::shared_ptr<ASTNode>> bases,
                 QVector<std::shared_ptr<ASTNode>> body,
                 std::vector<std::shared_ptr<ASTNode>> decorators = {})
        : name(std::move(name)),
        baseExprs(std::move(bases)),
        body(std::move(body)),
        decorators(std::move(decorators)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {
        std::vector<Value::ClassPtr> bases;

        for (auto& baseExpr : baseExprs) {
            Value baseVal = baseExpr->eval(env);

            if (!std::holds_alternative<Value::ClassPtr>(baseVal.data)) {
                throw std::runtime_error("Base must be a class");
            }

            bases.push_back(std::get<Value::ClassPtr>(baseVal.data));
        }

        // наследование по умолчанию
        if (bases.empty()) {
            bases.push_back(Runtime::objectClass);
        }

        const auto cls = std::make_shared<ClassValue>(name);
        cls->bases = bases;

        // создаём временное окружение
        const auto classEnv = std::make_shared<Environment>(env);

        // выполняем тело класса
        for (const auto& stmt : body) {
            [[maybe_unused]] const auto _ = stmt->eval(classEnv);
        }

        // копируем всё из classEnv в attributes
        for (auto it = classEnv->variables.cbegin();
             it != classEnv->variables.cend();
             ++it) {

            const QString& key = it.key();
            const Value& val = it.value();

            Value newVal = val;

            // обычная функция
            if (std::holds_alternative<Value::FunctionPtr>(newVal.data)) {

                auto func = std::get<Value::FunctionPtr>(newVal.data);
                func->ownerClass = cls;
            }

            // staticmethod
            else if (std::holds_alternative<Value::StaticMethodPtr>(newVal.data)) {

                auto sm = std::get<Value::StaticMethodPtr>(newVal.data);
                sm->func->ownerClass = cls;
            }

            // classmethod
            else if (std::holds_alternative<Value::ClassMethodPtr>(newVal.data)) {

                auto cm = std::get<Value::ClassMethodPtr>(newVal.data);
                cm->func->ownerClass = cls;
            }

            cls->attributes.insert(key, val);
        }

        Value classValue(cls);

        // применяем декораторы снизу вверх
        for (auto it = decorators.rbegin(); it != decorators.rend(); ++it) {
            Value decorator = (*it)->eval(env);

            classValue = call(decorator, { classValue }, {}, env);
        }

        env->set(name, classValue);

        return classValue;
    }

    [[nodiscard]] QString toString() const override { return "class " + name; }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

class AttributeAccessNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    QString attr;

    AttributeAccessNode(std::shared_ptr<ASTNode> object, QString attr)
        : object(std::move(object)), attr(std::move(attr)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        const Value objVal = object->eval(env);

        if (std::holds_alternative<Value::SuperPtr>(objVal.data)) {
            const auto super = std::get<Value::SuperPtr>(objVal.data);
            return getAttrFromSuper(super, attr);
        }

        return getAttrValue(objVal, attr);
    }

    [[nodiscard]] QString toString() const override {
        return object->toString() + "." + attr;
    }
};

class AttributeAssignNode final : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    QString attr;
    std::shared_ptr<ASTNode> valueExpr;

    AttributeAssignNode(std::shared_ptr<ASTNode> object,
                        QString attr,
                        std::shared_ptr<ASTNode> valueExpr)
        : object(std::move(object)),
          attr(std::move(attr)),
          valueExpr(std::move(valueExpr)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        Value objVal = object->eval(env);
        Value val = valueExpr->eval(env);

        setAttrValue(objVal, attr, val);

        return val;
    }

    [[nodiscard]] QString toString() const override {
        return object->toString() + "." + attr + " = " + valueExpr->toString();
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

class StarredNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> value;

    explicit StarredNode(std::shared_ptr<ASTNode> value)
        : value(std::move(value)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {
        return value->eval(env);
    }

    [[nodiscard]] QString toString() const override {
        return "*" + value->toString();
    }
};

class ListNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> elements;

    explicit ListNode(std::vector<std::shared_ptr<ASTNode>> elems)
        : elements(std::move(elems)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {

        std::vector<Value> values;

        for (const auto &el: elements) {

            if (const auto starred = dynamic_cast<StarredNode *>(el.get())) {

                Value iterable = starred->value->eval(env);

                const auto iter = iterable.getIterator();

                while (iter->hasNext()) {
                    values.push_back(iter->next());
                }
            }
            else {
                values.push_back(el->eval(env));
            }
        }

        return Value(std::make_shared<ListValue>(std::move(values)));
    }

    [[nodiscard]] QString toString() const override {
        QStringList parts;

        for (const auto& el : elements) {
            parts << el->toString();
        }

        return "[" + parts.join(", ") + "]";
    }
};

class IndexNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    std::shared_ptr<ASTNode> index;

    IndexNode(std::shared_ptr<ASTNode> object,
              std::shared_ptr<ASTNode> index)
        : object(std::move(object)),
          index(std::move(index)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        Value obj = object->eval(env);
        Value idx = index->eval(env);

        Value getter = genericGetAttr(obj, "__getitem__");

        return call(getter, {idx}, {}, env);
    }

    [[nodiscard]] QString toString() const override {
        return object->toString() + "[" + index->toString() + "]";
    }
};

class IndexAssignNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> object;
    std::shared_ptr<ASTNode> index;
    std::shared_ptr<ASTNode> value;

    IndexAssignNode(std::shared_ptr<ASTNode> object,
                    std::shared_ptr<ASTNode> index,
                    std::shared_ptr<ASTNode> value)
        : object(std::move(object)),
          index(std::move(index)),
          value(std::move(value)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        Value obj = object->eval(env);
        Value idx = index->eval(env);
        Value val = value->eval(env);

        Value setitem = getAttrValue(obj, "__setitem__");

        call(setitem, {idx, val}, {}, env);

        return val;
    }

    [[nodiscard]] QString toString() const override {
        return object->toString() + "[" + index->toString() + "] = " + value->toString();
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

class LambdaNode final : public ASTNode {
public:

    std::vector<Param> params;
    std::shared_ptr<ASTNode> body;

    LambdaNode(std::vector<Param> params,
        std::shared_ptr<ASTNode> body)
        : params(std::move(params)),
          body(std::move(body)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        std::vector<std::shared_ptr<ASTNode>> functionBody;

        functionBody.push_back(std::make_shared<ReturnNode>(body));

        const auto fn = std::make_shared<FunctionValue>(params, functionBody, env, "<lambda>");

        return Value(fn);
    }

    [[nodiscard]] QString toString() const override {
        return "<lambda>";
    }

    [[nodiscard]] bool shouldPrint() const override {
        return false;
    }

};

class DictElementNode : public ASTNode {
public:

    virtual void apply(
        const std::shared_ptr<DictValue>& dict,
        std::shared_ptr<Environment> env) const = 0;
};

class DictPairNode : public DictElementNode {
public:

    std::shared_ptr<ASTNode> key;
    std::shared_ptr<ASTNode> value;

    DictPairNode(std::shared_ptr<ASTNode> key,
                std::shared_ptr<ASTNode> value)
        : key(std::move(key)),
          value(std::move(value)) {}

    void apply(const std::shared_ptr<DictValue>& dict, std::shared_ptr<Environment> env) const override {
        dict->setItem(key->eval(env), value->eval(env));
    }

    [[nodiscard]] Value eval(EnvPtr) const override {
        throw std::runtime_error("DictKeyValueNode cannot be evaluated directly");
    }

    [[nodiscard]] QString toString() const override {
        return key->toString() + ": " + value->toString();
    }
};

class DictUnpackNode : public DictElementNode {
public:
    std::shared_ptr<ASTNode> value;

    explicit DictUnpackNode(std::shared_ptr<ASTNode> value)
        : value(std::move(value)) {}

    void apply(const std::shared_ptr<DictValue>& dict, std::shared_ptr<Environment> env) const override {

        const auto other = value->eval(env).asDict();

        for (const auto& key : other->getOrder()) {

            dict->setItem(key, other->getElements()[key]);
        }
    }

    [[nodiscard]] Value eval(EnvPtr env) const override {
        throw std::runtime_error("DictUnpackNode cannot be evaluated directly");
    }

    [[nodiscard]] QString toString() const override {
        return "**" + value->toString();
    }
};

class DictNode final : public ASTNode {

    std::vector<std::shared_ptr<DictElementNode>> items;

public:

    explicit DictNode(std::vector<std::shared_ptr<DictElementNode>> items)
        : items(std::move(items)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        const auto dict = std::make_shared<DictValue>();

        for (const auto& item : items) {
            item->apply(dict, env);
        }

        return Value(dict);
    }

    [[nodiscard]] QString toString() const override {

        QStringList parts;

        for (const auto& item : items) {
            parts << item->toString();
        }

        return "{" + parts.join(", ") + "}";
    }
};

class TupleNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> elements;

    explicit TupleNode(std::vector<std::shared_ptr<ASTNode>> elements)
        : elements(std::move(elements)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        std::vector<Value> values;

        for (const auto& element : elements) {

            if (const auto starred = dynamic_cast<StarredNode *>(element.get())) {

                Value iterable = starred->value->eval(env);

                const auto iter = iterable.getIterator();

                while (iter->hasNext()) {
                    values.push_back(iter->next());
                }
            }
            else {
                values.push_back(element->eval(env));
            }
        }

        return Value(std::make_shared<TupleValue>(std::move(values)));
    }

    [[nodiscard]] QString toString() const override {
        QStringList parts;

        for (const auto& element : elements) {
            parts << element->toString();
        }
        return "(" + parts.join(", ") + ")";
    }
};

class ForNode : public ASTNode {
public:

    QString varName;

    std::shared_ptr<ASTNode> iterable;

    std::vector<std::shared_ptr<ASTNode>> body;

    ForNode(QString varName,
            std::shared_ptr<ASTNode> iterable,
            std::vector<std::shared_ptr<ASTNode>> body)
        : varName(std::move(varName)),
          iterable(std::move(iterable)),
          body(std::move(body)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        Value iterableValue = iterable->eval(env);

        Value iterMethod = getAttrValue(iterableValue, "__iter__");

        Value iterator = call(iterMethod, {}, {}, env);

        Value last;

        while (true) {

            try {

                Value nextMethod = getAttrValue(iterator, "__next__");
                Value value = call(nextMethod, {}, {}, env);

                env->set(varName, value);

                try {

                    for (const auto& stmt : body) {
                        last = Interpreter::executeNode(stmt, env);
                    }

                }

                catch ([[maybe_unused]] const ContinueException& e) {}
                catch ([[maybe_unused]] const BreakException& e) {
                    break;
                }

            }
            catch (const StopIterationException&) {
                break;
            }
        }

        return last;
    }

    [[nodiscard]] QString toString() const override {
        return "for " + varName + " in " + iterable->toString() + ": ...";
    }

    [[nodiscard]] bool shouldPrint() const override {
        return false;
    }
};

class SetNode : public ASTNode {
public:
    std::vector<std::shared_ptr<ASTNode>> elements;

    explicit SetNode(std::vector<std::shared_ptr<ASTNode>> elements)
    : elements(std::move(elements)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {
        const auto set = std::make_shared<SetValue>();

        for (const auto& element : elements) {

            if (const auto starred = dynamic_cast<StarredNode *>(element.get())) {

                Value iterable = starred->value->eval(env);

                const auto iter = iterable.getIterator();

                while (iter->hasNext()) {

                    Value next = iter->next();

                    if (!set->elements.contains(next)) {

                        set->elements[next] = true;
                        set->order.push_back(next);
                    }
                }
            }
            else {

                Value value = element->eval(env);

                if (!set->elements.contains(value)) {

                    set->elements[value] = true;
                    set->order.push_back(value);
                }
            }
        }

        return Value(set);
    }

    [[nodiscard]] QString toString() const override {
        QStringList parts;

        for (const auto& element : elements) {
            parts << element->toString();
        }
        return "{" + parts.join(", ") + "}";
    }
};

class SliceNode : public ASTNode {
public:

    std::shared_ptr<ASTNode> start;
    std::shared_ptr<ASTNode> stop;
    std::shared_ptr<ASTNode> step;

    SliceNode(
        std::shared_ptr<ASTNode> start,
        std::shared_ptr<ASTNode> stop,
        std::shared_ptr<ASTNode> step)
        : start(std::move(start)),
          stop(std::move(stop)),
          step(std::move(step))
    {}


    [[nodiscard]] Value eval(const EnvPtr env) const override {

        std::optional<Value> startValue;
        std::optional<Value> stopValue;
        std::optional<Value> stepValue;

        if (start) {
            startValue = start->eval(env);
        }

        if (stop) {
            stopValue = stop->eval(env);
        }

        if (step) {
            stepValue = step->eval(env);
        }

        return Value(
            std::make_shared<SliceValue>(
                startValue,
                stopValue,
                stepValue
            )
        );

    }

    [[nodiscard]] QString toString() const override {

        QString result = "slice(";

        if (start) {
            result += start->toString();
        }
        else {
            result += "None";
        }

        result += ", ";

        if (stop) {
            result += stop->toString();
        }
        else {
            result += "None";
        }

        result += ", ";

        if (step) {
            result += step->toString();
        }
        else {
            result += "None";
        }

        result += ")";

        return result;

    }
};

//TODO: пока поддерживается только удаление содержимого в объектах
// чуть позже добавить поддержку:
// 1) del variable
// 2) del obj.attr
// 3) del a, b, c
class DeleteNode : public ASTNode {

public:

    std::shared_ptr<ASTNode> target;

    explicit DeleteNode(std::shared_ptr<ASTNode> target)
    : target(std::move(target)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {

        auto indexNode =
            std::dynamic_pointer_cast<IndexNode>(target);

        if (!indexNode) {

            throw std::runtime_error(
                "SyntaxError: invalid del target"
            );
        }

        Value obj = indexNode->object->eval(env);

        Value idx = indexNode->index->eval(env);

        Value delItem = getAttrValue(obj, "__delitem__");

        call(delItem, {idx}, {}, env);

        return {};
    }

    [[nodiscard]] QString toString() const override {
        return "del " + target->toString();
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

class AugAssignNode : public ASTNode {

    enum class Operation {
        Add,
        Subtract,
        Multiply,
        Divide,
        IntDivide,
        Modulo,
        Power
    };

    QString name;
    QString op;
    std::shared_ptr<ASTNode> value;

    static Operation parseOperation(const QString& op) {

        static const std::unordered_map<QString, Operation> opMap = {
            {"+=",  Operation::Add},
            {"-=",  Operation::Subtract},
            {"*=",  Operation::Multiply},
            {"/=",  Operation::Divide},
            {"//=", Operation::IntDivide},
            {"%=",  Operation::Modulo},
            {"**=", Operation::Power}
        };

        const auto it = opMap.find(op);

        if (it == opMap.end()) {
            throw std::runtime_error(
                "Unsupported augmented assignment: "
                + op.toStdString()
            );
        }

        return it->second;
    }

    static Value tryInplaceOperation(
    const Value& left,
    const QString& methodName,
    const Value& right,
    const EnvPtr& env,
    const std::function<Value()>& fallback) {

        try {

            const Value method = getAttrValue(left, methodName);

            return call(method, { right }, {}, env);

        } catch (const std::runtime_error& e) {

            if (const std::string msg = e.what();
                msg.find("AttributeError") == std::string::npos) {
                throw;
            }
        }

        return fallback();
    }

public:

    AugAssignNode(QString name, QString op, std::shared_ptr<ASTNode> value)
        : name(std::move(name)), op(std::move(op)), value(std::move(value)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {
        Value left = env->get(name);
        Value right = value->eval(env);

        Value result;

        switch (parseOperation(op)) {
            case Operation::Add:
                result = tryInplaceOperation(
                    left,
                    "__iadd__",
                    right,
                    env,
                    [&] { return left + right; }
                );
                break;

            case Operation::Subtract:
                result = tryInplaceOperation(
                    left,
                    "__isub__",
                    right,
                    env,
                    [&] { return left - right; }
                );
                break;

            case Operation::Multiply:
                result = tryInplaceOperation(
                    left,
                    "__imul__",
                    right,
                    env,
                    [&] { return left * right; }
                );
                break;

            case Operation::Divide:
                result = tryInplaceOperation(
                    left,
                    "__itruediv__",
                    right,
                    env,
                    [&] { return left / right; }
                );
                break;

            case Operation::IntDivide:
                result = tryInplaceOperation(
                    left,
                    "__ifloordiv__",
                    right,
                    env,
                    [&] { return left.intDivide(right); }
                );
                break;

            case Operation::Modulo:
                result = tryInplaceOperation(
                    left,
                    "__imod__",
                    right,
                    env,
                    [&] { return left % right; }
                );
                break;

            case Operation::Power:
                result = tryInplaceOperation(
                    left,
                    "__ipow__",
                    right,
                    env,
                    [&] { return left.power(right); }
                );
                break;

            default:
                throw std::runtime_error(
                    "Unsupported augmented assignment"
                );
        }

        env->set(name, result);

        return result;
    }

    [[nodiscard]] QString toString() const override {
        return name + " " + op + " " + value->toString();
    }

    [[nodiscard]] bool shouldPrint() const override { return false; }
};

/**
 * @class Parser
 * @brief Выполняет разбор последовательности токенов в абстрактное синтаксическое дерево (AST).
 *
 * Parser организует разбор выражений на основании грамматики с учетом приоритетов операций.
 * Класс принимает поток токенов на вход и предоставляет соответствующий AST в качестве результата.
 * Используется в интерпретаторах, компиляторах и других системах анализа кода.
 *
 * @details
 * Основной метод `parse` возвращает корневой узел AST, представляющего собой анализируемое выражение.
 * Парсинг происходит в несколько этапов, начиная с самых низкоприоритетных операций до высокоприоритетных.
 * Класс включает методы, которые обеспечивают парсинг конкретных конструкций, таких как выражения со скобками,
 * операции унарного минуса, математические операции, сравнения и присваивания.
 * Если входные токены содержат синтаксические ошибки, генерируются исключения.
 */
class Parser {
public:
    explicit Parser(const QVector<Token> &tokens);

    std::shared_ptr<ASTNode> parse(); //Главный метод

private:
    //Здесь методы разделены для анализа выражения согласно приоритету
    /**
     * @brief Разбирает операции присваивания (=)
     * @return Узел присваивания или выражение более высокого приоритета
     */
    std::shared_ptr<ASTNode> parseExpression();

    std::shared_ptr<ASTNode> parseStarredExpression();

    std::shared_ptr<ASTNode> parseDoubleStarredExpression();

    /**
     * @brief Разбирает операции сравнения (==, !=, <, <=, >, >=)
     * @return Узел сравнения или выражение более высокого приоритета
     */
    std::shared_ptr<ASTNode> parseComparison();

    /**
     * @brief Разбирает операции сложения и вычитания (+, -)
     * @return Узел арифметической операции или выражение более высокого приоритета
     */
    std::shared_ptr<ASTNode> parseAdditionAndSubtraction();
    /**
    * @brief Разбирает операции умножения, деления и остатка (*, /, //, %)
    * @return Узел арифметической операции или выражение более высокого приоритета
    */
    std::shared_ptr<ASTNode> parseTerm();

    /**
     * @brief Разбирает операцию возведения в степень (**)
     * @return Узел возведения в степень или первичное выражение
     */
    std::shared_ptr<ASTNode> parsePower();

    /**
     * @brief Разбирает первичные выражения (числа, строки, переменные, выражения в скобках)
     * @return Узел первичного выражения
     */
    std::shared_ptr<ASTNode> parsePrimary();

    /**
     * @brief Разбирает числовой токен
     * @return Узел числового значения
     */
    std::shared_ptr<ASTNode> parseNumberToken();

    /**
     * @brief Разбирает строковый токен
     * @return Узел строкового значения
     */
    std::shared_ptr<ASTNode> parseStringToken();

    std::shared_ptr<ASTNode> parseBytesToken();

    /**
     * @brief Разбирает логический токен
     * @return Узел логического значения
     */
    std::shared_ptr<ASTNode> parseBoolToken();

    std::shared_ptr<ASTNode> parseNoneToken();

    /**
     * @brief Разбирает токен идентификатора
     * @return Узел переменной
     */
    std::shared_ptr<ASTNode> parseIdentifierToken();

    /**
     * @brief Разбирает выражение в скобках
     * @return Узел выражения внутри скобок
     */
    std::shared_ptr<ASTNode> parseParenthesizedExpression();

    /**
     * @brief Возвращает текущий токен без продвижения
     * @return Текущий токен
     */
    [[nodiscard]] Token peek() const;

    /**
    * @brief Возвращает текущий токен и переходит к следующему
    * @return Текущий токен
    */
    Token advance();

    /**
     * @brief Выбрасывает ошибку о неожиданном токене
     * @param token Неожиданный токен
     */
    static void throwUnexpectedTokenError(const Token &token);

    /**
     * @brief Разбирает конструкцию условного оператора (`if`) и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий конструкцию условного оператора.
     * Возвращенный узел содержит информацию о главном условии, теле, а также необязательных блоках `elif` и `else`.
     */
    std::shared_ptr<ASTNode> parseIfStatement();

    /**
     * @brief Разбирает блок кода и возвращает список узлов AST, представляющих инструкции внутри блока.
     * @return Список узлов AST, представляющих проанализированные инструкции внутри блока кода.
     */
    std::vector<std::shared_ptr<ASTNode>> parseBlock();

    /**
     * @brief Разбирает конструкцию цикла `while` и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий конструкцию цикла `while`.
     */
    std::shared_ptr<ASTNode> parseWhileStatement();

    /**
     * @brief Разбирает инструкцию `break` и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий инструкцию `break`.
     */
    std::shared_ptr<ASTNode> parseBreakStatement();

    /**
     * @brief Разбирает инструкцию `continue` и возвращает соответствующий узел AST.
     * @return Узел AST, представляющий инструкцию `continue`.
     */
    std::shared_ptr<ASTNode> parseContinueStatement();

    std::shared_ptr<ASTNode> parseFunctionDef(const std::vector<std::shared_ptr<ASTNode>>& decorators = {});

    std::shared_ptr<ASTNode> parseReturn();

    std::shared_ptr<ASTNode> parsePass();

    std::shared_ptr<ASTNode> parseGlobalStatement();

    std::shared_ptr<ASTNode> parseNonlocalStatement();

    std::shared_ptr<ASTNode> parseClassDef(const std::vector<std::shared_ptr<ASTNode>>& decorators = {});

    std::shared_ptr<ASTNode> parsePostfix(std::shared_ptr<ASTNode>);

    std::shared_ptr<ASTNode> parseDecorated();

    std::shared_ptr<ASTNode> parseList();

    std::shared_ptr<ASTNode> parseLambda();

    ParsedCallArgs parseCallArguments();

    std::shared_ptr<ASTNode> parseDict();

    std::shared_ptr<ASTNode> parseSet();

    bool isDictLiteral();

    std::shared_ptr<ASTNode> parseForStatement();

    std::shared_ptr<ASTNode> parseDictOrSet();

    std::shared_ptr<ASTNode> parseIndexOrSlice();

    QString consume(TokenType type, const QString &value);

    [[nodiscard]] bool match(TokenType type, const QString& value) const;

    bool matchAndAdvance(TokenType type, const QString& value);

    [[nodiscard]] bool matchAny(TokenType type, const std::vector<QString>& values) const;

    bool matchAnyAndAdvance(TokenType type, const std::vector<QString>& values);

    [[nodiscard]] bool isComparisonOperator() const;

    QString parseComparisonOperator();

    std::shared_ptr<ASTNode> parseUnary();

    std::shared_ptr<ASTNode> parseNot();

    std::shared_ptr<ASTNode> parseAnd();

    std::shared_ptr<ASTNode> parseOr();

    std::shared_ptr<ASTNode> parseDelStatement();

    QVector<Token> tokens;
    int current = 0;
};
#endif //PARSER_H
