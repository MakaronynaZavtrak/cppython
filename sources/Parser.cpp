#include "Parser.h"

/**
 * @brief Конструирует объект Parser с заданным вектором токенов.
 *
 * Этот конструктор инициализирует экземпляр Parser, принимая владение переданным
 * QVector<Token> для последующих операций парсинга.
 *
 * @param tokens QVector объектов Token, представляющий лексические токены,
 *               которые будут анализироваться парсером.
 */
Parser::Parser(const QVector<Token>& tokens) : tokens(tokens) {}

/**
 * @brief Разбирает входные токены и формирует абстрактное синтаксическое дерево (AST).
 *
 * Этот метод служит основным входом для процесса парсинга. Он координирует
 * разбор, вызывая соответствующие вспомогательные методы для анализа последовательности токенов
 * и построения соответствующего представления AST.
 *
 * @return Умный указатель на корневой ASTNode созданного абстрактного синтаксического дерева.
 *         Возвращает nullptr, если разбор завершился неудачно или не удалось построить синтаксическое дерево.
 */
std::shared_ptr<ASTNode> Parser::parse() {
    if (peek().type == TOKEN_KEYWORD) {
        if (peek().value == "if")       return parseIfStatement();
        if (peek().value == "while")    return parseWhileStatement();
        if (peek().value == "break")    return parseBreakStatement();
        if (peek().value == "continue") return parseContinueStatement();
    }
    return parseAssignment();
}

/**
 * Разбирает выражение присваивания.
 *
 * Метод обрабатывает выражения с операцией присваивания "=". Если текущий
 * токен соответствует операции присваивания, производится разбор левой
 * и правой части оператора. Левая часть должна быть валидной переменной
 * (VarNode), иначе выбрасывается исключение. Для корректного выражения
 * создается узел AST типа AssignNode, который объединяет имя переменной
 * и выражение значения.
 *
 * @return Умный указатель на узел AST, представляющий разобранное выражение
 *         присваивания или выражение более высокого приоритета (например,
 *         сравнения), если присваивание отсутствует. Исключение
 *         std::runtime_error выбрасывается при неверной структуре присваивания
 *         (например, если левая часть не является переменной).
 */
std::shared_ptr<ASTNode> Parser::parseAssignment() {
    std::shared_ptr<ASTNode> left = parseComparison();
    if (peek().type == TOKEN_OP && peek().value == "=") {
        advance();
        std::shared_ptr<ASTNode> right = parseAssignment();
        const std::shared_ptr<VarNode> var = std::dynamic_pointer_cast<VarNode>(left);
        if (!var) throw std::runtime_error("Invalid assignment target");
        return std::make_shared<AssignNode>(var->name, right);
    }
    return left;
}

/**
 * Разбирает выражения с операциями сравнения (==, !=, <, <=, >, >=).
 *
 * Метод обрабатывает токены, соответствующие операциям сравнения,
 * через последовательный разбор выражений сложения и вычитания. Для каждой
 * операции создается бинарный узел AST (BinOpNode), объединяющий левый
 * и правый операнды. Построение дерева выражений выполняется последовательно
 * слева направо.
 *
 * @return Умный указатель на узел AST, представляющий разобранное выражение
 *         для операций сравнения. Исключения могут быть выброшены в случае
 *         синтаксических ошибок.
 */
std::shared_ptr<ASTNode> Parser::parseComparison() {
    std::shared_ptr<ASTNode> left = parseAdditionAndSubtraction();
    std::vector<QString> compOps;
    std::vector<std::shared_ptr<ASTNode>> compRights;

    while (peek().type == TOKEN_OP &&
           (peek().value == "==" || peek().value == "!=" ||
            peek().value == "<" || peek().value == "<=" ||
            peek().value == ">" || peek().value == ">=")) {
        QString op = advance().value;
        compOps.push_back(op);
        compRights.push_back(parseAdditionAndSubtraction());
    }

    if (compOps.empty()) {
        return left;
    }

    return std::make_shared<CompareNode> (
        left,
        std::move(compOps),
        std::move(compRights)
    );
}

