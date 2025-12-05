#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
// Операторы сравнения
#define NOTEQUAL        "NE"
#define EQUAL           "EQ"
#define LESS_THAN       "LT"
#define LESS_EQUAL      "LE"
#define GREATER_THAN    "GT"
#define GREATER_EQUAL   "GE"
// Операторы сложения
#define PLUS            "plus"
#define MINUS           "mun"
#define OR              "or"
// Операвторы умножения
#define MULT            "mult"
#define DIV             "div"
#define AND             "and"
// Унарный оператор
#define NOT             "~"
// Оператор присваивания
#define ASSIGNMENT       "ass"
// Типы данных
#define INT             "integer"
#define FLOAT           "real"
#define BOOL            "boolean"
// Комментарий
#define COMMENT_OPEN    '{'
#define COMMENT_CLOSE   '}'
// ключевое слово конец программы
#define END_PROGRAM     "end"
// 
#define END_PROGRAM_CHAR '\0'
#define NEXT_LINE '\n'
// булевая константа
#define TRUE "true"
#define FALSE "false"
// оператор условия
#define IF "if"
#define IF_THEN "then"
#define IF_ELSE "else"
// оператор цикла
#define FOR "for"
#define FOR_TO "to"
#define FOR_DO "do"
#define WHILE "while"
#define WHILE_DO "do"
// оператор ввода/вывода
#define READ "read"
#define WRITE "write"
// оператор описания данных
#define DIM "dim"


// Типы ликсем
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    INT_CONSTANT,
    FLOAT_CONSTANT,
    BOOL_CONSTANT,
    OPERATOR,
    TERMINAL,
    END_OF_FILE,
    ERROR,
    SERVICE
};

// Структура ликсемы
struct Token {
    TokenType type;
    string value = "";
    int column;
    int line;
    
    Token() {}
    Token(TokenType t, string v, int l, int c)
        : type(t), value(v), line(l), column(c) {}
    // Расшифровка типов ликсем
    string getTypeString() {
        if (type == 0) return "Ключевое слово";
        else if (type == 1) return "Идентификатор";
        else if (type == 2) return "Целочисленная константа";
        else if (type == 3) return "Константа с плавающей точкой";
        else if (type == 4) return "Булевая константа";
        else if (type == 5) return "Оператор";
        else if (type == 6) return "Терминальный символ";
        else if (type == 7) return "Ключевое слово";
        else if (type == 8) return "Ошибка";
        else return "Невозможное значение";
    }
};

class LexicalAnalyzer {
    private: 
        string source_code;
        char current_char;
        int line;
        int column;
        int position; // позиция символа в массиве исходного кода

        // Список ликсем и их типов
        unordered_map<string, TokenType> lixems = {
            {INT, KEYWORD},
            {FLOAT, KEYWORD},
            {BOOL, KEYWORD},
            {END_PROGRAM, KEYWORD},
            {IF, KEYWORD},
            {IF_THEN, KEYWORD},
            {IF_ELSE, KEYWORD},
            {FOR, KEYWORD},
            {FOR_TO, KEYWORD},
            {FOR_DO, KEYWORD},
            {WHILE_DO, KEYWORD},
            {WHILE, KEYWORD},
            {READ, KEYWORD},
            {WRITE, KEYWORD},
            {TRUE, BOOL_CONSTANT},
            {FALSE, BOOL_CONSTANT},
            {DIM, KEYWORD},

            {NOTEQUAL, OPERATOR},
            {EQUAL, OPERATOR},
            {LESS_THAN, OPERATOR},
            {LESS_EQUAL, OPERATOR},
            {GREATER_THAN, OPERATOR},
            {GREATER_EQUAL, OPERATOR},
            {PLUS, OPERATOR},
            {MINUS, OPERATOR},
            {OR, OPERATOR},
            {MULT, OPERATOR},
            {DIV, OPERATOR},
            {AND, OPERATOR},
            {NOT, OPERATOR},
            {ASSIGNMENT, OPERATOR}
        };

        // Таблица терминальных символов
        unordered_map<char, TokenType> terminals = {
            {',', TERMINAL},
            {'(', TERMINAL},
            {')', TERMINAL},
            {'\n', SERVICE},
            {'\0', SERVICE},
            {' ', SERVICE}
        };

        void nextChar(int count);
        void skipComment();
        Token readNumber();
        Token readKeyword();
        Token getToken();

    public:
        LexicalAnalyzer(string sc);
        vector<Token> getAllToken();
        void printAllToken(vector<Token> tokens);

};