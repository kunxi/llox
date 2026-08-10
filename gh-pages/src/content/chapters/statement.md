---
title: Statement
kind: chapter
part: II
number: 5
order: 7
---

Since we are using the LLVM backend for evaluation, let's implement
the minimum statements first. The `Stmt` is defined as an abstract
class with derived `PrintStmt`, and `ExprStmt`.

We also extract the `tokenize` from `parse`, so we could keep
the unit tests on parsing expression. The logic of the `parse`
is quite straightforward:

```cpp
void Parser::parse() {
  while (!ends()) {
    program.push_back(statement());
  }
}
```

See [PR #7](https://github.com/kunxi/llox/pull/7/) for more details.
