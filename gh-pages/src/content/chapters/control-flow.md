---
title: Control Flow
kind: chapter
part: II
number: 9
order: 11
---

In this chapter, we will add control flow to the _calculator_, notably:

- conditional execution, aka `if ... else`
- loop execution, aka `while`, `for` loop.

In either case, the program counter will jump to different code segment
based on the predict.

## Conditional Execution

Unlike the [Kaleidoscope language](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl05.html#) which evaluate the `if else`
as expression, the `lox` language support statement in branches.
Recall the `scopes` introduced in the last chapter: the variables
lookup traverse from nested inner scope to the external scopes.
The statements in `then` branch and `else` branch can _both_ mutate
the variable in any scopes. The runtime can evaluate the predict,
then choose one branch to execute. But the compiler cannot, we need
to capture both scopes, and use the [Phi operation](https://en.wikipedia.org/wiki/Static_single-assignment_form).

The primary logic can be found [here](https://github.com/kunxi/llox/pull/16/changes#diff-00ebb867b30fd99e9a077cf82dea5b5c7748b8b728e731a84ed5bc5490bdbfb1R94). First, we build three `BasicBlock`s:

```cpp
Function *function = builder->GetInsertBlock()->getParent();
BasicBlock *then_bb = BasicBlock::Create(*context, "then", function);
BasicBlock *else_bb = BasicBlock::Create(*context, "else");
BasicBlock *merge_bb = BasicBlock::Create(*context, "ifcont");
```

In the `then` branch, generate code against the `base`, then save the scopes
as `then_scope`, then jump to the merge block.

```cpp
builder->SetInsertPoint(then_bb);
scopes = base;
stmt.then_branch->accept(*this);
auto then_scopes = scopes;
builder->CreateBr(merge_bb);
BasicBlock *then_pred = builder->GetInsertBlock();
```

Repeat the above steps for else branch if necessary, and we can this method
to merge the scope:

```cpp
scopes = merge_scopes(base, then_scopes, else_scopes, then_pred, else_pred);
```

which is defined as:

```cpp
Interpreter::ScopeStack Interpreter::merge_scopes(const ScopeStack &base,
                                                  const ScopeStack &then_scopes,
                                                  const ScopeStack &else_scopes,
                                                  BasicBlock *then_pred, BasicBlock *else_pred) {
  ScopeStack merged = base;
  auto *fty = llvm::Type::getDoubleTy(*context);

  for (size_t i = 0; i < base.size(); ++i) {
    for (const auto &kv : base[i]) {
      const std::string &name = kv.first;
      Value *then_val = then_scopes[i].at(name);
      Value *else_val = else_scopes[i].at(name);
      if (then_val == else_val)
        continue; // unchanged in both branches; base value already dominates the join

      PHINode *phi = builder->CreatePHI(fty, 2, name);
      phi->addIncoming(then_val, then_pred);
      phi->addIncoming(else_val, else_pred);
      merged[i][name] = phi;
    }
  }
```

The merge logic iterates the nested scopes for both branches,
compares variables, and generates phi operators if bifurcated.
The `PHINode` attaches an SSA value `then_val` to the dominating block `then_pred`.
The `then_pred` points to the `then_bb` block, we don't use `then_bb`
here because it might point to the inner block introduced by the
nested if block after the `then_branch` is executed.

See [PR #16](https://github.com/kunxi/llox/pull/16) for more details.
