---
title: Representing Code
kind: chapter
part: II
number: 5
order: 6
---

Let's focus on the expression first, we need to build
a AST to resolve the precedence using
[recursive decent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser)
as suggested in the _Crafting Interpreter_.

The syntax tree have _four_ types of nodes:

- `Literal`: the leaf node with `number`, `string`, `boolean`, and `nil` types.
- `Unary`: unary operation with `(token, rhs)`.
- `Binary`: binary operation with `(lhs, token, rhs)`.
- `Grouping`: grouping operation.

They all derive from the abstract class `Expr`.

The `Literal` can holds various types of data, number, boolean,
string, and nil. We can use `std::variant` to store the value.
The `nil` can be mapped to `nullptr` with type of `nullptr_t`
in the variant template:

```cpp
struct Literal : Expr {
  template<typename T>
  explicit Literal(T&& val)
    : value(std::forward<T>(val)) {}

  std::variant<bool, double, nullptr_t, std::string> value;
};
```

As C++ does not support garbage collection, we will use
`unique_ptr` to claim the ownership of the child element.
This would work because each node of the AST has one and
only one parent except the root node.

## Visitor Pattern

In the _Craft Interpreter_, the `Visitor` use both generic and virtual function.

```java
abstract class Expr {
  interface Visitor<R> {
    R visitAssignExpr(Assign expr);
    ...
  }
  abstract <R> R accept(Visitor<R> visitor);
}
```

This is **NOT** supported in C++ unless we use `std::any` to bridge the gap
which eliminates the static type check in the compile time. The `ExprVisitor`
only support `void visit(const T&)`:

```cpp
struct ExprVisitor {
  virtual void visit(const Literal& expr) = 0;
  ... ...
};

struct Expr {
  virtual void accept(ExprVisitor& v) const = 0;
  virtual ~Expr() = default;
};


struct PrintVisitor : ExprVisitor {
  std::ostream& os;
  explicit PrintVisitor(std::ostream& os) : os(os) {}

  void visit(const Unary& expr) override {
    os << "(" << expr.op.lexeme << " ";
    expr.right->accept(*this);
    os << ")";
  }
```

This works for object dump, see [PR &6](https://github.com/kunxi/llox/pull/6) for more details; but cannot be used in the code generation.
We will explore the
[Visitor Pattern in Modern C++](https://learnmoderncpp.com/2022/11/01/visitor-pattern-in-modern-c/)
for the codegen.
