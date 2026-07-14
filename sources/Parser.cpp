#include "Parser.h"

#include "BytesValue.h"
#include "../ast/assignNode/AssignNode.h"
#include "../ast/attributeAccessNode/AttributeAccessNode.h"
#include "../ast/attributeAssignNode/AttributeAssignNode.h"
#include "../ast/augAssignNode/AugAssignNode.h"
#include "../ast/binOpNode/BinOpNode.h"
#include "../ast/breakNode/BreakNode.h"
#include "../ast/classDefNode/ClassDefNode.h"
#include "../ast/compareNode/CompareNode.h"
#include "../ast/continueNode/ContinueNode.h"
#include "../ast/deleteNode/DeleteNode.h"
#include "../ast/dictCompNode/DictCompNode.h"
#include "../ast/dictElementNode/dictPairNode/DictPairNode.h"
#include "../ast/dictElementNode/dictUnpackNode/DictUnpackNode.h"
#include "../ast/dictNode/DictNode.h"
#include "../ast/forNode/ForNode.h"
#include "../ast/functionDefNode/FunctionDefNode.h"
#include "../ast/genExprNode/GenExprNode.h"
#include "../ast/globalNode/GlobalNode.h"
#include "../ast/ifNode/IfNode.h"
#include "../ast/indexAssignNode/IndexAssignNode.h"
#include "../ast/indexNode/IndexNode.h"
#include "../ast/lambdaNode/LambdaNode.h"
#include "../ast/listCompNode/ListCompNode.h"
#include "../ast/listNode/ListNode.h"
#include "../ast/logicalOpNode/LogicalOpNode.h"
#include "../ast/nonlocalNode/NonlocalNode.h"
#include "../ast/passNode/PassNode.h"
#include "../ast/raiseNode/RaiseNode.h"
#include "../ast/returnNode/ReturnNode.h"
#include "../ast/setCompNode/SetCompNode.h"
#include "../ast/setNode/SetNode.h"
#include "../ast/sliceNode/SliceNode.h"
#include "../ast/starredNode/StarredNode.h"
#include "../ast/tryNode/TryNode.h"
#include "../ast/tupleAssignNode/TupleAssignNode.h"
#include "../ast/tupleNode/TupleNode.h"
#include "../ast/unaryOpNode/UnaryOpNode.h"
#include "../ast/valueNode/ValueNode.h"
#include "../ast/varNode/VarNode.h"
#include "../ast/whileNode/WhileNode.h"
#include "../ast/yieldFromNode/YieldFromNode.h"
#include "../ast/yieldNode/YieldNode.h"
#include "../exception/SyntaxErrorException.h"
#include "../exception/ValueErrorException.h"

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

    const Token startTok = peek();

    std::shared_ptr<ASTNode> node;

    if (peek().type == TOKEN_KEYWORD) {

        switch (peek().keyword.value()) {
            case Keyword::IF:       node = parseIfStatement(); break;
            case Keyword::WHILE:    node = parseWhileStatement(); break;
            case Keyword::BREAK:    node = parseBreakStatement(); break;
            case Keyword::CONTINUE: node = parseContinueStatement(); break;
            case Keyword::DEF:      node = parseFunctionDef(); break;
            case Keyword::RETURN:   node = parseReturn(); break;
            case Keyword::PASS:     node = parsePass(); break;
            case Keyword::CLASS:    node = parseClassDef(); break;
            case Keyword::LAMBDA:   node = parseLambda(); break;
            case Keyword::FOR:      node = parseForStatement(); break;
            case Keyword::DEL:      node = parseDelStatement(); break;
            case Keyword::TRY:      node = parseTryStatement(); break;
            case Keyword::RAISE:    node = parseRaiseStatement(); break;
            case Keyword::GLOBAL:   node = parseGlobalStatement(); break;
            case Keyword::NONLOCAL: node = parseNonlocalStatement(); break;
            case Keyword::YIELD:    node = parseYieldStatement(); break;
            default:                break;
        }
    }

    if (!node) {
        if (peek().type == TOKEN_AT) {
            node = parseDecorated();
        } else {
            node = parseExpressionStatement();
        }
    }

    const Token& endTok = tokens[current > 0 ? current - 1 : current];
    node->line = startTok.line;
    node->startColumn = startTok.startColumn;
    node->endColumn = endTok.endColumn;
    node->sourceId = Runtime::currentSourceId;
    return node;
}

