---
title: State and Variable
kind: chapter
part: II
number: 7
order: 9
---

The `llox` is no more than a calculator, let's level it up
to support _variables_. We can start with the minimum,
all variables are defined in a global namespace.

First We introduces a new syntax for variable declaration:

```
program        → declaration* EOF ;
declaration    → varDecl
               | statement ;

varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;
```

and use the visitor pattern for `Stmt` and the derived classes.
More concretely, the `Parser::declaration` peeks the token
stream, and resolve to `VarStmt`. The visitor of `VarStmt`
put the variable to the dictionary, `variables` with the
value resolved from the initializer:

```cpp
void Interpreter::visit(const VarStmt &stmt) {
  Value *val = nullptr;
  if (stmt.initializer != nullptr) {
    val = stmt.initializer->accept(*this);
  }

  // Resolve to assign expr.
  variables.insert({stmt.name.lexeme, val});
}
```

Then when the interpreter visits the `Literal`,
we would look up the `variables` dictionary:

```cpp
if (auto pval = std::get_if<std::string>(&literal.value)) {
  Value *val = variables.at(*pval);
  return val;
}
```

With this, our calculator is slightly more sophisticated:

```
> var a = 3; print a + 5;
8
```

Check out the [PR #11](https://github.com/kunxi/llox/pull/11) for more details.

## Assignment statement

Next, we would support the assignment statement, such as:

```
> var a = 3; var b; b = a = a + 1; print a + b;
8
```

This would introduce new syntax:

```
expression     → assignment ;
assignment     → IDENTIFIER "=" assignment
               | equality ;
```

and new `Expr` types, `Assign` and `Variable`.

```cpp
struct Assign : Expr {
  explicit Assign(std::unique_ptr<Expr> lvalue, std::unique_ptr<Expr> rvalue)
      : lvalue(std::move(lvalue)), rvalue(std::move(rvalue)) {}
  void accept(ExprVisitor &v) const override { v.visit(*this); }
  Value *accept(CodegenVisitor &v) const override { return v.visit(*this); }

  std::unique_ptr<Expr> lvalue;
  std::unique_ptr<Expr> rvalue;
};
```

Notice the `lvalue` and `rvalue` above. The `lvalue` represents a persistent
memory location, such as `breakfast.bagel`; but for now it *MUST* be resolved
to a *variable*.

```cpp
Value *Interpreter::visit(const Assign &assign) {
  // Make sure the variable is declared.
  auto lval = dynamic_cast<Variable *>(assign.lvalue.get());
  if (lval != nullptr) {
    auto name = lval->name.lexeme;
    auto it = variables.find(name);
    if (it == variables.end()) {
      return log_error_v("name is not declared");
    }
    auto rval = assign.rvalue->accept(*this);
    it->second = rval;
    return rval;
  }
  throw std::runtime_error("Cannot assign to lvalue");
}
```

The `Variable` would lookup the `variables` for the `Value*`:

```cpp
Value *Interpreter::visit(const Variable &variable) {
  Value *val = variables.at(variable.name.lexeme);
  return val;
}
```
