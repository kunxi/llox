#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "syntax.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <map>
#include <memory>

using Value = llvm::Value;

class Interpreter : public CodegenVisitor, public StmtVisitor {
public:
  Interpreter()
      : context(std::make_unique<llvm::LLVMContext>()),
        module(std::make_unique<llvm::Module>("llox jit", *context)),
        builder(std::make_unique<llvm::IRBuilder<>>(*context)), variables() {
    // ponytail: create a dummy function so the builder has somewhere to insert
    auto *ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
    auto *fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                      "__llox_main", *module);
    auto *bb = llvm::BasicBlock::Create(*context, "entry", fn);
    builder->SetInsertPoint(bb);
  };

  // StmtVisitor
  void visit(const ExprStmt &stmt) override;
  void visit(const PrintStmt &stmt) override;
  void visit(const VarStmt &stmt) override;

  void execute(const std::vector<std::unique_ptr<Stmt>> &program);

  // CodegenVisitor
  Value *visit(const Literal &expr) override;
  Value *visit(const Unary &expr) override;
  Value *visit(const Binary &expr) override;
  Value *visit(const Grouping &expr) override;

private:
  llvm::FunctionCallee get_printf();
  Value *log_error_v(const char *s);

  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::ExecutionEngine> engine;
  std::map<std::string, Value *> variables;
};

#endif
