#pragma once

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <iostream>
#include "../inc/LexicalAnalyzer.hpp"
#include "../inc/SyntacticAnalyzer.hpp"
#include "../inc/Optimizer.hpp"

using namespace std;

class TriadGenerator {
private:
    vector<Triad> triads;
    vector<Token> tokens;
    size_t current_pos;
    int temp_counter;
    int label_counter;
    int triad_id_counter;
    
    // Флаг ошибок
    bool has_error;
    string error_message;
    int error_line;
    int error_column;
    
    // Таблицы для семантического анализа
    map<string, string> var_types;
    map<string, bool> is_const;
    map<string, int> var_values;
    set<string> declared_vars;
    
    // Вспомогательные функции
    Token getCurrentToken() const {
        if (current_pos < tokens.size()) return tokens[current_pos];
        return Token(END_OF_FILE, "", 0, 0);
    }
    
    Token peekNextToken(int offset = 1) const {
        size_t peek_pos = current_pos + offset;
        if (peek_pos < tokens.size()) return tokens[peek_pos];
        return Token(END_OF_FILE, "", 0, 0);
    }
    
    bool match(const string& value) {
        if (getCurrentToken().value == value) {
            advance();
            return true;
        }
        return false;
    }
    
    bool matchType(TokenType type) {
        if (getCurrentToken().type == type) {
            advance();
            return true;
        }
        return false;
    }
    
    void advance() {
        if (current_pos < tokens.size()) current_pos++;
    }
    
    // Установка ошибки
    void setError(const string& message) {
        has_error = true;
        error_message = message;
        error_line = getCurrentToken().line;
        error_column = getCurrentToken().column;
        cout << "! ОШИБКА ГЕНЕРАЦИИ: " << message 
             << " (строка " << error_line 
             << ", позиция " << error_column << ")" << endl;
    }
    
    // Генерация новых имен
    string newTemp() {
        return "t" + to_string(++temp_counter);
    }
    
    string newLabel() {
        return "L" + to_string(++label_counter);
    }
    
    // Добавление триады
    void addTriad(const string& op, const string& arg1, const string& arg2, 
                  const string& result, bool temp = false) {
        triads.push_back(Triad(++triad_id_counter, op, arg1, arg2, result, temp));
    }
    
    // Проверка и добавление переменной
    bool checkVariable(const string& var_name) {
        if (declared_vars.find(var_name) == declared_vars.end()) {
            has_error = true;
            cout << "! ОШИБКА: Переменная '" << var_name << "' не объявлена" 
                 << " (строка " << getCurrentToken().line << ")" << endl;
            // Автоматическое объявление для демонстрации
            declared_vars.insert(var_name);
            var_types[var_name] = "integer";
        }
        return true;
    }
    
public:
    TriadGenerator(const vector<Token>& token_list) 
        : tokens(token_list), current_pos(0), temp_counter(0), 
          label_counter(0), triad_id_counter(0), has_error(false) {}
    
    // Проверка наличия ошибок
    bool hasError() const {
        return has_error;
    }
    
    string getErrorMessage() const {
        return error_message;
    }
    
    pair<int, int> getErrorPosition() const {
        return make_pair(error_line, error_column);
    }
    
    // Основной метод генерации
    vector<Triad> generate() {
        triads.clear();
        has_error = false;
        
        cout << "Начинаем генерацию триад..." << endl;
        
        try {
            generateProgram();
            
            if (has_error) {
                cout << "Генерация прервана из-за ошибок" << endl;
                return vector<Triad>();
            }
            
            cout << "Сгенерировано " << triads.size() << " триад" << endl;
            return triads;
        }
        catch (const exception& e) {
            setError("Исключение при генерации: " + string(e.what()));
            return vector<Triad>();
        }
    }
    
    // P5: Программа
    void generateProgram() {
        // Обработка описаний и операторов
        while (getCurrentToken().type != END_OF_FILE && 
               getCurrentToken().value != "end") {
            
            if (has_error) return;
            
            // Пропуск переводов строк
            if (match("\n")) continue;
            
            // Разбор оператора или описания
            generateStatement();
            
            // Пропуск переводов строк после оператора
            if (match("\n")) continue;
        }
        
        if (has_error) return;
        
        // Конец программы
        if (match("end")) {
            addTriad("halt", "", "", "");
            cout << "Обнаружен конец программы (end)" << endl;
        } else {
            setError("Ожидается 'end' в конце программы");
            return;
        }
    }
    
