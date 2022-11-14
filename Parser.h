
#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include <map>
#include <memory>
#include <utility>
class Parser {
public:
  Lexer Lex;
public:
  Parser(/* args */){};
  ~Parser(){};

  int lexPrecedence();
  std::unique_ptr<ExprAST> ParseNumberExpr();
  std::unique_ptr<ExprAST> ParseParenExpr();
  std::unique_ptr<ExprAST> ParseIdentifierExpr();
  std::unique_ptr<ExprAST> ParsePrimary();
  std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS);
  std::unique_ptr<ExprAST> ParseExpression();
  std::unique_ptr<PrototypeAST> ParsePrototype();
  std::unique_ptr<FunctionAST> ParseDefinition();
  std::unique_ptr<FunctionAST> ParseTopLevelExpr();
  std::unique_ptr<PrototypeAST> ParseExtern();
  std::unique_ptr<IfExprAST> ParseIfExpr();

  // Init the parser

  // Top level driver to drive the parser working
  void HandleDefinition();
  void HandleExtern();
  void HandleTopLevelExpression();
  void parse(std::string Content);
};

#endif