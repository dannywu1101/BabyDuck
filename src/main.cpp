// ============================================================
// BabyDuck - Programa principal (Etapa 2)
//
// Uso:
//   ./babyduck archivo.bd            -> analiza lexico, sintaxis y semantica
//   ./babyduck --tokens archivo.bd   -> solo imprime los tokens
//   ./babyduck --tablas archivo.bd   -> analiza e imprime el Directorio de
//                                       Funciones y las Tablas de Variables
//   ./babyduck --cubo                -> imprime el cubo semantico
//
// Codigo de salida: 0 = programa valido, 1 = hay errores, 2 = uso incorrecto
// ============================================================
#include <cstdio>
#include <cstring>
#include "cubo_semantico.hpp"
#include "semantica.hpp"
#include "parser.tab.hpp"

extern FILE* yyin;
extern char* yytext;
extern int yylineno;
extern int lexErrors;
int yylex();
int yyparse();

// Nombre de cada token para el modo --tokens
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

static int dumpTokens() {
    int tok;
    while ((tok = yylex()) != 0) {
        printf("%d %s %s\n", yylineno, tokenName(tok), yytext);
    }
    return lexErrors == 0 ? 0 : 1;
}

static int parseProgram(bool showTables) {
    int result = yyparse();
    // Las tablas solo estan completas si la sintaxis fue correcta
    if (showTables && result == 0) {
        semantica.directorio().imprimir();
        printf("\n");
    }
    if (result == 0 && lexErrors == 0 && semantica.errores() == 0) {
        printf("Programa valido\n");
        return 0;
    }
    printf("Programa invalido\n");
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "--cubo") == 0) {
        cubo::imprimir();
        return 0;
    }

    bool onlyTokens = argc == 3 && strcmp(argv[1], "--tokens") == 0;
    bool showTables = argc == 3 && strcmp(argv[1], "--tablas") == 0;
    if (argc != 2 && !onlyTokens && !showTables) {
        fprintf(stderr, "Uso: %s [--tokens | --tablas] archivo.bd\n", argv[0]);
        fprintf(stderr, "     %s --cubo\n", argv[0]);
        return 2;
    }

    const char* path = argv[argc - 1];
    yyin = fopen(path, "r");
    if (yyin == nullptr) {
        fprintf(stderr, "No se pudo abrir el archivo: %s\n", path);
        return 2;
    }

    int status = onlyTokens ? dumpTokens() : parseProgram(showTables);
    fclose(yyin);
    return status;
}
