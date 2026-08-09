#ifndef SYNTAX_H
#define SYNTAX_H

#include <variant>
#include <memory>
#include <ostream>
#include <string>

/*

The syntax defined for lox:

program        → statement* EOF ;

statement      → exprStmt
               | printStmt ;

exprStmt       → expression ";" ;
printStmt      → "print" expression ";" ;

expression     → equality ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" ;
 */

struct Literal;
struct Unary;
struct Binary;
struct Grouping;

struct ExprVisitor {
  virtual void visit(const Literal& expr) = 0;
  virtual void visit(const Unary& expr) = 0;
  virtual void visit(const Binary& expr) = 0;
  virtual void visit(const Grouping& expr) = 0;
  virtual ~ExprVisitor() = default;
};

struct Expr {
  virtual void accept(ExprVisitor& v) const = 0;
  virtual ~Expr() = default;
};

struct Literal : Expr {
  template<typename T>
  explicit Literal(T&& val) : value(std::forward<T>(val)) {}

  void accept(ExprVisitor& v) const override { v.visit(*this); }

  std::variant<bool, double, std::nullptr_t, std::string> value;
};

struct Unary : Expr {
  Unary(Token op, std::unique_ptr<Expr> right)
    : op(op), right(std::move(right)) {}

  void accept(ExprVisitor& v) const override { v.visit(*this); }

  Token op;
  std::unique_ptr<Expr> right;
};

struct Binary : Expr {
  Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}

  void accept(ExprVisitor& v) const override { v.visit(*this); }

  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;
};

struct Grouping : Expr {
  explicit Grouping(std::unique_ptr<Expr> expression)
    : expression(std::move(expression)) {}

  void accept(ExprVisitor& v) const override { v.visit(*this); }

  std::unique_ptr<Expr> expression;
};


/*
 * Stmt is distinct with the Expr.
 */
struct Stmt {
  virtual ~Stmt() = default;
};

struct PrintStmt : Stmt {
  PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {};

  std::unique_ptr<Expr> expression;
};

struct ExprStmt: Stmt {
  ExprStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {};

  std::unique_ptr<Expr> expression;
};


// PrintVisitor: parenthesized output, e.g. (+ 1 2), (! true)
struct PrintVisitor : ExprVisitor {
  std::ostream& os;
  explicit PrintVisitor(std::ostream& os) : os(os) {}

  void visit(const Literal& expr) override {
    std::visit([this](const auto& v) {
      if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::nullptr_t>)
        os << "nil";
      else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>)
        os << v;
      else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, bool>)
        os << (v ? "true" : "false");
      else
        os << v;
    }, expr.value);
  }

  void visit(const Unary& expr) override {
    os << "(" << expr.op.lexeme << " ";
    expr.right->accept(*this);
    os << ")";
  }

  void visit(const Binary& expr) override {
    os << "(" << expr.op.lexeme << " ";
    expr.left->accept(*this);
    os << " ";
    expr.right->accept(*this);
    os << ")";
  }

  void visit(const Grouping& expr) override {
    os << "(group ";
    expr.expression->accept(*this);
    os << ")";
  }
};

#endif
