#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "syntax.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>

using Value = llvm::Value;

class Interpreter : public CodegenVisitor, public StmtVisitor {
public:
  Interpreter()
      : context(std::make_unique<llvm::LLVMContext>()),
        module(std::make_unique<llvm::Module>("llox jit", *context)),
        builder(std::make_unique<llvm::IRBuilder<>>(*context)), variables() {};

  // StmtVisitor
  void visit(const ExprStmt &stmt) override;
  void visit(const PrintStmt &stmt) override;
  void visit(const VarStmt &stmt) override;

  void execute(const std::vector<std::unique_ptr<Stmt>> &program);

  // Everything `print` emitted during the last execute().
  std::string output() const;

  // CodegenVisitor
  Value *visit(const Literal &expr) override;
  Value *visit(const Unary &expr) override;
  Value *visit(const Binary &expr) override;
  Value *visit(const Grouping &expr) override;
  Value *visit(const Assign &expr) override;
  Value *visit(const Variable &expr) override;

private:
  llvm::FunctionCallee get_print_fn();
  template <typename... Args> Value *log_error_v(std::format_string<Args...> fmt, Args &&...args) {
    auto message = std::format(fmt, std::forward<Args>(args)...);
    std::cerr << message << "\n";
    return nullptr;
  }

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::ExecutionEngine> engine;
  std::map<std::string, Value *> variables;
  std::string out;
};

#endif