std::shared_ptr<ASTNode> Parser::parseExpressionStatement() {

    std::vector<std::shared_ptr<ASTNode>> targets;
    targets.push_back(parseStarredExpression());

    bool sawComma = false;

    while (matchAndAdvance(TOKEN_OP, ",")) {

        sawComma = true;

        if (match(TOKEN_OP, "=") ||
            peek().type == TOKEN_NEWLINE ||
            peek().type == TOKEN_EOF ||
            peek().type == TOKEN_DEDENT) {
            break; // trailing comma: "a, ="
        }

        targets.push_back(parseStarredExpression());
    }

    if (!sawComma) {
        // обычный случай — одна цель/выражение, без изменений в поведении
        return parseAssignmentTail(targets[0]);
    }

    if (!matchAndAdvance(TOKEN_OP, "=")) {
        // нет '=' — это просто голое выражение-кортеж как стейтмент: "1, 2"
        return std::make_shared<TupleNode>(targets);
    }

    std::shared_ptr<ASTNode> rightNode;

    if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::YIELD) {

        advance(); // yield

        if (matchAndAdvance(TOKEN_KEYWORD, "from")) {
            rightNode = std::make_shared<YieldFromNode>(parseOr());
        } else if (peek().type == TOKEN_NEWLINE ||
                   peek().type == TOKEN_EOF ||
                   peek().type == TOKEN_DEDENT) {
            rightNode = std::make_shared<YieldNode>(nullptr);
                   } else {
                       rightNode = std::make_shared<YieldNode>(parseOr());
                   }

    } else {

        std::vector<std::shared_ptr<ASTNode>> values;
        values.push_back(parseStarredExpression());

        while (matchAndAdvance(TOKEN_OP, ",")) {
            if (peek().type == TOKEN_NEWLINE ||
                peek().type == TOKEN_EOF ||
                peek().type == TOKEN_DEDENT) {
                break;
                }
            values.push_back(parseStarredExpression());
        }

        rightNode = values.size() > 1
            ? std::make_shared<TupleNode>(values)
            : values[0];
    }

    return std::make_shared<TupleAssignNode>(std::move(targets), rightNode);
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

    const auto left = parseExpressionNoAssign();
    return parseAssignmentTail(left);
}

std::shared_ptr<ASTNode> Parser::parseExpressionNoAssign() {
    return parseOr();
}

std::shared_ptr<ASTNode> Parser::parseAssignmentTail(std::shared_ptr<ASTNode> left) {

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

        if (const auto var = std::dynamic_pointer_cast<VarNode>(left)) {
            return std::make_shared<AugAssignNode>(var->name, op, right);
        }

        throw SyntaxErrorException("Invalid augmented assignment target");
    }

    if (matchAndAdvance(TOKEN_OP, "=")) {

        auto right = parseRightHandSide();

        if (const auto var = std::dynamic_pointer_cast<VarNode>(left)) {
            return std::make_shared<AssignNode>(var->name, right);
        }

        if (const auto attr = std::dynamic_pointer_cast<AttributeAccessNode>(left)) {
            return std::make_shared<AttributeAssignNode>(attr->object, attr->attr, right);
        }

        if (const auto idx = std::dynamic_pointer_cast<IndexNode>(left)) {
            return std::make_shared<IndexAssignNode>(idx->object, idx->index, right);
        }

        throw SyntaxErrorException("Invalid assignment target");
    }

    return left;
}

std::vector<ComprehensionClause> Parser::parseComprehensionClauses() {

    std::vector<ComprehensionClause> clauses;

    while (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::FOR) {

        advance(); // for

        if (peek().type != TOKEN_ID) {
            throw SyntaxErrorException("Expected identifier after 'for' in comprehension");
        }

        const QString varName = advance().value;

        if (!(peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::IN)) {
            throw SyntaxErrorException("Expected 'in' in comprehension");
        }

        advance(); // in

        auto iterable = parseOr();

        std::vector<std::shared_ptr<ASTNode>> conditions;

        while (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::IF) {
            advance(); // if
            conditions.push_back(parseOr());
        }

        clauses.push_back(ComprehensionClause{varName, iterable, std::move(conditions)});
    }

    if (clauses.empty()) {
        throw SyntaxErrorException("Expected 'for' in comprehension");
    }

    return clauses;
}

