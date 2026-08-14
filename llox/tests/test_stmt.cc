#include "interpreter.h"
#include "parser.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

static int tests = 0;

#define TEST(name)                                                                                 \
  do {                                                                                             \
    tests++;                                                                                       \
    std::cout << "  " << #name << "... ";                                                          \
  } while (0)
#define PASS() std::cout << "ok\n"

// Run a script and return everything it printed.
static std::string run_script(const std::string &src) {
  Parser parser;
  std::istringstream stream(src);
  parser.tokenize(stream);
  parser.parse();

  Interpreter interpreter;
  interpreter.execute(parser.program());
  return interpreter.output();
}

static void test_print() {
  TEST(print_number);
  assert(run_script("print 42;") == "42\n");
  PASS();
  TEST(print_float);
  assert(run_script("print 2.5;") == "2.5\n");
  PASS();
  TEST(print_expression);
  assert(run_script("print 1 + 2 * 3;") == "7\n");
  PASS();
  TEST(print_grouping);
  assert(run_script("print (1 + 2) * 3;") == "9\n");
  PASS();
  TEST(print_unary);
  assert(run_script("print -5;") == "-5\n");
  PASS();
  TEST(print_multiple);
  assert(run_script("print 1; print 2;") == "1\n2\n");
  PASS();
}

static void test_var() {
  TEST(var_decl);
  assert(run_script("var x = 5; print x;") == "5\n");
  PASS();
  TEST(var_assign);
  assert(run_script("var x = 1; x = 42; print x;") == "42\n");
  PASS();
  TEST(var_two);
  assert(run_script("var a = 10; var b = 20; print a; print b;") == "10\n20\n");
  PASS();
  TEST(var_no_initializer);
  assert(run_script("var x; x = 7; print x;") == "7\n");
  PASS();
  TEST(var_expression_init);
  assert(run_script("var x = 3 * 4; print x;") == "12\n");
  PASS();
}

static void test_expr_stmt() {
  TEST(expr_stmt_no_output);
  assert(run_script("1 + 2;") == "");
  PASS();
  TEST(expr_stmt_assignment);
  assert(run_script("var x = 5; x = x + 1; print x;") == "6\n");
  PASS();
  TEST(chained_assignment);
  assert(run_script("var a = 1; var b = 2; a = b = 42; print a; print b;") == "42\n42\n");
  PASS();
  TEST(assign_uses_old_value);
  assert(run_script("var x = 1; var y = x = 3; print x; print y;") == "3\n3\n");
  PASS();
}

int main() {
  std::cout << "Running statement tests...\n";
  test_print();
  test_var();
  test_expr_stmt();
  std::cout << "\nAll " << tests << " tests passed.\n";
  return 0;
}
