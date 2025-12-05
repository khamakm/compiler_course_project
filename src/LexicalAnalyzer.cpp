/* 
    TODO: добавить таблицу идентификаторов (ТИ) (пока можно без неё)
    TODO: обработка 8 и 16 (можно пока без этого)
*/

#include "../inc/LexicalAnalyzer.hpp"

// Инициализация лексического анализатора: загрузка исходного кода и первого символа
LexicalAnalyzer::LexicalAnalyzer(string sc) : source_code(sc), line(1), column(1), position(0) {
            if (source_code.empty()) current_char = END_PROGRAM_CHAR;
            else current_char = source_code[0];
        }

// Смещение на следующий символ
void LexicalAnalyzer::nextChar(int count) {
    for (int i=1; i<=count; i++) {
        position++;
        column++;
        if (position >= source_code.length()) current_char = END_PROGRAM_CHAR;
        else {
            current_char = source_code[position];
            if (current_char == NEXT_LINE) {
                column = 0;
                line++;
            }
        }
    }
}

// Пропуск комментария
void LexicalAnalyzer::skipComment() {
    nextChar(1); // Пропуск шапки комментария
    while (current_char != COMMENT_CLOSE) {
        nextChar(1); // Пропуск тела комментария
    }
    nextChar(1); // Пропуск подвала коментария
}

// Возврат константных типов ликсемы
Token LexicalAnalyzer::readNumber() {
    string num = "";
    bool is_float = false;
    Token token;
    token.line = line;
    token.column = column;

    while (terminals.find(current_char) == terminals.end()) {
        if (isdigit(current_char)) {
            if (source_code[position + 1] == '.') {
                is_float = true;
            }
        }

        num += current_char;
        nextChar(1);
    }

    token.value = num;

    if (is_float) {
        token.type = FLOAT_CONSTANT;
        return token; //* Token(FLOAT_CONSTANT, num, line, column);
    }
    else {
        token.type = INT_CONSTANT;
        return token; //* Token(INT_CONSTANT, num, line, column);
    } 
}

// Возврат типа ликсемы 
Token LexicalAnalyzer::readKeyword() {
    string word = "";
    Token token;
    token.line = line;
    token.column = column;
    
    // Пока текущий символ не терминальный или не оператор сбор слова
    while (terminals.find(current_char) == terminals.end()) {
        word += current_char;
        nextChar(1);
    }
    token.value = word;

    auto lixem = lixems.find(word);
    // Если слово есть в списке ликсем, то возврат ликсемы
    if (lixem != lixems.end()) {
        if (lixem->first == END_PROGRAM)
            current_char = END_PROGRAM_CHAR;   
        token.type = lixem->second;
        return token;
        //* return Token(lixem->second, word, line, column);
    }
    else {
        token.type = IDENTIFIER;
        return token;
    }
    //* return Token(IDENTIFIER, word, line, column);
}

Token LexicalAnalyzer::getToken() {
    // Пропуск пробелов и комментариев, обработка терминальных символов
    Token token;
    //* string word = "";
    while (current_char != END_PROGRAM_CHAR) {
        if (current_char == ' ') nextChar(1);
        else if (current_char == COMMENT_OPEN) skipComment();
        else {
            auto terminal_token = terminals.find(current_char);
            if (terminal_token != terminals.end()) {
                token.value = current_char;
                token.line = line;
                token.column = column;
                //* word = current_char;
                nextChar(1);
                if (terminal_token->second != SERVICE) {
                    token.type = terminal_token->second;
                    return token;
                    //* return Token(terminal_token->second, word, line, column);
                }

            }
            else break;
        }
    }
    token.value = current_char;
    token.line = line;
    token.column = column;
    //* word = current_char;

    // Иначе Если терминальный символ, то возврат
    if (current_char == END_PROGRAM_CHAR) {
        token.type = KEYWORD;
        return token;
        //* return Token(KEYWORD, word, line, column);
    }
    // Иначе Если число, что чтение числа
    else if (isdigit(current_char)) return readNumber();
    // Иначе Если ключевое слово или идентификатор
    else if (isalpha(current_char)) return readKeyword();
    // Иначе неизвествный символ
    else {
        nextChar(1);
        token.type = ERROR;
        return token;
        //* return Token(ERROR, word, line, column);
    }
}

// Сбор вектора токенов по исходному коду
vector<Token> LexicalAnalyzer::getAllToken() {
    vector<Token> table_id;
    Token token;

    do {
        token = getToken();
        table_id.push_back(token);
    } while (current_char != END_PROGRAM_CHAR); 
    return table_id;
}

// Вывод в консоль таблицы ликсем
void LexicalAnalyzer::printAllToken(vector<Token> tokens) {
    cout << "+========================================================+" << endl;
    cout << "| Таблица ликсем |" << endl;
    cout << "+========================================================+" << endl;

    cout << left << "| " << setw(10) << "Ликсема" << " | " << setw(35) << "Тип ликсемы" << " | "
        << setw(3) <<"Строка" << " | " << setw(3) << "Столбец" << " |" << endl;
    cout << "+--------------------------------------------------------+" << endl;
    
    int error_count = 0;
    for (Token token : tokens) {
        cout << left << "| " << setw(10) << token.value << " | "
                << setw(35) << token.getTypeString() 
                << " | ";
        if (token.type == ERROR) {
            error_count++;
            cout << left << setw(3) << token.line << " | "
                << setw(3) << token.column << " |" << endl;
        }
        else {
            cout << "   " << " | "
                << "   " << " |" << endl;
        }
    }

    cout << "+========================================================+" << endl;
    cout << left << "| Количество ликсем: " << setw(3) <<tokens.size()
        << "     | Кол-во ошибок: " << setw(3) << error_count << "        |" << endl;
    cout << "+========================================================+" << endl;

}