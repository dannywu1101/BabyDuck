#include "cubo_semantico.hpp"
#include <cstdio>
#include <vector>

namespace {

constexpr int NUM_OPERADORES = 8;
constexpr int NUM_OPERANDOS = 2;   // solo int y float pueden ser operandos

constexpr Tipo I = Tipo::ENTERO;
constexpr Tipo F = Tipo::FLOTANTE;
constexpr Tipo E = Tipo::ERROR;

// CUBO[operador][izq][der]. Indice de tipo: 0 = int, 1 = float.
// Cada bloque se lee como una tabla:   der: int  float
//                                   izq int  [ .    .  ]
//                                   izq float[ .    .  ]
constexpr Tipo CUBO[NUM_OPERADORES][NUM_OPERANDOS][NUM_OPERANDOS] = {
    /* +  */ { { I, F }, { F, F } },
    /* -  */ { { I, F }, { F, F } },
    /* *  */ { { I, F }, { F, F } },
    /* /  */ { { I, F }, { F, F } },
    /* >  */ { { I, I }, { I, I } },
    /* <  */ { { I, I }, { I, I } },
    /* != */ { { I, I }, { I, I } },
    /* =  */ { { I, E }, { F, F } },
};

bool esNumerico(Tipo tipo) { return tipo == Tipo::ENTERO || tipo == Tipo::FLOTANTE; }

int indice(Tipo tipo) { return tipo == Tipo::ENTERO ? 0 : 1; }

const char* simbolo(Operador op) {
    switch (op) {
        case Operador::SUMA:           return "+";
        case Operador::RESTA:          return "-";
        case Operador::MULTIPLICACION: return "*";
        case Operador::DIVISION:       return "/";
        case Operador::MAYOR:          return ">";
        case Operador::MENOR:          return "<";
        case Operador::DIFERENTE:      return "!=";
        default:                       return "=";
    }
}

}

Tipo cubo::resultado(Operador op, Tipo izq, Tipo der) {
    if (!esNumerico(izq) || !esNumerico(der)) return Tipo::ERROR;
    return CUBO[static_cast<int>(op)][indice(izq)][indice(der)];
}

Tipo cubo::resultadoUnario(Operador op, Tipo operando) {
    bool esSigno = op == Operador::SUMA || op == Operador::RESTA;
    return esSigno && esNumerico(operando) ? operando : Tipo::ERROR;
}

void cubo::imprimir() {
    struct Grupo {
        const char* titulo;
        std::vector<Operador> operadores;
    };
    const Grupo grupos[] = {
        { "Aritmeticos",
          { Operador::SUMA, Operador::RESTA, Operador::MULTIPLICACION, Operador::DIVISION } },
        { "Relacionales (el int resultante vale 1 si es verdadero y 0 si es falso)",
          { Operador::MAYOR, Operador::MENOR, Operador::DIFERENTE } },
        { "Asignacion (variable = expresion)",
          { Operador::ASIGNA } },
    };
    const Tipo operandos[] = { Tipo::ENTERO, Tipo::FLOTANTE };

    printf("Cubo semantico de BabyDuck\n");
    printf("Formato: izquierdo operador derecho -> resultado\n");
    for (const Grupo& grupo : grupos) {
        printf("\n%s\n", grupo.titulo);
        for (Operador op : grupo.operadores)
            for (Tipo izq : operandos)
                for (Tipo der : operandos)
                    printf("  %-6s %-2s %-6s -> %s\n", nombreTipo(izq), simbolo(op),
                           nombreTipo(der), nombreTipo(resultado(op, izq, der)));
    }

    printf("\nSigno delante de un valor\n");
    for (Operador op : { Operador::SUMA, Operador::RESTA })
        for (Tipo tipo : operandos)
            printf("  %s%-6s -> %s\n", simbolo(op), nombreTipo(tipo),
                   nombreTipo(resultadoUnario(op, tipo)));

    printf("\nCualquier operacion con un operando void o error da error.\n");
}
