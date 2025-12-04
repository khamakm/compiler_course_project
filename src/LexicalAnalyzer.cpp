/* 
    TODO: добавить типы ликсем
    TODO: добавить вывод в виде выровненной таблицы ликсем (ТЛ)
    TODO: добавить таблицу идентификаторов (ТИ) (пока можно без неё)
    TODO: дописать комметарии и перевести на русский
    TODO: пофиксить: сначала сдвигается cur char на след со сдвигом позиции, а потом только возврат Token. Поднять до getAllToken nextChar
    TODO: обработка 8 и 16 (можно пока без этого)
    TODO: раскидать в приват всё, что надо
*/

#include "../inc/LexicalAnalyzer.hpp"

// Инициализация лексического анализатора: загрузка исходного кода и первого символа
LexicalAnalyzer::LexicalAnalyzer(string sc) : source_code(sc), line(1), column(1), position(0) {
            if (source_code.empty()) current_char = '\0';
            else current_char = source_code[0];
        }

// Смещение на следующий символ
void LexicalAnalyzer::nextChar(int count) {
    for (int i=1; i<=count; i++) {
        position++;
        column++;
        if (position >= source_code.length()) current_char = '\0';
        else {
            current_char = source_code[position];
            if (current_char == '\n') {
                column = 0;
                line++;
            }
        }
    }
    cout << "Curr chr: " << current_char << endl;
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

// Возврат типа ликсемы 
Token LexicalAnalyzer::readKeyword() {
    string word = "";
    
    // Пока текущий символ не терминальный или не оператор сбор слова
    while (terminals.find(current_char) == terminals.end()) {
        word += current_char;
        nextChar(1);
    }

    auto lixem = lixems.find(word);
    // Если слово есть в списке ликсем, то возврат ликсемы
    if (lixem != lixems.end()) {
        if (lixem->first == END_PROGRAM)
            current_char = END_PROGRAM_CHAR;    
        return Token(lixem->second, word, line, column);
    }
    else return Token(IDENTIFIER, word, line, column);
}

Token LexicalAnalyzer::getToken() {
    // Пропуск пробелов и комментариев, обработка терминальных символов
    string word = "";
    while (current_char != END_PROGRAM_CHAR) {
        if (current_char == ' ') nextChar(1);
        else if (current_char == COMMENT_OPEN) skipComment();
        else {
            auto terminal_token = terminals.find(current_char);
            if (terminal_token != terminals.end()) {
                word = current_char;
                nextChar(1);
                if (terminal_token->second != SERVICE) 
                    return Token(terminal_token->second, word, line, column);
            }
            else break;
        }
    }
    word = current_char;
    // Иначе Если терминальный символ, то возврат
    if (current_char == END_PROGRAM_CHAR) 
        return Token(KEYWORD, word, line, column);
    // Иначе Если число, что чтение числа
    else if (isdigit(current_char)) return readNumber();
    // Иначе Если ключевое слово или идентификатор
    else if (isalpha(current_char)) return readKeyword();
    // Иначе неизвествный символ
    else {
        nextChar(1);
        return Token(ERROR, word, line, column);
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
    cout << tokens.size() << endl;
    cout << "| " << "Ликсема" << " | " << "Тип ликсемы" << " | " << "Строка" << " | " << "Столбец" << " |" << endl;
    for (Token token : tokens) {
        cout << "| " << token.value << " | " << token.getTypeString() << " | " << token.line << " | " << token.column << " |" << endl;
    }
}