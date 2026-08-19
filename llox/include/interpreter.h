#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "syntax.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using Value = llvm::Value;

class Interpreter : public CodegenVisitor, public StmtVisitor {
public:
  Interpreter()
      : context(std::make_unique<llvm::LLVMContext>()),
        module(std::make_unique<llvm::Module>("llox jit", *context)),
        builder(std::make_unique<llvm::IRBuilder<>>(*context)) {
    scopes.emplace_back();
  }

  // StmtVisitor
  void visit(const ExprStmt &stmt) override;
  void visit(const PrintStmt &stmt) override;
  void visit(const VarStmt &stmt) override;
  void visit(const BlockStmt &stmt) override;
  void visit(const IfStmt &stmt) override;

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
  using Scope = std::map<std::string, Value *>;
  using ScopeStack = std::vector<Scope>;

  llvm::FunctionCallee get_print_fn();

  // Phi-merge the two branch scope-stack copies back into one stack at the join.
  ScopeStack merge_scopes(const ScopeStack &base, const ScopeStack &then_scopes,
                          const ScopeStack &else_scopes, llvm::BasicBlock *then_pred,
                          llvm::BasicBlock *else_pred);

  template <typename... Args> Value *log_error_v(std::format_string<Args...> fmt, Args &&...args) {
    auto message = std::format(fmt, std::forward<Args>(args)...);
    std::cerr << message << "\n";
    return nullptr;
  }

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::ExecutionEngine> engine;
  ScopeStack scopes;
  std::string out;
};

#endif
