---
title: Block and Scope
kind: chapter
part: II
number: 8
order: 10
---

Every variable we have defined so far live in one flat, global
namespace — the `variables` map in `Interpreter`.
In this chapter we introduce the concept _block_, which
defines the accessibility of the variables.

## Block syntax

A block is a sequence of declarations wrapped in curly braces:

```
statement      → exprStmt
               | printStmt
               | block ;

block          → "{" declaration* "}" ;
```

The syntax tree gets a `BlockStmt` holding a list of statements:

```cpp
struct BlockStmt : Stmt {
  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)) {}
  void accept(StmtVisitor &v) const override { v.visit(*this); }

  std::vector<std::unique_ptr<Stmt>> statements;
};
```

`Parser::statement()` is extended to dispatch to a new `block_stmt()`
method, which keeps parsing declarations until the closing brace:

```cpp
std::unique_ptr<Stmt> Parser::block_stmt() {
  std::vector<std::unique_ptr<Stmt>> statements;

  while (!check(TOKEN_RIGHT_BRACE) && !ends()) {
    statements.push_back(declaration());
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
  return std::make_unique<BlockStmt>(std::move(statements));
}
```

## Scopes

A block defines a _scope_ for the variables. A variable defined
inside the scope is no longer accessible once the code execution
leaves the scope. The inner variable can hide the outer one inside
the scope, aka [variable shadowing](https://en.wikipedia.org/wiki/Variable_shadowing). We will replace the flat `variables` with
`scopes`:

```cpp
std::vector<std::map<std::string, Value *>> scopes;
```

When visiting a block, the statements are evaluated against a fresh scope
appended to the `scopes`, then popped to restore the previous one.

```cpp
void Interpreter::visit(const BlockStmt &stmt) {
  // Push a fresh scope for the block, then pop it when the block ends.
  scopes.emplace_back();
  for (const auto &statement : stmt.statements) {
    statement->accept(*this);
  }
  scopes.pop_back();
}
```

Variable declaration always writes to the current scope, while
assignment and reference first look in the current scope, then
walk upwards through the scopes.

```cpp
// Resolve from the innermost scope outward.
for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
  auto found = it->find(name);
  if (found != it->end()) {
    auto rval = assign.rvalue->accept(*this);
    found->second = rval;
    return rval;
  }
}

return log_error_v("name is not declared");
```

See [PR #15](https://github.com/kunxi/llox/pull/15) for details.
