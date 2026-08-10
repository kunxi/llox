#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "syntax.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include <memory>

using Value = llvm::Value;

class Interpreter : public CodegenVisitor {
public:
  Interpreter()
      : context(std::make_unique<llvm::LLVMContext>()),
        module(std::make_unique<llvm::Module>("llox jit", *context)),
        builder(std::make_unique<llvm::IRBuilder<>>(*context)) {};

  void execute(ExprStmt& stmt) { (void)stmt; }
  void execute(PrintStmt& stmt);
  void execute(const std::vector<std::unique_ptr<Stmt>> &program);

  // CodegenVisitor
  Value* visit(const Literal& expr) override;
  Value* visit(const Unary& expr) override;
  Value* visit(const Binary& expr) override;
  Value* visit(const Grouping& expr) override;

private:
  // Utility
  llvm::FunctionCallee get_printf();
  Value* log_error_v(const char* s);

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::ExecutionEngine> engine;
};

#endif
