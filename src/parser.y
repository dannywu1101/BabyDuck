/* ============================================================
 * BabyDuck - Parser (analizador sintactico)
 * Herramienta: Bison (LALR(1))
 *
 * Este archivo tiene 3 partes separadas por %%:
 *   1. Declaraciones: tokens y tipos de valores.
 *   2. Reglas: la gramatica. Cada regla dice como se forma una parte
 *      del programa. El codigo entre { } es una ACCION: Bison lo
 *      ejecuta justo cuando el parser pasa por ese lugar de la regla.
 *      Esas acciones son los PUNTOS NEURALGICOS (PN).
 *   3. Codigo C++ extra (la funcion que reporta errores de sintaxis).
 *
 * Dentro de una accion:
 *   $1, $2, ...       valor del simbolo 1, 2, ... de la regla
 *                     (una accion en medio de la regla tambien cuenta)
 *   $$                valor que regresa la regla
 *   @1.first_line     linea donde esta el simbolo 1
 * ============================================================ */

%{
#include <cstdio>
#include "semantica.hpp"
#include "cuadruplos.hpp"

int yylex();                      /* el scanner (lo genera Flex) */
void yyerror(const char* msg);    /* reporta un error de sintaxis */

extern int yylineno;              /* linea actual (la lleva Flex) */

/* Analizador semantico: guarda el Directorio de Funciones y las Tablas de Variables */
Semantica semantica;

/* Generador de codigo intermedio: guarda las pilas y la fila de cuadruplos */
GeneradorCuadruplos cuadruplos;
%}

/* Mensajes de error con "unexpected X, expecting Y" (sintaxis de Bison 2.x) */
%error-verbose

/* @N.first_line da la linea de cada simbolo (la pone el scanner) */
%locations

/* Valores que pueden cargar los simbolos */
%union {
    const char* nombre;   /* texto de un ID o de una constante */
    Tipo tipo;            /* tipo de "type" y "func_type" */
}

/* ---------- Tokens (35) ---------- */
/* El texto entre comillas es el nombre que se muestra en los mensajes de error */

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

/* Identificadores y constantes (4): el scanner manda su texto en "nombre" */
%token <nombre> ID "identificador"
%token <nombre> CTE_INT    "constante entera"
%token <nombre> CTE_FLOAT  "constante flotante"
%token <nombre> CTE_STRING "constante string"

/* Reglas que regresan un valor */
%type <tipo> type func_type
%type <nombre> f_call_expr

/* La gramatica empieza en la regla "programa" */
%start programa

%%

/* Puntos neuralgicos:
     PN1  a PN10  -> metodos de Semantica           (src/semantica.hpp)   Etapa 2
     PN11 a PN22  -> metodos de GeneradorCuadruplos (src/cuadruplos.hpp)  Etapa 3 */

/* ---------- Programa y variables ---------- */

/* program nombre; variables funciones main { ... } end */
programa
    : PROGRAM ID { semantica.iniciarPrograma($2, @2.first_line); }   /* PN1 */
      SEMICOLON vars_opc funcs_list MAIN body END
    ;

/* Las variables son opcionales */
vars_opc
    : vars
    | /* vacio */
    ;

/* Cero o mas funciones */
funcs_list
    : funcs funcs_list
    | /* vacio */
    ;

/* var a, b : int;  c : float; */
vars
    : VAR var_decl var_decl_list
    ;

/* Una declaracion: lista de ids, dos puntos, tipo y punto y coma */
var_decl
    : id_list COLON type SEMICOLON { semantica.declararVariables($3); }   /* PN3 */
    ;

var_decl_list
    : var_decl var_decl_list
    | /* vacio */
    ;

/* a, b, c: cada id se guarda en la fila hasta que llegue el tipo */
id_list
    : ID { semantica.agregarIdPendiente($1, @1.first_line); }   /* PN2 */
      id_list_p
    ;

id_list_p
    : COMMA ID { semantica.agregarIdPendiente($2, @2.first_line); }   /* PN2 */
      id_list_p
    | /* vacio */
    ;

/* La regla regresa el tipo ($$) para que lo use quien la llamo */
type
    : INT     { $$ = Tipo::ENTERO; }
    | FLOAT   { $$ = Tipo::FLOTANTE; }
    ;

/* ---------- Funciones ---------- */

/* void f(a : int) [ var x : int; { ... } ]; */
funcs
    : func_type ID { semantica.iniciarFuncion($1, $2, @2.first_line); }   /* PN4 */
      LPAREN params RPAREN LBRACKET vars_opc body RBRACKET SEMICOLON
      { semantica.terminarFuncion(); }                                   /* PN6 */
    ;

func_type
    : VOID    { $$ = Tipo::VACIO; }
    | type    { $$ = $1; }
    ;

/* Cero o mas parametros separados por comas */
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

/* { estatutos } */
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

/* id = expresion;
   Al leer el "=" se revisa que la variable exista (PN7), se guarda como
   operando (PN11) y se guarda el "=" como operador (PN12). Cuando termina
   la expresion se genera el cuadruplo de la asignacion (PN19). */
assign
    : ID ASSIGN
      { cuadruplos.agregarOperando($1, semantica.usarVariable($1, @1.first_line));   /* PN7, PN11 */
        cuadruplos.agregarOperador("="); }                                           /* PN12 */
      expresion SEMICOLON
      { cuadruplos.generarAsignacion(@1.first_line); }                               /* PN19 */
    ;

/* if (expresion) estatuto else estatuto;
   Los saltos del if se generan en la Etapa 4. Por ahora solo se saca
   de la pila el resultado de la condicion. */
