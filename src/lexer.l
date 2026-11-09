/* ============================================================
 * BabyDuck - Scanner (analizador lexico)
 * Herramienta: Flex
 * ============================================================ */

%{
#include <cstdio>
#include <string>
#include <unordered_set>
#include "tipos.hpp"          /* parser.tab.hpp usa Tipo en su %union */
#include "parser.tab.hpp"

/* Contador de errores lexicos (lo lee main.cpp) */
int lexErrors = 0;

/* Antes de cada accion: guarda la linea del token para el parser (@N.first_line) */
#define YY_USER_ACTION yylloc.first_line = yylineno;

/* Guarda cada nombre distinto una sola vez. El puntero que recibe el
   parser sigue siendo valido hasta que termina el programa. */
static const char* guardarLexema(const char* texto) {
    static std::unordered_set<std::string> lexemas;
    return lexemas.insert(texto).first->c_str();
}

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
 /* El ID manda su lexema al parser (para las tablas de la Etapa 2).
    El valor de las constantes se usara en la Etapa 3. */
{cte_float}     { return CTE_FLOAT; }
{cte_int}       { return CTE_INT; }
{cte_string}    { return CTE_STRING; }
{id}            { yylval.nombre = guardarLexema(yytext); return ID; }

 /* ----- Ignorados ----- */
{espacios}      { /* se descartan */ }

 /* ----- Errores lexicos ----- */
{digito}+\.     { lexError("constante flotante incompleta"); }
\"[^"\n]*       { lexError("string sin cerrar"); }
.               { lexError("caracter no reconocido"); }

%%
