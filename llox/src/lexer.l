%{
#include "lexer.h"
%}

/* Configure Flex to output a C++ class */
%option c++
%option noyywrap

%%
\(              { return TOKEN_LEFT_PAREN; }
\)              { return TOKEN_RIGHT_PAREN; }
\{              { return TOKEN_LEFT_BRACE; }
\}              { return TOKEN_RIGHT_BRACE; }
,               { return TOKEN_COMMA; }
\.              { return TOKEN_DOT; }
-               { return TOKEN_MINUS; }
\+              { return TOKEN_PLUS; }
;               { return TOKEN_SEMICOLON; }
\/\/.*          { /* ignore the comment */}
\/              { return TOKEN_SLASH; }
\*              { return TOKEN_STAR; }
!=              { return TOKEN_BANG_EQUAL; }
!               { return TOKEN_BANG; }
==              { return TOKEN_EQUAL_EQUAL; }
=               { return TOKEN_EQUAL; }
>=              { return TOKEN_GREATER_EQUAL; }
>               { return TOKEN_GREATER; }
\<=             { return TOKEN_LESS_EQUAL; }
\<              { return TOKEN_LESS; }

and             { return TOKEN_AND; }
class           { return TOKEN_CLASS; }
else            { return TOKEN_ELSE; }
false           { return TOKEN_FALSE; }
fun             { return TOKEN_FUN; }
for             { return TOKEN_FOR; }
if              { return TOKEN_IF; }
nil             { return TOKEN_NIL; }
or              { return TOKEN_OR; }
print           { return TOKEN_PRINT; }
return          { return TOKEN_RETURN; }
super           { return TOKEN_SUPER; }
this            { return TOKEN_THIS; }
true            { return TOKEN_TRUE; }
var             { return TOKEN_VAR; }
while           { return TOKEN_WHILE; }

[a-zA-Z_][a-zA-Z0-9_]*      { return TOKEN_IDENTIFIER; }
\"[^"]*\"                   { return TOKEN_STRING; }
[0-9]+(\.[0-9]+)?           { return TOKEN_NUMBER; }

.               { /* Ignore any other character like spaces */ }

%%

// No main function here; it will be handled in main.cpp