condition
    : IF LPAREN expresion RPAREN { cuadruplos.descartarOperando(); }
      statement else_opc SEMICOLON
    ;

else_opc
    : ELSE statement
    | /* vacio */
    ;

/* while (expresion) do estatuto;   (los saltos se generan en la Etapa 4) */
cycle
    : WHILE LPAREN expresion RPAREN { cuadruplos.descartarOperando(); }
      DO statement SEMICOLON
    ;

/* print(expresion, "letrero", ...);  se genera un cuadruplo print por cada cosa */
print
    : PRINT LPAREN print_item print_list RPAREN SEMICOLON
    ;

print_item
    : expresion   { cuadruplos.generarPrint(); }              /* PN20 */
    | CTE_STRING  { cuadruplos.generarPrintLetrero($1); }     /* PN21 */
    ;

print_list
    : COMMA print_item print_list
    | /* vacio */
    ;

/* read(id); */
read
    : READ LPAREN ID RPAREN SEMICOLON
      { semantica.usarVariable($3, @3.first_line);   /* PN7 */
        cuadruplos.generarRead($3); }                /* PN22 */
    ;

/* return expresion;   (su cuadruplo se genera en la etapa de funciones) */
return_st
    : RETURN expresion SEMICOLON
      { semantica.validarReturn(@1.first_line);   /* PN10 */
        cuadruplos.descartarOperando(); }
    ;

/* Llamada como estatuto (con ';'): puede ser de cualquier funcion, incluso void */
f_call
    : f_call_expr SEMICOLON { }
    ;

/* Llamada como valor dentro de una expresion (sin ';').
   Regresa el nombre de la funcion para que "factor" lo pueda usar. */
f_call_expr
    : ID LPAREN args RPAREN
      { semantica.llamarFuncion($1, @1.first_line); $$ = $1; }   /* PN8 */
    ;

/* Argumentos de la llamada. Sus cuadruplos se generan en la etapa de
   funciones; por ahora cada resultado solo se saca de la pila. */
args
    : expresion { cuadruplos.descartarOperando(); } args_p
    | /* vacio */
    ;

args_p
    : COMMA expresion { cuadruplos.descartarOperando(); } args_p
    | /* vacio */
    ;

/* ---------- Expresiones (de menor a mayor precedencia) ----------
   La gramatica tiene un nivel por cada precedencia:
     expresion -> relacionales  > < !=   (se resuelven al ultimo)
     exp       -> suma y resta  + -
     termino   -> mult. y div.  * /
     factor    -> parentesis, signo, variable, constante o llamada
   Algoritmo de traduccion: cada operando entra a su pila (PN11) y cada
   operador a la suya (PN12). En cuanto se termina de leer el operando
   derecho de un operador, se genera su cuadruplo (PN13, PN14, PN15). */

/* exp, o exp OPERADOR_RELACIONAL exp */
expresion
    : exp rel_opc
    ;

rel_opc
    : rel_op exp { cuadruplos.resolverRelacional(); }   /* PN15 */
    | /* vacio */
    ;

rel_op
    : GT  { cuadruplos.agregarOperador(">"); }    /* PN12 */
    | LT  { cuadruplos.agregarOperador("<"); }    /* PN12 */
    | NE  { cuadruplos.agregarOperador("!="); }   /* PN12 */
    ;

/* termino + termino - termino ... */
exp
    : termino exp_p
    ;

exp_p
    : PLUS  { cuadruplos.agregarOperador("+"); }        /* PN12 */
      termino { cuadruplos.resolverSumaResta(); }       /* PN13 */
      exp_p
    | MINUS { cuadruplos.agregarOperador("-"); }        /* PN12 */
      termino { cuadruplos.resolverSumaResta(); }       /* PN13 */
      exp_p
    | /* vacio */
    ;

/* factor * factor / factor ... */
termino
    : factor termino_p
    ;

termino_p
    : MULT { cuadruplos.agregarOperador("*"); }         /* PN12 */
      factor { cuadruplos.resolverMultDiv(); }          /* PN14 */
      termino_p
    | DIV  { cuadruplos.agregarOperador("/"); }         /* PN12 */
      factor { cuadruplos.resolverMultDiv(); }          /* PN14 */
      termino_p
    | /* vacio */
    ;

/* SIGNO se expande en tres alternativas para evitar un conflicto
   shift/reduce entre "SIGNO vacio + id" y "f_call_expr". */
factor
    : LPAREN { cuadruplos.abrirParentesis(); }          /* PN16 */
      expresion RPAREN { cuadruplos.cerrarParentesis(); }   /* PN17 */
    | PLUS valor                                        /* +x es lo mismo que x */
    | MINUS valor { cuadruplos.generarNegativo(); }     /* PN18 */
    | valor
    | f_call_expr
      /* PN9 revisa que la funcion regrese un valor. Ese valor queda como
         operando (PN11); el cuadruplo de la llamada se genera despues. */
      { cuadruplos.agregarOperando($1, semantica.usarLlamadaComoValor($1, @1.first_line)); }
    ;

/* Una variable o una constante: es un operando */
valor
    : ID { cuadruplos.agregarOperando($1, semantica.usarVariable($1, @1.first_line)); }   /* PN7, PN11 */
    | cte
    ;

cte
    : CTE_INT    { cuadruplos.agregarOperando($1, Tipo::ENTERO); }     /* PN11 */
    | CTE_FLOAT  { cuadruplos.agregarOperando($1, Tipo::FLOTANTE); }   /* PN11 */
    ;

%%

/* Bison llama a esta funcion cuando encuentra un error de sintaxis */
void yyerror(const char* msg) {
    fprintf(stderr, "Error sintactico (linea %d): %s\n", yylineno, msg);
}