    // P26: Оператор
    void generateStatement() {
        if (has_error) return;
        
        Token current = getCurrentToken();
        Token next = peekNextToken();
        
        cout << "Генерация оператора: " << current.value 
             << " (тип: " << current.getTypeString() << ")" << endl;
        
        if (current.value == "dim") {
            generateDescription();
        }
        else if (current.type == IDENTIFIER && next.value == "ass") {
            generateAssignment();
        }
        else if (current.value == "if") {
            generateConditional();
        }
        else if (current.value == "for") {
            generateForLoop();
        }
        else if (current.value == "while") {
            generateWhileLoop();
        }
        else if (current.value == "read") {
            generateReadStatement();
        }
        else if (current.value == "write") {
            generateWriteStatement();
        }
        else if (current.value == "{" || current.value == "(*") {
            generateComment();
        }
        else {
            // Простое выражение
            string result = generateExpression();
            if (!result.empty() && result[0] == 't') {
                addTriad("=", result, "", "_discard");
            }
        }
    }
    
    // P6: Описание данных
    void generateDescription() {
        if (has_error) return;
        
        if (!match("dim")) {
            setError("Ожидается 'dim'");
            return;
        }
        
        vector<string> vars;
        
        // Первый идентификатор обязателен
        if (matchType(IDENTIFIER)) {
            string var = tokens[current_pos - 1].value;
            vars.push_back(var);
            declared_vars.insert(var);
            
            // Дополнительные идентификаторы
            while (match(",")) {
                if (matchType(IDENTIFIER)) {
                    var = tokens[current_pos - 1].value;
                    vars.push_back(var);
                    declared_vars.insert(var);
                } else {
                    setError("Ожидается идентификатор после ','");
                    return;
                }
            }
        } else {
            setError("Ожидается идентификатор после 'dim'");
            return;
        }
        
        // Тип данных (P7)
        string type;
        if (match("integer")) type = "integer";
        else if (match("real")) type = "real";
        else if (match("boolean")) type = "boolean";
        else {
            setError("Ожидается тип (integer, real, boolean)");
            return;
        }
        
        // Генерация триад для объявления переменных
        for (const auto& var : vars) {
            var_types[var] = type;
            addTriad("dim", type, "", var);
            cout << "Объявлена переменная: " << var << " типа " << type << endl;
        }
    }
    
    // P9: Присваивание
    void generateAssignment() {
        if (has_error) return;
        
        string var_name = getCurrentToken().value;
        
        if (!checkVariable(var_name)) {
            return;
        }
        
        // Проверка константности
        if (is_const[var_name]) {
            setError("Попытка изменения константы '" + var_name + "'");
            return;
        }
        
        advance(); // Пропускаем идентификатор
        
        if (!match("ass")) {
            setError("Ожидается 'ass'");
            return;
        }
        
        string expr_result = generateExpression();
        if (expr_result.empty()) {
            setError("Пустое выражение в присваивании");
            return;
        }
        
        addTriad("=", expr_result, "", var_name);
        cout << "Присваивание: " << var_name << " = " << expr_result << endl;
    }
    
    // P10: Условный оператор
    void generateConditional() {
        if (has_error) return;
        
        if (!match("if")) {
            setError("Ожидается 'if'");
            return;
        }
        
        string condition_result = generateExpression();
        if (condition_result.empty()) {
            setError("Пустое условие в if");
            return;
        }
        
        if (!match("then")) {
            setError("Ожидается 'then'");
            return;
        }
        
        // Метка для else-блока (если есть)
        string else_label = newLabel();
        // Метка для конца if
        string end_label = newLabel();
        
        // Условный переход на else-блок если условие ложно
        addTriad("if", condition_result, else_label, "");
        cout << "Условный переход: IF " << condition_result << " GOTO " << else_label << endl;
        
        // Then-блок
        generateStatement();
        
        if (has_error) return;
        
        // Безусловный переход в конец
        addTriad("goto", end_label, "", "");
        
        // Метка else-блока
        addTriad("label", else_label, "", "");
        
        // Проверяем наличие else
        if (match("else")) {
            generateStatement();
            if (has_error) return;
        }
        
        // Метка конца if
        addTriad("label", end_label, "", "");
        
        cout << "Сгенерирован условный оператор if" << endl;
    }
    
