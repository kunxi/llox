#include "interpreter.h"
#include "syntax.h"
#include <iostream>

#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"

using namespace llvm;

void Interpreter::execute(const std::vector<std::unique_ptr<Stmt>> &program) {
  // Setup the __main function
  auto *ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
  auto *mainFn =
      llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "__main", module.get());
  auto *bb = llvm::BasicBlock::Create(*context, "entry", mainFn);
  builder->SetInsertPoint(bb);

  for (const auto &stmt : program) {
    stmt->accept(*this);
  }

  builder->CreateRetVoid();

  // ponytail: EngineBuilder takes ownership of the module
  std::string err;
  engine.reset(llvm::EngineBuilder(std::move(module))
                   .setEngineKind(llvm::EngineKind::Interpreter)
                   .setErrorStr(&err)
                   .create());
  if (!engine) {
    std::cerr << "EngineBuilder failed: " << err << "\n";
    return;
  }
  engine->runFunction(engine->FindFunctionNamed("__main"), {});

  engine.reset();
}

void Interpreter::visit(const ExprStmt &stmt) {
  stmt.expression->accept(*this);
}

void Interpreter::visit(const PrintStmt &stmt) {
  Value *val = stmt.expression->accept(*this);
  if (!val)
    return;

  auto printf = get_printf();
  auto *fmt = builder->CreateGlobalString("%g\n");
  builder->CreateCall(printf, {fmt, val});
}

void Interpreter::visit(const VarStmt &stmt) {
  Value *val = nullptr;
  if (stmt.initializer != nullptr) {
    val = stmt.initializer->accept(*this);
  }

  // Resolve to assign expr.
  variables.insert({stmt.name.lexeme, val});
}

Value *Interpreter::visit(const Literal &literal) {
  if (auto pval = std::get_if<double>(&literal.value)) {
    return ConstantFP::get(*context, APFloat(*pval));
  }
  if (auto pval = std::get_if<bool>(&literal.value)) {
    return ConstantFP::get(*context, APFloat(*pval ? 1.0 : 0.0));
  }
  return nullptr;
}

Value *Interpreter::visit(const Unary &unary) {
  Value *right = unary.right->accept(*this);

  switch (unary.op.type) {
  case TOKEN_MINUS:
    return builder->CreateFNeg(right, "negtmp");
  case TOKEN_BANG:
    return builder->CreateNot(right, "nottmp");
  default:
    return log_error_v("invalid unary operator");
  }
}

Value *Interpreter::visit(const Binary &binary) {
  Value *L = binary.left->accept(*this);
  Value *R = binary.right->accept(*this);
  if (!L || !R)
    return nullptr;

  switch (binary.op.type) {
  case TOKEN_PLUS:
    return builder->CreateFAdd(L, R, "addtmp");
  case TOKEN_MINUS:
    return builder->CreateFSub(L, R, "subtmp");
  case TOKEN_STAR:
    return builder->CreateFMul(L, R, "multmp");
  case TOKEN_LESS:
    return builder->CreateFCmpULT(L, R, "cmptmp");
  default:
    return log_error_v("invalid binary operator");
  }
}

Value *Interpreter::visit(const Grouping &grouping) {
  return grouping.expression->accept(*this);
}

Value *Interpreter::visit(const Assign &assign) {
  // Make sure the variable is declared.
  auto lval = dynamic_cast<Variable *>(assign.lvalue.get());
  if (lval != nullptr) {
    auto name = lval->name.lexeme;
    auto it = variables.find(name);
    if (it == variables.end()) {
      return log_error_v("name is not declared");
    }
    auto rval = assign.rvalue->accept(*this);
    it->second = rval;
    return rval;
  }
  throw std::runtime_error("cannot assign to lvalue");
}

Value *Interpreter::visit(const Variable &variable) {
  Value *val = variables.at(variable.name.lexeme);
  return val;
}

llvm::FunctionCallee Interpreter::get_printf() {
  auto *ty = FunctionType::get(llvm::Type::getInt32Ty(*context),
                               {llvm::PointerType::get(*context, 0)}, true);
  return module->getOrInsertFunction("printf", ty);
}

Value *Interpreter::log_error_v(const char *message) {
  std::cerr << message << "\n";
  return nullptr;
}
