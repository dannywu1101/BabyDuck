/* ============================================================
 * BabyDuck - Parser (analizador sintactico)
 * Herramienta: Bison (LALR(1))
 * ============================================================ */

%{
#include <cstdio>
#include "semantica.hpp"

int yylex();
void yyerror(const char* msg);

extern int yylineno;

/* Analizador semantico: guarda el Directorio de Funciones y las Tablas de Variables */
Semantica semantica;
%}

/* Mensajes de error con "unexpected X, expecting Y" (sintaxis de Bison 2.x) */
%error-verbose

/* @N.first_line da la linea de cada simbolo (la pone el scanner) */
%locations

/* Valores semanticos (Etapa 2) */
%union {
    const char* nombre;   /* lexema de un ID */
    Tipo tipo;            /* tipo de "type" y "func_type" */
}

/* ---------- Tokens (35) ---------- */

/* Palabras reservadas (14) */
%token PROGRAM "program"  MAIN  "main"   END    "end"
%token VAR     "var"      VOID  "void"   INT    "int"
%token FLOAT   "float"    PRINT "print"  READ   "read"
%token WHILE   "while"    DO    "do"     IF     "if"
%token ELSE    "else"     RETURN "return"

/* Operadores (8) */
%token ASSIGN "="
%token PLUS "+"  MINUS "-"  MULT "*"  DIV "/"
%token GT ">"    LT "<"     NE "!="

/* Delimitadores (9) */
%token SEMICOLON ";"  COMMA ","  COLON ":"
%token LPAREN "("     RPAREN ")"
%token LBRACE "{"     RBRACE "}"
%token LBRACKET "["   RBRACKET "]"

/* Identificadores y constantes (4) */
%token <nombre> ID "identificador"
%token CTE_INT    "constante entera"
%token CTE_FLOAT  "constante flotante"
%token CTE_STRING "constante string"

%type <tipo> type func_type
%type <nombre> f_call_expr

%start programa

%%

/* Los puntos neuralgicos se marcan con PN1 ... PN10.
   Cada uno llama a un metodo de Semantica (src/semantica.hpp). */

/* ---------- Programa y variables ---------- */

programa
    : PROGRAM ID { semantica.iniciarPrograma($2, @2.first_line); }   /* PN1 */
      SEMICOLON vars_opc funcs_list MAIN body END
    ;

vars_opc
    : vars
    | /* vacio */
    ;

funcs_list
    : funcs funcs_list
    | /* vacio */
    ;

vars
    : VAR var_decl var_decl_list
    ;

var_decl
    : id_list COLON type SEMICOLON { semantica.declararVariables($3); }   /* PN3 */
    ;

var_decl_list
    : var_decl var_decl_list
    | /* vacio */
    ;

id_list
    : ID { semantica.agregarIdPendiente($1, @1.first_line); }   /* PN2 */
      id_list_p
    ;

id_list_p
    : COMMA ID { semantica.agregarIdPendiente($2, @2.first_line); }   /* PN2 */
      id_list_p
    | /* vacio */
    ;

type
    : INT     { $$ = Tipo::ENTERO; }
    | FLOAT   { $$ = Tipo::FLOTANTE; }
    ;

/* ---------- Funciones ---------- */

funcs
    : func_type ID { semantica.iniciarFuncion($1, $2, @2.first_line); }   /* PN4 */
      LPAREN params RPAREN LBRACKET vars_opc body RBRACKET SEMICOLON
      { semantica.terminarFuncion(); }                                   /* PN6 */
    ;

func_type
    : VOID    { $$ = Tipo::VACIO; }
    | type    { $$ = $1; }
    ;

params
    : param params_p
    | /* vacio */
    ;

params_p
    : COMMA param params_p
    | /* vacio */
    ;

param
    : ID COLON type { semantica.agregarParametro($1, $3, @1.first_line); }   /* PN5 */
    ;

/* ---------- Cuerpo y estatutos ---------- */

body
    : LBRACE stmt_list RBRACE
    ;

stmt_list
    : statement stmt_list
    | /* vacio */
    ;

statement
    : assign
    | condition
    | cycle
    | f_call
    | print
    | read
    | return_st
    | LBRACKET stmt_list RBRACKET
    ;

assign
    : ID ASSIGN { semantica.usarVariable($1, @1.first_line); }   /* PN7 */
      expresion SEMICOLON
    ;

condition
    : IF LPAREN expresion RPAREN statement else_opc SEMICOLON
    ;

else_opc
    : ELSE statement
    | /* vacio */
    ;

cycle
    : WHILE LPAREN expresion RPAREN DO statement SEMICOLON
    ;

print
    : PRINT LPAREN print_item print_list RPAREN SEMICOLON
    ;

print_item
    : expresion
    | CTE_STRING
    ;

print_list
    : COMMA print_item print_list
    | /* vacio */
    ;

read
    : READ LPAREN ID RPAREN SEMICOLON { semantica.usarVariable($3, @3.first_line); }   /* PN7 */
    ;

return_st
    : RETURN expresion SEMICOLON { semantica.validarReturn(@1.first_line); }   /* PN10 */
    ;

/* Llamada como estatuto (con ';'): puede ser de cualquier funcion, incluso void */
f_call
    : f_call_expr SEMICOLON { }
    ;

/* Llamada como valor dentro de una expresion (sin ';') */
f_call_expr
    : ID LPAREN args RPAREN
      { semantica.llamarFuncion($1, @1.first_line); $$ = $1; }   /* PN8 */
    ;

args
    : expresion args_p
    | /* vacio */
    ;

args_p
    : COMMA expresion args_p
    | /* vacio */
    ;

/* ---------- Expresiones (de menor a mayor precedencia) ---------- */

expresion
    : exp rel_opc
    ;

rel_opc
    : rel_op exp
    | /* vacio */
    ;

rel_op
    : GT
    | LT
    | NE
    ;

exp
    : termino exp_p
    ;

exp_p
    : PLUS termino exp_p
    | MINUS termino exp_p
    | /* vacio */
    ;

termino
    : factor termino_p
    ;

termino_p
    : MULT factor termino_p
    | DIV factor termino_p
    | /* vacio */
    ;

/* SIGNO se expande en tres alternativas para evitar un conflicto
   shift/reduce entre "SIGNO vacio + id" y "f_call_expr". */
factor
    : LPAREN expresion RPAREN
    | PLUS valor
    | MINUS valor
    | valor
    | f_call_expr { semantica.usarLlamadaComoValor($1, @1.first_line); }   /* PN9 */
    ;

valor
    : ID { semantica.usarVariable($1, @1.first_line); }   /* PN7 */
    | cte
    ;

cte
    : CTE_INT
    | CTE_FLOAT
    ;

%%

void yyerror(const char* msg) {
    fprintf(stderr, "Error sintactico (linea %d): %s\n", yylineno, msg);
}