    // P11: Цикл for
    void generateForLoop() {
        if (has_error) return;
        
        if (!match("for")) {
            setError("Ожидается 'for'");
            return;
        }
        
        string start_label = newLabel();
        string end_label = newLabel();
        
        cout << "Генерация цикла for" << endl;
        
        // Инициализация счетчика
        generateAssignment();
        if (has_error) return;
        
        if (!match("to")) {
            setError("Ожидается 'to'");
            return;
        }
        
        string limit_result = generateExpression();
        if (limit_result.empty()) {
            setError("Пустое выражение в for");
            return;
        }
        
        // Метка начала цикла
        addTriad("label", start_label, "", "");
        
        // Проверка условия (счетчик <= предел)
        string counter_var = tokens[current_pos - 3].value; // Имя счетчика
        string temp_compare = newTemp();
        addTriad("<=", counter_var, limit_result, temp_compare);
        addTriad("if", temp_compare, end_label, "");
        
        if (!match("do")) {
            setError("Ожидается 'do'");
            return;
        }
        
        // Тело цикла
        generateStatement();
        if (has_error) return;
        
        // Инкремент счетчика
        string temp_inc = newTemp();
        addTriad("+", counter_var, "1", temp_inc);
        addTriad("=", temp_inc, "", counter_var);
        
        // Возврат к проверке условия
        addTriad("goto", start_label, "", "");
        
        // Метка выхода из цикла
        addTriad("label", end_label, "", "");
        
        cout << "Сгенерирован цикл for" << endl;
    }
    
    // P12: Цикл while
    void generateWhileLoop() {
        if (has_error) return;
        
        if (!match("while")) {
            setError("Ожидается 'while'");
            return;
        }
        
        string start_label = newLabel();
        string end_label = newLabel();
        
        cout << "Генерация цикла while" << endl;
        
        // Метка начала цикла
        addTriad("label", start_label, "", "");
        
        // Условие цикла
        string condition_result = generateExpression();
        if (condition_result.empty()) {
            setError("Пустое условие в while");
            return;
        }
        
        // Проверка условия (выход если ложно)
        addTriad("if", condition_result, end_label, "");
        
        if (!match("do")) {
            setError("Ожидается 'do'");
            return;
        }
        
        // Тело цикла
        generateStatement();
        if (has_error) return;
        
        // Возврат к проверке условия
        addTriad("goto", start_label, "", "");
        
        // Метка выхода из цикла
        addTriad("label", end_label, "", "");
        
        cout << "Сгенерирован цикл while" << endl;
    }
    
    // P13: Оператор ввода
    void generateReadStatement() {
        if (has_error) return;
        
        if (!match("read")) {
            setError("Ожидается 'read'");
            return;
        }
        
        if (!match("(")) {
            setError("Ожидается '(' после 'read'");
            return;
        }
        
        // Первый идентификатор
        if (matchType(IDENTIFIER)) {
            string var = tokens[current_pos - 1].value;
            checkVariable(var);
            addTriad("read", "", "", var);
            cout << "Read: " << var << endl;
        } else {
            setError("Ожидается идентификатор в read()");
            return;
        }
        
        // Дополнительные идентификаторы
        while (match(",")) {
            if (matchType(IDENTIFIER)) {
                string var = tokens[current_pos - 1].value;
                checkVariable(var);
                addTriad("read", "", "", var);
            } else {
                setError("Ожидается идентификатор после ','");
                return;
            }
        }
        
        if (!match(")")) {
            setError("Ожидается ')'");
            return;
        }
        
        cout << "Сгенерирован оператор read" << endl;
    }
    
    // P14: Оператор вывода
    void generateWriteStatement() {
        if (has_error) return;
        
        if (!match("write")) {
            setError("Ожидается 'write'");
            return;
        }
        
        if (!match("(")) {
            setError("Ожидается '(' после 'write'");
            return;
        }
        
        // Первое выражение
        string expr_result = generateExpression();
        if (expr_result.empty()) {
            setError("Пустое выражение в write");
            return;
        }
        
        addTriad("write", expr_result, "", "");
        
        // Дополнительные выражения
        while (match(",")) {
            expr_result = generateExpression();
            if (expr_result.empty()) {
                setError("Пустое выражение в write");
                return;
            }
            addTriad("write", expr_result, "", "");
        }
        
        if (!match(")")) {
            setError("Ожидается ')'");
            return;
        }
        
        cout << "Сгенерирован оператор write" << endl;
    }
    
    // P27/P15: Комментарий
    void generateComment() {
        if (has_error) return;
        
        if (match("{")) {
            while (!match("}") && getCurrentToken().type != END_OF_FILE) {
                advance();
            }
            if (getCurrentToken().type == END_OF_FILE) {
                setError("Не закрыт комментарий '}'");
            }
        }
        else if (match("(*")) {
            while (!match("*)") && getCurrentToken().type != END_OF_FILE) {
                advance();
            }
            if (getCurrentToken().type == END_OF_FILE) {
                setError("Не закрыт комментарий '*)'");
            }
        }
        
        cout << "Пропущен комментарий" << endl;
    }
    
