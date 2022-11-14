
#include "Parser.h"
/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.
std::map<char, int> BinopPrecedence;

/// lexPrecedence - Get the precedence of the pending binary operator token.
int Parser::lexPrecedence() {
  if (!isascii(Lex.CurTok))
    return -1;

  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[Lex.CurTok];
  if (TokPrec <= 0)
    return -1;
  return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

/// numberexpr ::= number
std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(Lex.NumVal);
  Lex.lex(); // consume the number
  return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
  Lex.lex(); // eat (.
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (Lex.CurTok != ')')
    return LogError("expected ')'");
  Lex.lex(); // eat ).
  return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
  std::string IdName = Lex.IdentifierStr;

  Lex.lex(); // eat identifier.

  if (Lex.CurTok != '(') // Simple variable ref.
    return std::make_unique<VariableExprAST>(IdName);

  // Call.
  Lex.lex(); // eat (
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (Lex.CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression())
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (Lex.CurTok == ')')
        break;

      if (Lex.CurTok != ',')
        return LogError("Expected ')' or ',' in argument list");
      Lex.lex();
    }
  }

  // Eat the ')'.
  Lex.lex();

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
std::unique_ptr<ExprAST> Parser::ParsePrimary() {
  switch (Lex.CurTok) {
  default:
    return LogError("unknown token when expecting an expression");
  case tok_identifier:
    return ParseIdentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  }
}

/// binoprhs
///   ::= ('+' primary)*
std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec,
                                               std::unique_ptr<ExprAST> LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = lexPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    int BinOp = Lex.CurTok;
    Lex.lex(); // eat binop

    // Parse the primary expression after the binary operator.
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = lexPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    // Merge LHS/RHS.
    LHS =
        std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  }
}

/// expression
///   ::= primary binoprhs
///
std::unique_ptr<ExprAST> Parser::ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
  if (Lex.CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = Lex.IdentifierStr;
  Lex.lex();

  if (Lex.CurTok != tok_leftParen)
    return LogErrorP("Expected '(' in prototype");
  std::vector<std::string> ArgNames;
  while (Lex.lex() == tok_identifier) {
    ArgNames.push_back(Lex.IdentifierStr);
  }

  if (Lex.CurTok != tok_rightParen)
    return LogErrorP("Expected ')' in prototype");

  Lex.lex();
  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
  Lex.lex(); // eat def.
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

/// toplevelexpr ::= expression
std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto
    // Make_unique构建函数
    auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/// external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> Parser::ParseExtern() {
  Lex.lex(); // eat extern.
  return ParsePrototype();
}

// Driver to dive the parser goes on
//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

void Parser::HandleDefinition() {
  if (auto FnAST = ParseDefinition()) {
    fprintf(stderr, "Parsed a function definition.\n");
    if (auto *FnIR = FnAST->codegen()) {
      FnIR->print(errs());
    }
  } else {
    // Skip token for error recovery.
    Lex.lex();
  }
}

void Parser::HandleExtern() {
  if (auto ProtoAST = ParseExtern()) {
    fprintf(stderr, "Parsed an extern\n");
    if (auto *FnIR = ProtoAST->codegen()) {
      FnIR->print(errs());
    }
  } else {
    // Skip token for error recovery.
    Lex.lex();
  }
}

void Parser::HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (auto FnASt = ParseTopLevelExpr()) {
    fprintf(stderr, "Parsed a top-level expr\n");
    if (auto FnIR = FnASt->codegen()) {
      FnIR->print(errs());
    }
  } else {
    // Skip token for error recovery.
    Lex.lex();
  }
}

/// top ::= definition | external | expression | ';'
void Parser::parse(std::string Content) {
  Lex.init(Content);
  while (true) {
    fprintf(stderr, "ready> ");
    switch (Lex.CurTok) {
    case tok_eof:
      return;
    case ';': // ignore top-level semicolons.
      Lex.lex();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}