/**
 * Разбирает выражения с операциями сложения (+) и вычитания (-).
 *
 * Метод обрабатывает токены, соответствующие операциям сложения и вычитания,
 * используя результаты разбора термов. Для каждой найденной операции создается
 * бинарный узел AST (BinOpNode), который объединяет левый и правый операнды.
 * После этого дерево выражений строится последовательно слева направо.
 *
 * @return Умный указатель на узел AST, представляющий разобранное выражение
 *         для операций сложения и вычитания. Исключения могут быть выброшены
 *         в случае ошибок синтаксического анализа.
 */
std::shared_ptr<ASTNode> Parser::parseAdditionAndSubtraction() {
    std::shared_ptr<ASTNode> left = parseTerm();
    while (peek().type == TOKEN_OP && (peek().value == "+" || peek().value == "-")) {
        QString op = advance().value;
        std::shared_ptr<ASTNode> right = parseTerm();
        left = std::make_shared<BinOpNode>(left, op, right);
    }
    return left;
}

/**
 * @brief Разбирает терм в последовательности токенов и строит соответствующий узел AST.
 *
 * Этот метод разбирает терм, определённый как последовательность выражений-степеней,
 * объединённых операторами умножения (*), деления (/), целочисленного деления (//) и остатка от деления (%).
 * Для каждого встреченного оператора формируется узел бинарной операции, что позволяет построить
 * левосторонне ассоциативное представление выражения.
 *
 * @return Умный указатель на корневой узел AST, представляющий разобранный терм.
 *         Если терм не может быть разобран, поведение не определено.
 */
std::shared_ptr<ASTNode> Parser::parseTerm() {
    std::shared_ptr<ASTNode> left = parseUnaryMinus();
    while (peek().type == TOKEN_OP &&
        (peek().value == "*" || peek().value == "/" || peek().value == "//" || peek().value == "%")) {
        QString op = advance().value;
        std::shared_ptr<ASTNode> right = parseUnaryMinus();
        left = std::make_shared<BinOpNode>(left, op, right);
    }
    return left;
}

/**
 * Разбирает выражение с унарным минусом из потока токенов.
 *
 * Метод обрабатывает унарный оператор "-" с соответствующим приоритетом.
 * Если текущий токен является унарным минусом ("-"), создается узел AST,
 * представляющий выражение с унарным минусом, где операндом выступает
 * результат рекурсивного вызова `parseUnaryMinus`. Для представления
 * выражения создается бинарный узел (BinOpNode), в котором левым операндом
 * является значение 0, а правым — значение вычисляемого выражения.
 * Если унарный минус не обнаружен, осуществляется переход к следующему
 * уровню приоритетов (в данном случае к обработке возведения в степень).
 *
 * @return Умный указатель на узел AST, представляющий выражение с унарным минусом,
 *         либо узел, возвращаемый методом `parsePower` для выражений без унарного минуса.
 *         Исключения могут быть выброшены, если возникает ошибка синтаксического анализа.
 */
std::shared_ptr<ASTNode> Parser::parseUnaryMinus() {
    if (peek().type == TOKEN_OP && peek().value == "-") {
        advance();
        std::shared_ptr<ASTNode> rhs = parseUnaryMinus();
        auto zero = std::make_shared<ValueNode>(Value(0));
        return std::make_shared<BinOpNode>(zero, "-", rhs);
    }
    return parsePower();
}

/**
 * @brief Разбирает выражение возведения в степень в потоке входных токенов.
 *
 * Этот метод обрабатывает выражения с оператором возведения в степень ("**"),
 * учитывая соответствующие правила ассоциативности и приоритетов. Рекурсивно строит
 * узлы абстрактного синтаксического дерева (AST), представляющие такие операции. Если
 * оператор "**" не встречается, метод передаёт управление разбору
 * отдельных факторов.
 *
 * @return Умный указатель на корневой узел разобранного выражения, который
 *         может быть либо отдельным фактором, либо узлом бинарной операции
 *         возведения в степень.
 */
std::shared_ptr<ASTNode> Parser::parsePower()
{
    std::shared_ptr<ASTNode> left = parsePrimary();
    if (peek().type == TOKEN_OP && peek().value == "**") {
        QString op = advance().value;
        std::shared_ptr<ASTNode> right = parseUnaryMinus();
        left = std::make_shared<BinOpNode>(left, op, right);
    }
    return left;
}

