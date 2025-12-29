#pragma once

#include <vector>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include "../inc/LexicalAnalyzer.hpp"
#include "../inc/SyntacticAnalyzer.hpp"

using namespace std;

struct Triad {
    int id;
    string operation;
    string operand1;
    string operand2;
    string result;
    bool is_temporary;
    bool active;
    
    Triad(int id, string op, string arg1, string arg2, string res, bool temp = false) 
        : id(id), operation(op), operand1(arg1), operand2(arg2), result(res), 
          is_temporary(temp), active(true) {}
    
    // Проверка на тривиальное присваивание: x = x
    bool isTrivial() const {
        return operation == "=" && operand1 == result && operand2.empty();
    }
    
    // Проверка на бессмысленную арифметику (операции с 0 или 1)
    bool isMeaninglessArithmetic() const {
        // Используем операторы вашего языка
        if (operation == "plus" || operation == "min" || operation == "mult" || operation == "div") {
            return operand2 == "0" || operand2 == "1";
        }
        return false;
    }
    
    // Генерация строкового представления
    string toString() const {
        if (!active) return "УДАЛЕНА";
        
        if (operation == "=") {
            return result + " = " + operand1;
        }
        else if (operation == "if") {
            return "IF " + operand1 + " GOTO " + operand2;
        }
        else if (operation == "goto") {
            return "GOTO " + operand1;
        }
        else if (operation == "label") {
            return operand1 + ":";
        }
        else if (operation == "read") {
            return "READ " + result;
        }
        else if (operation == "write") {
            return "WRITE " + operand1;
        }
        else if (operation == "dim") {
            return "DIM " + result + " " + operand1;
        }
        else if (operation == "halt") {
            return "HALT";
        }
        else {
            // Преобразуем операторы для читаемого вывода
            string readable_op = operation;
            if (operation == "plus") readable_op = "+";
            else if (operation == "min") readable_op = "-";
            else if (operation == "mult") readable_op = "*";
            else if (operation == "div") readable_op = "/";
            else if (operation == "and") readable_op = "&";
            else if (operation == "or") readable_op = "|";
            else if (operation == "EQ") readable_op = "==";
            else if (operation == "NE") readable_op = "!=";
            else if (operation == "LT") readable_op = "<";
            else if (operation == "LE") readable_op = "<=";
            else if (operation == "GT") readable_op = ">";
            else if (operation == "GE") readable_op = ">=";
            
            return result + " = " + operand1 + " " + readable_op + " " + operand2;
        }
    }
};

class TriadOptimizer {
private:
    vector<Triad> triads;
    vector<string> optimization_log;
    bool has_error;
    
    // Проверка, является ли строка числом
    bool isNumber(const string& s) const {
        if (s.empty()) return false;
        
        // Проверка для десятичных чисел
        if (isdigit(s[0]) || (s[0] == '-' && s.length() > 1 && isdigit(s[1]))) {
            for (char c : s) {
                if (!isdigit(c) && c != '-') return false;
            }
            return true;
        }
        
        // Проверка для true/false
        if (s == "true" || s == "false") return true;
        
        return false;
    }
    
    // Преобразование строки в число
    int toNumber(const string& s) const {
        if (s == "true") return 1;
        if (s == "false") return 0;
        
        try {
            return stoi(s);
        } catch (...) {
            return 0;
        }
    }
    
    // Преобразование числа в строку
    string numberToString(int n) const {
        return to_string(n);
    }
    
    // Удаление триады с логированием
    void removeTriad(int index, const string& reason) {
        if (index < 0 || index >= triads.size() || !triads[index].active) return;
        
        optimization_log.push_back("Триада " + to_string(triads[index].id) + 
                                  " удалена: " + reason);
        triads[index].active = false;
    }
    
    // Проверка использования результата триады после указанной позиции
    bool isResultUsed(const string& result, int after_index) {
        if (result.empty()) return true;
        
        for (size_t i = after_index + 1; i < triads.size(); i++) {
            if (!triads[i].active) continue;
            
            if (triads[i].operand1 == result || triads[i].operand2 == result) {
                return true;
            }
            
            // Если это присваивание в ту же переменную, использование прекращается
            if (triads[i].operation == "=" && triads[i].result == result) {
                return false;
            }
        }
        return false;
    }
    
