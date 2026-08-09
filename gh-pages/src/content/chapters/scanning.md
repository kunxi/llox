---
title: Scanning
kind: chapter
part: II
number: 4
order: 8
---

## Tokens

A token is the smallest unit of meaning in a programming language. Given
this line of Lox:

```javascript
var language = "lox";
```

The scanner produces these tokens:

| Token       | Lexeme      |
|-------------|-------------|
| `VAR`       | `var`       |
| `IDENTIFIER`| `language`  |
| `EQUAL`     | `=`         |
| `STRING`    | `"lox"`     |
| `SEMICOLON` | `;`         |
| `EOF`       |             |

## Using Flex

The first step in any interpreter is **scanning** (also called *lexing*).
Instead of crafting the lexer by scanning characters, we use [flex].

First, we need to define a `lexer.l`, it uses the regular expression
for pattern matching, and the order matters as it defines the precedence:

```
%{
#include "lexer.h"
%}

/* Configure Flex to output a C++ class */
%option c++
%option noyywrap

%%
\(              { return TOKEN_LEFT_PAREN; }
\)              { return TOKEN_RIGHT_PAREN; }
... ...

[a-zA-Z_][a-zA-Z0-9_]*      { return TOKEN_IDENTIFIER; }
\"[^"]*\"                   { return TOKEN_STRING; }
([0-9]+\.?[0-9]*)           { return TOKEN_NUMBER; }
```

It is worthy noting that pattern uses regular expression,
so the special characters *must* be escaped. For numbers,
we support `3`, `3.14`, `0.5`, but *not* `.5` or `3.`.

The `lexer.l` is compiled by the *flex* to generate `lexer.cpp`:

```cmake
find_package(FLEX REQUIRED)
FLEX_TARGET(LoxScanner src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
```

and we can add generated source to the `SOURCES`:

```cmake
# Target: llox
set(SOURCES
    src/main.cc
    ${FLEX_LoxScanner_OUTPUTS}
)
add_executable(llox ${SOURCES})
target_include_directories(llox PRIVATE include/)
```

[flex]: https://github.com/westes/flex

## Next Up

With tokens in hand, we'll feed them into the parser, which builds the
syntax tree that the interpreter will eventually execute.
