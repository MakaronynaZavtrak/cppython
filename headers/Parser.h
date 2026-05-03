#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Value.h"
#include "Environment.h"
#include <memory>
#include <cmath>
#include <utility>

#include "BoundMethod.h"
#include "CallRuntime.h"
#include "DescriptorUtils.h"
#include "InstanceValue.h"

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

    /**
     * @brief Возвращает строковое представление значения узла.
     *
     * Метод преобразует значение узла в строку на основе типа данных, содержащегося в `Value`.
     * Поддерживаемые типы данных включают `double`, `int`, `QString` и `bool`. Для типов данных,
     * которые не поддерживаются, возвращается строка "<Unknown type of value>".
     *
     * @return Строковое представление значения узла.
     */
    [[nodiscard]] QString toString() const override {
        const auto& data = value.data;

        if (std::holds_alternative<Value::BigFloat>(data)) {
            return QString::fromStdString(
                std::get<Value::BigFloat>(data).convert_to<std::string>()
                );
        }

        if (std::holds_alternative<Value::BigInt>(data)) {
            return QString::fromStdString(
                std::get<Value::BigInt>(data).convert_to<std::string>()
                );
        }

        if (std::holds_alternative<QString>(data)) {
            return "\'" + std::get<QString>(data) + "\'";
        }

        if (std::holds_alternative<bool>(data)) {
            return std::get<bool>(data) ? "True" : "False";
        }

        return "<Unknown type of value>";
    }

    [[nodiscard]] Value eval(EnvPtr env) const override {
        return value;
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
public:
    using Num = std::variant<Value::BigInt, Value::BigFloat>;

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

        if (
            (std::holds_alternative<Value::BigFloat>(l.data) || std::holds_alternative<Value::BigInt>(l.data))
            &&
            (std::holds_alternative<Value::BigFloat>(r.data) || std::holds_alternative<Value::BigInt>(r.data))
        ) { return evalTwoNumbers(l, r); }

        if (std::holds_alternative<QString>(l.data) && std::holds_alternative<QString>(r.data))
            { return evalTwoStrings(l, r); }

        if (
            (std::holds_alternative<QString>(l.data) && std::holds_alternative<Value::BigInt>(r.data))
            ||
            (std::holds_alternative<QString>(r.data) && std::holds_alternative<Value::BigInt>(l.data))
        ) { return evalNumAndString(l, r); }

        if (
            (std::holds_alternative<bool>(l.data) && (std::holds_alternative<Value::BigInt>(r.data) || std::holds_alternative<Value::BigFloat>(r.data)))
            ||
            (std::holds_alternative<bool>(r.data) && (std::holds_alternative<Value::BigInt>(l.data) || std::holds_alternative<Value::BigFloat>(l.data)))
        ) { return evalNumAndBool(l, r); }

        if (std::holds_alternative<bool>(l.data) && std::holds_alternative<bool>(r.data))
            { return evalTwoBools(l, r); }

        throw std::runtime_error("Unsupported operation: " + op.toStdString());
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
        Equal,
        NotEqual,
        Greater,
        GreaterEqual,
        Less,
        LessEqual
    };

    template<typename Op>
    static Value applyBinaryOp(const Value& l, const Value& r, const bool isFloat, Op operation) {
        return isFloat
        ? Value(operation(l.toBigFloat(), r.toBigFloat()))
        : Value(operation(l.toBigInt(), r.toBigInt()));
    }

    /**
     * @brief Вычисляет бинарную операцию между двумя числовыми значениями.
     *
     * Этот метод вычисляет результат бинарной операции на основе предоставленных левого и правого операндов.
     * Он поддерживает арифметические операции, такие как сложение, вычитание, умножение, деление,
     * а также операции сравнения, такие как равенство, неравенство и реляционные сравнения.
     * Выполняются дополнительные проверки для операций, которые могут привести к ошибкам времени выполнения,
     * таких как деление или взятие остатка от деления на ноль.
     *
     * @param l Левый операнд бинарной операции.
     * @param r Правый операнд бинарной операции.
     * @return Результат вычисленной операции, инкапсулированный как `Value`. Точная природа результата
     *         (числовой, логический и т.д.) зависит от выполненной операции.
     * @throws std::runtime_error Если операция не поддерживается или недопустима, или если происходит ошибка времени выполнения,
     *                           такая как деление на ноль.
     */
    [[nodiscard]] Value evalTwoNumbers(const Value &l, const Value &r) const {
        const bool isFloat =
            std::holds_alternative<Value::BigFloat>(l.data) ||
            std::holds_alternative<Value::BigFloat>(r.data);

        switch (parseOperation(op)) {
            case Operation::Add:      return applyBinaryOp(l, r, isFloat, std::plus<>());
            case Operation::Subtract: return applyBinaryOp(l, r, isFloat, std::minus<>());
            case Operation::Multiply: return applyBinaryOp(l, r, isFloat, std::multiplies<>());

            case Operation::Power: {
                if (isFloat) {
                    return Value(pow(l.toBigFloat(), r.toBigFloat()));
                }

                const auto base = l.toBigInt();
                const auto exp  = r.toBigInt();

                if (exp < 0) {
                    return Value(pow(l.toBigFloat(), r.toBigFloat()));
                }

                Value::BigInt result = 1;
                for (Value::BigInt i = 0; i < exp; ++i)
                    result *= base;

                return Value(result);
            }

            case Operation::Divide: {
                const auto lf = l.toBigFloat();
                const auto rf = r.toBigFloat();
                checkDivisionByZero(rf);
                return Value(lf / rf);
            }

            case Operation::Modulo: {
                auto rf = r.toBigFloat();
                checkDivisionByZero(rf);

                if (!isFloat) {
                    return Value(l.toBigInt() % r.toBigInt());
                }

                auto lf = l.toBigFloat();

                auto quotient = floor(lf / rf);
                auto remainder = lf - rf * quotient;

                return Value(remainder);
            }

            case Operation::IntDivide: {
                auto lf = l.toBigFloat();
                auto rf = r.toBigFloat();
                checkDivisionByZero(rf);

                auto result = floor(lf / rf);

                return isFloat
                ? Value(result)
                : Value(Value::BigInt(result));
            }

            case Operation::Equal:        return applyBinaryOp(l, r, isFloat, std::equal_to<>());
            case Operation::NotEqual:     return applyBinaryOp(l, r, isFloat, std::not_equal_to<>());
            case Operation::Greater:      return applyBinaryOp(l, r, isFloat, std::greater<>());
            case Operation::GreaterEqual: return applyBinaryOp(l, r, isFloat, std::greater_equal<>());
            case Operation::Less:         return applyBinaryOp(l, r, isFloat, std::less<>());
            case Operation::LessEqual:    return applyBinaryOp(l, r, isFloat, std::less_equal<>());
            default:                      throw std::runtime_error("Unsupported operation: " + op.toStdString() + " in evalTwoNumbers");
        }
    }

    /**
     * @brief Проверяет, делится ли число на ноль.
     *
     * Данный метод используется для проверки делителя на равенство нулю
     * с целью предотвращения ошибки деления на ноль, что может привести
     * к сбою выполнения программы.
     *
     * @param denominator Делитель, который проверяется на равенство нулю.
     * @throws std::invalid_argument Если делитель равен нулю.
     */
    static void checkDivisionByZero(const Value::BigFloat &denominator) {
        if (denominator == 0) {
            throw std::runtime_error("Division by zero");
        }
    }

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
            {"==", Operation::Equal},
            {"!=", Operation::NotEqual},
            {">", Operation::Greater},
            {">=", Operation::GreaterEqual},
            {"<", Operation::Less},
            {"<=", Operation::LessEqual}
        };

        const auto it = opMap.find(op);
        if (it == opMap.end()) {
            throw std::runtime_error("Unsupported operation: " + op.toStdString());
        }
        return it->second;
    }

    /**
     * @brief Вычисляет бинарную операцию над двумя строковыми значениями.
     *
     * Выполняет указанную операцию, такую как конкатенация или сравнение, между двумя строковыми значениями.
     * Если операция не поддерживается, выбрасывается исключение.
     *
     * @param l Левый операнд, представленный как `Value`, содержащий `QString`.
     * @param r Правый операнд, представленный как `Value`, содержащий `QString`.
     * @return `Value`, представляющий результат операции. Тип результата может варьироваться в зависимости от операции (например, конкатенация возвращает строку, сравнение возвращает логическое значение).
     * @throws std::runtime_error Если встречается неподдерживаемая операция.
     */
    [[nodiscard]] Value evalTwoStrings(const Value &l, const Value &r) const {
        const QString lv = std::get<QString>(l.data);
        const QString rv = std::get<QString>(r.data);

        switch (parseOperation(op)) {
            case Operation::Add:          return Value(lv + rv);
            case Operation::Equal:        return Value(lv == rv);
            case Operation::NotEqual:     return Value(lv != rv);
            case Operation::LessEqual:    return Value(lv.compare(rv) <= 0);
            case Operation::Less:         return Value(lv.compare(rv) < 0);
            case Operation::GreaterEqual: return Value(lv.compare(rv) >= 0);
            case Operation::Greater:      return Value(lv.compare(rv) > 0);
            default:                      throw std::runtime_error("Unsupported operation: " + op.toStdString() + "in evalTwoStrings");
        }
    }

    /**
     * @brief Вычисляет операцию между числовым и строковым значением.
     *
     * Этот метод выполняет операции между числовым значением (целым числом) и строковым значением
     * в соответствии с типом операции (`op`). В настоящее время поддерживается только оператор умножения (`*`),
     * который повторяет строку заданное количество раз.
     *
     * @param l Левый операнд, который может содержать целое число или строку.
     * @param r Правый операнд, который может содержать целое число или строку.
     * @return Объект Value, который содержит результат операции.
     *         Если `l` - целое число, а `r` - строка, результатом будет `r`, повторенная `l` раз.
     *         Если `l` - строка, а `r` - целое число, результатом будет `l`, повторенная `r` раз.
     * @throw std::runtime_error Если тип операции (`op`) не поддерживается.
     */
    [[nodiscard]] Value evalNumAndString(const Value &l, const Value &r) const {
        const bool swap = std::holds_alternative<Value::BigInt>(l.data) && std::holds_alternative<QString>(r.data);
        auto [numVal, strVal] = swap
        ? std::pair{ std::get<Value::BigInt>(l.data), std::get<QString>(r.data) }
        : std::pair{ std::get<Value::BigInt>(r.data), std::get<QString>(l.data) };

        switch (parseOperation(op)) {
            case Operation::Multiply: {
                if (numVal <= 0)
                    return Value("");

                if (numVal > std::numeric_limits<qsizetype>::max()) {
                    throw std::runtime_error("String repetition too large");
                }

                return Value(strVal.repeated(static_cast<qsizetype>(numVal)));
            }
            default: throw std::runtime_error("Unsupported operation: " + op.toStdString());
        }
    }

    [[nodiscard]] Value evalNumAndBool(const Value& l, const Value& r) const {
        return std::holds_alternative<bool>(l.data)
        ? evalTwoNumbers(Value(l.toBigInt()), r)
        : evalTwoNumbers(l, Value(r.toBigInt()));
    }

    [[nodiscard]] Value evalTwoBools(const Value& l, const Value& r) const {
        return evalTwoNumbers(Value(l.toBigInt()), Value(r.toBigInt()));
    }

    std::shared_ptr<ASTNode> left;
    QString op; // "+", "-", "=", "/", "%", "*", "**", "//", "=="
    std::shared_ptr<ASTNode> right;
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
                lastValue = stmt->eval(env);
            }
            return lastValue;
        }

        for (const auto& elif: elifs) {
            if (elif.first->eval(env).toBool()) {
                Value lastValue;
                for (const auto& stmt : elif.second) {
                    lastValue = stmt->eval(env);
                }
                return lastValue;
            }
        }

        if (!elseBody.empty()) {
            Value lastValue;
            for (const auto& stmt : elseBody) {
                lastValue = stmt->eval(env);
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
                    last = stmt->eval(env);
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
                last = stmt->eval(env);
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

public:
    CompareNode(std::shared_ptr<ASTNode> lhs,
                std::vector<QString> operators,
                std::vector<std::shared_ptr<ASTNode>> rgs)
      : left(std::move(lhs)), ops(std::move(operators)), rights(std::move(rgs)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        Value a = left->eval(env);
        for (size_t i = 0; i < ops.size(); ++i) {
            Value b = rights[i]->eval(env);
            bool res = false;

            if (std::holds_alternative<QString>(a.data) &&
                std::holds_alternative<QString>(b.data))
            {
                auto sa = a.toString();
                auto sb = b.toString();
                const int cmp = QString::compare(sa, sb, Qt::CaseSensitive);

                if      (ops[i] == "<" )  res = cmp <  0;
                else if (ops[i] == "<=")  res = cmp <= 0;
                else if (ops[i] == ">" )  res = cmp >  0;
                else if (ops[i] == ">=")  res = cmp >= 0;
                else if (ops[i] == "==")  res = cmp == 0;
                else if (ops[i] == "!=")  res = cmp != 0;
            }
            else {
                const auto da = a.toBigFloat();
                const auto db = b.toBigFloat();

                if      (ops[i] == "<" )  res = da <  db;
                else if (ops[i] == "<=")  res = da <= db;
                else if (ops[i] == ">" )  res = da >  db;
                else if (ops[i] == ">=")  res = da >= db;
                else if (ops[i] == "==")  res = da == db;
                else if (ops[i] == "!=")  res = da != db;
            }

            if (!res) return Value(false);
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

    FunctionDefNode(QString name,
                    std::vector<Param> params,
                    std::vector<std::shared_ptr<ASTNode>> body)
    : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        const auto func = std::make_shared<FunctionValue>();

        func->params = params;
        func->body = body;
        func->closure = env;
        func->name = name;


        Value v(func);
        env.get()->set(name, v);

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

class CallNode final : public ASTNode {
public:
    std::shared_ptr<ASTNode> callee;
    std::vector<std::shared_ptr<ASTNode>> args;

    CallNode(std::shared_ptr<ASTNode> callee, std::vector<std::shared_ptr<ASTNode>> args)
        : callee(std::move(callee)), args(std::move(args)) {}

    [[nodiscard]] Value eval(const EnvPtr env) const override {
        const Value calleeVal = callee->eval(env);

        std::vector<Value> evaluatedArgs;
        evaluatedArgs.reserve(args.size());

        for (auto& a : args)
            evaluatedArgs.push_back(a->eval(env));

        return call(calleeVal, evaluatedArgs, env);
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
    QVector<std::shared_ptr<ASTNode>> body;

    ClassDefNode(QString name,
                 QVector<std::shared_ptr<ASTNode>> body)
        : name(std::move(name)), body(std::move(body)) {}

    [[nodiscard]] Value eval(EnvPtr env) const override {
        const auto cls = std::make_shared<ClassValue>(name);

        // создаём временное окружение
        const auto classEnv = std::make_shared<Environment>(env);

        // выполняем тело класса
        for (const auto& stmt : body) {
            [[maybe_unused]] const auto _ = stmt->eval(classEnv);
        }

        // копируем всё из classEnv в attributes
        for (const auto& [key, val] : classEnv->variables) {

            if (Value newVal = val; std::holds_alternative<Value::FunctionPtr>(newVal.data)) {
                auto func = std::get<Value::FunctionPtr>(newVal.data);
                func->ownerClassName = cls.get()->name;
            }

            cls->attributes.insert(key, val);
        }

        env->set(name, Value(cls));

        return Value(cls);
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

    [[nodiscard]] Value eval(EnvPtr env) const override {
        Value objVal = object->eval(env);

        Value::InstancePtr instance = nullptr;
        Value::ClassPtr cls = nullptr;

        if (std::holds_alternative<Value::InstancePtr>(objVal.data)) {
            instance = std::get<Value::InstancePtr>(objVal.data);
            cls = instance->klass;
        }
        else if (std::holds_alternative<Value::ClassPtr>(objVal.data)) {
            cls = std::get<Value::ClassPtr>(objVal.data);
        }
        else {
            throw std::runtime_error("AttributeError: object has no attribute '" +
                attr.toStdString() + "'");
        }

        Value val;

        // 1. сначала ищем в полях объекта
        if (instance && instance->fields.contains(attr)) {
            return instance->fields[attr];
        }


        // 2. потом в классе
        if (cls && cls->attributes.contains(attr)) {
            val = cls->attributes[attr];
        } else {
            throw std::runtime_error("Attribute not found: " + attr.toStdString());
        }

        return applyDescriptor(val, instance, cls);
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

        if (!std::holds_alternative<Value::InstancePtr>(objVal.data)) {
            throw std::runtime_error("Attribute assignment on non-instance");
        }

        auto instance = std::get<Value::InstancePtr>(objVal.data);

        // ВАЖНО: записываем в fields объекта
        instance->fields[attr] = val;

        return val;
    }

    [[nodiscard]] QString toString() const override {
        return object->toString() + "." + attr + " = " + valueExpr->toString();
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
    std::shared_ptr<ASTNode> parseAssignment();

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
     * @brief Разбирает унарный минус (-)
     * @return Узел унарного минуса или выражение более высокого приоритета
     */
    std::shared_ptr<ASTNode> parseUnaryMinus();

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

    /**
     * @brief Разбирает логический токен
     * @return Узел логического значения
     */
    std::shared_ptr<ASTNode> parseBoolToken();

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

    std::shared_ptr<ASTNode> parseFunctionDef();

    std::shared_ptr<ASTNode> parseReturn();

    std::shared_ptr<ASTNode> parsePass();

    std::shared_ptr<ASTNode> parseGlobalStatement();

    std::shared_ptr<ASTNode> parseNonlocalStatement();

    std::shared_ptr<ASTNode> parseClassDef();

    std::shared_ptr<ASTNode> parsePostfix(std::shared_ptr<ASTNode>);

    QVector<Token> tokens;
    int current = 0;
};
#endif //PARSER_H