    // Оптимизация арифметических операций
    void optimizeArithmetic(int index) {
        if (has_error) return;
        if (index < 0 || index >= triads.size() || !triads[index].active) return;
        
        // 1. Удаление тривиальных присваиваний: x = x
        if (triads[index].isTrivial()) {
            removeTriad(index, "тривиальное присваивание");
            return;
        }
        
        // 2. Упрощение арифметических операций
        if (triads[index].isMeaninglessArithmetic()) {
            // plus (x + 0 → x)
            if (triads[index].operation == "plus" && triads[index].operand2 == "0") {
                optimization_log.push_back("Упрощено: x + 0 → x");
                triads[index].operation = "=";
                triads[index].operand2 = "";
            }
            // min (x - 0 → x)
            else if (triads[index].operation == "min" && triads[index].operand2 == "0") {
                optimization_log.push_back("Упрощено: x - 0 → x");
                triads[index].operation = "=";
                triads[index].operand2 = "";
            }
            // mult (x * 1 → x)
            else if (triads[index].operation == "mult" && triads[index].operand2 == "1") {
                optimization_log.push_back("Упрощено: x * 1 → x");
                triads[index].operation = "=";
                triads[index].operand2 = "";
            }
            // mult (x * 0 → 0)
            else if (triads[index].operation == "mult" && triads[index].operand2 == "0") {
                optimization_log.push_back("Упрощено: x * 0 → 0");
                triads[index].operation = "=";
                triads[index].operand1 = "0";
                triads[index].operand2 = "";
            }
            // div (x / 1 → x)
            else if (triads[index].operation == "div" && triads[index].operand2 == "1") {
                optimization_log.push_back("Упрощено: x / 1 → x");
                triads[index].operation = "=";
                triads[index].operand2 = "";
            }
        }
        
        // 3. Сворачивание констант
        if (!triads[index].operand1.empty() && !triads[index].operand2.empty() &&
            isNumber(triads[index].operand1) && isNumber(triads[index].operand2)) {
            
            try {
                int val1 = toNumber(triads[index].operand1);
                int val2 = toNumber(triads[index].operand2);
                int result = 0;
                bool can_fold = true;
                
                // Арифметические операции
                if (triads[index].operation == "plus") result = val1 + val2;
                else if (triads[index].operation == "min") result = val1 - val2;
                else if (triads[index].operation == "mult") result = val1 * val2;
                else if (triads[index].operation == "div") {
                    if (val2 != 0) {
                        result = val1 / val2;
                    } else {
                        has_error = true;
                        cout << "! СЕМАНТИЧЕСКАЯ ОШИБКА : Деление на ноль" << endl;
                        return;
                    }
                }
                // Логические операции
                else if (triads[index].operation == "and") result = val1 && val2;
                else if (triads[index].operation == "or") result = val1 || val2;
                // Операции отношения
                else if (triads[index].operation == "EQ") result = val1 == val2;
                else if (triads[index].operation == "NE") result = val1 != val2;
                else if (triads[index].operation == "LT") result = val1 < val2;
                else if (triads[index].operation == "LE") result = val1 <= val2;
                else if (triads[index].operation == "GT") result = val1 > val2;
                else if (triads[index].operation == "GE") result = val1 >= val2;
                else {
                    can_fold = false;
                }
                
                if (can_fold) {
                    optimization_log.push_back("Свернуто: " + triads[index].operand1 + " " + 
                                              triads[index].operation + " " + triads[index].operand2 + 
                                              " = " + to_string(result));
                    
                    triads[index].operation = "=";
                    triads[index].operand1 = numberToString(result);
                    triads[index].operand2 = "";
                }
            }
            catch (const exception& e) {
                has_error = true;
                cout << "! ОШИБКА ОПТИМИЗАЦИИ при свертке констант: " << e.what() << endl;
            }
        }
    }
    
