#include "Lexer.h"

void Lexer::init(std::string Content) {
  CurBuf = Content;
  CurPtr = &*CurBuf.cbegin();
}

Token Lexer::lex() {

  char LastChar = ' ';

  // Skip any whiteSpace
  while (isspace(LastChar))
    // getchar is std function
    LastChar = getNextChar();

  // Judge string identifier
  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    std::cout << LastChar << std::endl;
    IdentifierStr = LastChar;
    while (isalnum((LastChar = getNextChar()))) {

      IdentifierStr += LastChar;
    }
    CurPtr--;
    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
      return tok_extern;
    return tok_identifier;
  }

  // Judge number identifier
  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getNextChar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), 0);
    return tok_number;
  }

  // Judge comment line
  if (LastChar == '#') {
    // Comment until end of line.
    do
      LastChar = getNextChar();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return lex();
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return tok_eof;
  if (LastChar == '(')
    return tok_leftParen;
  if (LastChar == ')')
    return tok_rightParen;
  // Otherwise, just return the character as its ascii value.
  return tok_unknown;
}
