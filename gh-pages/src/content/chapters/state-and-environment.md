---
title: State and Environment
kind: chapter
part: II
number: 7
order: 9
---

The `llox` is no more than a calculator, we need to level it up
to support _variables_. Let's start with the minimum, a global
namespace: the variables are registered when declared, then map it
to a `llvm::Value` resolved from the initializer.

We will introduce a new syntax:

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



🚧 Under construction.
