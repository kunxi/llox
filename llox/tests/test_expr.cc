#include "parser.h"
#include <cassert>
#include <iostream>
#include <sstream>

static int tests = 0;

#define TEST(name)            \
  do { tests++;               \
       std::cout << "  " << #name << "... "; } while (0)
#define PASS() std::cout << "ok\n"

template <typename T> static T *as(Expr *e) { return dynamic_cast<T *>(e); }

static std::unique_ptr<Expr> parse(const std::string &src) {
  Parser parser;
  std::istringstream stream(src);
  parser.tokenize(stream);
  return parser.expression();
}

static Literal *expectLit(Expr *e) { auto *l = as<Literal>(e); assert(l); return l; }
static Unary *expectUnary(Expr *e) { auto *u = as<Unary>(e); assert(u); return u; }
static Binary *expectBin(Expr *e) { auto *b = as<Binary>(e); assert(b); return b; }
static Grouping *expectGroup(Expr *e) { auto *g = as<Grouping>(e); assert(g); return g; }

static bool isTrue(const Literal *l) {
  if (auto *b = std::get_if<bool>(&l->value)) return *b;
  return false;
}
static bool isFalse(const Literal *l) {
  if (auto *b = std::get_if<bool>(&l->value)) return !*b;
  return false;
}
static bool isNil(const Literal *l) {
  return std::holds_alternative<std::nullptr_t>(l->value);
}
static bool isNum(const Literal *l, double val) {
  if (auto *d = std::get_if<double>(&l->value)) return *d == val;
  return false;
}

// ponytail: save parse() result in a variable so unique_ptr outlives raw pointers

static void test_literals() {
  TEST(test_literal_true); {
    auto e = parse("true");
    assert(isTrue(expectLit(e.get())));
  } PASS();

  TEST(test_literal_false); {
    auto e = parse("false");
    assert(isFalse(expectLit(e.get())));
  } PASS();

  TEST(test_literal_nil); {
    auto e = parse("nil");
    assert(isNil(expectLit(e.get())));
  } PASS();

  TEST(test_literal_number); {
    auto e = parse("42");
    assert(isNum(expectLit(e.get()), 42.0));
  } PASS();

  TEST(test_literal_string); {
    auto e = parse("\"hello\"");
    auto *l = expectLit(e.get());
    auto *s = std::get_if<std::string>(&l->value);
    assert(s && *s == "hello");
  } PASS();
}

static void test_unary() {
  TEST(test_unary_bang); {
    auto e = parse("!true");
    auto *u = expectUnary(e.get());
    assert(u->op.type == TOKEN_BANG);
  } PASS();

  TEST(test_unary_minus); {
    auto e = parse("-5");
    auto *u = expectUnary(e.get());
    assert(u->op.type == TOKEN_MINUS);
  } PASS();

  TEST(test_unary_nested); {
    auto e = parse("!!false");
    auto *u1 = expectUnary(e.get());
    assert(u1->op.type == TOKEN_BANG);
    auto *u2 = expectUnary(u1->right.get());
    assert(u2->op.type == TOKEN_BANG);
  } PASS();
}

static void test_binary() {
  #define CHECK_OP(src, tok) do { auto e = parse(src); assert(expectBin(e.get())->op.type == tok); } while(0)
  TEST(test_mul); CHECK_OP("3 * 4", TOKEN_STAR); PASS();
  TEST(test_div); CHECK_OP("8 / 2", TOKEN_SLASH); PASS();
  TEST(test_add); CHECK_OP("1 + 2", TOKEN_PLUS); PASS();
  TEST(test_sub); CHECK_OP("5 - 3", TOKEN_MINUS); PASS();
  TEST(test_gt);  CHECK_OP("a > b", TOKEN_GREATER); PASS();
  TEST(test_le);  CHECK_OP("x <= 10", TOKEN_LESS_EQUAL); PASS();
  TEST(test_eq);  CHECK_OP("x == nil", TOKEN_EQUAL_EQUAL); PASS();
  TEST(test_ne);  CHECK_OP("a != b", TOKEN_BANG_EQUAL); PASS();
  #undef CHECK_OP

  TEST(test_mul_values); {
    auto e = parse("3 * 4");
    auto *b = expectBin(e.get());
    assert(isNum(expectLit(b->left.get()), 3.0));
    assert(isNum(expectLit(b->right.get()), 4.0));
  } PASS();
}

static void test_grouping() {
  TEST(test_grouping); {
    auto e = parse("(42)");
    auto *g = expectGroup(e.get());
    assert(isNum(expectLit(g->expression.get()), 42.0));
  } PASS();
}

static void test_precedence() {
  TEST(test_mul_before_add); {
    auto e = parse("1 + 2 * 3");
    auto *b = expectBin(e.get());
    assert(b->op.type == TOKEN_PLUS);
    auto *inner = expectBin(b->right.get());
    assert(inner->op.type == TOKEN_STAR);
  } PASS();

  TEST(test_grouping_overrides); {
    auto e = parse("(1 + 2) * 3");
    auto *b = expectBin(e.get());
    assert(b->op.type == TOKEN_STAR);
    assert(as<Grouping>(b->left.get()));
  } PASS();

  TEST(test_comparison_chains); {
    auto e = parse("1 < 2 == true");
    auto *b = expectBin(e.get());
    assert(b->op.type == TOKEN_EQUAL_EQUAL);
  } PASS();
}

int main() {
  std::cout << "Running parser tests...\n";
  test_literals();
  test_unary();
  test_binary();
  test_grouping();
  test_precedence();
  std::cout << "\nAll " << tests << " tests passed.\n";
  return 0;
}
