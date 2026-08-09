---
title: Scanning
kind: chapter
part: II
number: 4
order: 8
---

The first step in any interpreter is **scanning** (also called *lexing*).
The scanner takes the raw source code as a string and transforms it into
a sequence of **tokens** — the meaningful words and punctuation of the
language.

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

## The Scanner Loop

At its core, a scanner is a simple loop:

```
while not at end of source:
    skip whitespace
    look at current character
    if it's a digit → scan a number
    if it's a letter → scan an identifier or keyword
    if it's a quote  → scan a string
    otherwise       → scan an operator or punctuation
```

### Lexical Grammar

The scanner is guided by the *lexical grammar* of Lox — a set of rules that
define what sequences of characters form valid tokens. Here's a taste:

- **Numbers**: `[0-9]+(\.[0-9]+)?`
- **Identifiers**: `[a-zA-Z_][a-zA-Z0-9_]*`
- **Keywords**: `var`, `fun`, `class`, `if`, `else`, `for`, `while`, `return`, etc.
- **Strings**: `"` followed by any non-quote characters, then `"`

## Design Decisions

### Maximal Munch

When multiple tokens could match, the scanner uses the **maximal munch**
rule: it always matches the longest possible token. So `>=` is scanned as
a single `GREATER_EQUAL` token, not as `GREATER` followed by `EQUAL`.

### Reserved Words

Keywords are reserved — you can't use `class` as a variable name. The
scanner distinguishes keywords from identifiers by checking against a
dictionary of reserved words after scanning what looks like an identifier.

> **Design Note: Implicit Semicolons**
>
> Some languages (like JavaScript and Go) allow omitting semicolons through
> automatic semicolon insertion. Lox takes the simpler approach: semicolons
> are required statement terminators. This keeps the scanner simpler and
> avoids the subtle bugs that implicit semicolons introduce.

## Next Up

With tokens in hand, we'll feed them into the parser, which builds the
syntax tree that the interpreter will eventually execute.
