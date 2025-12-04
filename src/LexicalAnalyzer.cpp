/* 
    TODO: Дописать обработку типов
    TODO: изменить распозноваие типов на уровне самих возможных типов
    TODO: Автоматически определть тип ликсемы. сейчас вручную пишется
*/

#include "../inc/LexicalAnalyzer.hpp"

// Input source_code and first char
LexicalAnalyzer::LexicalAnalyzer(string sc) : source_code(sc), line(0), column(0) {
            if (source_code.empty()) current_char = '\0';
            else current_char = source_code[0];
        }

// move to next char
void LexicalAnalyzer::nextChar(int count) {
    for (int i=1; i<count+1; i++) {
        position++;
        column++;
        if (position >= source_code.length()) current_char = '\0';
        else {
            current_char = source_code[position];
            if (current_char == '\n') {
                column == 1;
                line++;
            }
        }
    }
    
}

// skip comment
void LexicalAnalyzer::skipComment() {
    nextChar(1);
    while (current_char != COMMENT_CLOSE) {
        nextChar(1);
    }
    nextChar(1);
}

// return token const num
Token LexicalAnalyzer::readNumber() {
    string num = "";
    bool is_float = false;

    while (terminals.find(current_char) == terminals.end()) {
        if (isdigit(current_char)) {
            if (source_code[position + 1] == '.') {
                is_float = true;
            }
        }

        num += current_char;
        nextChar(1);
    }

    if (is_float) return Token(FLOAT_CONSTANT, num, line, column);
    else return Token(INT_CONSTANT, num, line, column);
}

// Возврат токена типа ключевое слово или идентификатор
Token LexicalAnalyzer::readKeyword() {
    string word = "";
    
    // Пока текущий символ не терминальный или не оператор сбор слова
    while (terminals.find(current_char) == terminals.end()
            && operations.find(to_string(current_char)) == operations.end()) {
        auto test1 = terminals.find(current_char);
        auto test2 = terminals.end();
        word += current_char;
        nextChar(1);
    }

    // Если слово есть в списке ключевых слов, то ключевое слово, иначе - идентификатор
    if (keywords.find(word) != keywords.end()) 
        return Token(KEYWORD, word, line, column);
    else if (operations.find(word) != operations.end()) 
        return Token(OPERATOR, word, line, column);
    else return Token(IDENTIFIER, word, line, column);
}

// Token LexicalAnalyzer::readTreminal() {
//     if ()
// }

Token LexicalAnalyzer::getToken() {
    while (current_char != END_PROGRAM_CHAR) {
        if (current_char == ' ') nextChar(1);
        else if (current_char == COMMENT_OPEN) skipComment();
        else break;
    }
    // Если разделитель, то чтение делиметра
    if (terminals.find(current_char) != terminals.end()) {
        return readTreminal();
    }
    // Иначе Если терминальный символ, то возврат
    else if (current_char == END_PROGRAM_CHAR) {
        //!
        return Token(KEYWORD, to_string(current_char), line, column);
    }
    // Иначе Если число, что чтение числа
    else if (isdigit(current_char)) {
        return readNumber();
    }
    // Иначе Если ключевое слово или идентификатор
    else if (isalpha(current_char)) {
        return readKeyword();
    }
    // Иначе Если оператор, то чтение оператора
    else if (operations.find(to_string(current_char)) != operations.end()) {
        // return readOperator();
        nextChar(1);
    }
    
    
    // Иначе неизвествный символ
    nextChar(1);
    return Token(ERROR, to_string(current_char), line, column);
}

// Output vector of tokens from source_code
vector<Token> LexicalAnalyzer::getAllToken() {
    vector<Token> table_id;
    Token token;

    do {
        token = getToken();
        table_id.push_back(token);
    } while (current_char != '\0'); //token.type != END_OF_FILE
    return table_id;
}

// TODO: допилить вывод, вынести в подфункцию мю
void LexicalAnalyzer::printAllToken(vector<Token> tokens) {
    cout << tokens.size() << endl;
    cout << "| " << "Ликсема" << " | " << "Тип ликсемы" << " | " << "Строка" << " | " << "Столбец" << " |" << endl;
    for (Token token : tokens) {
        cout << "| " << token.value << " | " << token.getTypeString() << " | " << token.line << " | " << token.column << " |" << endl;
    }
}