/**
 * Разбирает следующее первичное выражение из потока токенов.
 * Первичные выражения включают литералы (числа, строки, логические значения),
 * идентификаторы переменных, сгруппированные выражения (заключенные в скобки)
 * или маркер конца файла.
 *
 * Метод анализирует тип текущего токена и определяет
 * соответствующий узел Абстрактного Синтаксического Дерева (AST) для создания.
 * Также обрабатывает группировку в скобках для вложенных выражений и преобразует
 * токены в соответствующие узлы значений или узлы переменных в зависимости от их типа.
 *
 * @return Умный указатель на результирующий узел AST, представляющий
 *         разобранное первичное выражение. Возвращает nullptr, если тип токена
 *         TOKEN_EOF.
 *         Выбрасывает std::runtime_error при некорректном синтаксисе
 *         или при обнаружении неожиданного токена.
 */
std::shared_ptr<ASTNode> Parser::parsePrimary() {
    switch (const Token token = peek(); token.type) {
        case TOKEN_NUMBER:
            return parseNumberToken();
        case TOKEN_STRING:
            return parseStringToken();
        case TOKEN_BOOL:
            return parseBoolToken();
        case TOKEN_ID:
            return parseIdentifierToken();
        case TOKEN_OP:
            if (token.value == "(") {
                return parseParenthesizedExpression();
            }
            break;
        case TOKEN_EOF:
            return nullptr;
        default:
            throwUnexpectedTokenError(token);
    }
    return nullptr;
}

/**
 * Парсит токен числа и преобразует его в узел AST, представляющий числовое значение.
 *
 * Метод анализирует текущий токен, ожидая, что это числовое значение. Если в значении
 * токена есть одна десятичная точка, оно интерпретируется как число с плавающей точкой.
 * Если точек нет, значение интерпретируется как целое число. В случае некорректного формата
 * выбрасывается исключение.
 *
 * @return Умный указатель на узел AST, представляющий числовое значение.
 *         Узел может содержать либо целое число, либо число с плавающей точкой.
 * @throws std::runtime_error В случае некорректного формата числа.
 */
std::shared_ptr<ASTNode> Parser::parseNumberToken() {
    switch (const Token token = advance(); token.value.count('.')) {
        case 0:
            return std::make_shared<ValueNode>(Value(token.value.toInt()));
        case 1:
            return std::make_shared<ValueNode>(Value(token.value.toDouble()));
        default:
            throw std::runtime_error("Invalid number format");
    }
}

/**
 * Разбирает токен строки и создает узел синтаксического дерева (ASTNode), представляющий строковое значение.
 *
 * Метод извлекает текущий токен с помощью advance() и создает узел ValueNode,
 * содержащий строковое значение токена.
 *
 * @return Узел AST (ValueNode), представляющий строковое значение, извлеченное из токена.
 */
std::shared_ptr<ASTNode> Parser::parseStringToken() { return std::make_shared<ValueNode>(Value(advance().value)); }

/**
 * Парсит логический токен в узел синтаксического дерева.
 *
 * Метод создаёт и возвращает узел, представляющий значение логического
 * выражения. Ожидается, что текущий токен будет иметь тип TOKEN_BOOL и значение
 * "True" или "False". Логическое значение передаётся в конструктор узла как значение,
 * где "True" преобразуется в true, а остальные значения рассматриваются как false.
 * После обработки текущий токен продвигается.
 *
 * @return Умный указатель на созданный узел ASTNode, представляющий логическое значение.
 *         Если токен невалиден, поведение не определено.
 */
std::shared_ptr<ASTNode> Parser::parseBoolToken() { return std::make_shared<ValueNode>(Value(advance().value == "True")); }

/**
 * Парсит токен идентификатора и создает узел абстрактного синтаксического дерева (AST) для переменной.
 *
 * Метод интерпретирует текущий токен как идентификатор переменной, создает соответствующий объект
 * VarNode и перемещает указатель чтения на следующий токен.
 *
 * @return Умный указатель на вновь созданный узел VarNode, представляющий переменную.
 */
std::shared_ptr<ASTNode> Parser::parseIdentifierToken() { return std::make_shared<VarNode>(advance().value); }

