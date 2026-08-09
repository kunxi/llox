#ifndef SYNTAX_H
#define SYNTAX_H

#include <variant>
#include <memory>
#include <ostream>

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

struct Expr {
  virtual ~Expr() = default;
  virtual void print(std::ostream& os, int indent = 0) const = 0;

protected:
  void indent(std::ostream& os, int depth) const {
    std::string indent(depth * 2, ' ');
    os << indent;
  }
};

struct Literal : Expr {
  template<typename T>
  explicit Literal(T&& val)
    : value(std::forward<T>(val)) {}

  void print(std::ostream& os, int n) const override {
    indent(os, n);
    os << "literal: ";
    std::visit([&](auto& v) { os << v; }, value);
    os << "\n";
  }

  std::variant<bool, double, std::string> value;
};

struct Unary : Expr {
  Unary(Token op, std::unique_ptr<Expr> right)
    : op(op), right(std::move(right)) {}

  void print(std::ostream& os, int n) const override {
    indent(os, n);
    os << "unary: " << op.lexeme << "\n";
    right->print(os, n + 1);
  }

  Token op;
  std::unique_ptr<Expr> right;
};

struct Binary : Expr {
  Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
    : left(std::move(left)), op(op), right(std::move(right)) {}

  void print(std::ostream& os, int n) const override {
    indent(os, n);
    os << "binary: " << op.lexeme << "\n";
    left->print(os, n + 1);
    right->print(os, n + 1);
  }

  std::unique_ptr<Expr> left;
  Token op;
  std::unique_ptr<Expr> right;
};

struct Grouping : Expr {
  explicit Grouping(std::unique_ptr<Expr> expression)
    : expression(std::move(expression)) {}

  void print(std::ostream& os, int n) const override {
    indent(os, n);
    os << "grouping\n";
    expression->print(os, n + 1);
  }

  std::unique_ptr<Expr> expression;
};


#endif
