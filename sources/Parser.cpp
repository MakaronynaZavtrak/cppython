#include "Parser.h"

#include "BytesValue.h"

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

        switch (peek().keyword.value()) {
            case Keyword::IF:       return parseIfStatement();
            case Keyword::WHILE:    return parseWhileStatement();
            case Keyword::BREAK:    return parseBreakStatement();
            case Keyword::CONTINUE: return parseContinueStatement();
            case Keyword::DEF:      return parseFunctionDef();
            case Keyword::RETURN:   return parseReturn();
            case Keyword::PASS:     return parsePass();
            case Keyword::CLASS:    return parseClassDef();
            case Keyword::LAMBDA:   return parseLambda();
            case Keyword::FOR:      return parseForStatement();
            case Keyword::DEL:      return parseDelStatement();
            default:                break;
        }
    }

    if (peek().type == TOKEN_AT) {
        return parseDecorated();
    }

    return parseExpression();
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
std::shared_ptr<ASTNode> Parser::parseExpression() {

    std::shared_ptr<ASTNode> left = parseOr();

    if (matchAny(
        TOKEN_OP,
        {
            "+=",
            "-=",
            "*=",
            "/=",
            "//=",
            "%=",
            "**=",
            "|=",
            "&=",
            "^="
        }
    )) {

        QString op = advance().value;

        auto right = parseOr();

        if (const auto var =
            std::dynamic_pointer_cast<VarNode>(left)) {

            return std::make_shared<AugAssignNode>(var->name, op, right);
        }

        throw std::runtime_error(
            "Invalid augmented assignment target"
        );
    }

    if (matchAndAdvance(TOKEN_OP, "=")) {

        auto right = parseOr();

        if (const auto var =
            std::dynamic_pointer_cast<VarNode>(left)) {

            return std::make_shared<AssignNode>(var->name, right);
        }

        if (const auto attr =
            std::dynamic_pointer_cast<AttributeAccessNode>(left)) {

            return std::make_shared<AttributeAssignNode>(
                attr->object,
                attr->attr,
                right
            );
        }

        if (const auto idx =
            std::dynamic_pointer_cast<IndexNode>(left)) {

            return std::make_shared<IndexAssignNode>(
                idx->object,
                idx->index,
                right
            );
        }

        throw std::runtime_error("Invalid assignment target");
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseStarredExpression() {

    if (matchAndAdvance(TOKEN_OP, "*")) {
        return std::make_shared<StarredNode>(parseExpression());
    }

    return parseExpression();
}

std::shared_ptr<ASTNode> Parser::parseDoubleStarredExpression() {

    if (matchAndAdvance(TOKEN_OP, "**")) {
        return std::make_shared<DictUnpackNode>(parseExpression());
    }

    return parseExpression();
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

    std::shared_ptr<ASTNode> left = parseBitOr();
    std::vector<QString> compOps;
    std::vector<std::shared_ptr<ASTNode>> compRights;

    while (isComparisonOperator()) {

        compOps.push_back(parseComparisonOperator());
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

    while (matchAny(TOKEN_OP, {"+", "-"})) {

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

    std::shared_ptr<ASTNode> left = parseUnary();

    while (matchAny(TOKEN_OP, {"*", "/", "//", "%"})) {

        QString op = advance().value;

        std::shared_ptr<ASTNode> right = parseUnary();

        left = std::make_shared<BinOpNode>(left, op, right);
    }

    return left;
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

    if (match(TOKEN_OP, "**")) {

        QString op = advance().value;

        std::shared_ptr<ASTNode> right = parseUnary();

        left = std::make_shared<BinOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseNoneToken() {

    advance();
    return std::make_shared<ValueNode>(Value());
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
    std::shared_ptr<ASTNode> node;

    switch (const Token token = peek(); token.type) {
        case TOKEN_NUMBER: node = parseNumberToken(); break;
        case TOKEN_STRING: node = parseStringToken(); break;
        case TOKEN_BYTES:  node = parseBytesToken(); break;
        case TOKEN_BOOL:   node = parseBoolToken(); break;
        case TOKEN_NONE:   node = parseNoneToken(); break;
        case TOKEN_ID:     node = parseIdentifierToken(); break;

        case TOKEN_KEYWORD:
            if (token.keyword.value() == Keyword::LAMBDA)
                return parseLambda();

        case TOKEN_OP:
            if (token.value == "(")
                node = parseParenthesizedExpression();
            else if (token.value == "[")
                node = parseList();
            else if (token.value == "{") {
                node = parseDictOrSet();
            }
            else
                throwUnexpectedTokenError(token);
            break;

        case TOKEN_EOF: return nullptr;
        default: throwUnexpectedTokenError(token);
    }

    return parsePostfix(node);
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
    const Token token = advance();

    QString normalized = token.value;
    normalized.remove('_');

    const std::string str = normalized.toStdString();

    try {
        if (normalized.contains('.') ||
            normalized.contains('e') ||
            normalized.contains('E')) {

            return std::make_shared<ValueNode>(
                Value(Value::BigFloat(str))
            );
        }
        else {
            return std::make_shared<ValueNode>(
                Value(Value::BigInt(str))
            );
        }
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Invalid number format: " + token.value.toStdString());
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
std::shared_ptr<ASTNode> Parser::parseStringToken() {

    return std::make_shared<ValueNode>(
        Value(advance().value)
    );
}

std::shared_ptr<ASTNode> Parser::parseBytesToken() {

    return std::make_shared<ValueNode>(
        Value(
            std::make_shared<BytesValue>(
                advance().value.toLatin1()
            )
        )
    );
}

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
std::shared_ptr<ASTNode> Parser::parseBoolToken() {
    return std::make_shared<ValueNode>(
        Value(
            advance().value == "True"
            )
    );
}

/**
 * Парсит токен идентификатора и создает узел абстрактного синтаксического дерева (AST) для переменной.
 *
 * Метод интерпретирует текущий токен как идентификатор переменной, создает соответствующий объект
 * VarNode и перемещает указатель чтения на следующий токен.
 *
 * @return Умный указатель на вновь созданный узел VarNode, представляющий переменную.
 */
std::shared_ptr<ASTNode> Parser::parseIdentifierToken() {
    QString name = advance().value;

    if (matchAndAdvance(TOKEN_OP, "(")) {

        auto parsedArgs = parseCallArguments();

        if (!match(TOKEN_OP, ")")) {

            while (true) {

                if (matchAndAdvance(TOKEN_OP, ",")) {
                    continue;
                }

                break;
            }
        }

        consume(TOKEN_OP, ")");

        return std::make_shared<CallNode>(
            std::make_shared<VarNode>(name),
            parsedArgs.positional,
            parsedArgs.keyword
        );
    }

    return std::make_shared<VarNode>(name);
}

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

    advance(); // (

    // ()
    if (matchAndAdvance(TOKEN_OP, ")")) {

        return std::make_shared<TupleNode>(
            std::vector<std::shared_ptr<ASTNode>>{}
        );
    }

    auto first = parseStarredExpression();

    // tuple?
    if (match(TOKEN_OP, ",")) {

        std::vector<std::shared_ptr<ASTNode>> elements;
        elements.push_back(first);

        while (matchAndAdvance(TOKEN_OP, ",")) {

            // trailing comma: (1,)
            if (match(TOKEN_OP, ")")) {
                break;
            }

            elements.push_back(parseStarredExpression());
        }

        consume(TOKEN_OP, ")");

        return std::make_shared<TupleNode>(
            std::move(elements)
        );
    }

    consume(TOKEN_OP, ")");

    return first;
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
void Parser::throwUnexpectedTokenError(const Token &token) {
    throw std::runtime_error("Unexpected token: \"" + token.value.toStdString() + "\"");
}

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

    auto condition = parseExpression();

    consume(TOKEN_OP, ":");

    auto body = parseBlock();

    std::vector<std::pair<std::shared_ptr<ASTNode>, std::vector<std::shared_ptr<ASTNode>>>> elifs;

    while (matchAndAdvance(TOKEN_KEYWORD, "elif")) {

        auto elifCondition = parseExpression();

        consume(TOKEN_OP, ":");

        auto elifBody = parseBlock();

        elifs.emplace_back(elifCondition, elifBody);
    }

    std::vector<std::shared_ptr<ASTNode>> elseBody;

    if (matchAndAdvance(TOKEN_KEYWORD, "else")) {

        consume(TOKEN_OP, ":");
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
    }
    else {
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

    auto condition = parseExpression();

    consume(TOKEN_OP, ":");

    auto body = parseBlock();

    std::vector<std::shared_ptr<ASTNode>> elseBody;

    if (matchAndAdvance(TOKEN_KEYWORD, "else")) {

        consume(TOKEN_OP, ":");

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

std::shared_ptr<ASTNode> Parser::parseFunctionDef(const std::vector<std::shared_ptr<ASTNode>>& decorators) {

    advance();

    if (peek().type != TOKEN_ID) {
        throw std::runtime_error("Expected function name");
    }

    QString name = advance().value;

    consume(TOKEN_OP, "(");

    std::vector<Param> params;

    if (!match(TOKEN_OP, ")")) {

        while (true) {

            if (peek().type != TOKEN_ID)
                throw std::runtime_error("Expected parameter name");

            Param param {advance().value, ""};

            if (matchAndAdvance(TOKEN_OP, ":")) {

                if (peek().type != TOKEN_ID)
                    throw std::runtime_error("Expected type after ':'");

                param.type = advance().value;
            }

            params.push_back(param);

            if (matchAndAdvance(TOKEN_OP, ",")) {
                continue;
            }

            break;
        }
    }

    matchAndAdvance(TOKEN_OP, ")");

    if (matchAndAdvance(TOKEN_OP, "->")) {

        if (peek().type != TOKEN_ID)
            throw std::runtime_error("Expected return type after '->'");

        advance();
    }

    consume(TOKEN_OP, ":");

    auto body = parseBlock();

    return std::make_shared<FunctionDefNode>(name, params, body, decorators);
}

std::shared_ptr<ASTNode> Parser::parseReturn() {
    advance();

    switch (peek().type) {
        case TOKEN_NEWLINE:
        case TOKEN_DEDENT:
        case TOKEN_EOF:
            return std::make_shared<ReturnNode>(nullptr);
        default:
            return std::make_shared<ReturnNode>(parseExpression());
    }
}

std::shared_ptr<ASTNode> Parser::parsePass() {

    advance();
    return std::make_shared<PassNode>();
}

std::shared_ptr<ASTNode> Parser::parseClassDef(const std::vector<std::shared_ptr<ASTNode>>& decorators) {

    advance(); // class

    if (peek().type != TOKEN_ID) {
        throw std::runtime_error("Expected class name");
    }

    QString name = advance().value;

    std::vector<std::shared_ptr<ASTNode>> bases;

    // проверяем, есть ли наследование
    if (matchAndAdvance(TOKEN_OP, "(")) {

        // если не пусто
        if (!match(TOKEN_OP, ")")) {

            while (true) {
                // парсим выражение базового класса
                bases.push_back(parseExpression());

                if (matchAndAdvance(TOKEN_OP, ",")) {
                    continue;
                }

                break;
            }
        }

        consume(TOKEN_OP, ")");
    }

    consume(TOKEN_OP, ":");

    const auto body = parseBlock();

    QVector<std::shared_ptr<ASTNode>> qBody;

    for (auto& stmt : body) {
        qBody.push_back(stmt);
    }

    return std::make_shared<ClassDefNode>(name, bases, qBody, decorators);
}

std::shared_ptr<ASTNode> Parser::parsePostfix(std::shared_ptr<ASTNode> node) {

    while (peek().type != TOKEN_EOF) {

        // a.b
        if (matchAndAdvance(TOKEN_OP, ".")) {

            if (peek().type != TOKEN_ID)
                throw std::runtime_error("Expected attribute name after '.'");

            QString attr = advance().value;
            node = std::make_shared<AttributeAccessNode>(node, attr);

            continue;
        }

        // вызов: obj(...)
        if (matchAndAdvance(TOKEN_OP, "(")) {

            auto parsedArgs = parseCallArguments();

            if (!match(TOKEN_OP, ")")) {

                while (true) {

                    if (matchAndAdvance(TOKEN_OP, ",")) {
                        continue;
                    }

                    break;
                }
            }

            consume(TOKEN_OP, ")");

            node = std::make_shared<CallNode>(node, parsedArgs.positional, parsedArgs.keyword);

            continue;
        }

        // obj[index]
        if (matchAndAdvance(TOKEN_OP, "[")) {

            auto index = parseIndexOrSlice();

            consume(TOKEN_OP, "]");

            node = std::make_shared<IndexNode>(node, index);

            continue;
        }

        break;
    }

    return node;
}

std::shared_ptr<ASTNode> Parser::parseDecorated() {

    std::vector<std::shared_ptr<ASTNode>> decorators;

    while (peek().type == TOKEN_AT) {

        advance(); // @

        decorators.push_back(parseExpression());

        if (peek().type == TOKEN_NEWLINE) {
            advance();
        }
    }

    if (peek().type != TOKEN_KEYWORD) {
        throw std::runtime_error("Expected def or class after decorator");
    }

    const auto kw = peek().keyword.value();

    if (kw == Keyword::DEF) {
        return parseFunctionDef(decorators);
    }

    if (kw == Keyword::CLASS) {
        return parseClassDef(decorators);
    }

    throw std::runtime_error("Decorator can only be applied to def/class");
}

std::shared_ptr<ASTNode> Parser::parseList() {

    std::vector<std::shared_ptr<ASTNode>> elements;
    advance(); // [

    // пустой список: []
    if (matchAndAdvance(TOKEN_OP, "]")) {
        return std::make_shared<ListNode>(std::move(elements));
    }

    while (true) {

        elements.push_back(parseStarredExpression());

        // конец списка, например [1, 2, 3]
        if  (matchAndAdvance(TOKEN_OP, "]")) {
            break;
        }

        consume(TOKEN_OP, ",");

        // запятая в конце ([1, 2,])
        if (matchAndAdvance(TOKEN_OP, "]")) {
            break;
        }
    }

    return std::make_shared<ListNode>(std::move(elements));
}

std::shared_ptr<ASTNode> Parser::parseLambda() {

    advance(); // lambda

    std::vector<Param> params;

    if (!match(TOKEN_OP, ":")) {

        while (true) {

            if (peek().type != TOKEN_ID) {
                throw std::runtime_error("Expected parameter name in lambda");
            }

            params.push_back(Param{advance().value, ""});

            if (matchAndAdvance(TOKEN_OP, ",")) {
                continue;
            }

            break;
        }
    }

    consume(TOKEN_OP, ":");

    auto expr = parseExpression();

    return std::make_shared<LambdaNode>(std::move(params), expr);
}

QString Parser::consume(const TokenType type, const QString& value) {

    if (peek().type != type || peek().value != value) {
        throw std::runtime_error(
            "Expected token: " + value.toStdString()
        );
    }

    return advance().value;
}

bool Parser::match(const TokenType type, const QString &value) const {

    return peek().type == type && peek().value == value;
}

bool Parser::matchAndAdvance(const TokenType type, const QString& value) {

    if (peek().type == type && peek().value == value) {

        advance();
        return true;
    }

    return false;
}

bool Parser::matchAny(const TokenType type, const std::vector<QString> &values) const {

    if (peek().type != type) {
        return false;
    }

    return std::any_of(
        values.begin(),
        values.end(),
        [&](const QString& value) { return peek().value == value; }
    );

}

bool Parser::matchAnyAndAdvance(const TokenType type, const std::vector<QString> &values) {

    if (matchAny(type, values)) {

        advance();
        return true;
    }

    return false;
}

bool Parser::isComparisonOperator() const {
    if (matchAny(
            TOKEN_OP,
            {"==","!=","<","<=",">",">="}))
        return true;

    if (peek().type == TOKEN_KEYWORD &&
        peek().keyword == Keyword::IN)
        return true;

    if (peek().type == TOKEN_KEYWORD &&
        peek().keyword == Keyword::NOT &&
        current + 1 < tokens.size() &&
        tokens[current + 1].type == TOKEN_KEYWORD &&
        tokens[current + 1].keyword == Keyword::IN)
        return true;

    return false;
}

QString Parser::parseComparisonOperator() {
    if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::NOT) {

        advance();

        if (peek().type != TOKEN_KEYWORD ||
            peek().keyword != Keyword::IN) {
            throw std::runtime_error("Expected 'in' after 'not'");
        }

        advance();

        return "not in";
    }

    if (peek().type == TOKEN_KEYWORD &&
        peek().keyword == Keyword::IN) {

        advance();
        return "in";
    }

    return advance().value;
}

std::shared_ptr<ASTNode> Parser::parseUnary() {

    if (matchAndAdvance(TOKEN_OP, "+"))
        return std::make_shared<UnaryOpNode>(
            "+",
            parseUnary()
        );

    if (matchAndAdvance(TOKEN_OP, "-"))
        return std::make_shared<UnaryOpNode>(
            "-",
            parseUnary()
        );

    return parsePower();
}

std::shared_ptr<ASTNode> Parser::parseNot() {
    if (peek().type == TOKEN_KEYWORD &&
        peek().keyword == Keyword::NOT) {
        advance();

        return std::make_shared<UnaryOpNode>(
            "not",
            parseNot()
        );
    }

    return parseComparison();
}

std::shared_ptr<ASTNode> Parser::parseAnd() {
    auto left = parseNot();

    while (peek().type == TOKEN_KEYWORD &&
        peek().keyword == Keyword::AND) {

        advance();

        auto right = parseNot();

        left = std::make_shared<LogicalOpNode>(left, "and", right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseOr()
{
    auto left = parseAnd();

    while (peek().type == TOKEN_KEYWORD &&
        peek().keyword == Keyword::OR) {

        advance();

        auto right = parseAnd();

        left = std::make_shared<LogicalOpNode>(left, "or", right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitOr() {

    auto left = parseBitXor();

    while (matchAndAdvance(TOKEN_OP, "|")) {

        auto right = parseBitXor();

        left = std::make_shared<BinOpNode>(
            left,
            "|",
            right
        );
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitXor() {

    auto left = parseBitAnd();

    while (matchAndAdvance(TOKEN_OP, "^")) {

        auto right = parseBitAnd();

        left = std::make_shared<BinOpNode>(
            left,
            "^",
            right
        );
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitAnd() {

    auto left = parseShift();

    while (matchAndAdvance(TOKEN_OP, "&")) {

        auto right = parseShift();

        left = std::make_shared<BinOpNode>(
            left,
            "&",
            right
        );
    }

    return left;
}

//TODO: пока это заглушка
std::shared_ptr<ASTNode> Parser::parseShift() {
    return parseAdditionAndSubtraction();
}

std::shared_ptr<ASTNode>Parser::parseDelStatement() {

    consume(TOKEN_KEYWORD, "del");

    auto target = parseExpression();

    if (
    !std::dynamic_pointer_cast<VarNode>(target) &&
    !std::dynamic_pointer_cast<IndexNode>(target) &&
    !std::dynamic_pointer_cast<AttributeAccessNode>(target)) {

        throw std::runtime_error(
        "SyntaxError: cannot delete expression"
        );
    }

    return std::make_shared<DeleteNode>(target);
}

ParsedCallArgs Parser::parseCallArguments() {

    ParsedCallArgs result;

    if (match(TOKEN_OP, ")")) {
        return result;
    }

    while (true) {

        if (peek().type == TOKEN_ID &&
            tokens[current + 1].type == TOKEN_OP &&
            tokens[current + 1].value == "=") {

            const QString name = advance().value;

            advance(); // =

            const auto value = parseExpression();

            result.keyword.push_back({name, value});

        }
        else {
            result.positional.push_back(parseExpression());
        }

        if (matchAndAdvance(TOKEN_OP, ",")) {
            continue;
        }

        break;
    }

    return result;
}

std::shared_ptr<ASTNode> Parser::parseDict() {

    std::vector<std::shared_ptr<DictElementNode>> items;

    consume(TOKEN_OP, "{");

    if (matchAndAdvance(TOKEN_OP, "}")) {
        return std::make_shared<DictNode>(std::move(items));
    }

    while (true) {

        // **expr
        if (matchAndAdvance(TOKEN_OP, "**")) {

            auto unpackExpr = parseExpression();

            items.emplace_back(
                std::make_shared<DictUnpackNode>(unpackExpr)
            );
        }
        else {

            auto key = parseExpression();

            consume(TOKEN_OP, ":");

            auto value = parseExpression();

            items.emplace_back(std::make_shared<DictPairNode>(key, value));
        }

        // конец dict
        if (matchAndAdvance(TOKEN_OP, "}")) {
            break;
        }

        consume(TOKEN_OP, ",");

        // trailing comma
        if (matchAndAdvance(TOKEN_OP, "}")) {
            break;
        }
    }

    return std::make_shared<DictNode>(std::move(items));
}

std::shared_ptr<ASTNode> Parser::parseSet() {

    std::vector<std::shared_ptr<ASTNode>> elements;

    consume(TOKEN_OP, "{");

    while (true) {

        elements.push_back(parseStarredExpression());

        if (matchAndAdvance(TOKEN_OP, "}")) {
            break;
        }

        consume(TOKEN_OP, ",");

        if (matchAndAdvance(TOKEN_OP, "}")) {
            break;
        }
    }

    return std::make_shared<SetNode>(std::move(elements));
}

bool Parser::isDictLiteral() {
    int pos = current + 1;

    int nesting = 0;

    while (pos < tokens.size()) {
        const Token &tok = tokens[pos];

        if (tok.type == TOKEN_OP) {
            if (tok.value == "{"
                || tok.value == "["
                || tok.value == "(") {
                nesting++;
            } else if (
                tok.value == "}"
                || tok.value == "]"
                || tok.value == ")") {
                if (nesting == 0) {
                    break;
                }

                nesting--;
            } else if ((tok.value == ":" || tok.value == "**") && nesting == 0) {
                return true;
            }
        }

        pos++;
    }

    return false;
}

std::shared_ptr<ASTNode> Parser::parseForStatement() {

    advance(); // for

    if (peek().type != TOKEN_ID) {
        throw std::runtime_error("Expected variable name after 'for'");
    }

    QString varName = advance().value;

    if (peek().type != TOKEN_KEYWORD ||
        peek().keyword.value() != Keyword::IN) {

        throw std::runtime_error("Expected 'in' after for variable");
    }

    advance(); // in

    auto iterable = parseExpression();

    consume(TOKEN_OP, ":");

    auto body = parseBlock();

    return std::make_shared<ForNode>(
        varName,
        iterable,
        body
    );
}

std::shared_ptr<ASTNode> Parser::parseDictOrSet() {

    // {}
    if (tokens[current].value == "{" &&
        current + 1 < tokens.size() &&
        tokens[current + 1].value == "}") {

        return parseDict();
    }

    if (isDictLiteral()) {
        return parseDict();
    }

    return parseSet();

}

std::shared_ptr<ASTNode> Parser::parseIndexOrSlice() {

    // [:...]
    if (match(TOKEN_OP, ":")) {

        advance(); // :

        std::shared_ptr<ASTNode> stop = nullptr;
        std::shared_ptr<ASTNode> step = nullptr;

        // [:5]
        if (!match(TOKEN_OP, "]") &&
            !match(TOKEN_OP, ":")) {

            stop = parseExpression();
        }

        // [:5:2]
        if (matchAndAdvance(TOKEN_OP, ":")) {

            if (!match(TOKEN_OP, "]")) {
                step = parseExpression();
            }
        }

        return std::make_shared<SliceNode>(
            nullptr,
            stop,
            step
        );
    }

    auto first = parseExpression();

    // обычный индекс
    if (!match(TOKEN_OP, ":")) {
        return first;
    }

    advance(); // :

    std::shared_ptr<ASTNode> stop = nullptr;
    std::shared_ptr<ASTNode> step = nullptr;

    // [1:5]
    if (!match(TOKEN_OP, "]") &&
        !match(TOKEN_OP, ":")) {

        stop = parseExpression();
        }

    // [1:5:2]
    if (matchAndAdvance(TOKEN_OP, ":")) {

        if (!match(TOKEN_OP, "]")) {
            step = parseExpression();
        }
    }

    return std::make_shared<SliceNode>(
        first,
        stop,
        step
    );
}

/**
 * @brief Получает текущий токен в потоке токенов, не сдвигая позицию.
 *
 * Этот метод возвращает токен на текущей позиции в процессе разбора.
 * Если достигнут конец потока токенов, возвращается специальный токен конца файла (EOF).
 *
 * @return Текущий токен, если позиция в потоке допустима; иначе токен типа TOKEN_EOF.
 */
Token Parser::peek() const {

    return current < tokens.size()
    ? tokens[current]
    : Token(TOKEN_EOF, "", 0);
}

/**
 * @brief Продвигает парсер к следующему токену и возвращает текущий токен.
 *
 * Этот метод перемещает указатель текущего токена вперёд, если доступны ещё токены.
 * Если указатель выходит за пределы имеющихся токенов, возвращается токен с типом TOKEN_EOF.
 *
 * @return Текущий Token до продвижения. Если достигнут конец потока токенов,
 *         возвращается токен TOKEN_EOF.
 */
Token Parser::advance() {

    return current < tokens.size()
    ? tokens[current++]
    : Token(TOKEN_EOF, "", 0);
}
