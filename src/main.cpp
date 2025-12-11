#include <iostream>

#include "../inc/LexicalAnalyzer.hpp"

using namespace std;

int main () {
    string code = R"(5y 15o 8O 2fH 1Gh 11d 10D H8h abh
a integer
read(a)
bca real
bca ass 0.12
b_b boolean
b_b ass true
{comment text}
if b_b EQ true then a ass a plus 1 else minus 1
i integer
for i ass 0 to 5 do write(i)
while i LT 1 do i ass i minus 1
=-
end
)";
    LexicalAnalyzer lAnalys(code);
    vector<Token> list_tokens = lAnalys.getAllToken();
    lAnalys.printAllToken(list_tokens);


}