Parser::BraceKind Parser::classifyBraces() {

    int pos = current + 1; // сразу после "{"
    int nesting = 0;

    bool sawColonOrUnpack = false;
    bool sawFor = false;

    while (pos < tokens.size()) {

        const Token& tok = tokens[pos];

        if (tok.type == TOKEN_OP) {

            if (tok.value == "{" || tok.value == "[" || tok.value == "(") {
                nesting++;
            }
            else if (tok.value == "}" || tok.value == "]" || tok.value == ")") {
                if (nesting == 0) break;
                nesting--;
            }
            else if (nesting == 0 && (tok.value == ":" || tok.value == "**")) {
                sawColonOrUnpack = true;
            }
        }

        if (nesting == 0 &&
            tok.type == TOKEN_KEYWORD &&
            tok.keyword == Keyword::FOR) {
            sawFor = true;
        }

        pos++;
    }

    if (sawFor) {
        return sawColonOrUnpack ? BraceKind::DictComp : BraceKind::SetComp;
    }

    return sawColonOrUnpack ? BraceKind::Dict : BraceKind::Set;
}

std::shared_ptr<ASTNode> Parser::parseStarredExpression() {

    if (matchAndAdvance(TOKEN_OP, "*")) {
        return std::make_shared<StarredNode>(
            parseExpressionNoAssign()
        );
    }

    return parseExpressionNoAssign();
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

std::shared_ptr<ASTNode> Parser::makeBinOp(
    std::shared_ptr<ASTNode> left,
    const Token& opToken,
    std::shared_ptr<ASTNode> right) {

    auto node = std::make_shared<BinOpNode>(left, opToken.value, right);

    node->line = left->line;
    node->startColumn = left->startColumn;
    node->endColumn = right->endColumn;
    node->sourceId = left->sourceId;

    node->opStartColumn = opToken.startColumn;
    node->opEndColumn = opToken.endColumn;

    return node;
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

        Token opToken = peek();
        advance();

        const std::shared_ptr<ASTNode> right = parseTerm();

        left = makeBinOp(left, opToken, right);
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

        Token opToken = peek();
        advance();

        const std::shared_ptr<ASTNode> right = parseUnary();

        left = makeBinOp(left, opToken, right);
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
std::shared_ptr<ASTNode> Parser::parsePower() {

    std::shared_ptr<ASTNode> left = parsePrimary();

    if (match(TOKEN_OP, "**")) {

        Token opToken = peek();
        advance();

        const std::shared_ptr<ASTNode> right = parseUnary();

        left = makeBinOp(left, opToken, right);
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

    const Token startTok = peek();

    std::shared_ptr<ASTNode> node;

    switch (const Token token = peek(); token.type) {
        case TOKEN_NUMBER: node = parseNumberToken(); break;
        case TOKEN_STRING: node = parseStringToken(); break;
        case TOKEN_BYTES:  node = parseBytesToken(); break;
        case TOKEN_BOOL:   node = parseBoolToken(); break;
        case TOKEN_NONE:   node = parseNoneToken(); break;
        case TOKEN_ID:     node = parseIdentifierToken(); break;

        case TOKEN_KEYWORD:
            if (token.keyword.value() == Keyword::LAMBDA) {
                node = parseLambda();
                break;
            }
            [[fallthrough]];

        case TOKEN_OP:
            if (token.value == "(")
                node = parseParenthesizedExpression();
            else if (token.value == "[")
                node = parseList();
            else if (token.value == "{")
                node = parseDictOrSet();
            else
                throwUnexpectedTokenError(token);
            break;

        case TOKEN_EOF: throw makeSyntaxError("invalid syntax", peek());
        default: throwUnexpectedTokenError(token);
    }

    const Token& midTok = tokens[current > 0 ? current - 1 : current];
    node->line = startTok.line;
    node->startColumn = startTok.startColumn;
    node->endColumn = midTok.endColumn;

    auto result = parsePostfix(node);

    const Token& endTok = tokens[current > 0 ? current - 1 : current];
    node->line = startTok.line;
    node->startColumn = startTok.startColumn;
    node->endColumn = endTok.endColumn;
    node->sourceId = Runtime::currentSourceId;

    return result;
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
        throw ValueErrorException("Invalid number format: " + token.value);
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

    const Token nameTok = peek();
    QString name = advance().value;

    if (match(TOKEN_OP, "(")) {

        const Token openParen = peek();   // позиция "(" — граница callee/args
        advance();

        auto parsedArgs = parseCallArguments();


        if (!match(TOKEN_OP, ")")) {
            while (true) {
                if (matchAndAdvance(TOKEN_OP, ",")) {
                    continue;
                }
                break;
            }
        }

        const Token closeParen = peek();
        consume(TOKEN_OP, ")");

        auto callee = std::make_shared<VarNode>(name);
        callee->line = nameTok.line;
        callee->startColumn = nameTok.startColumn;
        callee->endColumn = nameTok.endColumn;
        callee->sourceId = Runtime::currentSourceId;

        auto callNode = std::make_shared<CallNode>(
            callee,
            parsedArgs.positional,
            parsedArgs.keyword
        );

        callNode->calleeEndColumn = openParen.startColumn;
        callNode->line = nameTok.line;
        callNode->startColumn = nameTok.startColumn;
        callNode->endColumn = closeParen.endColumn;
        callNode->sourceId = Runtime::currentSourceId;

        return callNode;
    }

    auto var = std::make_shared<VarNode>(name);
    var->line = nameTok.line;
    var->startColumn = nameTok.startColumn;
    var->endColumn = nameTok.endColumn;
    var->sourceId = Runtime::currentSourceId;

    return var;
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
        return std::make_shared<TupleNode>(std::vector<std::shared_ptr<ASTNode>>{});
    }

    auto first = parseStarredExpression();

    // генераторное выражение: (expr for ...)
    if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::FOR) {

        auto clauses = parseComprehensionClauses();
        consume(TOKEN_OP, ")");

        return std::make_shared<GenExprNode>(first, std::move(clauses));
    }

    if (match(TOKEN_OP, ",")) {

        std::vector<std::shared_ptr<ASTNode>> elements;
        elements.push_back(first);

        while (matchAndAdvance(TOKEN_OP, ",")) {
            if (match(TOKEN_OP, ")")) break;
            elements.push_back(parseStarredExpression());
        }

        consume(TOKEN_OP, ")");
        return std::make_shared<TupleNode>(std::move(elements));
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
    throw SyntaxErrorException("Unexpected token: \"" + token.value + "\"");
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
        throw SyntaxErrorException("Expected newline after statement");

    advance();

    if (peek().type != TOKEN_INDENT)
        throw SyntaxErrorException("Expected indent after statement");

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
        throw SyntaxErrorException("Expected dedent after block");
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
        throw SyntaxErrorException("Expected function name");
    }

    QString name = advance().value;

    consume(TOKEN_OP, "(");

    std::vector<Param> params;

    if (!match(TOKEN_OP, ")")) {

        while (true) {

            if (peek().type != TOKEN_ID)
                throw SyntaxErrorException("Expected parameter name");

            Param param {advance().value, ""};

            if (matchAndAdvance(TOKEN_OP, ":")) {

                if (peek().type != TOKEN_ID)
                    throw SyntaxErrorException("Expected type after ':'");

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
            throw SyntaxErrorException("Expected return type after '->'");

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

std::shared_ptr<ASTNode> Parser::parseGlobalStatement() {

    advance(); // global

    QVector<QString> names;

    if (peek().type != TOKEN_ID) {
        throw SyntaxErrorException("Expected identifier after 'global'");
    }

    names.push_back(advance().value);

    while (matchAndAdvance(TOKEN_OP, ",")) {

        if (peek().type != TOKEN_ID) {
            throw SyntaxErrorException("Expected identifier after ','");
        }

        names.push_back(advance().value);
    }

    auto node = std::make_shared<GlobalNode>();
    node->names = names;

    return node;
}

std::shared_ptr<ASTNode> Parser::parseNonlocalStatement() {

    advance(); // nonlocal

    QVector<QString> names;

    if (peek().type != TOKEN_ID) {
        throw SyntaxErrorException("Expected identifier after 'nonlocal'");
    }

    names.push_back(advance().value);

    while (matchAndAdvance(TOKEN_OP, ",")) {

        if (peek().type != TOKEN_ID) {
            throw SyntaxErrorException("Expected identifier after ','");
        }

        names.push_back(advance().value);
    }

    auto node = std::make_shared<NonlocalNode>();
    node->names = names;

    return node;
}

std::shared_ptr<ASTNode> Parser::parseYieldStatement() {

    advance(); // yield

    if (matchAndAdvance(TOKEN_KEYWORD, "from")) {
        return std::make_shared<YieldFromNode>(parseExpression());
    }

    if (peek().type == TOKEN_NEWLINE ||
        peek().type == TOKEN_DEDENT ||
        peek().type == TOKEN_EOF) {

        return std::make_shared<YieldNode>(nullptr);
        }

    return std::make_shared<YieldNode>(parseExpression());
}

std::shared_ptr<ASTNode> Parser::parseRightHandSide() {

    if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::YIELD) {

        advance(); // yield

        if (matchAndAdvance(TOKEN_KEYWORD, "from")) {
            return std::make_shared<YieldFromNode>(parseOr());
        }

        if (peek().type == TOKEN_NEWLINE ||
            peek().type == TOKEN_EOF ||
            peek().type == TOKEN_DEDENT) {
            return std::make_shared<YieldNode>(nullptr);
        }

        return std::make_shared<YieldNode>(parseOr());
    }

    return parseOr();
}

std::shared_ptr<ASTNode> Parser::parseClassDef(
    const std::vector<std::shared_ptr<ASTNode>>& decorators) {

    advance(); // class

    if (peek().type != TOKEN_ID) {
        throw SyntaxErrorException("Expected class name");
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
        if (match(TOKEN_OP, ".")) {

            const int objectEnd = node->endColumn;   // конец объекта = начало ".attr"
            advance(); // .

            if (peek().type != TOKEN_ID)
                throw SyntaxErrorException("Expected attribute name after '.'");

            const Token attrTok = peek();
            QString attr = advance().value;

            auto attrNode = std::make_shared<AttributeAccessNode>(node, attr);
            attrNode->objectEndColumn = objectEnd;
            attrNode->line = node->line;
            attrNode->startColumn = node->startColumn;
            attrNode->endColumn = attrTok.endColumn;
            attrNode->sourceId = Runtime::currentSourceId;

            node = attrNode;
            continue;
        }

        // вызов: obj(...)
        if (match(TOKEN_OP, "(")) {

            const int objectEnd = node->endColumn;   // конец callee = начало "("
            advance(); // (

            auto parsedArgs = parseCallArguments();

            if (!match(TOKEN_OP, ")")) {
                while (true) {
                    if (matchAndAdvance(TOKEN_OP, ",")) {
                        continue;
                    }
                    break;
                }
            }

            const Token closeParen = peek();
            consume(TOKEN_OP, ")");

            auto callNode = std::make_shared<CallNode>(node, parsedArgs.positional, parsedArgs.keyword);
            callNode->calleeEndColumn = objectEnd;
            callNode->line = node->line;
            callNode->startColumn = node->startColumn;
            callNode->endColumn = closeParen.endColumn;
            callNode->sourceId = Runtime::currentSourceId;

            node = callNode;
            continue;
        }

        // obj[index]
        if (match(TOKEN_OP, "[")) {

            const int objectEnd = node->endColumn;   // конец объекта = начало "["
            advance(); // [

            auto index = parseIndexOrSlice();

            const Token closeBracket = peek();
            consume(TOKEN_OP, "]");

            auto indexNode = std::make_shared<IndexNode>(node, index);
            indexNode->objectEndColumn = objectEnd;
            indexNode->line = node->line;
            indexNode->startColumn = node->startColumn;
            indexNode->endColumn = closeBracket.endColumn;
            indexNode->sourceId = Runtime::currentSourceId;

            node = indexNode;
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
        throw SyntaxErrorException("Expected def or class after decorator");
    }

    const auto kw = peek().keyword.value();

    if (kw == Keyword::DEF) {
        return parseFunctionDef(decorators);
    }

    if (kw == Keyword::CLASS) {
        return parseClassDef(decorators);
    }

    throw SyntaxErrorException("Decorator can only be applied to def/class");
}

std::shared_ptr<ASTNode> Parser::parseList() {

    advance(); // [

    if (matchAndAdvance(TOKEN_OP, "]")) {
        return std::make_shared<ListNode>(std::vector<std::shared_ptr<ASTNode>>{});
    }

    auto first = parseStarredExpression();

    // list comprehension: [expr for ...]
    if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::FOR) {

        auto clauses = parseComprehensionClauses();
        consume(TOKEN_OP, "]");

        return std::make_shared<ListCompNode>(first, std::move(clauses));
    }

    std::vector<std::shared_ptr<ASTNode>> elements;
    elements.push_back(first);

    while (true) {

        if (matchAndAdvance(TOKEN_OP, "]")) {
            break;
        }

        consume(TOKEN_OP, ",");

        if (matchAndAdvance(TOKEN_OP, "]")) {
            break;
        }

        elements.push_back(parseStarredExpression());
    }

    return std::make_shared<ListNode>(std::move(elements));
}

std::shared_ptr<ASTNode> Parser::parseLambda() {

    advance(); // lambda

    std::vector<Param> params;

    if (!match(TOKEN_OP, ":")) {

        while (true) {

            if (peek().type != TOKEN_ID) {
                throw SyntaxErrorException("Expected parameter name in lambda");
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
        throw makeSyntaxError("invalid syntax", peek());
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

    if (peek().type == TOKEN_KEYWORD &&
        peek().keyword == Keyword::IS)
        return true;

    if (peek().type == TOKEN_KEYWORD &&
    peek().keyword == Keyword::IS &&
    current + 1 < tokens.size() &&
    tokens[current + 1].type == TOKEN_KEYWORD &&
    tokens[current + 1].keyword == Keyword::NOT)
        return true;

    return false;
}

QString Parser::parseComparisonOperator() {

    if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::IS) {

        advance();

        if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::NOT) {
            advance();
            return "is not";
        }

        return "is";
    }

    if (peek().type == TOKEN_KEYWORD && peek().keyword == Keyword::NOT) {

        advance();

        if (peek().type != TOKEN_KEYWORD ||
            peek().keyword != Keyword::IN) {
            throw SyntaxErrorException("Expected 'in' after 'not'");
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

std::shared_ptr<ASTNode> Parser::parseOr() {

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

    while (match(TOKEN_OP, "|")) {

        const Token opToken = peek();
        advance();

        const auto right = parseBitXor();

        left = makeBinOp(left, opToken, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitXor() {

    auto left = parseBitAnd();

    while (match(TOKEN_OP, "^")) {

        const Token opToken = peek();
        advance();

        const auto right = parseBitAnd();

        left = makeBinOp(left, opToken, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseBitAnd() {

    auto left = parseShift();

    while (match(TOKEN_OP, "&")) {

        const Token opToken = peek();
        advance();

        const auto right = parseShift();

        left = makeBinOp(left, opToken, right);
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

        throw SyntaxErrorException("cannot delete expression");
    }

    return std::make_shared<DeleteNode>(target);
}

std::shared_ptr<ASTNode> Parser::parseTryStatement() {
    consume(TOKEN_KEYWORD, "try");

    consume(TOKEN_OP, ":");

    auto tryBody = parseBlock();

    std::vector<TryNode::ExceptClause> excepts;

    while (matchAndAdvance(TOKEN_KEYWORD, "except")) {

        std::shared_ptr<ASTNode> exceptionExpr = nullptr;
        QString variableName;

        // except:
        if (!match(TOKEN_OP, ":")) {

            exceptionExpr = parseExpression();

            if (matchAndAdvance(TOKEN_KEYWORD, "as")) {

                if (peek().type != TOKEN_ID)
                    throw SyntaxErrorException(
                        "Expected identifier after 'as'"
                    );

                variableName = advance().value;
            }
        }

        consume(TOKEN_OP, ":");

        const auto body = parseBlock();

        excepts.push_back(
            TryNode::ExceptClause{
                exceptionExpr,
                variableName,
                body
            }
        );
    }

    std::vector<std::shared_ptr<ASTNode>> elseBody;

    if (matchAndAdvance(TOKEN_KEYWORD, "else")) {

        consume(TOKEN_OP, ":");

        elseBody = parseBlock();
    }

    std::vector<std::shared_ptr<ASTNode>> finallyBody;

    if (matchAndAdvance(TOKEN_KEYWORD, "finally")) {

        consume(TOKEN_OP, ":");

        finallyBody = parseBlock();
    }

    if (excepts.empty() && finallyBody.empty()) {
        throw SyntaxErrorException(
            "expected except or finally"
        );
    }

    return std::make_shared<TryNode>(
        tryBody,
        excepts,
        elseBody,
        finallyBody
    );
}

std::shared_ptr<ASTNode> Parser::parseRaiseStatement() {

    advance(); // raise

    if (peek().type == TOKEN_NEWLINE ||
        peek().type == TOKEN_DEDENT ||
        peek().type == TOKEN_EOF) {

        return std::make_shared<RaiseNode>(nullptr, nullptr);
        }

    std::shared_ptr<ASTNode> exceptionExpr = parseExpression();

    std::shared_ptr<ASTNode> causeExpr = nullptr;

    if (matchAndAdvance(TOKEN_KEYWORD, "from")) {
        causeExpr = parseExpression();
    }

    return std::make_shared<RaiseNode>(exceptionExpr, causeExpr);
}

ParsedCallArgs Parser::parseCallArguments() {

    ParsedCallArgs result;

    if (match(TOKEN_OP, ")")) {
        return result;
    }

    bool first = true;

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

            auto value = parseExpression();

            // sum(x for x in range(10)) — генераторное выражение
            // без обёрточных скобок, разрешено ТОЛЬКО как единственный аргумент
            if (first &&
                peek().type == TOKEN_KEYWORD &&
                peek().keyword == Keyword::FOR) {

                auto clauses = parseComprehensionClauses();

                if (!match(TOKEN_OP, ")")) {
                    throw SyntaxErrorException(
                        "Generator expression must be parenthesized if not sole argument"
                    );
                }

                result.positional.push_back(
                    std::make_shared<GenExprNode>(value, std::move(clauses))
                );

                return result;
                }

            result.positional.push_back(value);
        }

        first = false;

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
        throw SyntaxErrorException("Expected variable name after 'for'");
    }

    QString varName = advance().value;

    if (peek().type != TOKEN_KEYWORD ||
        peek().keyword.value() != Keyword::IN) {

        throw SyntaxErrorException("Expected 'in' after for variable");
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

    switch (classifyBraces()) {
        case BraceKind::DictComp: return parseDictComp();
        case BraceKind::SetComp:  return parseSetComp();
        case BraceKind::Dict:     return parseDict();
        case BraceKind::Set:      return parseSet();
        default:                  throwUnexpectedTokenError(peek());
    }
}

std::shared_ptr<ASTNode> Parser::parseDictComp() {

    consume(TOKEN_OP, "{");

    auto key = parseExpression();
    consume(TOKEN_OP, ":");
    auto value = parseExpression();

    auto clauses = parseComprehensionClauses();

    consume(TOKEN_OP, "}");

    return std::make_shared<DictCompNode>(key, value, std::move(clauses));
}

std::shared_ptr<ASTNode> Parser::parseSetComp() {

    consume(TOKEN_OP, "{");

    auto expr = parseStarredExpression();

    auto clauses = parseComprehensionClauses();

    consume(TOKEN_OP, "}");

    return std::make_shared<SetCompNode>(expr, std::move(clauses));
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

SyntaxErrorException Parser::makeSyntaxError(const QString& msg, const Token& tok) {
    SyntaxErrorException e(msg);
    e.line = tok.line;
    e.startColumn = tok.startColumn;
    e.endColumn = tok.endColumn;
    e.sourceId = Runtime::currentSourceId;
    e.hasPosition = true;
    return e;
}
