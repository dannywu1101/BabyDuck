// ============================================================
// BabyDuck - Generador de cuadruplos (Etapa 3)
//
// Un cuadruplo es una instruccion sencilla con 4 partes:
//     (operador, operando izquierdo, operando derecho, resultado)
// Ejemplo:  a + b * c  se traduce a
//     (*, b, c, t1)      t1 guarda b * c
//     (+, a, t1, t2)     t2 guarda a + t1
//
// Para traducir se usan 3 pilas y 1 fila:
//   - pilaOperandos:  nombres de variables, constantes y temporales
//   - pilaTipos:      el tipo de cada operando (van siempre a la par)
//   - pilaOperadores: operadores que esperan a su operando derecho
//   - fila:           los cuadruplos ya generados, en orden
//
// El parser (parser.y) llama a estos metodos en los puntos
// neuralgicos PN11 a PN22.
// ============================================================
#ifndef CUADRUPLOS_HPP
#define CUADRUPLOS_HPP

#include <string>
#include "Queue.h"
#include "Stack.h"
#include "tipos.hpp"

// Las 4 partes de un cuadruplo. Una parte que no se usa queda vacia ("").
struct Cuadruplo {
    std::string operador;
    std::string izquierdo;
    std::string derecho;
    std::string resultado;
};

class GeneradorCuadruplos {
public:
    // ----- Expresiones -----
    // PN11  Llega una variable o constante: se guarda con su tipo
    void agregarOperando(const std::string& nombre, Tipo tipo);
    // PN12  Llega un operador (+ - * / > < != =): se guarda en su pila
    void agregarOperador(const std::string& simbolo);
    // PN13  Termino un TERMINO: si hay un + o - esperando, se genera su cuadruplo
    void resolverSumaResta();
    // PN14  Termino un FACTOR: si hay un * o / esperando, se genera su cuadruplo
    void resolverMultDiv();
    // PN15  Termino la segunda EXP: si hay un > < != esperando, se genera su cuadruplo
    void resolverRelacional();
    // PN16  "(": mete un fondo falso para proteger a los operadores de afuera
    void abrirParentesis();
    // PN17  ")": quita el fondo falso
    void cerrarParentesis();
    // PN18  Signo "-" delante de un valor: genera 0 - valor
    void generarNegativo();

    // ----- Estatutos lineales -----
    // PN19  Fin de "id = expresion;": genera (=, expresion, , id)
    void generarAsignacion(int linea);
    // PN20  print de una expresion: genera (print, , , resultado)
    void generarPrint();
    // PN21  print de un letrero: genera (print, , , "letrero")
    void generarPrintLetrero(const std::string& letrero);
    // PN22  read(id): genera (read, , , id)
    void generarRead(const std::string& nombre);

    // Saca un operando que todavia no se usa. Lo llaman if, while, return y
    // los argumentos de una llamada: sus cuadruplos se generan en la Etapa 4.
    void descartarOperando();

    // Imprime la fila de cuadruplos (modo --cuadruplos)
    void imprimir() const;

private:
    Stack<std::string> pilaOperandos;
    Stack<Tipo> pilaTipos;
    Stack<std::string> pilaOperadores;
    Queue<Cuadruplo> fila;
    int numTemporales = 0;   // cuantos temporales (t1, t2, ...) se han creado

    bool topeEs(const std::string& a, const std::string& b, const std::string& c = "") const;
    std::string nuevoTemporal();
    void generarOperacion();
};

// Instancia unica que usan el parser y main (definida en parser.y)
extern GeneradorCuadruplos cuadruplos;

#endif
