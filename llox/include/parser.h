#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <initializer_list>
#include <iostream>
#include "lexer.h"
#include "syntax.h"


class Parser {
public:
  Parser(): tokens(), current(0) {};
  // expr
  std::unique_ptr<Expr> expression();
  std::unique_ptr<Expr> equality();
  std::unique_ptr<Expr> comparison();
  std::unique_ptr<Expr> term();
  std::unique_ptr<Expr> factor();
  std::unique_ptr<Expr> unary();
  std::unique_ptr<Expr> primary();

  // core functions
  void parse(std::istream &stream, std::ostream &out);

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
  int current;
};

#endif
