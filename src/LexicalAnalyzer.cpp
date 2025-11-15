#include "LexicalAnalyzer.hpp"

// Input source_code and first char
LexicalAnalyzer::LexicalAnalyzer(string sc) : source_code(sc), line(0), column(0) {
            if (source_code.empty()) current_char = '\0';
            else current_char = source_code[0];
        }

Token LexicalAnalyzer::getToken() {
    string current_char_string = to_string(current_char);

    // Если комментарий, то удалить комментарий
    if (current_char == '(' && source_code[position+1] == '*') {
        readComment();
    }
    // Иначе Если терминальный символ, то возврат
    else if (current_char == '(' && source_code[position+1] == '*') {
        ; //!
    }
    // Иначе Если кавычки, то чтение строки
    else if (current_char == '\"') {
        readString();
    }
    // Иначе Если число, что чтение числа
    else if (isdigit(current_char)) {
        readNumber();
    }
    // Иначе Если ключевое слова
    else if (keywords.find(current_char_string) != keywords.end()) {
        readKeyword();
    }
    // Иначе Если оператор, то чтение оператора
    else if (operators.find(current_char_string) != operators.end()) {
        readOperator();
    }
    // Иначе Если разделитель, то чтение делиметра
    else if (delimiters.find(current_char) != delimiters.end()) {
        readOperator();
    }
}

// Output vector of tokens from source_code
vector<Token> LexicalAnalyzer::getAllToken() {
    vector<Token> table_id;
    Token token;

    do {
        token = getToken();
        table_id.push_back(token);
    } while (token.type != END_OF_FILE);
}