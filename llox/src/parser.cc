#include "parser.h"
#include <FlexLexer.h>

// Expr
std::unique_ptr<Expr> Parser::binary(std::unique_ptr<Expr> (Parser::*next)(),
                                     std::initializer_list<TokenType> types) {
  auto expr = (this->*next)();
  while (match(types)) {
    Token op = previous();
    auto right = (this->*next)();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::expression() {
  return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
  auto lvalue = equality();
  if (match({TOKEN_EQUAL})) {
    auto equals = previous();
    auto rvalue = assignment();
    return make_unique<Assign>(std::move(lvalue), std::move(rvalue));
  }
  return lvalue;
}

std::unique_ptr<Expr> Parser::equality() {
  return binary(&Parser::comparison, {TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL});
}

std::unique_ptr<Expr> Parser::comparison() {
  return binary(&Parser::term, {TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL});
}

std::unique_ptr<Expr> Parser::term() {
  return binary(&Parser::factor, {TOKEN_MINUS, TOKEN_PLUS});
}

std::unique_ptr<Expr> Parser::factor() {
  return binary(&Parser::unary, {TOKEN_SLASH, TOKEN_STAR});
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({TOKEN_BANG, TOKEN_MINUS})) {
    Token op = previous();
    auto right = unary();
    return std::make_unique<Unary>(op, std::move(right));
  }
  return primary();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match({TOKEN_FALSE}))
    return std::make_unique<Literal>(false);
  if (match({TOKEN_TRUE}))
    return std::make_unique<Literal>(true);
  if (match({TOKEN_NIL}))
    return std::make_unique<Literal>(nullptr);
  if (match({TOKEN_NUMBER}))
    return std::make_unique<Literal>(std::stod(previous().lexeme));
  if (match({TOKEN_STRING})) {
    auto lexeme = previous().lexeme;
    // ponytail: strip surrounding double quotes
    return std::make_unique<Literal>(lexeme.substr(1, lexeme.size() - 2));
  }
  if (match({TOKEN_IDENTIFIER}))
    return std::make_unique<Variable>(previous());

  if (match({TOKEN_LEFT_PAREN})) {
    auto expr = expression();
    if (!match({TOKEN_RIGHT_PAREN}))
      throw std::runtime_error("Expect ')' after expression.");
    return std::make_unique<Grouping>(std::move(expr));
  }

  throw std::runtime_error("Expect expression.");
}

// Stmt
std::unique_ptr<Stmt> Parser::declaration() {
  try {
    if (match({TOKEN_VAR})) {
      Token name = consume(TOKEN_IDENTIFIER, "Expect variable name.");

      std::unique_ptr<Expr> initializer = nullptr;
      if (match({TOKEN_EQUAL})) {
        initializer = expression();
      }
      consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
      return make_unique<VarStmt>(name, std::move(initializer));
    }

    return statement();
  } catch (std::runtime_error &error) {
    // sync
    return nullptr;
  }
}

std::unique_ptr<Stmt> Parser::statement() {
  if (match({TOKEN_PRINT}))
    return print_stmt();
  return expr_stmt();
}

std::unique_ptr<Stmt> Parser::expr_stmt() {
  auto expr = expression();
  consume(TOKEN_SEMICOLON, "Expect semicolon");
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::print_stmt() {
  auto expr = expression();
  consume(TOKEN_SEMICOLON, "Expect semicolon");
  return std::make_unique<PrintStmt>(std::move(expr));
}

// Core function: tokenize and parse
void Parser::tokenize(std::istream &stream) {
  FlexLexer *lexer = new yyFlexLexer(&stream);
  int token_type;
  do {
    token_type = lexer->yylex();
    tokens.push_back(Token(static_cast<TokenType>(token_type), lexer->YYText()));
  } while (token_type != TOKEN_EOF);
}

void Parser::parse() {
  while (!ends()) {
    statements.push_back(declaration());
  }
}

// Utility functions
bool Parser::match(std::initializer_list<TokenType> types) {
  for (auto type : types) {
    if (check(type)) {
      advance();
      return true;
    }
  }
  return false;
}

bool Parser::check(TokenType type) {
  if (ends())
    return false;
  return peek().type == type;
}

Token Parser::peek() {
  return tokens.at(current);
}

Token Parser::advance() {
  if (!ends())
    current++;
  return previous();
}

Token Parser::previous() {
  return tokens.at(current - 1);
}

Token Parser::consume(TokenType type, std::string message) {
  if (check(type))
    return advance();
  throw std::runtime_error(message);
}

bool Parser::ends() {
  return peek().type == TOKEN_EOF;
}
