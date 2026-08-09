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

// Value check helpers (flex keyword/number tokenization is compiler-dependent)
static bool isTrue(const Literal *l) {
  if (auto *b = std::get_if<bool>(&l->value)) return *b;
  return false;
}

static bool isFalse(const Literal *l) {
  if (auto *b = std::get_if<bool>(&l->value)) return !*b;
  return false;
}

static bool isNil(const Literal *l) {
  if (std::holds_alternative<std::nullptr_t>(l->value)) return true;
  return false;
}

static bool isNum(const Literal *l, double val) {
  if (auto *d = std::get_if<double>(&l->value)) return *d == val;
  return false;
}

// --- tests ---

static void test_literals() {
  TEST(test_literal_true);
  assert(isTrue(expectLit(parse("true").get())));
  PASS();

  TEST(test_literal_false);
  assert(isFalse(expectLit(parse("false").get())));
  PASS();

  TEST(test_literal_nil);
  assert(isNil(expectLit(parse("nil").get())));
  PASS();

  TEST(test_literal_number);
  assert(isNum(expectLit(parse("42").get()), 42.0));
  PASS();

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
    std::cerr << "expr type: " << typeid(*e).name() << std::endl;
    if (auto *u = as<Unary>(e.get())) {
      std::cerr << "op.type=" << u->op.type << " lexeme='" << u->op.lexeme << "'" << std::endl;
    } else if (auto *l = as<Literal>(e.get())) {
      std::cerr << "got Literal instead" << std::endl;
    }
    auto *u = expectUnary(e.get());
    assert(u->op.type == TOKEN_BANG);
  } PASS();

  TEST(test_unary_minus); {
    auto *u = expectUnary(parse("-5").get());
    assert(u->op.type == TOKEN_MINUS);
  } PASS();

  TEST(test_unary_nested); {
    auto *u1 = expectUnary(parse("!!false").get());
    assert(u1->op.type == TOKEN_BANG);
    auto *u2 = expectUnary(u1->right.get());
    assert(u2->op.type == TOKEN_BANG);
  } PASS();
}

static void test_binary() {
  TEST(test_mul);  assert(expectBin(parse("3 * 4").get())->op.type == TOKEN_STAR); PASS();
  TEST(test_div);  assert(expectBin(parse("8 / 2").get())->op.type == TOKEN_SLASH); PASS();
  TEST(test_add);  assert(expectBin(parse("1 + 2").get())->op.type == TOKEN_PLUS); PASS();
  TEST(test_sub);  assert(expectBin(parse("5 - 3").get())->op.type == TOKEN_MINUS); PASS();
  TEST(test_gt);   assert(expectBin(parse("a > b").get())->op.type == TOKEN_GREATER); PASS();
  TEST(test_le);   assert(expectBin(parse("x <= 10").get())->op.type == TOKEN_LESS_EQUAL); PASS();
  TEST(test_eq);   assert(expectBin(parse("x == nil").get())->op.type == TOKEN_EQUAL_EQUAL); PASS();
  TEST(test_ne);   assert(expectBin(parse("a != b").get())->op.type == TOKEN_BANG_EQUAL); PASS();

  TEST(test_mul_values); {
    auto *b = expectBin(parse("3 * 4").get());
    assert(isNum(expectLit(b->left.get()), 3.0));
    assert(isNum(expectLit(b->right.get()), 4.0));
  } PASS();
}

static void test_grouping() {
  TEST(test_grouping); {
    auto *g = expectGroup(parse("(42)").get());
    assert(isNum(expectLit(g->expression.get()), 42.0));
  } PASS();
}

static void test_precedence() {
  // 1 + 2 * 3  →  (+ 1 (* 2 3))
  TEST(test_mul_before_add); {
    auto *b = expectBin(parse("1 + 2 * 3").get());
    assert(b->op.type == TOKEN_PLUS);
    auto *inner = expectBin(b->right.get());
    assert(inner->op.type == TOKEN_STAR);
  } PASS();

  // (1 + 2) * 3  →  (* (group (+ 1 2)) 3)
  TEST(test_grouping_overrides); {
    auto *b = expectBin(parse("(1 + 2) * 3").get());
    assert(b->op.type == TOKEN_STAR);
    assert(as<Grouping>(b->left.get()));
  } PASS();

  // 1 < 2 == true  →  (== (< 1 2) true)
  TEST(test_comparison_chains); {
    auto *b = expectBin(parse("1 < 2 == true").get());
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
