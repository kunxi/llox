#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "syntax.h"
#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>


class Parser {
public:

  Parser() : tokens(), program(), current(0) {};
  // expr
  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> primary();


  // statement
  std::unique_ptr<Stmt> statement();
  std::unique_ptr<Stmt> expr_stmt();
  std::unique_ptr<Stmt> print_stmt();

  // core functions
  void tokenize(std::istream &stream);
  void parse();

private:
  std::unique_ptr<Expr> binary(std::unique_ptr<Expr> (Parser::*next)(),
                               std::initializer_list<TokenType> types);
  bool match(std::initializer_list<TokenType> types);
  bool check(TokenType type);
  Token peek();
  Token advance();
  Token previous();
  Token consume(TokenType type, std::string message);
  bool ends();

  std::vector<Token> tokens;
  std::vector<std::unique_ptr<Stmt>> program;
  int current;
};

#endif
