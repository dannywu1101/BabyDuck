/* ============================================================
 * BabyDuck - Parser (analizador sintactico)
 * Herramienta: Bison (LALR(1))
 * ============================================================ */

%{
#include <cstdio>

int yylex();
void yyerror(const char* msg);

extern int yylineno;
%}

/* Mensajes de error con "unexpected X, expecting Y" (sintaxis de Bison 2.x) */
%error-verbose

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
%token ID         "identificador"
%token CTE_INT    "constante entera"
%token CTE_FLOAT  "constante flotante"
%token CTE_STRING "constante string"

%start programa

%%

/* ---------- Programa y variables ---------- */

programa
    : PROGRAM ID SEMICOLON vars_opc funcs_list MAIN body END
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
    : id_list COLON type SEMICOLON
    ;

var_decl_list
    : var_decl var_decl_list
    | /* vacio */
    ;

id_list
    : ID id_list_p
    ;

id_list_p
    : COMMA ID id_list_p
    | /* vacio */
    ;

type
    : INT
    | FLOAT
    ;

/* ---------- Funciones ---------- */

funcs
    : func_type ID LPAREN params RPAREN LBRACKET vars_opc body RBRACKET SEMICOLON
    ;

func_type
    : VOID
    | type
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
    : ID COLON type
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
    : ID ASSIGN expresion SEMICOLON
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
    : READ LPAREN ID RPAREN SEMICOLON
    ;

return_st
    : RETURN expresion SEMICOLON
    ;

/* Llamada como estatuto (con ';') */
f_call
    : f_call_expr SEMICOLON
    ;

/* Llamada como valor dentro de una expresion (sin ';') */
f_call_expr
    : ID LPAREN args RPAREN
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
    | f_call_expr
    ;

valor
    : ID
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
