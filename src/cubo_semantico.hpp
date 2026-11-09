// ============================================================
// BabyDuck - Cubo semantico
// Dice que tipo resulta de combinar dos operandos con un operador.
// ============================================================
#ifndef CUBO_SEMANTICO_HPP
#define CUBO_SEMANTICO_HPP

#include "tipos.hpp"

// Operadores de BabyDuck (el orden es el indice en el cubo)
enum class Operador { SUMA, RESTA, MULTIPLICACION, DIVISION, MAYOR, MENOR, DIFERENTE, ASIGNA };

namespace cubo {

// Tipo de "izq op der", o Tipo::ERROR si la combinacion no es valida.
// En ASIGNA, izq es la variable que recibe y der es la expresion.
Tipo resultado(Operador op, Tipo izq, Tipo der);

// Tipo de un signo delante de un valor: +x o -x
Tipo resultadoUnario(Operador op, Tipo operando);

// Imprime el cubo completo (modo --cubo)
void imprimir();

}

#endif
