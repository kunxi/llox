---
title: Scanning
kind: chapter
part: II
number: 3
order: 5
---

## Tokens

A token is the smallest unit of meaning in a programming language. Given
this line of Lox:

```javascript
var language = "lox";
```

The scanner produces these tokens:

| Token        | Lexeme     |
| ------------ | ---------- |
| `VAR`        | `var`      |
| `IDENTIFIER` | `language` |
| `EQUAL`      | `=`        |
| `STRING`     | `"lox"`    |
| `SEMICOLON`  | `;`        |
| `EOF`        |            |

## Using Flex

The first step in any interpreter is **scanning** (also called _lexing_).
We can use the off-shelf lexer, such as [flex].

First, define a `lexer.l`, it uses the regular expression
for pattern matching, **the order matters** as it defines the precedence:

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

Note: the parentheses has to be escaped due to its special meaning
in the regular expression.
For numbers, we support `3`, `3.14`, `0.5`, but _not_ `.5` or `3.`.

The `lexer.l` is compiled by the _flex_ to generate `lexer.cpp`:

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

We can dump the token for verification, see [PR #4](https://github.com/kunxi/llox/pull/4)
for more details.

```cpp
void run(std::istream &stream, std::ostream &out) {
  FlexLexer *lexer = new yyFlexLexer(&stream);
  int token;
  while ((token = lexer->yylex()) != TOKEN_EOF) {
    out << "Token ID: " << token << " | Matched: [" << lexer->YYText() << "]"
        << " | Length: " << lexer->YYLeng() << "\n";
  }
}
```

[flex]: https://github.com/westes/flex
