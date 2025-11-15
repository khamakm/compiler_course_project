#pragma once

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Types of lixem
enum TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    OPERATOR,
    DELIMITER,
    END_OF_FILE,
    ERROR
};

// Struct for keep lixems
struct Token {
    TokenType type;
    string value;
    int line;
    int column;
};

class LexicalAnalyzer {
    private: 
        string source_code;
        char current_char;
        int line;
        int column;
        int position; // position in list source_code

        // Table of keywords
        unordered_map<string, TokenType> keywords = {
            {"int", KEYWORD},
            {"float", KEYWORD},
            {"bool", KEYWORD},
            {"begin", KEYWORD},
            {"end", KEYWORD},
            {"if", KEYWORD},
            {"then", KEYWORD},
            {"else", KEYWORD},
            {"for", KEYWORD},
            {"to", KEYWORD},
            {"step", KEYWORD},
            {"next", KEYWORD},
            {"do", KEYWORD},
            {"while", KEYWORD},
            {"readln", KEYWORD},
            {"writeln", KEYWORD},
            {"true", CONSTANT},
            {"false", CONSTANT}
        };

        // Таблица операторов
        unordered_map<string, TokenType> operators = {
            {"!=", OPERATOR},
            {"==", OPERATOR},
            {"<", OPERATOR},
            {"<=", OPERATOR},
            {">", OPERATOR},
            {">=", OPERATOR},
            {"+", OPERATOR},
            {"-", OPERATOR},
            {"||", OPERATOR},
            {"*", OPERATOR},
            {"/", OPERATOR},
            {"&&", OPERATOR},
            {"!", OPERATOR},
            {":=", OPERATOR}
        };
        
        // Table of delititers
        unordered_map<char, TokenType> delimiters = {
            {';', DELIMITER},
            {',', DELIMITER},
            {'(', DELIMITER},
            {')', DELIMITER}
        };

    public:
        // Input source_code and first char
        LexicalAnalyzer(string sc);
        Token readComment();
        Token readNumber();
        Token readString();
        Token readKeyword();
        Token readOperator();
        Token readDelimiter();



        Token getToken();
        vector<Token> getAllToken();




};