---
title: Introduction
kind: chapter
part: I
number: 1
order: 2
---

Welcome to llox! This document tracks my study of the book,
[Crafting Interpreters](https://craftinginterpreters.com/).
I diverged from the craftsmanship to adopting the LLVM backend,
and Cranelift JIT.


## What we'll Build

Over the course of this document, we'll build two complete implementations of
Lox:

1. **llox** — A tree-walk interpreter and compiler in C++, focused on simplicity and
   correctness.
3. **clox** — A bytecode virtual machine in rust, focused on performance and
   low-level implementation details.

## Why Lox?

Lox is a small, dynamically-typed scripting language designed by Robert Nystrom.
Its syntax is similar to JavaScript and Python. 
The language is small enough to implement fully, but rich enough to explore
the most interesting corners of language design:

```javascript
// A taste of Lox
class Breakfast {
  init(meat, bread) {
    this.meat = meat;
    this.bread = bread;
  }

  serve() {
    print "Enjoy your " + this.meat + " and " + this.bread + ".";
  }
}

var bacon = Breakfast("bacon", "toast");
bacon.serve();
```

Read the [official document](https://craftinginterpreters.com/the-lox-language.html)
for more details.

## How to Read This Document

Each chapter builds on the previous one. The chapters are grouped into three
parts:

- **Part I** introduces the Lox language and gives you the landscape.
- **Part II** walks through building llox.
- **Part III** takes you through clox, the bytecode virtual machine.

Code along as you read. The best way to understand an interpreter is to build one.
