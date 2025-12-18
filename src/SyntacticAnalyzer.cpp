#include "../inc/SyntacticAnalyzer.hpp"
#include <iostream>

SyntacticAnalyzer::SyntacticAnalyzer(const vector<Token>& tokenList) 
    : tokens(tokenList), currentPos(0), hasError(false) {
    if (!tokens.empty()) {
        current_token = tokens[0];
    } else {
        current_token = Token(END_OF_FILE, "", 0, 0);
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
    // Изменено с "mult" на "mul" согласно P3
    return current_token.value == "mul" || current_token.value == "div" || 
           current_token.value == "and";
}

// === Новая грамматика выражений (P16-P25) ===

/**
 * @brief P22: <логическая_константа>::= true | false
 */
void SyntacticAnalyzer::booleanConstant() {
    cout << "[DEBUG] booleanConstant() called, current token: " << current_token.value << endl;
    
    if (current_token.value == "true" || current_token.value == "false") {
        cout << "[DEBUG] Consuming boolean constant: " << current_token.value << endl;
        consumeToken();
    } else {
        hasError = true;
        errorMessage = "Ожидается логическая константа (true или false)";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    cout << "[DEBUG] booleanConstant() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P21: <число>::= <целое> | <действительное>
 */
void SyntacticAnalyzer::number() {
    cout << "[DEBUG] number() called, current token: " << current_token.value << endl;
    
    if (current_token.type == INT_CONST || current_token.type == FLOAT_CONST ||
        current_token.type == OCT_CONST || current_token.type == HEX_CONST) {
        cout << "[DEBUG] Consuming number: " << current_token.value << endl;
        consumeToken();
    } else {
        hasError = true;
        errorMessage = "Ожидается число";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    cout << "[DEBUG] number() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P20: <унарная_операция>::='~'
 */
void SyntacticAnalyzer::unaryOperation() {
    cout << "[DEBUG] unaryOperation() called, current token: " << current_token.value << endl;
    
    if (current_token.value == "~") {
        cout << "[DEBUG] Found unary operator: ~" << endl;
        consumeToken();
    } else {
        hasError = true;
        errorMessage = "Ожидается унарный оператор ~";
        errorLine = current_token.line;
        errorColumn = current_token.column;
    }
    
    cout << "[DEBUG] unaryOperation() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P19: <множитель>::= <идентификатор> | <число> | <логическая_константа> | 
 *           <унарная_операция> <множитель> | (<выражение>)
 */
void SyntacticAnalyzer::factor() {
    cout << "[DEBUG] factor() called, current token: " << current_token.value << endl;
    
    // Унарная операция
    if (current_token.value == "~") {
        cout << "[DEBUG] Processing unary operation" << endl;
        unaryOperation();
        factor();
    }
    // Выражение в скобках
    else if (current_token.value == "(") {
        cout << "[DEBUG] Found '(' starting parenthesized expression" << endl;
        consumeToken();
        expression();
        if (!matchTokenValue(")")) {
            hasError = true;
            errorMessage = "Ожидается ')'";
            errorLine = current_token.line;
            errorColumn = current_token.column;
        }
    }
    // Идентификатор
    else if (current_token.type == IDENTIFIER) {
        cout << "[DEBUG] Consuming identifier: " << current_token.value << endl;
        consumeToken();
    }
    // Число
    else if (current_token.type == INT_CONST || current_token.type == FLOAT_CONST ||
             current_token.type == OCT_CONST || current_token.type == HEX_CONST) {
        cout << "[DEBUG] Consuming number: " << current_token.value << endl;
        consumeToken();
    }
    // Логическая константа
    else if (current_token.value == "true" || current_token.value == "false") {
        cout << "[DEBUG] Consuming boolean constant: " << current_token.value << endl;
        consumeToken();
    }
    // Ошибка
    else {
        hasError = true;
        errorMessage = "Ожидается множитель (идентификатор, число, логическая константа или '(')";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        cout << "[DEBUG] Error in factor(): " << errorMessage << endl;
    }
    
    cout << "[DEBUG] factor() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P18: <слагаемое>::= <множитель> {<операции_группы_умножения> <множитель>}
 */
void SyntacticAnalyzer::term() {
    cout << "[DEBUG] term() called, current token: " << current_token.value << endl;
    
    factor();
    
    if (hasError) return;
    
    // Обработка операторов умножения
    while (isMultiplicativeOperator()) {
        string op = current_token.value;
        cout << "[DEBUG] Found multiplicative operator: " << op << endl;
        consumeToken();
        factor();
        
        if (hasError) return;
    }
    
    cout << "[DEBUG] term() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P17: <операнд>::= <слагаемое> {<операции_группы_сложения> <слагаемое>}
 */
void SyntacticAnalyzer::operand() {
    cout << "[DEBUG] operand() called, current token: " << current_token.value << endl;
    
    term();
    
    if (hasError) return;
    
    // Обработка операторов сложения
    while (isAdditiveOperator()) {
        string op = current_token.value;
        cout << "[DEBUG] Found additive operator: " << op << endl;
        consumeToken();
        term();
        
        if (hasError) return;
    }
    
    cout << "[DEBUG] operand() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P16: <выражение>::= <операнд>{<операции_группы_отношения> <операнд>}
 */
void SyntacticAnalyzer::expression() {
    cout << "[DEBUG] expression() called, current token: " << current_token.value 
         << " (line: " << current_token.line << ")" << endl;
    usedRules.push_back("P16");
    
    operand();
    
    if (hasError) return;
    
    // Обработка операторов отношения
    while (isRelationalOperator()) {
        string op = current_token.value;
        cout << "[DEBUG] Found relational operator: " << op << endl;
        consumeToken();
        operand();
        
        if (hasError) return;
    }
    
    cout << "[DEBUG] expression() completed, current token: " << current_token.value << endl;
}

// === Основные правила грамматики ===

/**
 * @brief P6: Описание данных - dim <идентификатор> {, <идентификатор> } <тип>
 */
void SyntacticAnalyzer::description() {
    usedRules.push_back("P6");
    cout << "[DEBUG] description() called, current token: " << current_token.value << endl;
    
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
    
    cout << "[DEBUG] description() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P9: Оператор присваивания - <идентификатор> ass <выражение>
 */
void SyntacticAnalyzer::assignment() {
    usedRules.push_back("P9");
    cout << "[DEBUG] assignment() called, current token: " << current_token.value << endl;
    
    // Сохраняем идентификатор
    string identifier = current_token.value;
    
    if (!matchTokenType(IDENTIFIER)) {
        hasError = true;
        errorMessage = "Ожидается идентификатор перед 'ass'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    cout << "[DEBUG] Consumed identifier: " << identifier << endl;
    
    if (!matchTokenValue("ass")) {
        hasError = true;
        errorMessage = "Ожидается 'ass'";
        errorLine = current_token.line;
        errorColumn = current_token.column;
        return;
    }
    
    cout << "[DEBUG] Consumed 'ass', now parsing expression" << endl;
    
    // Правая часть присваивания - выражение (P16)
    expression();
    
    cout << "[DEBUG] assignment() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P10: Условный оператор - if <выражение> then <оператор> [else <оператор>]
 */
void SyntacticAnalyzer::conditional() {
    usedRules.push_back("P10");
    cout << "[DEBUG] conditional() called, current token: " << current_token.value << endl;
    
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
    
    cout << "[DEBUG] Before then-block statement(), current token: " << current_token.value << endl;
    
    // Блок then
    statement();
    
    // Необязательный блок else
    if (matchTokenValue("else")) {
        cout << "[DEBUG] Found 'else', processing else-block" << endl;
        statement();
    }
    
    cout << "[DEBUG] conditional() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P11: Цикл for - for <присваивания> to <выражение> do <оператор>
 */
void SyntacticAnalyzer::forLoop() {
    usedRules.push_back("P11");
    cout << "[DEBUG] forLoop() called, current token: " << current_token.value << endl;
    
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
    
    cout << "[DEBUG] forLoop() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P12: Цикл while - while <выражение> do <оператор>
 */
void SyntacticAnalyzer::whileLoop() {
    usedRules.push_back("P12");
    cout << "[DEBUG] whileLoop() called, current token: " << current_token.value << endl;
    
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
    
    cout << "[DEBUG] whileLoop() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P13: Оператор ввода - read(идентификатор {, <идентификатор> })
 */
void SyntacticAnalyzer::readStatement() {
    usedRules.push_back("P13");
    cout << "[DEBUG] readStatement() called, current token: " << current_token.value << endl;
    
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
    
    cout << "[DEBUG] readStatement() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P14: Оператор вывода - write(<выражение> {, <выражение> })
 */
void SyntacticAnalyzer::writeStatement() {
    usedRules.push_back("P14");
    cout << "[DEBUG] writeStatement() called, current token: " << current_token.value << endl;
    
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
    
    cout << "[DEBUG] writeStatement() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P15: Многострочные комментарии - (* *)
 * P27: Однострочные комментарии - '{' {<любой символ>} '}'
 */
void SyntacticAnalyzer::comment() {
    usedRules.push_back("P15");
    cout << "[DEBUG] comment() called, current token: " << current_token.value << endl;
    
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
    
    cout << "[DEBUG] comment() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P26: <оператор>::= <составной> | <присваивания> | <условный> | 
 *           <фиксированного_цикла> | <условного_цикла> | <ввода> | <вывода>
 * P8: Синтаксис составного оператора
 */
void SyntacticAnalyzer::statement() {
    usedRules.push_back("P8");
    cout << "[DEBUG] statement() called, current token: " << current_token.value 
         << " (type: " << current_token.getTypeString() << ")" << endl;
    
    // Проверяем следующий токен (смещение 1)
    Token nextToken = peekNextToken(1);
    cout << "[DEBUG] Next token (peek 1): " << nextToken.value 
         << " (type: " << nextToken.getTypeString() << ")" << endl;
    
    if (current_token.value == "dim") {
        cout << "[DEBUG] Found 'dim', calling description()" << endl;
        description();
    } 
    // Проверяем оператор присваивания: идентификатор + 'ass'
    else if (current_token.type == IDENTIFIER && nextToken.value == "ass") {
        cout << "[DEBUG] Found assignment pattern: " << current_token.value 
             << " + " << nextToken.value << ", calling assignment()" << endl;
        assignment();
    }
    else if (current_token.value == "if") {
        cout << "[DEBUG] Found 'if', calling conditional()" << endl;
        conditional();
    } else if (current_token.value == "for") {
        cout << "[DEBUG] Found 'for', calling forLoop()" << endl;
        forLoop();
    } else if (current_token.value == "while") {
        cout << "[DEBUG] Found 'while', calling whileLoop()" << endl;
        whileLoop();
    } else if (current_token.value == "read") {
        cout << "[DEBUG] Found 'read', calling readStatement()" << endl;
        readStatement();
    } else if (current_token.value == "write") {
        cout << "[DEBUG] Found 'write', calling writeStatement()" << endl;
        writeStatement();
    } else if (current_token.value == "(*" || current_token.value == "{") {
        cout << "[DEBUG] Found comment, calling comment()" << endl;
        comment();
    } else {
        // Простое выражение (может быть допустимо в некоторых контекстах)
        cout << "[DEBUG] Calling expression() as fallback" << endl;
        expression();
    }
    
    if (hasError) {
        cout << "[DEBUG] Error in statement(): " << errorMessage 
             << " at line " << errorLine << ", column " << errorColumn << endl;
        return;
    }
    
    cout << "[DEBUG] statement() completed, current token: " << current_token.value << endl;
}

/**
 * @brief P5: Структура программы - {/ (<описание> | <оператор>)(перевод строки) /} end
 */
void SyntacticAnalyzer::program() {
    usedRules.push_back("P5");
    cout << "[DEBUG] program() called, current token: " << current_token.value << endl;
    
    // Обработка последовательности описаний и операторов
    while (current_token.type != END_OF_FILE && current_token.value != "end") {
        // Пропускаем переводы строк между операторами
        while (matchTokenValue("\n")) {
            cout << "[DEBUG] Skipping newline" << endl;
        }
        
        // Если после пропуска переводов строк мы нашли 'end', выходим
        if (current_token.value == "end") break;
        
        // Разбор очередного оператора или описания
        cout << "[DEBUG] Before statement(), current token: " << current_token.value << endl;
        statement();
        
        if (hasError) {
            cout << "[DEBUG] Program stopped due to error" << endl;
            return;
        }
        
        // Пропускаем возможные переводы строк после оператора
        while (matchTokenValue("\n")) {
            cout << "[DEBUG] Skipping newline after statement" << endl;
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
    
    cout << "[DEBUG] Found 'end' at end of program" << endl;
    cout << "[DEBUG] program() completed, current token: " << current_token.value << endl;
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
    
    for (size_t i = 0; i < usedRules.size(); ++i) {
        if (i > 0) cout << "->";
        cout << usedRules[i];
    }
    cout << endl;
    
    cout << "=========================================" << endl;
}