    // P16-P23: Выражения
    string generateExpression() {
        if (has_error) return "";
        return generateRelational();
    }
    
    // P16: Операции отношения
    string generateRelational() {
        if (has_error) return "";
        
        string left = generateOperand();
        if (left.empty()) {
            setError("Пустой левый операнд в операции отношения");
            return "";
        }
        
        while (true) {
            Token t = getCurrentToken();
            if (t.value == "EQ" || t.value == "NE" || t.value == "LT" || 
                t.value == "LE" || t.value == "GT" || t.value == "GE") {
                string op = t.value;
                advance();
                
                string right = generateOperand();
                if (right.empty()) {
                    setError("Пустой правый операнд в операции отношения");
                    return "";
                }
                
                string temp = newTemp();
                addTriad(op, left, right, temp, true);
                left = temp;
            } else {
                break;
            }
        }
        return left;
    }
    
    // P17: Операнд (операции сложения)
    string generateOperand() {
        if (has_error) return "";
        
        string left = generateTerm();
        if (left.empty()) {
            setError("Пустой левый операнд в операции сложения");
            return "";
        }
        
        while (true) {
            Token t = getCurrentToken();
            if (t.value == "plus" || t.value == "min" || t.value == "or") {
                string op = t.value;
                advance();
                
                string right = generateTerm();
                if (right.empty()) {
                    setError("Пустой правый операнд в операции сложения");
                    return "";
                }
                
                string temp = newTemp();
                addTriad(op, left, right, temp, true);
                left = temp;
            } else {
                break;
            }
        }
        return left;
    }
    
    // P18: Слагаемое (операции умножения)
    string generateTerm() {
        if (has_error) return "";
        
        string left = generateFactor();
        if (left.empty()) {
            setError("Пустой левый операнд в операции умножения");
            return "";
        }
        
        while (true) {
            Token t = getCurrentToken();
            if (t.value == "mult" || t.value == "div" || t.value == "and") {
                string op = t.value;
                advance();
                
                string right = generateFactor();
                if (right.empty()) {
                    setError("Пустой правый операнд в операции умножения");
                    return "";
                }
                
                string temp = newTemp();
                addTriad(op, left, right, temp, true);
                left = temp;
            } else {
                break;
            }
        }
        return left;
    }
    
    // P19: Множитель
    string generateFactor() {
        if (has_error) return "";
        
        Token t = getCurrentToken();
        
        // Унарная операция (P20)
        if (t.value == "~") {
            advance();
            string operand = generateFactor();
            if (operand.empty()) {
                setError("Пустой операнд для унарной операции");
                return "";
            }
            
            string temp = newTemp();
            addTriad("~", operand, "", temp, true);
            return temp;
        }
        // Выражение в скобках
        else if (t.value == "(") {
            advance();
            string result = generateExpression();
            if (result.empty()) {
                setError("Пустое выражение в скобках");
                return "";
            }
            
            if (!match(")")) {
                setError("Ожидается ')'");
                return "";
            }
            return result;
        }
        // Идентификатор (P23)
        else if (t.type == IDENTIFIER) {
            string var_name = t.value;
            checkVariable(var_name);
            advance();
            return var_name;
        }
        // Число (P21)
        else if (t.type == INT_CONST || t.type == FLOAT_CONST || 
                 t.type == OCT_CONST || t.type == HEX_CONST) {
            string value = t.value;
            advance();
            return value;
        }
        // Логическая константа (P22)
        else if (t.value == "true" || t.value == "false") {
            string value = (t.value == "true") ? "1" : "0";
            advance();
            return value;
        }
        else {
            setError("Неожиданный токен в выражении: " + t.value);
            return "";
        }
    }
    
    // Статический метод для вывода триад
    static void printTriads(const vector<Triad>& triads, const string& title) {
        if (triads.empty()) {
            cout << "\n!  " << title << ": пусто" << endl;
            return;
        }
        
        cout << "\n=== " << title << " ===" << endl;
        cout << "ID\tТриада" << endl;
        cout << "-----------------------" << endl;
        
        for (const auto& triad : triads) {
            cout << triad.id << ":\t" << triad.toString() << endl;
        }
        
        cout << "=======================" << endl;
    }
};
