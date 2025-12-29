#include <iostream>
#include "../inc/LexicalAnalyzer.hpp"
#include "../inc/SyntacticAnalyzer.hpp"
#include "../inc/Optimizer.hpp"
#include "../inc/Generator.hpp"

using namespace std;

int main() {

    string code = R"(dim b_b boolean
dim node, i integer
b_b ass true
b_b ass b_b
node ass 1 plus 2
dim a integer
a ass 2
while i LT 1 do i ass i min 1
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
        if (!parser.hasError()) {
            cout << "\n\n=== ГЕНЕРАЦИЯ ТРИАД ===" << endl;
        
            TriadGenerator generator(list_tokens);
            vector<Triad> original_triads = generator.generate();
            
            

            if (!generator.hasError()) {
                TriadGenerator::printTriads(original_triads, "ИСХОДНЫЕ ТРИАДЫ");

                cout << "\n\n=== ОПТИМИЗАЦИЯ ТРИАД ===" << endl;
            
                TriadOptimizer optimizer(original_triads);
                vector<Triad> optimized_triads = optimizer.optimize();
                
                optimizer.printOptimizationLog();
                TriadGenerator::printTriads(optimized_triads, "ОПТИМИЗИРОВАННЫЕ ТРИАДЫ");
                TriadOptimizer::printOptimizationStats(original_triads, optimized_triads);
            }
        }
    }
}
