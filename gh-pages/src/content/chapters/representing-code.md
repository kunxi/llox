---
title: Representing Code
kind: chapter
part: II
number: 5
order: 10
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

We will explore the visitor pattern in the code generation.
