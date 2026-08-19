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

static void test_if() {
  TEST(if_true_then);
  assert(run_script("if (true) print 1; else print 2;") == "1\n");
  PASS();
  TEST(if_false_else);
  assert(run_script("if (false) print 1; else print 2;") == "2\n");
  PASS();
  TEST(if_assign_then);
  assert(run_script("var x = 0; if (x < 5) x = 1; else x = 2; print x;") == "1\n");
  PASS();
  TEST(if_assign_else);
  assert(run_script("var x = 10; if (x < 5) x = 1; else x = 2; print x;") == "2\n");
  PASS();
  TEST(if_no_else_taken);
  assert(run_script("var x = 0; if (x < 5) x = 100; print x;") == "100\n");
  PASS();
  TEST(if_no_else_skipped);
  assert(run_script("var x = 0; if (x < 0) x = 100; print x;") == "0\n");
  PASS();
  TEST(if_phi_arithmetic);
  assert(run_script("var x = 0; if (x < 5) x = x + 1; else x = x + 10; print x;") == "1\n");
  PASS();
  TEST(if_nested);
  assert(run_script("var x = 3; if (x < 5) { if (x < 1) print 0; else print 1; } else print 2;") ==
         "1\n");
  PASS();
  TEST(if_assign_outer_scope);
  assert(run_script("var x = 0; { if (true) x = 1; else x = 2; print x; } print x;") == "1\n1\n");
  PASS();
  TEST(if_assign_outer_scope_else);
  assert(run_script("var x = 0; { if (false) x = 1; else x = 2; print x; } print x;") == "2\n2\n");
  PASS();
}

static void test_block() {
  TEST(block_assign_outer);
  assert(run_script("var a = 1; { a = 5; } print a;") == "5\n");
  PASS();

  TEST(block_shadow_assign);
  assert(run_script("var a = 1; { var a = 2; a = 3; print a; } print a;") == "3\n1\n");
  PASS();

  TEST(block_scope_does_not_leak);
  assert(run_script("var a = 1; { var a = 2; print a; } print a;") == "2\n1\n");
  PASS();

  TEST(block_nested_assignment);
  assert(run_script("var a = 1; { var b = 2; { b = 20; print b; } print b; } print a;") ==
         "20\n20\n1\n");
  PASS();

  TEST(block_empty);
  assert(run_script("print 1; {} print 2;") == "1\n2\n");
  PASS();
}

int main() {
  std::cout << "Running statement tests...\n";
  test_print();
  test_var();
  test_expr_stmt();
  test_if();
  test_block();
  std::cout << "\nAll " << tests << " tests passed.\n";
  return 0;
}
