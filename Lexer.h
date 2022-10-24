#include "AST.h"
#include <ctype.h>
#include <string>
#include <iostream>
// Def basic tokens for
enum Token {
  tok_eof = -1,
  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,
  
  tok_leftParen = -6,
  tok_rightParen = -7,

  // unkown token
  tok_unknown = -8
};

class Lexer {
public:
  // Declare a string to store the basic string identifier
  std::string IdentifierStr;

  // Declare a double val to store a number val
  double NumVal;

  const char *CurPtr = nullptr;
  std::string CurBuf;

  /// Information about the current token
  const char *TokStart = nullptr;

public:
  Token lex();
  void init(std::string Content);
  char getNextChar() {
    char CurChar = *CurPtr++;
    // cout << CurChar << int(CurChar) << endl;
    switch (CurChar) {
    default:
      return (unsigned char)CurChar;

    case 0: {
      // A NUL character in the stream is either the end of the current buffer
      // of a spurious NUL in the file. Disambiguate that here.
      if (CurPtr - 1 == &*CurBuf.cend()) {
        --CurPtr; // Arrange for another call to return EOF again.
        return EOF;
      }

      std::cerr << "NUL character is invalid in source, treated as space.\n";
      return ' ';
    }

    case '\n':
    case '\r':
      // Handle the newline character by ignoring it and incrementing the line
      // count. However, be careful about 'dos style' files with \n\r in them.
      // Only treat a \n\r or \r\n as a single line.
      if ((*CurPtr == '\n' || *CurPtr == '\r') && *CurPtr != CurChar)
        ++CurPtr;

      return '\n';
    }
  }
};
