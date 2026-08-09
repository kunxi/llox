#include "parser.h"
#include <cassert>
#include <iostream>
#include <sstream>

#define TEST(name)                                                             \
  do {                                                                         \
    tests++;                                                                   \
    std::cout << "  " << #name << "... ";                                      \
  } while (0)
#define PASS() std::cout << "ok\n"

static int tests = 0;

template <typename T> static T *as(Expr *e) { return dynamic_cast<T *>(e); }

static std::unique_ptr<Expr> parse(const std::string &src) {
  Parser parser;
  std::istringstream stream(src);
  parser.tokenize(stream);
  return parser.expression();
}

static void test_literal_true() {
  TEST(test_literal_true);
  auto e = parse("true;");
  auto *l = as<Literal>(e.get());
  assert(l && std::get<bool>(l->value) == true);
  PASS();
}

static void test_literal_false() {
  TEST(test_literal_false);
  auto e = parse("false;");
  auto *l = as<Literal>(e.get());
  assert(l && std::get<bool>(l->value) == false);
  PASS();
}

static void test_literal_nil() {
  TEST(test_literal_nil);
  auto e = parse("nil;");
  auto *l = as<Literal>(e.get());
  assert(l && std::holds_alternative<std::nullptr_t>(l->value));
  PASS();
}

static void test_literal_number() {
  TEST(test_literal_number);
  auto e = parse("42;");
  auto *l = as<Literal>(e.get());
  assert(l && std::get<std::string>(l->value) == "42");
  PASS();
}

static void test_literal_string() {
  TEST(test_literal_string);
  auto e = parse("\"hello\";");
  auto *l = as<Literal>(e.get());
  assert(l && std::get<std::string>(l->value) == "\"hello\"");
  PASS();
}

static void test_unary_bang() {
  TEST(test_unary_bang);
  auto e = parse("!true;");
  auto *u = as<Unary>(e.get());
  assert(u && u->op.type == TOKEN_BANG);
  auto *l = as<Literal>(u->right.get());
  assert(l && std::get<bool>(l->value) == true);
  PASS();
}

static void test_unary_minus() {
  TEST(test_unary_minus);
  auto e = parse("-5;");
  auto *u = as<Unary>(e.get());
  assert(u && u->op.type == TOKEN_MINUS);
  PASS();
}

static void test_unary_nested() {
  TEST(test_unary_nested);
  auto e = parse("!!false;");
  auto *u1 = as<Unary>(e.get());
  assert(u1 && u1->op.type == TOKEN_BANG);
  auto *u2 = as<Unary>(u1->right.get());
  assert(u2 && u2->op.type == TOKEN_BANG);
  PASS();
}

static void test_factor_multiply() {
  TEST(test_factor_multiply);
  auto e = parse("3 * 4;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_STAR);
  assert(std::get<std::string>(as<Literal>(b->left.get())->value) == "3");
  assert(std::get<std::string>(as<Literal>(b->right.get())->value) == "4");
  PASS();
}

static void test_factor_divide() {
  TEST(test_factor_divide);
  auto e = parse("8 / 2;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_SLASH);
  PASS();
}

static void test_term_add() {
  TEST(test_term_add);
  auto e = parse("1 + 2;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_PLUS);
  PASS();
}

static void test_term_subtract() {
  TEST(test_term_subtract);
  auto e = parse("5 - 3;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_MINUS);
  PASS();
}

static void test_comparison_greater() {
  TEST(test_comparison_greater);
  auto e = parse("a > b;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_GREATER);
  PASS();
}

static void test_comparison_less_equal() {
  TEST(test_comparison_less_equal);
  auto e = parse("x <= 10;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_LESS_EQUAL);
  PASS();
}

static void test_equality_equal_equal() {
  TEST(test_equality_equal_equal);
  auto e = parse("x == nil;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_EQUAL_EQUAL);
  PASS();
}

static void test_equality_bang_equal() {
  TEST(test_equality_bang_equal);
  auto e = parse("a != b;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_BANG_EQUAL);
  PASS();
}

static void test_grouping() {
  TEST(test_grouping);
  auto e = parse("(42);");
  auto *g = as<Grouping>(e.get());
  assert(g);
  auto *l = as<Literal>(g->expression.get());
  assert(l && std::get<std::string>(l->value) == "42");
  PASS();
}

static void test_precedence_mul_before_add() {
  TEST(test_precedence_mul_before_add);
  // 1 + 2 * 3  →  (+ 1 (* 2 3))
  auto e = parse("1 + 2 * 3;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_PLUS);
  assert(as<Literal>(b->left.get()));
  auto *inner = as<Binary>(b->right.get());
  assert(inner && inner->op.type == TOKEN_STAR);
  PASS();
}

static void test_precedence_grouping_overrides() {
  TEST(test_precedence_grouping_overrides);
  // (1 + 2) * 3  →  (* (group (+ 1 2)) 3)
  auto e = parse("(1 + 2) * 3;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_STAR);
  assert(as<Grouping>(b->left.get()));
  assert(as<Literal>(b->right.get()));
  PASS();
}

static void test_comparison_chains() {
  TEST(test_comparison_chains);
  // 1 < 2 == true  →  (== (< 1 2) true) — left-associative
  auto e = parse("1 < 2 == true;");
  auto *b = as<Binary>(e.get());
  assert(b && b->op.type == TOKEN_EQUAL_EQUAL);
  PASS();
}

int main() {
  std::cout << "Running parser tests...\n";

  test_literal_true();
  test_literal_false();
  test_literal_nil();
  test_literal_number();
  test_literal_string();

  test_unary_bang();
  test_unary_minus();
  test_unary_nested();

  test_factor_multiply();
  test_factor_divide();
  test_term_add();
  test_term_subtract();

  test_comparison_greater();
  test_comparison_less_equal();
  test_equality_equal_equal();
  test_equality_bang_equal();

  test_grouping();
  test_precedence_mul_before_add();
  test_precedence_grouping_overrides();
  test_comparison_chains();

  std::cout << "\nAll " << tests << " tests passed.\n";
  return 0;
}
