#include "../inc/SyntacticAnalyzer.hpp"
#include <iostream>

// Инициализация статической переменной
bool SyntacticAnalyzer::debugMode = false;

SyntacticAnalyzer::SyntacticAnalyzer(const vector<Token>& tokenList) 
    : tokens(tokenList), currentPos(0), hasError(false) {
    if (!tokens.empty()) {
        current_token = tokens[0];
    } else {
        current_token = Token(END_OF_FILE, "", 0, 0);
    }
}

// Метод для отладочного вывода
void SyntacticAnalyzer::debugPrint(const string& message) {
    if (debugMode) {
        cout << message << endl;
    }
}

void SyntacticAnalyzer::updateCurrentToken() {
    if (currentPos < tokens.size()) {
        current_token = tokens[currentPos];
    } else {
        current_token = Token(END_OF_FILE, "", 0, 0);
    }
}

Token SyntacticAnalyzer::peekNextToken(int offset) {
    int peekPos = currentPos + offset;
    if (peekPos < tokens.size()) {
        return tokens[peekPos];
    }
    return Token(END_OF_FILE, "", 0, 0);
}

void SyntacticAnalyzer::consumeToken() {
    if (currentPos < tokens.size()) {
        currentPos++;
        updateCurrentToken();
    }
}

bool SyntacticAnalyzer::matchToken(TokenType expectedType, const string& expectedValue) {
    if (current_token.type == expectedType && 
        (expectedValue.empty() || current_token.value == expectedValue)) {
        consumeToken();
        return true;
    }
    return false;
}

bool SyntacticAnalyzer::matchTokenValue(const string& expectedValue) {
    if (current_token.value == expectedValue) {
        consumeToken();
        return true;
    }
    return false;
}

bool SyntacticAnalyzer::matchTokenType(TokenType expectedType) {
    if (current_token.type == expectedType) {
        consumeToken();
        return true;
    }
    return false;
}

bool SyntacticAnalyzer::isRelationalOperator() {
    return current_token.value == "LT" || current_token.value == "LE" ||
           current_token.value == "GT" || current_token.value == "GE" ||
           current_token.value == "EQ" || current_token.value == "NE";
}

bool SyntacticAnalyzer::isAdditiveOperator() {
    return current_token.value == "plus" || current_token.value == "min" || 
           current_token.value == "or";
}

bool SyntacticAnalyzer::isMultiplicativeOperator() {
    return current_token.value == "mult" || current_token.value == "div" || 
           current_token.value == "and";
}

// === Новая грамматика выражений (P16-P25) ===

/**
 * @brief P22: <логическая_константа>::= true | false
 */
