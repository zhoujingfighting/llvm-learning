#include "Parser.h"
extern std::map<char, int> BinopPrecedence;
int main() {
  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40; // highest.

  Parser parser;
  // Run the main "interpreter loop" now.
  std::string test = "extern cos(x)";
  parser.parse(test);

  return 0;
}