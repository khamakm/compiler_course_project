#include <iostream>
#include "../inc/LexicalAnalyzer.hpp"
#include "../inc/SyntacticAnalyzer.hpp"

using namespace std;
int main() {




    string code = R"(if b_b EQ true then a ass a mult 1 else minus 1
read(a)
{comment text}
for i ass 0 to 5 do write(i)
while i LT 1 do i ass i minus 1
dim i, abc integer
b_b ass true
end)";

    // Включение/выключение отладочного вывода
    // SyntacticAnalyzer::enableDebugMode();
    SyntacticAnalyzer::disableDebugMode();
    
    LexicalAnalyzer lAnalys(code);
    vector<Token> list_tokens = lAnalys.getAllToken();

    // Старт синтаксического анализатора если нет ликсеческих ошибок
    if (lAnalys.printAllToken(list_tokens)) {
        SyntacticAnalyzer parser(list_tokens);
        parser.analyze();
        parser.printResult();
    }
}