/**
 * Разбирает выражение, заключенное в круглые скобки, и возвращает узел AST,
 * представляющий это выражение.
 *
 * Метод ожидает открывающую круглую скобку, затем выполняет разбор выражения
 * вплоть до операции присваивания, проверяет наличие закрывающей скобки и
 * завершает разбор, если все условия выполнены. Если закрывающая скобка
 * отсутствует, выбрасывается исключение.
 *
 * @return Указатель на узел AST, представляющий разобранное выражение внутри
 *         круглых скобок.
 * @throws std::runtime_error Если ожидаемая закрывающая скобка ')' не найдена.
 */
std::shared_ptr<ASTNode> Parser::parseParenthesizedExpression() {
    advance();
    std::shared_ptr<ASTNode> expr = parseAssignment();

    if (peek().type == TOKEN_OP && peek().value == ")") {
        advance();
        return expr;
    }
    throw std::runtime_error("Expected ')'");
}

/**
 * Генерирует исключение, если обнаружен неожиданный токен во время синтаксического анализа.
 *
 * Этот метод вызывается в случае, если текущий токен не соответствует ожидаемым
 * в контексте синтаксического анализа. Исключение содержит информацию о неожиданном токене,
 * включая его значение, что облегчает диагностику и исправление ошибок в синтаксисе.
 *
 * @param token Токен, который оказался неожиданным в текущем контексте.
 */
void Parser::throwUnexpectedTokenError(const Token &token) { throw std::runtime_error("Unexpected token: \"" + token.value.toStdString() + "\""); }

/**
     * @brief Разбирает конструкцию условного оператора (`if`) и возвращает соответствующий узел AST.
     *
     * @details
     * Метод анализирует конструкцию условного оператора, включая его обязательные и необязательные части:
     *  - `if` условие и тело.
     *  - Необязательные блоки `elif` с условиями и телами.
     *  - Необязательный блок `else` с телом.
     *
     * После ключевого слова `if` или `elif` ожидается логическое выражение,
     * за которым обязательно должен следовать оператор `:`. После оператора `:` парсится соответствующий блок кода.
     * Аналогичным образом, после ключевого слова `else` также требуется оператор `:` и блок кода.
     * В случае, если токены не соответствуют ожидаемому синтаксису, выбрасывается исключение.
     *
     * @return Узел AST, представляющий конструкцию условного оператора.
     *         Возвращенный узел содержит информацию о главном условии, теле, а также необязательных блоках `elif` и `else`.
     *
     * @throws std::runtime_error Если структура не соответствует ожидаемому синтаксису
     *                            (например, отсутствует `:` после `if`, `elif` или `else`).
     */
std::shared_ptr<ASTNode> Parser::parseIfStatement() {
    advance();

    auto condition = parseAssignment();

    if (peek().type != TOKEN_OP || peek().value != ":") {
        throw std::runtime_error("Expected ':' after if condition");
    }
    advance();

    auto body = parseBlock();

    std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs;
    while (peek().type == TOKEN_KEYWORD && peek().value == "elif") {
        advance();
        auto elifCondition = parseAssignment();

        if (peek().type != TOKEN_OP || peek().value != ":")
            throw std::runtime_error("Expected ':' after elif condition");
        advance();

        auto elifBody = parseBlock();
        elifs.emplace_back(elifCondition, elifBody);
    }

    std::vector<std::shared_ptr<ASTNode>> elseBody;
    if (peek().type == TOKEN_KEYWORD && peek().value == "else") {
        advance();
        if (peek().type != TOKEN_OP || peek().value != ":")
            throw std::runtime_error("Expected ':' after else");
        advance();
        elseBody = parseBlock();
    }

    return std::make_shared<IfNode>(condition, body, elifs, elseBody);
}

/**
     * @brief Разбирает блок кода и возвращает список узлов AST, представляющих инструкции внутри блока.
     *
     * @details
     * Метод обрабатывает отступы и структуры на основе токенов, представляющих начало
     * и конец блока кода. Считывает строки, содержащие инструкции, до завершения блока.
     * Ожидается отступ перед началом блока и соответствующее "разотступление" после его конца.
     * Блоки кода обычно используются в управляющих конструкциях, таких как `if`, `while` и функциях.
     *
     * @return Список узлов AST, представляющих проанализированные инструкции внутри блока кода.
     * В случае синтаксической ошибки (например, отсутствия отступов или их несогласованности)
     * выбрасывается исключение `std::runtime_error`.
     */
