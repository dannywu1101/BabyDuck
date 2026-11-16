// ============================================================
// BabyDuck - Programa principal (Etapa 3)
//
// Uso:
//   ./babyduck archivo.bd                -> analiza lexico, sintaxis y semantica
//   ./babyduck --tokens archivo.bd       -> solo imprime los tokens
//   ./babyduck --tablas archivo.bd       -> analiza e imprime el Directorio de
//                                           Funciones y las Tablas de Variables
//   ./babyduck --cuadruplos archivo.bd   -> analiza e imprime la fila de cuadruplos
//   ./babyduck --cubo                    -> imprime el cubo semantico
//
// Codigo de salida: 0 = programa valido, 1 = hay errores, 2 = uso incorrecto
// ============================================================
#include <cstdio>
#include <cstring>
#include "cuadruplos.hpp"
#include "cubo_semantico.hpp"
#include "semantica.hpp"
#include "parser.tab.hpp"

// Cosas que definen el scanner (Flex) y el parser (Bison)
extern FILE* yyin;       // archivo que lee el scanner
extern char* yytext;     // texto del token actual
extern int yylineno;     // linea actual
extern int lexErrors;    // errores lexicos encontrados (lexer.l)
int yylex();             // regresa el siguiente token (0 = fin del archivo)
int yyparse();           // analiza todo el programa (0 = sintaxis correcta)

// Nombre de cada token para el modo --tokens.
// Los tokens son numeros (los define Bison); aqui se les pone su nombre.
static const char* tokenName(int tok) {
    switch (tok) {
        case PROGRAM: return "PROGRAM";     case MAIN: return "MAIN";
        case END: return "END";             case VAR: return "VAR";
        case VOID: return "VOID";           case INT: return "INT";
        case FLOAT: return "FLOAT";         case PRINT: return "PRINT";
        case READ: return "READ";           case WHILE: return "WHILE";
        case DO: return "DO";               case IF: return "IF";
        case ELSE: return "ELSE";           case RETURN: return "RETURN";
        case ASSIGN: return "ASSIGN";       case PLUS: return "PLUS";
        case MINUS: return "MINUS";         case MULT: return "MULT";
        case DIV: return "DIV";             case GT: return "GT";
        case LT: return "LT";               case NE: return "NE";
        case SEMICOLON: return "SEMICOLON"; case COMMA: return "COMMA";
        case COLON: return "COLON";         case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";       case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";       case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";   case ID: return "ID";
        case CTE_INT: return "CTE_INT";     case CTE_FLOAT: return "CTE_FLOAT";
        case CTE_STRING: return "CTE_STRING";
        default: return "DESCONOCIDO";
    }
}

// Modo --tokens: solo corre el scanner e imprime "linea TOKEN texto"
static int dumpTokens() {
    int tok;
    while ((tok = yylex()) != 0) {
        printf("%d %s %s\n", yylineno, tokenName(tok), yytext);
    }
    return lexErrors == 0 ? 0 : 1;
}

// Analiza el programa completo. yyparse() va pidiendo tokens al scanner
// y ejecuta los puntos neuralgicos, que llenan las tablas y los cuadruplos.
static int parseProgram(bool showTables, bool showQuads) {
    int result = yyparse();
    bool valido = result == 0 && lexErrors == 0 && semantica.errores() == 0;

    // Las tablas solo estan completas si la sintaxis fue correcta
    if (showTables && result == 0) {
        semantica.directorio().imprimir();
        printf("\n");
    }
    // Los cuadruplos solo se muestran si el programa no tiene ningun error
    if (showQuads && valido) {
        cuadruplos.imprimir();
        printf("\n");
    }

    printf(valido ? "Programa valido\n" : "Programa invalido\n");
    return valido ? 0 : 1;
}

int main(int argc, char* argv[]) {
    // --cubo es el unico modo que no lleva archivo
    if (argc == 2 && strcmp(argv[1], "--cubo") == 0) {
        cubo::imprimir();
        return 0;
    }

    // Con 3 argumentos, el primero es la opcion y el segundo el archivo
    const char* opcion = argc == 3 ? argv[1] : "";
    bool onlyTokens = strcmp(opcion, "--tokens") == 0;
    bool showTables = strcmp(opcion, "--tablas") == 0;
    bool showQuads  = strcmp(opcion, "--cuadruplos") == 0;

    if (argc != 2 && !onlyTokens && !showTables && !showQuads) {
        fprintf(stderr, "Uso: %s [--tokens | --tablas | --cuadruplos] archivo.bd\n", argv[0]);
        fprintf(stderr, "     %s --cubo\n", argv[0]);
        return 2;
    }

    // El archivo siempre es el ultimo argumento
    const char* path = argv[argc - 1];
    yyin = fopen(path, "r");
    if (yyin == nullptr) {
        fprintf(stderr, "No se pudo abrir el archivo: %s\n", path);
        return 2;
    }

    int status = onlyTokens ? dumpTokens() : parseProgram(showTables, showQuads);
    fclose(yyin);
    return status;
}