    // 4. Распространение констант
    void propagateConstants() {
        if (has_error) return;
        
        map<string, string> const_values;
        
        for (size_t i = 0; i < triads.size(); i++) {
            if (has_error) return;
            if (!triads[i].active) continue;
            
            // Если присваивается константа: x = 5
            if (triads[i].operation == "=" && !triads[i].operand1.empty() && 
                isNumber(triads[i].operand1)) {
                const_values[triads[i].result] = triads[i].operand1;
            }
            
            // Если переменная переопределяется, удаляем её из таблицы констант
            if (triads[i].operation == "=" && !triads[i].operand1.empty() && 
                !isNumber(triads[i].operand1) && 
                const_values.find(triads[i].result) != const_values.end()) {
                const_values.erase(triads[i].result);
            }
            
            // Заменяем использование переменной на константу в operand1
            if (!triads[i].operand1.empty() && 
                const_values.find(triads[i].operand1) != const_values.end()) {
                string old_value = triads[i].operand1;
                triads[i].operand1 = const_values[triads[i].operand1];
                optimization_log.push_back("Заменено " + old_value + " на константу " + triads[i].operand1);
            }
            // Заменяем использование переменной на константу в operand2
            if (!triads[i].operand2.empty() && 
                const_values.find(triads[i].operand2) != const_values.end()) {
                string old_value = triads[i].operand2;
                triads[i].operand2 = const_values[triads[i].operand2];
                optimization_log.push_back("Заменено " + old_value + " на константу " + triads[i].operand2);
            }
        }
    }
    
    // 5. Устранение общих подвыражений
    void eliminateCommonSubexpressions() {
        if (has_error) return;
        
        map<string, string> expr_to_var; // выражение → переменная
        
        for (size_t i = 0; i < triads.size(); i++) {
            if (has_error) return;
            if (!triads[i].active || triads[i].operation == "=" || 
                triads[i].operation == "if" || triads[i].operation == "goto" ||
                triads[i].operation == "label" || triads[i].operation == "read" ||
                triads[i].operation == "write" || triads[i].operation == "dim" ||
                triads[i].operation == "halt") continue;
            
            // Создаем ключ для выражения
            string expr_key = triads[i].operation + ":" + triads[i].operand1 + ":" + triads[i].operand2;
            
            if (expr_to_var.find(expr_key) != expr_to_var.end()) {
                // Общее подвыражение найдено
                string existing_var = expr_to_var[expr_key];
                optimization_log.push_back("Устранено общее подвыражение: " + expr_key);
                
                // Заменяем триаду на присваивание
                triads[i].operation = "=";
                triads[i].operand1 = existing_var;
                triads[i].operand2 = "";
                triads[i].is_temporary = true;
                
                // Теперь нужно заменить все последующие использования этой триады
                string old_result = triads[i].result;
                for (size_t j = i + 1; j < triads.size(); j++) {
                    if (!triads[j].active) continue;
                    if (triads[j].operand1 == old_result) triads[j].operand1 = existing_var;
                    if (triads[j].operand2 == old_result) triads[j].operand2 = existing_var;
                }
            } else {
                expr_to_var[expr_key] = triads[i].result;
            }
        }
    }
    
    // 6. Удаление мертвого кода (неиспользуемых присваиваний)
    void removeDeadCode() {
        if (has_error) return;
        
        set<string> used_vars;
        
        // Сначала собираем все используемые переменные (справа от '=')
        for (size_t i = 0; i < triads.size(); i++) {
            if (!triads[i].active) continue;
            
            if (!triads[i].operand1.empty() && !isNumber(triads[i].operand1)) {
                used_vars.insert(triads[i].operand1);
            }
            if (!triads[i].operand2.empty() && !isNumber(triads[i].operand2)) {
                used_vars.insert(triads[i].operand2);
            }
            
            // Для операций if, goto, label - учитываем метки
            if (triads[i].operation == "goto" && !isNumber(triads[i].operand1)) {
                used_vars.insert(triads[i].operand1);
            }
            if (triads[i].operation == "if" && !isNumber(triads[i].operand2)) {
                used_vars.insert(triads[i].operand2);
            }
        }
        
        // Удаляем присваивания, результат которых не используется
        for (size_t i = 0; i < triads.size(); i++) {
            if (has_error) return;
            if (!triads[i].active || triads[i].operation != "=") continue;
            
            if (triads[i].is_temporary && used_vars.find(triads[i].result) == used_vars.end()) {
                removeTriad(i, "неиспользуемая временная переменная");
            }
        }
    }
    