void SyntacticAnalyzer::booleanConstant() {
    debugPrint("[DEBUG] booleanConstant() called, current token: " + current_token.value);
    
    if (current_token.value == "true" || current_token.value == "false") {
        debugPrint("[DEBUG] Consuming boolean constant: " + current_token.value);
        consumeToken();
    } else {
        hasError = true;
        errorMessage = "Ожидается логическая константа (true или false)";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    debugPrint("[DEBUG] booleanConstant() completed, current token: " + current_token.value);
}

/**
 * @brief P21: <число>::= <целое> | <действительное>
 */
void SyntacticAnalyzer::number() {
    usedRules.push_back("P21"); // ДОБАВЛЕНО ПРАВИЛО P21
    debugPrint("[DEBUG] number() called, current token: " + current_token.value);
    
    if (current_token.type == INT_CONST || current_token.type == FLOAT_CONST ||
        current_token.type == OCT_CONST || current_token.type == HEX_CONST) {
        debugPrint("[DEBUG] Consuming number: " + current_token.value);
        consumeToken();
    } else {
        hasError = true;
        errorMessage = "Ожидается число";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    debugPrint("[DEBUG] number() completed, current token: " + current_token.value);
}

/**
 * @brief P20: <унарная_операция>::='~'
 */
void SyntacticAnalyzer::unaryOperation() {
    usedRules.push_back("P20"); // ДОБАВЛЕНО ПРАВИЛО P20
    debugPrint("[DEBUG] unaryOperation() called, current token: " + current_token.value);
    
    if (current_token.value == "~") {
        debugPrint("[DEBUG] Found unary operator: ~");
        consumeToken();
    } else {
        hasError = true;
        errorMessage = "Ожидается унарный оператор ~";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    debugPrint("[DEBUG] unaryOperation() completed, current token: " + current_token.value);
}

/**
 * @brief P19: <множитель>::= <идентификатор> | <число> | <логическая_константа> | 
 *           <унарная_операция> <множитель> | (<выражение>)
 */
void SyntacticAnalyzer::factor() {
    usedRules.push_back("P19"); // ДОБАВЛЕНО ПРАВИЛО P19
    debugPrint("[DEBUG] factor() called, current token: " + current_token.value);
    
    // Унарная операция
    if (current_token.value == "~") {
        debugPrint("[DEBUG] Processing unary operation");
        unaryOperation();
        factor();
    }
    // Выражение в скобках
    else if (current_token.value == "(") {
        debugPrint("[DEBUG] Found '(' starting parenthesized expression");
        consumeToken();
        expression();
        if (!matchTokenValue(")")) {
            hasError = true;
            errorMessage = "Ожидается ')'";
            errorLine = current_token.line;
            errorColumn = current_token.column;
        }
    }
    // Идентификатор (P23)
    else if (current_token.type == IDENTIFIER) {
        usedRules.push_back("P23"); // ДОБАВЛЕНО ПРАВИЛО P23
        debugPrint("[DEBUG] Consuming identifier: " + current_token.value);
        consumeToken();
    }
    // Число (P21)
    else if (current_token.type == INT_CONST || current_token.type == FLOAT_CONST ||
             current_token.type == OCT_CONST || current_token.type == HEX_CONST) {
        number(); // Вызываем number() вместо consumeToken()
    }
    // Логическая константа (P22)
    else if (current_token.value == "true" || current_token.value == "false") {
        booleanConstant(); // Вызываем booleanConstant() вместо consumeToken()
    }
    // Ошибка
    else {
        hasError = true;
        errorMessage = "Ожидается множитель (идентификатор, число, логическая константа или '(')";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        debugPrint("[DEBUG] Error in factor(): " + errorMessage);
    }
    
    debugPrint("[DEBUG] factor() completed, current token: " + current_token.value);
}

/**
 * @brief P18: <слагаемое>::= <множитель> {<операции_группы_умножения> <множитель>}
 */
void SyntacticAnalyzer::term() {
    usedRules.push_back("P18"); // ДОБАВЛЕНО ПРАВИЛО P18
    debugPrint("[DEBUG] term() called, current token: " + current_token.value);
    
    factor();
    
    if (hasError) return;
    
    // Обработка операторов умножения (P3)
    while (isMultiplicativeOperator()) {
        usedRules.push_back("P3"); // ДОБАВЛЕНО ПРАВИЛО P3
        string op = current_token.value;
        debugPrint("[DEBUG] Found multiplicative operator: " + op);
        consumeToken();
        factor();
        
        if (hasError) return;
    }
    
    debugPrint("[DEBUG] term() completed, current token: " + current_token.value);
}

/**
 * @brief P17: <операнд>::= <слагаемое> {<операции_группы_сложения> <слагаемое>}
 */
void SyntacticAnalyzer::operand() {
    usedRules.push_back("P17"); // ДОБАВЛЕНО ПРАВИЛО P17
    debugPrint("[DEBUG] operand() called, current token: " + current_token.value);
    
    term();
    
    if (hasError) return;
    
    // Обработка операторов сложения (P2)
    while (isAdditiveOperator()) {
        usedRules.push_back("P2"); // ДОБАВЛЕНО ПРАВИЛО P2
        string op = current_token.value;
        debugPrint("[DEBUG] Found additive operator: " + op);
        consumeToken();
        term();
        
        if (hasError) return;
    }
    
    debugPrint("[DEBUG] operand() completed, current token: " + current_token.value);
}

/**
 * @brief P16: <выражение>::= <операнд>{<операции_группы_отношения> <операнд>}
 */
void SyntacticAnalyzer::expression() {
    string message = "[DEBUG] expression() called, current token: " + current_token.value +
                     " (line: " + to_string(current_token.line) + ")";
    debugPrint(message);
    usedRules.push_back("P16");
    
    operand();
    
    if (hasError) return;
    
    // Обработка операторов отношения (P1)
    while (isRelationalOperator()) {
        usedRules.push_back("P1"); // ДОБАВЛЕНО ПРАВИЛО P1
        string op = current_token.value;
        debugPrint("[DEBUG] Found relational operator: " + op);
        consumeToken();
        operand();
        
        if (hasError) return;
    }
    
    debugPrint("[DEBUG] expression() completed, current token: " + current_token.value);
}

// === Основные правила грамматики ===

/**
 * @brief P6: Описание данных - dim <идентификатор> {, <идентификатор> } <тип>
 */
void SyntacticAnalyzer::description() {
    usedRules.push_back("P6");
    debugPrint("[DEBUG] description() called, current token: " + current_token.value);
    
    if (!matchTokenValue("dim")) {
        hasError = true;
        errorMessage = "Ожидается 'dim'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    if (!matchTokenType(IDENTIFIER)) {
        hasError = true;
        errorMessage = "Ожидается идентификатор после 'dim'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Обработка списка идентификаторов через запятую
    while (matchTokenValue(",")) {
        if (!matchTokenType(IDENTIFIER)) {
            hasError = true;
            errorMessage = "Ожидается идентификатор после ','";
            errorLine = current_token.line;
            errorColumn = current_token.column;
            return;
        }
    }
    
    // P7: Проверка типа данных
    usedRules.push_back("P7");
    if (!matchTokenValue("integer") && !matchTokenValue("real") && 
        !matchTokenValue("boolean")) {
        hasError = true;
        errorMessage = "Ожидается тип (integer, real, boolean)";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    debugPrint("[DEBUG] description() completed, current token: " + current_token.value);
}

/**
 * @brief P9: Оператор присваивания - <идентификатор> ass <выражение>
 */
void SyntacticAnalyzer::assignment() {
    usedRules.push_back("P9");
    debugPrint("[DEBUG] assignment() called, current token: " + current_token.value);
    
    // Сохраняем идентификатор
    string identifier = current_token.value;
    
    if (!matchTokenType(IDENTIFIER)) {
        hasError = true;
        errorMessage = "Ожидается идентификатор перед 'ass'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    debugPrint("[DEBUG] Consumed identifier: " + identifier);
    
    if (!matchTokenValue("ass")) {
        hasError = true;
        errorMessage = "Ожидается 'ass'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    debugPrint("[DEBUG] Consumed 'ass', now parsing expression");
    
    // Правая часть присваивания - выражение (P16)
    expression();
    
    debugPrint("[DEBUG] assignment() completed, current token: " + current_token.value);
}

/**
 * @brief P10: Условный оператор - if <выражение> then <оператор> [else <оператор>]
 */
void SyntacticAnalyzer::conditional() {
    usedRules.push_back("P10");
    debugPrint("[DEBUG] conditional() called, current token: " + current_token.value);
    
    if (!matchTokenValue("if")) {
        hasError = true;
        errorMessage = "Ожидается 'if'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Условие if - выражение (P16)
    expression();
    
    if (!matchTokenValue("then")) {
        hasError = true;
        errorMessage = "Ожидается 'then'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    debugPrint("[DEBUG] Before then-block statement(), current token: " + current_token.value);
    
    // Блок then
    statement();
    
    // Необязательный блок else
    if (matchTokenValue("else")) {
        debugPrint("[DEBUG] Found 'else', processing else-block");
        statement();
    }
    
    debugPrint("[DEBUG] conditional() completed, current token: " + current_token.value);
}

/**
 * @brief P11: Цикл for - for <присваивания> to <выражение> do <оператор>
 */
void SyntacticAnalyzer::forLoop() {
    usedRules.push_back("P11");
    debugPrint("[DEBUG] forLoop() called, current token: " + current_token.value);
    
    if (!matchTokenValue("for")) {
        hasError = true;
        errorMessage = "Ожидается 'for'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Инициализация счетчика
    assignment();
    
    if (!matchTokenValue("to")) {
        hasError = true;
        errorMessage = "Ожидается 'to'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Конечное значение - выражение
    expression();
    
    // Ключевое слово 'do' обязательно
    if (!matchTokenValue("do")) {
        hasError = true;
        errorMessage = "Ожидается 'do' после выражения";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Тело цикла
    statement();
    
    debugPrint("[DEBUG] forLoop() completed, current token: " + current_token.value);
}

/**
 * @brief P12: Цикл while - while <выражение> do <оператор>
 */
void SyntacticAnalyzer::whileLoop() {
    usedRules.push_back("P12");
    debugPrint("[DEBUG] whileLoop() called, current token: " + current_token.value);
    
    if (!matchTokenValue("while")) {
        hasError = true;
        errorMessage = "Ожидается 'while'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Условие цикла - выражение (P16)
    expression();
    
    if (!matchTokenValue("do")) {
        hasError = true;
        errorMessage = "Ожидается 'do'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Тело цикла
    statement();
    
    debugPrint("[DEBUG] whileLoop() completed, current token: " + current_token.value);
}

/**
 * @brief P13: Оператор ввода - read(идентификатор {, <идентификатор> })
 */
void SyntacticAnalyzer::readStatement() {
    usedRules.push_back("P13");
    debugPrint("[DEBUG] readStatement() called, current token: " + current_token.value);
    
    // Сохраняем координаты для сообщений об ошибках
    int line = current_token.line;
    int column = current_token.column;
    
    if (!matchTokenValue("read")) {
        hasError = true;
        errorMessage = "Ожидается 'read'";
        errorLine = line;
        errorColumn = column;
        return;
    }
    
    // Открывающая скобка обязательна
    if (!matchTokenValue("(")) {
        hasError = true;
        errorMessage = "Ожидается '(' после 'read'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Первый идентификатор обязателен
    if (!matchTokenType(IDENTIFIER)) {
        hasError = true;
        errorMessage = "Ожидается идентификатор внутри 'read()'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Дополнительные идентификаторы через запятую (необязательные)
    while (matchTokenValue(",")) {
        if (!matchTokenType(IDENTIFIER)) {
            hasError = true;
            errorMessage = "Ожидается идентификатор после ','";
            errorLine = current_token.line;
            errorColumn = current_token.column;
            return;
        }
    }
    
    // Закрывающая скобка обязательна
    if (!matchTokenValue(")")) {
        hasError = true;
        errorMessage = "Ожидается ')' после списка идентификаторов";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    debugPrint("[DEBUG] readStatement() completed, current token: " + current_token.value);
}

/**
 * @brief P14: Оператор вывода - write(<выражение> {, <выражение> })
 */
void SyntacticAnalyzer::writeStatement() {
    usedRules.push_back("P14");
    debugPrint("[DEBUG] writeStatement() called, current token: " + current_token.value);
    
    // Сохраняем координаты для сообщений об ошибках
    int line = current_token.line;
    int column = current_token.column;
    
    if (!matchTokenValue("write")) {
        hasError = true;
        errorMessage = "Ожидается 'write'";
        errorLine = line;
        errorColumn = column;
        return;
    }
    
    // Открывающая скобка обязательна
    if (!matchTokenValue("(")) {
        hasError = true;
        errorMessage = "Ожидается '(' после 'write'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    // Первое выражение обязательно
    expression();
    
    if (hasError) return;
    
    // Дополнительные выражения через запятую (необязательные)
    while (matchTokenValue(",")) {
        expression();
        if (hasError) return;
    }
    
    // Закрывающая скобка обязательна
    if (!matchTokenValue(")")) {
        hasError = true;
        errorMessage = "Ожидается ')' после списка выражений";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    debugPrint("[DEBUG] writeStatement() completed, current token: " + current_token.value);
}

/**
 * @brief P15: Многострочные комментарии - (* *)
 * P27: Однострочные комментарии - '{' {<любой символ>} '}'
 */
void SyntacticAnalyzer::comment() {
    // Используем P27 для однострочных комментариев, P15 для многострочных
    if (current_token.value == "(*") {
        usedRules.push_back("P15");
    } else if (current_token.value == "{") {
        usedRules.push_back("P27");
    }
    
    debugPrint("[DEBUG] comment() called, current token: " + current_token.value);
    
    if (current_token.value == "(*") {
        // Многострочный комментарий
        matchTokenValue("(*");
        
        while (!matchTokenValue("*)")) {
            if (current_token.type == END_OF_FILE) {
                hasError = true;
                errorMessage = "Не закрыт комментарий '*)'";
                errorLine = current_token.line;
                errorColumn = current_token.column;
                return;
            }
            consumeToken();
        }
    }
    else if (current_token.value == "{") {
        // Однострочный комментарий (P27)
        matchTokenValue("{");
        
        while (!matchTokenValue("}")) {
            if (current_token.type == END_OF_FILE) {
                hasError = true;
                errorMessage = "Не закрыт комментарий '}'";
                errorLine = current_token.line;
                errorColumn = current_token.column;
                return;
            }
            consumeToken();
        }
    }
    else {
        hasError = true;
        errorMessage = "Ожидается начало комментария '(*' или '{'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    debugPrint("[DEBUG] comment() completed, current token: " + current_token.value);
}

/**
 * @brief P26: <оператор>::= <составной> | <присваивания> | <условный> | 
 *           <фиксированного_цикла> | <условного_цикла> | <ввода> | <вывода>
 * P8: Синтаксис составного оператора
 */
void SyntacticAnalyzer::statement() {
    usedRules.push_back("P26");
    string message = "[DEBUG] statement() called, current token: " + current_token.value +
                     " (type: " + current_token.getTypeString() + ")";
    debugPrint(message);
    
    // Проверяем следующий токен (смещение 1)
    Token nextToken = peekNextToken(1);
    message = "[DEBUG] Next token (peek 1): " + nextToken.value +
              " (type: " + nextToken.getTypeString() + ")";
    debugPrint(message);
    
    if (current_token.value == "dim") {
        debugPrint("[DEBUG] Found 'dim', calling description()");
        description();
    } 
    // Проверяем оператор присваивания: идентификатор + 'ass'
    else if (current_token.type == IDENTIFIER && nextToken.value == "ass") {
        message = "[DEBUG] Found assignment pattern: " + current_token.value + 
                  " + " + nextToken.value + ", calling assignment()";
        debugPrint(message);
        assignment();
    }
    else if (current_token.value == "if") {
        debugPrint("[DEBUG] Found 'if', calling conditional()");
        conditional();
    } else if (current_token.value == "for") {
        debugPrint("[DEBUG] Found 'for', calling forLoop()");
        forLoop();
    } else if (current_token.value == "while") {
        debugPrint("[DEBUG] Found 'while', calling whileLoop()");
        whileLoop();
    } else if (current_token.value == "read") {
        debugPrint("[DEBUG] Found 'read', calling readStatement()");
        readStatement();
    } else if (current_token.value == "write") {
        debugPrint("[DEBUG] Found 'write', calling writeStatement()");
        writeStatement();
    } else if (current_token.value == "(*" || current_token.value == "{") {
        debugPrint("[DEBUG] Found comment, calling comment()");
        comment();
    } else {
        // Простое выражение (может быть допустимо в некоторых контекстах)
        debugPrint("[DEBUG] Calling expression() as fallback");
        expression();
    }
    
    if (hasError) {
        message = "[DEBUG] Error in statement(): " + errorMessage +
                  " at line " + to_string(errorLine) + ", column " + to_string(errorColumn);
        debugPrint(message);
        return;
    }
    
    debugPrint("[DEBUG] statement() completed, current token: " + current_token.value);
}

/**
 * @brief P5: Структура программы - {/ (<описание> | <оператор>)(перевод строки) /} end
 */
void SyntacticAnalyzer::program() {
    usedRules.push_back("P5");
    debugPrint("[DEBUG] program() called, current token: " + current_token.value);
    
    // Обработка последовательности описаний и операторов
    while (current_token.type != END_OF_FILE && current_token.value != "end") {
        // Пропускаем переводы строк между операторов (часть P8)
        if (matchTokenValue("\n")) {
            usedRules.push_back("P8");
            debugPrint("[DEBUG] Skipping newline (P8)");
        }
        
        // Если после пропуска переводов строк мы нашли 'end', выходим
        if (current_token.value == "end") break;
        
        // Разбор очередного оператора или описания
        debugPrint("[DEBUG] Before statement(), current token: " + current_token.value);
        statement();
        
        if (hasError) {
            debugPrint("[DEBUG] Program stopped due to error");
            return;
        }
        
        // Пропускаем возможные переводы строк после оператора (часть P8)
        if (current_token.value == "\n") {
            usedRules.push_back("P8");
            debugPrint("[DEBUG] Skipping newline after statement (P8)");
            matchTokenValue("\n");
        }
        
        // Проверяем, не конец ли программы
        if (current_token.value == "end") break;
    }
    
    // Проверка обязательного 'end' в конце программы
    if (!matchTokenValue("end")) {
        hasError = true;
        errorMessage = "Ожидается 'end' в конце программы";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    debugPrint("[DEBUG] Found 'end' at end of program");
    debugPrint("[DEBUG] program() completed, current token: " + current_token.value);
}

void SyntacticAnalyzer::analyze() {
    cout << "=== НАЧАЛО СИНТАКСИЧЕСКОГО АНАЛИЗА ===" << endl;
    
    program();
    
    if (!hasError && current_token.type != END_OF_FILE) {
        hasError = true;
        errorMessage = "Лишние токены после 'end'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    cout << "=== ЗАВЕРШЕНИЕ СИНТАКСИЧЕСКОГО АНАЛИЗА ===" << endl;
}

void SyntacticAnalyzer::printResult() {
    cout << "\n=== РЕЗУЛЬТАТ СИНТАКСИЧЕСКОГО АНАЛИЗА ===" << endl;
    cout << "=========================================" << endl;
    
    if (hasError) {
        cout << "Обнаружены синтаксические ошибки" << endl;
        cout << errorMessage << endl;
        cout << "Строка " << errorLine << ", позиция " << errorColumn << endl;
    } else {
        cout << "Ошибки не обнаружены" << endl;
    }
    
    cout << "=========================================" << endl;
    cout << "Результат:" << endl;
    
    if (hasError) {
        cout << "Обнаружены синтаксические ошибки!" << endl;
    } else {
        cout << "Успешно" << endl;
    }
    
    cout << "=========================================" << endl;
    cout << "Сработавшие правила:" << endl;
    
    // Удаляем дубликаты правил для более чистого вывода
    vector<string> uniqueRules;
    for (const string& rule : usedRules) {
        if (uniqueRules.empty() || uniqueRules.back() != rule) {
            uniqueRules.push_back(rule);
        }
    }
    
    for (size_t i = 0; i < uniqueRules.size(); ++i) {
        if (i > 0) cout << "->";
        cout << uniqueRules[i];
    }
    cout << endl;
    
    cout << "=========================================" << endl;
}