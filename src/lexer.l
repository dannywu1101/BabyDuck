/* ============================================================
 * BabyDuck - Scanner (analizador lexico)
 * Herramienta: Flex
 * ============================================================ */

%{
#include <cstdio>
#include "parser.tab.hpp"

/* Contador de errores lexicos (lo lee main.cpp) */
int lexErrors = 0;

static void lexError(const char* msg) {
    fprintf(stderr, "Error lexico (linea %d): %s '%s'\n", yylineno, msg, yytext);
    lexErrors++;
}
%}

%option noyywrap
%option yylineno
%option nounput
%option noinput

/* ---------- Definiciones auxiliares ---------- */
letra       [a-zA-Z]
digito      [0-9]

/* ---------- Identificadores y constantes ---------- */
id          {letra}({letra}|{digito}|_)*
cte_int     {digito}+
cte_float   {digito}+\.{digito}+
cte_string  \"[^"\n]*\"

/* ---------- Elementos ignorados ---------- */
espacios    [ \t\r\n]+

%%

 /* ----- Palabras reservadas (van ANTES de id: prioridad) ----- */
"program"       { return PROGRAM; }
"main"          { return MAIN; }
"end"           { return END; }
"var"           { return VAR; }
"void"          { return VOID; }
"int"           { return INT; }
"float"         { return FLOAT; }
"print"         { return PRINT; }
"read"          { return READ; }
"while"         { return WHILE; }
"do"            { return DO; }
"if"            { return IF; }
"else"          { return ELSE; }
"return"        { return RETURN; }

 /* ----- Operadores ----- */
"!="            { return NE; }
"="             { return ASSIGN; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIV; }
">"             { return GT; }
"<"             { return LT; }

 /* ----- Delimitadores ----- */
";"             { return SEMICOLON; }
","             { return COMMA; }
":"             { return COLON; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }

 /* ----- Identificadores y constantes ----- */
 /* (En la Etapa 1 solo importa el tipo de token; el lexema queda en yytext.
     El valor se pasara al parser en la Etapa 2, con la tabla de simbolos.) */
{cte_float}     { return CTE_FLOAT; }
{cte_int}       { return CTE_INT; }
{cte_string}    { return CTE_STRING; }
{id}            { return ID; }

 /* ----- Ignorados ----- */
{espacios}      { /* se descartan */ }

 /* ----- Errores lexicos ----- */
{digito}+\.     { lexError("constante flotante incompleta"); }
\"[^"\n]*       { lexError("string sin cerrar"); }
.               { lexError("caracter no reconocido"); }

%%
