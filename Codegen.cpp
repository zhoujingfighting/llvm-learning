#include "AST.h"

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static std::unique_ptr<Module> TheModule;
static std::map<std::string, Value *> NamedValues;
extern std::unique_ptr<ExprAST> LogError(const char *Str);

Value *LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}
Value *NumberExprAST::codegen() {
  return ConstantFP::get(TheContext, APFloat(Val));
}
Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  Value *V = NamedValues[Name];
  if (!V)
    LogErrorV("Unknown variable name");
  return V;
}

Value *BinaryExprAST::codegen() {
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;
  switch (Op) {
  case '+':
    // 创建指令的IR
    return Builder.CreateFAdd(L, R, "addtmp");
  case '-':
    return Builder.CreateFSub(L, R, "subtmp");
  case '*':
    return Builder.CreateFMul(L, R, "multmp");
  case '<':
    L = Builder.CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return Builder.CreateUIToFP(L, Type::getDoubleTy(TheContext), "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}

Value *CallExprAST::codegen() {

  // Look up the name in the global module table.
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  // 函数调用使用的IR build指令
  return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

// External一个函数时需要这个
Function *PrototypeAST::codegen() {
  //首先需要创造一个函数类型
  // FIXME: 这个Double是啥
  std::vector<Type *> Doubles(Args.size(), Type::getDoublePtrTy(TheContext));
  //首先创造函数类型的声明
  FunctionType *FT =
      FunctionType::get(Type::getDoubleTy(TheContext), Doubles, false);
  //然后声明函数
  Function *F =
      Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());
  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

//创建一个函数实例的codegen
Function *FunctionAST::codegen() {
  //这里会引入BasicBlock的概念
  // First step : 先判断function的合法性
  Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (!TheFunction)
    TheFunction = Proto->codegen();

  if (!TheFunction)
    return nullptr;

  if (!TheFunction->empty())
    return (Function *)LogErrorV("Function cannot be redefined.");

  // Create a new basic block to start insertation into
  BasicBlock *BB =
      BasicBlock::Create(TheContext, "entry", TheFunction); // TheFunction是
  Builder.SetInsertPoint(BB); // 这里的意思就是后面的指令必须排在这个基本块后面

  // Validate the generated code, checking for consistency
  NamedValues.clear();

  for (auto &Arg : TheFunction->args()) {
    NamedValues[static_cast<std::string>(Arg.getName())] = &Arg;
  }

  if (Value *RetVal = Body->codegen()) {
    // Finish off the function.
    Builder.CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    return TheFunction;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();
  return nullptr;

}