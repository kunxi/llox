---
title: Evaluating Expressions
kind: chapter
part: II
number: 7
order: 8
---

There are two steps to integrate the LLVM backend:

- Generate the LLVM IR when evaluating expression.
- Execute the statement with the `ExecutionEngine`.

Read the [LLVM tutorial](https://llvm.org/docs/tutorial/index.html)
for the basic concepts.

## Build with llvm

Install the `llvm.dev`, `zlib`, and `libffi` dependencies,
and add the dependency in `CMakeLists.txt`:

```cmake
# Dependency: LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM found: ${LLVM_PACKAGE_VERSION}")
llvm_map_components_to_libnames(LLVM_LIBS core interpreter executionengine)
```

Also add `${LLVM_INCLUDE_DIRS}` in the target include directories,
and `${LLVM_LIBS}` in the target link libraries.

## Codegen

I follow the `ExprVisitor`'s visitor pattern for the code generation
as I scratched head but could not figure out how parameterized
visitor pattern works.

Define a new `accept` virtual function with returns `llvm::Value*`
in `Expr`:

```cpp
struct Expr {
  virtual void accept(ExprVisitor& v) const = 0;
  virtual Value* accept(CodegenVisitor& v) const = 0;
  virtual ~Expr() = default;
};
```

and implement it in derived classes:

```cpp
Value* accept(CodegenVisitor& v) const override {
  return v.visit(*this);
}
```

This step is _necessary_ because the type is resolved by the
virtual function, then match to the overloaded `visit` function.

We introduce `Interpreter` to encapsulate the LLVM scaffolding,
and implement the `CodegenVisitor` interface. For example:

```cpp
Value *Interpreter::visit(const Unary &unary) {
  Value *right = unary.right->accept(*this);

  switch (unary.op.type) {
  case TOKEN_MINUS:
    return builder->CreateFNeg(right, "negtmp");
  case TOKEN_BANG:
    return builder->CreateNot(right, "nottmp");
  default:
    return log_error_v("invalid unary operator");
  }
}
```

It resolve the `right` and generate the instruction in LLVM
IR.

## Scaffolding

Before executing the code, we need to create `__main` function:

```cpp
auto *ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*context), false);
auto *mainFn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                      "__main", module.get());
auto *bb = llvm::BasicBlock::Create(*context, "entry", mainFn);
builder->SetInsertPoint(bb);
```

The `SetInsertPoint` instructs the succeeding IR code generated
in the `__main` function.

The `PrintStmt` will yield the `printf` function call:

```cpp
auto printf = get_printf();
auto *fmt = builder->CreateGlobalString("%g\n");
builder->CreateCall(printf, {fmt, val});
```

The utility function `get_printf` imports the function `printf`from
the stdlib:

```cpp
auto *ty = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context),
                             {llvm::PointerType::get(*context, 0)}, true);
return module->getOrInsertFunction("printf", ty);
```

## Execution

We use the `EngineBuilder` to bind the `Interpreter`, then
run the `__main` function.

```cpp
engine.reset(llvm::EngineBuilder(std::move(module))
    .setEngineKind(llvm::EngineKind::Interpreter)
    .setErrorStr(&err)
    .create());
if (!engine) {
    std::cerr << "EngineBuilder failed: " << err << "\n";
    return;
}
engine->runFunction(engine->FindFunctionNamed("__main"), {});

engine.reset();
```

Build the project, we can run `llox` in the command line:

```bash
❯ ./llox
Type something (press Ctrl+D on Linux/Mac or Ctrl+Z on Windows to stop):
> print 1 + 2 * (3 + 4);
15
>
```