std::vector<std::shared_ptr<ASTNode>> Parser::parseBlock() {
    if (peek().type != TOKEN_NEWLINE)
        throw std::runtime_error("Expected newline after statement");
    advance();

    if (peek().type != TOKEN_INDENT)
        throw std::runtime_error("Expected indent after statement");
    advance();

    std::vector<std::shared_ptr<ASTNode>> statements;
    while (peek().type != TOKEN_DEDENT && peek().type != TOKEN_EOF) {
        statements.push_back(parse());
        if (peek().type == TOKEN_NEWLINE)
            advance();
    }

    if (peek().type == TOKEN_DEDENT) {
        advance();
    } else {
        throw std::runtime_error("Expected dedent after block");
    }

    return statements;
}

/**
 * @brief Разбирает конструкцию цикла `while` и возвращает соответствующий узел AST.
 *
 * Этот метод анализирует токены, представляющие оператор `while`, условие цикла,
 * тело цикла, а также необязательный блок `else`. При обнаружении синтаксических
 * ошибок выбрасывается исключение.
 *
 * @return Узел AST, представляющий конструкцию цикла `while`, включая состояние,
 * тело цикла и необязательный блок `else` (если он присутствует).
 */
std::shared_ptr<ASTNode> Parser::parseWhileStatement() {
    advance();
    auto condition = parseAssignment();

    if (peek().type != TOKEN_OP || peek().value != ":")
        throw std::runtime_error("Expected ':' after while-condition");

    advance();
    auto body = parseBlock();
    std::vector<std::shared_ptr<ASTNode>> elseBody;

    if (peek().type == TOKEN_KEYWORD && peek().value == "else") {
        advance();
        if (peek().type != TOKEN_OP || peek().value != ":")
            throw std::runtime_error("Expected ':' after else");
        advance();
        elseBody = parseBlock();
    }

    return std::make_shared<WhileNode>(condition, body, elseBody);
}

/**
     * @brief Разбирает инструкцию `break` и возвращает соответствующий узел AST.
     *
     * Метод анализирует текущий токен, проверяя соответствие ключевому слову `break`,
     * продвигает текущую позицию и создает узел AST, представляющий инструкцию `break`.
     * Такой узел используется для указания на необходимость выхода из текущего цикла
     * или блока кода.
     *
     * @return Узел AST, представляющий инструкцию `break`.
     */
std::shared_ptr<ASTNode> Parser::parseBreakStatement() {
    advance();
    return std::make_shared<BreakNode>();
}

/**
 * @brief Разбирает инструкцию `continue` и возвращает соответствующий узел AST.
 *
 * Этот метод анализирует токен, представляющий инструкцию `continue`,
 * продвигает парсер к следующему токену, и создает узел AST для инструкции `continue`.
 *
 * @return Узел AST, представляющий инструкцию `continue`.
 */
std::shared_ptr<ASTNode> Parser::parseContinueStatement() {
    advance();
    return std::make_shared<ContinueNode>();
}


/**
 * @brief Получает текущий токен в потоке токенов, не сдвигая позицию.
 *
 * Этот метод возвращает токен на текущей позиции в процессе разбора.
 * Если достигнут конец потока токенов, возвращается специальный токен конца файла (EOF).
 *
 * @return Текущий токен, если позиция в потоке допустима; иначе токен типа TOKEN_EOF.
 */
Token Parser::peek() const { return (current < tokens.size()) ? tokens[current] : Token(TOKEN_EOF, "", 0); }

/**
 * @brief Продвигает парсер к следующему токену и возвращает текущий токен.
 *
 * Этот метод перемещает указатель текущего токена вперёд, если доступны ещё токены.
 * Если указатель выходит за пределы имеющихся токенов, возвращается токен с типом TOKEN_EOF.
 *
 * @return Текущий Token до продвижения. Если достигнут конец потока токенов,
 *         возвращается токен TOKEN_EOF.
 */
Token Parser::advance() { return (current < tokens.size()) ? tokens[current++] : Token(TOKEN_EOF, "", 0); }