    // 7. Удаление ненужных переходов
    void removeUnnecessaryJumps() {
        if (has_error) return;
        
        for (size_t i = 0; i < triads.size(); i++) {
            if (!triads[i].active) continue;
            
            // Безусловный переход на следующую инструкцию
            if (triads[i].operation == "goto" && i + 1 < triads.size()) {
                // Проверяем, является ли следующая триада меткой, на которую мы переходим
                if (triads[i + 1].operation == "label" && 
                    triads[i + 1].operand1 == triads[i].operand1) {
                    removeTriad(i, "ненужный переход на следующую инструкцию");
                }
            }
            
            // IF с постоянным условием
            if (triads[i].operation == "if" && isNumber(triads[i].operand1)) {
                int condition = toNumber(triads[i].operand1);
                if (condition == 0) {
                    // IF 0 GOTO L → всегда переходим
                    triads[i].operation = "goto";
                    triads[i].operand1 = triads[i].operand2;
                    triads[i].operand2 = "";
                    optimization_log.push_back("Преобразован IF с ложным условием в GOTO");
                } else {
                    // IF 1 GOTO L → переход никогда не выполняется, удаляем
                    removeTriad(i, "IF с всегда истинным условием");
                }
            }
        }
    }
    
public:
    TriadOptimizer(const vector<Triad>& triad_list) : triads(triad_list), has_error(false) {}
    
    // Проверка наличия ошибок
    bool hasError() const {
        return has_error;
    }
    
    vector<Triad> optimize() {
        optimization_log.clear();
        has_error = false;
        
        if (triads.empty()) {
            cout << "!  Нет триад для оптимизации" << endl;
            return vector<Triad>();
        }
        
        cout << "Начинаем оптимизацию " << triads.size() << " триад..." << endl;
        
        bool changed;
        int iteration = 0;
        
        do {
            iteration++;
            changed = false;
            size_t old_log_size = optimization_log.size();
            
            cout << "Итерация оптимизации " << iteration << "..." << endl;
            
            // Выполняем все виды оптимизации
            for (size_t i = 0; i < triads.size(); i++) {
                optimizeArithmetic(i);
                if (has_error) {
                    cout << "! Оптимизация прервана из-за ошибок" << endl;
                    return vector<Triad>();
                }
            }
            
            propagateConstants();
            if (has_error) {
                cout << "! Оптимизация прервана из-за ошибок" << endl;
                return vector<Triad>();
            }
            
            eliminateCommonSubexpressions();
            removeUnnecessaryJumps();
            removeDeadCode();
            
            if (optimization_log.size() > old_log_size) {
                changed = true;
                cout << "Обнаружены улучшения, продолжаем..." << endl;
            }
            
            // Защита от бесконечного цикла
            if (iteration > 10) {
                cout << "!  Достигнуто максимальное количество итераций оптимизации" << endl;
                break;
            }
            
        } while (changed);
        
        cout << "Оптимизация завершена за " << iteration << " итераций" << endl;
        
        return getActiveTriads();
    }
    
    // Получение только активных триад
    vector<Triad> getActiveTriads() const {
        if (has_error) return vector<Triad>();
        
        vector<Triad> result;
        for (const auto& triad : triads) {
            if (triad.active) {
                result.push_back(triad);
            }
        }
        return result;
    }
    
    // Вывод лога оптимизации
    void printOptimizationLog() const {
        if (optimization_log.empty()) {
            cout << "\n=== ЛОГ ОПТИМИЗАЦИИ ===" << endl;
            cout << "Нет оптимизаций" << endl;
            cout << "======================" << endl;
            return;
        }
        
        cout << "\n=== ЛОГ ОПТИМИЗАЦИИ ===" << endl;
        for (const auto& log_entry : optimization_log) {
            cout << log_entry << endl;
        }
        cout << "======================" << endl;
    }
    
    // Статистика оптимизации
    static void printOptimizationStats(const vector<Triad>& original, 
                                      const vector<Triad>& optimized) {
        int original_count = original.size();
        int optimized_count = optimized.size();
        
        if (original_count == 0) {
            cout << "\n=== СТАТИСТИКА ОПТИМИЗАЦИИ ===" << endl;
            cout << "Нет исходных триад" << endl;
            cout << "==============================" << endl;
            return;
        }
        
        int removed = original_count - optimized_count;
        double reduction = (100.0 * removed) / original_count;
        
        cout << "\n=== СТАТИСТИКА ОПТИМИЗАЦИИ ===" << endl;
        cout << "Исходных триад: " << original_count << endl;
        cout << "После оптимизации: " << optimized_count << endl;
        cout << "Удалено триад: " << removed << endl;
        cout << "Сокращение: " << reduction << "%" << endl;
        cout << "==============================" << endl;
    }
};
