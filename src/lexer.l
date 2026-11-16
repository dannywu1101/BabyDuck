/* ============================================================
 * BabyDuck - Scanner (analizador lexico)
 * Herramienta: Flex
 *
 * El scanner lee el programa letra por letra y lo parte en TOKENS
 * (palabras reservadas, operadores, ids, constantes...). El parser
 * le pide un token a la vez llamando a yylex().
 *
 * Este archivo tiene 3 partes separadas por %%:
 *   1. Definiciones: nombres para las expresiones regulares.
 *   2. Reglas: "patron { accion }". Cuando el texto coincide con el
 *      patron se ejecuta la accion, que regresa el token.
 *   3. Codigo C++ extra (aqui no se usa).
 * ============================================================ */

%{
#include <cstdio>
#include <string>
#include <unordered_set>
#include "tipos.hpp"          /* parser.tab.hpp usa Tipo en su %union */
#include "parser.tab.hpp"     /* nombres de los tokens (lo genera Bison) */

/* Contador de errores lexicos (lo lee main.cpp) */
int lexErrors = 0;

/* Antes de cada accion: guarda la linea del token para el parser (@N.first_line) */
#define YY_USER_ACTION yylloc.first_line = yylineno;

/* yytext (el texto del token) cambia con cada token, asi que hay que
   guardar una copia. Cada texto distinto se guarda una sola vez y el
   puntero que recibe el parser sigue siendo valido hasta que termina
   el programa. */
static const char* guardarLexema(const char* texto) {
    static std::unordered_set<std::string> lexemas;
    return lexemas.insert(texto).first->c_str();
}

/* Reporta un error lexico y lo cuenta. El scanner sigue leyendo. */
static void lexError(const char* msg) {
    fprintf(stderr, "Error lexico (linea %d): %s '%s'\n", yylineno, msg, yytext);
    lexErrors++;
}
%}

 /* Opciones de Flex:
    noyywrap: solo se lee un archivo.  yylineno: Flex cuenta las lineas.
    nounput y noinput: no generar funciones que no se usan. */
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

 /* Si dos patrones coinciden, Flex elige el texto mas largo;
    si empatan, gana la regla que esta escrita primero. */

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
 /* Ademas del token, mandan su texto al parser en yylval.nombre:
    el ID para las tablas (Etapa 2) y las constantes para los
    cuadruplos (Etapa 3). cte_float va antes que cte_int para que
    3.14 sea un solo token. */
{cte_float}     { yylval.nombre = guardarLexema(yytext); return CTE_FLOAT; }
{cte_int}       { yylval.nombre = guardarLexema(yytext); return CTE_INT; }
{cte_string}    { yylval.nombre = guardarLexema(yytext); return CTE_STRING; }
{id}            { yylval.nombre = guardarLexema(yytext); return ID; }

 /* ----- Ignorados ----- */
{espacios}      { /* se descartan */ }

 /* ----- Errores lexicos ----- */
 /* Van al final: solo aplican si ningun patron valido coincidio */
{digito}+\.     { lexError("constante flotante incompleta"); }
\"[^"\n]*       { lexError("string sin cerrar"); }
.               { lexError("caracter no reconocido"); }

%%
