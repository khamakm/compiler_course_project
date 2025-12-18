#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "../inc/LexicalAnalyzer.hpp"

using namespace std;

class SyntacticAnalyzer {
private:
    vector<Token> tokens;
    int currentPos;
    Token current_token;
    vector<string> usedRules;
    bool hasError;
    string errorMessage;
    int errorLine;
    int errorColumn;
    
    // Флаг для отладочного вывода
    static bool debugMode;

    // === Вспомогательные методы ===
    void updateCurrentToken();
    Token peekNextToken(int offset = 0);
    void consumeToken();
    bool matchToken(TokenType expectedType, const string& expectedValue = "");
    bool matchTokenValue(const string& expectedValue);
    bool matchTokenType(TokenType expectedType);
    bool isRelationalOperator();
    bool isAdditiveOperator();
    bool isMultiplicativeOperator();
    
    // Вспомогательный метод для отладочного вывода
    void debugPrint(const string& message);

    // === Новая грамматика выражений (P16-P25) ===
    /**
     * @brief P16: <выражение>::= <операнд>{<операции_группы_отношения> <операнд>}
     */
    void expression();
    
    /**
     * @brief P17: <операнд>::= <слагаемое> {<операции_группы_сложения> <слагаемое>}
     */
    void operand();
    
    /**
     * @brief P18: <слагаемое>::= <множитель> {<операции_группы_умножения> <множитель>}
     */
    void term();
    
    /**
     * @brief P19: <множитель>::= <идентификатор> | <число> | <логическая_константа> | 
     *           <унарная_операция> <множитель> | (<выражение>)
     */
    void factor();
    
    /**
     * @brief P20: <унарная_операция>::='~'
     */
    void unaryOperation();
    
    /**
     * @brief P21: <число>::= <целое> | <действительное>
     * (обрабатывается лексическим анализатором)
     */
    void number();
    
    /**
     * @brief P22: <логическая_константа>::= true | false
     */
    void booleanConstant();
    
    /**
     * @brief P23: <идентификатор>::= <буква> {<буква> | <цифра>}
     * (обрабатывается лексическим анализатором)
     */

    // === Основные правила грамматики (P1-P15, P26-P27) ===
    /**
     * @brief P5: Структура программы - {/ (<описание> | <оператор>)(перевод строки) /} end
     */
    void program();
    
    /**
     * @brief P6: Описание данных - dim <идентификатор> {, <идентификатор> } <тип>
     */
    void description();
    
    /**
     * @brief P26: <оператор>::= <составной> | <присваивания> | <условный> | 
     *           <фиксированного_цикла> | <условного_цикла> | <ввода> | <вывода>
     * P8: Синтаксис составного оператора
     */
    void statement();
    
    /**
     * @brief P9: Оператор присваивания - <идентификатор> ass <выражение>
     */
    void assignment();
    
    /**
     * @brief P10: Условный оператор - if <выражение> then <оператор> [else <оператор>]
     */
    void conditional();
    
    /**
     * @brief P11: Цикл for - for <присваивания> to <выражение> do <оператор>
     */
    void forLoop();
    
    /**
     * @brief P12: Цикл while - while <выражение> do <оператор>
     */
    void whileLoop();
    
    /**
     * @brief P13: Оператор ввода - read(идентификатор {, <идентификатор> })
     */
    void readStatement();
    
    /**
     * @brief P14: Оператор вывода - write(<выражение> {, <выражение> })
     */
    void writeStatement();
    
    /**
     * @brief P15: Многострочные комментарии - (* *)
     * P27: Однострочные комментарии - '{' {<любой символ>} '}'
     */
    void comment();

public:
    SyntacticAnalyzer(const vector<Token>& tokenList);
    void analyze();
    void printResult();
    
    // Статические методы для управления отладочным выводом
    static void enableDebugMode() { debugMode = true; }
    static void disableDebugMode() { debugMode = false; }
    static bool isDebugMode() { return debugMode; }
};
