---
title: Statement
kind: chapter
part: II
number: 5
order: 7
---

Let's move forward to the minimum statement support, 
The `Stmt` is defined as an abstract class to hold `Expr`,
with derived  `PrintStmt` and `ExprStmt`.
The `PrintStmt` would print the value to communicate with the external world.

The statement parsing is pretty much the same:


```cpp
std::unique_ptr<Stmt> Parser::statement() {
  if (match({TOKEN_PRINT}))
    return print_stmt();
  return expr_stmt();
}
```

We also extract the `tokenize` from `parse`, so we could keep
the unit tests on parsing expressions. The logic of the `parse`
is quite straightforward:

```cpp
void Parser::parse() {
  while (!ends()) {
    program.push_back(statement());
  }
}
```

In the [PR #7](https://github.com/kunxi/llox/pull/7/), we are able to
parse the statements. In the next chapter, we will integrate the LLVM
backend for code generation.
