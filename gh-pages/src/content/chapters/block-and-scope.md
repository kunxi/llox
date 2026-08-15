---
title: Block and Scope
kind: chapter
part: II
number: 8
order: 10
---

Every variable we have defined so far lives in one flat, global
namespace — the `variables` map in `Interpreter`.
In this chapter we introduce _blocks_ and _lexical scope_, so a
variable will be resolved for the scope.

## Block syntax

A block is a sequence of declarations wrapped in curly braces:

```
statement      → exprStmt
               | printStmt
               | block ;

block          → "{" declaration* "}" ;
```

The lexer already emits `TOKEN_LEFT_BRACE` and `TOKEN_RIGHT_BRACE`,
so only the parser gains a new rule. The syntax tree gets a
`BlockStmt` node holding a list of statements:

```cpp
struct BlockStmt : Stmt {
  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)) {}
  void accept(StmtVisitor &v) const override { v.visit(*this); }

  std::vector<std::unique_ptr<Stmt>> statements;
};
```

`Parser::statement()` is extended to dispatch to a new `block()`
method, which keeps parsing declarations until the closing brace:

```cpp
std::unique_ptr<Stmt> Parser::block() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!check(TOKEN_RIGHT_BRACE) && !ends())
    statements.push_back(declaration());
  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
  return std::make_unique<BlockStmt>(std::move(statements));
}
```

## Environments

A block is only useful if a variable declared inside it stays
inside it. The flat `std::map<std::string, Value *> variables` in
`Interpreter` cannot express that, so we replace it with an
`Environment` — a scope holding a map of names plus a pointer to
the scope that encloses it:

```cpp
class Environment {
public:
  explicit Environment(std::shared_ptr<Environment> enclosing = nullptr)
      : enclosing(std::move(enclosing)) {}

  void define(const std::string &name, Value *value) {
    values[name] = value;
  }

  Value *get(const std::string &name);
  void assign(const std::string &name, Value *value);

private:
  std::map<std::string, Value *> values;
  std::shared_ptr<Environment> enclosing;
};
```

The rules are the heart of lexical scoping:

- `define` always writes into the _current_ environment.
- `get` and `assign` first look in the current environment, then
  walk up through the enclosing chain.

In other words, a declaration is visible in its own block and in
every block nested inside it, but never outside.

## Visiting a block

`Interpreter` keeps a pointer to the _current_ environment. Visiting
a block executes its statements against a fresh, nested environment,
then restores the previous one:

```cpp
void Interpreter::visit(const BlockStmt &stmt) {
  auto previous = environment;
  environment = std::make_shared<Environment>(previous);
  for (const auto &statement : stmt.statements)
    statement->accept(*this);
  environment = previous;
}
```

`visit(VarStmt)` switches from inserting into `variables` to calling
`environment->define`, while `visit(Variable)` and `visit(Assign)`
call `environment->get` and `environment->assign`.

## Shadowing

Because `define` targets the innermost environment, an inner block
can redeclare a name from an outer block. The inner binding
_shadows_ the outer one for the rest of the block:

```
> var a = 1; { var a = 2; print a; } print a;
2
1
```

The block becomes a private workspace: names declared inside it do
not leak out, and assigning to an outer name still reaches it, as
long as no inner binding shadows it first.
