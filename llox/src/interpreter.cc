#include "interpreter.h"
#include "syntax.h"
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"

using namespace llvm;

// Called from JIT'd code instead of printf. `sink` is the address of the
// active Interpreter's output buffer, threaded through the IR as an argument.
extern "C" void llox_print(std::string *sink, double value) {
  char buf[64];
  snprintf(buf, sizeof buf, "%g", value);
  *sink += buf;
  *sink += '\n';
}

std::string Interpreter::output() const {
  return out;
}

void Interpreter::execute(const std::vector<std::unique_ptr<Stmt>> &program) {
  out.clear();

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
  // Make the host `llox_print` symbol resolvable by the JIT'd code.
  engine->addGlobalMapping("llox_print", reinterpret_cast<uint64_t>(&llox_print));
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

  auto *i8ptr = llvm::PointerType::get(*context, 0);
  auto *addr =
      llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), reinterpret_cast<uint64_t>(&out));
  auto *sink = builder->CreateIntToPtr(addr, i8ptr, "sink");
  builder->CreateCall(get_print_fn(), {sink, val});
}

void Interpreter::visit(const VarStmt &stmt) {
  Value *val = nullptr;
  if (stmt.initializer != nullptr) {
    val = stmt.initializer->accept(*this);
  }

  // Declare the variable in the current scope. `operator[]` overwrites an
  // existing binding so an inner declaration shadows an outer one.
  scopes.back()[stmt.name.lexeme] = val;
}

void Interpreter::visit(const BlockStmt &stmt) {
  // Push a fresh scope for the block, then pop it when the block ends.
  scopes.emplace_back();
  for (const auto &statement : stmt.statements) {
    statement->accept(*this);
  }
  scopes.pop_back();
}

void Interpreter::visit(const IfStmt &stmt) {
  Value *condition = stmt.condition->accept(*this);
  if (!condition)
    return;

  // Comparison expressions already yield i1; numeric values need truthiness
  // conversion (nonzero => true).
  if (!condition->getType()->isIntegerTy(1))
    condition =
        builder->CreateFCmpONE(condition, ConstantFP::get(*context, APFloat(0.0)), "ifcond");

  Function *function = builder->GetInsertBlock()->getParent();
  BasicBlock *then_bb = BasicBlock::Create(*context, "then", function);
  BasicBlock *else_bb = BasicBlock::Create(*context, "else");
  BasicBlock *merge_bb = BasicBlock::Create(*context, "ifcont");

  builder->CreateCondBr(condition, then_bb, else_bb);

  // Each branch mutates its own copy of the scope stack; the copies are
  // phi-merged back together in the merge block.
  auto base = scopes;

  // Then branch.
  builder->SetInsertPoint(then_bb);
  scopes = base;
  stmt.then_branch->accept(*this);
  auto then_scopes = scopes;
  builder->CreateBr(merge_bb);
  BasicBlock *then_pred = builder->GetInsertBlock();

  // Else branch (empty when there is no `else`).
  function->insert(function->end(), else_bb);
  builder->SetInsertPoint(else_bb);
  scopes = base;
  if (stmt.else_branch)
    stmt.else_branch->accept(*this);
  auto else_scopes = scopes;
  builder->CreateBr(merge_bb);
  BasicBlock *else_pred = builder->GetInsertBlock();

  // Merge block.
  function->insert(function->end(), merge_bb);
  builder->SetInsertPoint(merge_bb);
  scopes = merge_scopes(base, then_scopes, else_scopes, then_pred, else_pred);
}

Interpreter::ScopeStack Interpreter::merge_scopes(const ScopeStack &base,
                                                  const ScopeStack &then_scopes,
                                                  const ScopeStack &else_scopes,
                                                  BasicBlock *then_pred, BasicBlock *else_pred) {
  ScopeStack merged = base;
  auto *fty = llvm::Type::getDoubleTy(*context);

  for (size_t i = 0; i < base.size(); ++i) {
    for (const auto &kv : base[i]) {
      const std::string &name = kv.first;
      Value *then_val = then_scopes[i].at(name);
      Value *else_val = else_scopes[i].at(name);
      if (then_val == else_val)
        continue; // unchanged in both branches; base value already dominates the join

      PHINode *phi = builder->CreatePHI(fty, 2, name);
      phi->addIncoming(then_val, then_pred);
      phi->addIncoming(else_val, else_pred);
      merged[i][name] = phi;
    }
  }

  return merged;
}

// CodegenVisitor
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
    return log_error_v("Invalid unary operator: {}", unary.op.lexeme);
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
  case TOKEN_LESS_EQUAL:
    return builder->CreateFCmpULE(L, R, "cmptmp");
  case TOKEN_GREATER:
    return builder->CreateFCmpUGT(L, R, "cmptmp");
  case TOKEN_GREATER_EQUAL:
    return builder->CreateFCmpUGE(L, R, "cmptmp");
  default:
    return log_error_v("Invalid binary operator: {}", binary.op.lexeme);
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
    // Resolve from the innermost scope outward.
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
      auto found = it->find(name);
      if (found != it->end()) {
        auto rval = assign.rvalue->accept(*this);
        found->second = rval;
        return rval;
      }
    }
    return log_error_v("name is not declared");
  }
  throw std::runtime_error("Cannot assign to lvalue");
}

Value *Interpreter::visit(const Variable &variable) {
  const auto &name = variable.name.lexeme;
  // Resolve from the innermost scope outward.
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end()) {
      return found->second;
    }
  }
  return log_error_v("name is not declared");
}

llvm::FunctionCallee Interpreter::get_print_fn() {
  auto *ty = FunctionType::get(
      llvm::Type::getVoidTy(*context),
      {llvm::PointerType::get(*context, 0), llvm::Type::getDoubleTy(*context)}, false);
  return module->getOrInsertFunction("llox_print", ty);
}
