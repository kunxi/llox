#ifndef SYNTAX_H
#define SYNTAX_H

#include "lexer.h"
#include <memory>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

/*

The syntax defined for lox:

program        → declaration* EOF ;

declaration    → varDecl
               | statement ;

varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;

statement      → exprStmt
               | ifStmt
               | printStmt
               | block ;


ifStmt         → "if" "(" expression ")" statement
               ( "else" statement )? ;


exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;
block          → "{" declaration* "}" ;

expression     → assignment ;
assignment     → IDENTIFIER "=" assignment
               | equality ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")"  | IDENTIFIER;
 */

#include "llvm/IR/Value.h"

using Value = llvm::Value;
struct Literal;
struct Unary;
struct Binary;
struct Grouping;
struct Assign;
struct Variable;
struct ExprStmt;
struct PrintStmt;
struct VarStmt;
struct BlockStmt;
struct IfStmt;

// --- Visitors ---

struct ExprVisitor {
  virtual void visit(const Literal &expr) = 0;
  virtual void visit(const Unary &expr) = 0;
  virtual void visit(const Binary &expr) = 0;
  virtual void visit(const Grouping &expr) = 0;
  virtual void visit(const Assign &expr) = 0;
  virtual void visit(const Variable &expr) = 0;
  virtual ~ExprVisitor() = default;
};

struct CodegenVisitor {
  virtual Value *visit(const Literal &expr) = 0;
  virtual Value *visit(const Unary &expr) = 0;
  virtual Value *visit(const Binary &expr) = 0;
  virtual Value *visit(const Grouping &expr) = 0;
  virtual Value *visit(const Assign &expr) = 0;
  virtual Value *visit(const Variable &expr) = 0;
  virtual ~CodegenVisitor() = default;
};

struct StmtVisitor {
  virtual void visit(const ExprStmt &stmt) = 0;
  virtual void visit(const PrintStmt &stmt) = 0;
  virtual void visit(const VarStmt &stmt) = 0;
  virtual void visit(const BlockStmt &stmt) = 0;
  virtual void visit(const IfStmt &stmt) = 0;
  virtual ~StmtVisitor() = default;
};

// --- Expr ---

struct Expr {
  virtual void accept(ExprVisitor &v) const = 0;
  virtual Value *accept(CodegenVisitor &v) const = 0;
  virtual ~Expr() = default;
};

struct Literal : Expr {
  template <typename T> explicit Literal(T &&val) : value(std::forward<T>(val)) {}
  void accept(ExprVisitor &v) const override { v.visit(*this); }
  Value *accept(CodegenVisitor &v) const override { return v.visit(*this); }
  std::variant<bool, double, std::nullptr_t, std::string> value;
};

struct Unary : Expr {
  Unary(Token op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}
  void accept(ExprVisitor &v) const override { v.visit(*this); }
  Value *accept(CodegenVisitor &v) const override { return v.visit(*this); }
  Token op;
  std::unique_ptr<Expr> right;
};

struct Binary : Expr {
  Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}
  void accept(ExprVisitor &v) const override { v.visit(*this); }
  Value *accept(CodegenVisitor &v) const override { return v.visit(*this); }
  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;
};

struct Grouping : Expr {
  explicit Grouping(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
  void accept(ExprVisitor &v) const override { v.visit(*this); }
  Value *accept(CodegenVisitor &v) const override { return v.visit(*this); }
  std::unique_ptr<Expr> expression;
};

struct Assign : Expr {
  explicit Assign(std::unique_ptr<Expr> lvalue, std::unique_ptr<Expr> rvalue)
      : lvalue(std::move(lvalue)), rvalue(std::move(rvalue)) {}
  void accept(ExprVisitor &v) const override { v.visit(*this); }
  Value *accept(CodegenVisitor &v) const override { return v.visit(*this); }

  std::unique_ptr<Expr> lvalue;
  std::unique_ptr<Expr> rvalue;
};

struct Variable : Expr {
  explicit Variable(Token name) : name(name) {};
  void accept(ExprVisitor &v) const override { v.visit(*this); }
  Value *accept(CodegenVisitor &v) const override { return v.visit(*this); }

  Token name;
};

// --- Stmt ---

struct Stmt {
  virtual void accept(StmtVisitor &v) const = 0;
  virtual ~Stmt() = default;
};

struct ExprStmt : Stmt {
  ExprStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
  void accept(StmtVisitor &v) const override { v.visit(*this); }
  std::unique_ptr<Expr> expression;
};

struct PrintStmt : Stmt {
  PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
  void accept(StmtVisitor &v) const override { v.visit(*this); }
  std::unique_ptr<Expr> expression;
};

struct VarStmt : Stmt {
  VarStmt(Token name, std::unique_ptr<Expr> initializer)
      : name(name), initializer(std::move(initializer)) {}
  void accept(StmtVisitor &v) const override { v.visit(*this); }

  Token name;
  std::unique_ptr<Expr> initializer;
};

struct BlockStmt : Stmt {
  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)) {}
  void accept(StmtVisitor &v) const override { v.visit(*this); }

  std::vector<std::unique_ptr<Stmt>> statements;
};

struct IfStmt : Stmt {
  explicit IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> then_branch,
                  std::unique_ptr<Stmt> else_branch)
      : condition(std::move(condition)), then_branch(std::move(then_branch)),
        else_branch(std::move(else_branch)) {}
  void accept(StmtVisitor &v) const override { v.visit(*this); }

  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> then_branch;
  std::unique_ptr<Stmt> else_branch;
};

// --- PrintVisitor ---

struct PrintVisitor : ExprVisitor {
  std::ostream &os;
  explicit PrintVisitor(std::ostream &os) : os(os) {}

  void visit(const Literal &expr) override {
    std::visit(
        [this](const auto &v) {
          if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::nullptr_t>)
            os << "nil";
          else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>)
            os << '"' << v << '"';
          else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, bool>)
            os << (v ? "true" : "false");
          else
            os << v;
        },
        expr.value);
  }

  void visit(const Unary &expr) override {
    os << "(" << expr.op.lexeme << " ";
    expr.right->accept(*this);
    os << ")";
  }

  void visit(const Binary &expr) override {
    os << "(" << expr.op.lexeme << " ";
    expr.left->accept(*this);
    os << " ";
    expr.right->accept(*this);
    os << ")";
  }

  void visit(const Grouping &expr) override {
    os << "(group ";
    expr.expression->accept(*this);
    os << ")";
  }

  void visit(const Assign &expr) override {
    os << "(= ";
    expr.lvalue->accept(*this);
    os << " ";
    expr.rvalue->accept(*this);
  }

  void visit(const Variable &expr) override { os << "(" << expr.name.lexeme << ")"; }
};

#endif
