// ============================================================
// BabyDuck - Generador de cuadruplos (Etapas 3 y 4)
//
// Un cuadruplo es una instruccion sencilla con 4 partes:
//     (operador, operando izquierdo, operando derecho, resultado)
// Ejemplo:  a + b * c  se traduce a
//     (*, b, c, t1)      t1 guarda b * c
//     (+, a, t1, t2)     t2 guarda a + t1
//
// Desde la Etapa 4 los operandos ya no son nombres, sino DIRECCIONES
// VIRTUALES (ver memoria.hpp). Si a=1000, b=1001 y c=1002:
//     (*, 1001, 1002, 5000)
//     (+, 1000, 5000, 5001)
//
// Estructuras que se usan para traducir:
//   - pilaOperandos:  direcciones de variables, constantes y temporales
//   - pilaTipos:      el tipo de cada operando (van siempre a la par)
//   - pilaOperadores: operadores que esperan a su operando derecho
//   - pilaSaltos:     cuadruplos de salto que todavia no saben a donde ir
//   - pilaLlamadas:   llamadas a funcion que se estan leyendo
//   - fila:           los cuadruplos ya generados, en orden
//
// El parser (parser.y) llama a estos metodos en los puntos
// neuralgicos PN11 a PN37.
//
// Cuadruplos que existen:
//     + - * / > < !=   (op, izq, der, resultado)
//     =                (=, valor, _, variable)
//     print            (print, _, _, valor)
//     read             (read, _, _, variable)
//     goto             (goto, _, _, cuadruplo)            salta siempre
//     gotof            (gotof, condicion, _, cuadruplo)   salta si la condicion es falsa (0)
//     era              (era, _, _, funcion)               aparta la memoria de la funcion
//     param            (param, valor, _, parametro)       copia un argumento a su parametro
//     gosub            (gosub, funcion, _, cuadruplo)     salta al codigo de la funcion
//     return           (return, valor, _, dirRetorno)     guarda el valor y sale de la funcion
//     endfunc          (endfunc, _, _, _)                 termina la funcion y regresa
//     end              (end, _, _, _)                     termina el programa
// ============================================================
#ifndef CUADRUPLOS_HPP
#define CUADRUPLOS_HPP

#include <string>
#include <vector>
#include "Stack.h"
#include "directorio_funciones.hpp"
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
    // ----- Expresiones (Etapa 3) -----
    // PN11  Llega una variable: se guarda su direccion y su tipo.
    //       Si la variable no existe (nullptr) se guarda como error.
    void agregarVariable(const VarInfo* variable);
    // PN11  Llega una constante: igual, con la direccion de la constante
    void agregarConstante(const std::string& texto, Tipo tipo);
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

    // ----- Estatutos lineales (Etapa 3) -----
    // PN19  Fin de "id = expresion;": genera (=, expresion, , id)
    void generarAsignacion(const std::string& nombre, int linea);
    // PN20  print de una expresion: genera (print, , , resultado)
    void generarPrint();
    // PN21  print de un letrero: genera (print, , , letrero)
    void generarPrintLetrero(const std::string& letrero);
    // PN22  read(id): genera (read, , , id)
    void generarRead(const VarInfo* variable);

    // ----- Programa (Etapa 4) -----
    // PN23  Inicio del programa: genera un goto a main (se rellena despues)
    void iniciarPrograma();
    // PN24  Al leer "main": rellena el goto del inicio
    void iniciarMain();
    // PN25  Al leer "end": genera (end, , , )
    void terminarPrograma();

    // ----- if y while (Etapa 4) -----
    // PN26  Termino la condicion: genera un gotof pendiente
    void generarGotoFalso(int linea);
    // PN27  Al leer "else": genera un goto pendiente y rellena el gotof
    void generarElse();
    // PN28  Fin del if: rellena el salto que quedo pendiente
    void terminarIf();
    // PN29  Al leer "while": guarda a donde hay que regresar
    void iniciarWhile();
    // PN30  Fin del while: genera el goto de regreso y rellena el gotof
    void terminarWhile();

    // ----- Funciones (Etapa 4) -----
    // PN32  Fin de una funcion: genera (endfunc, , , )
    void terminarFuncion();
    // PN33  "id (": empieza una llamada, genera (era, , , funcion)
    void iniciarLlamada(const FuncInfo* funcion);
    // PN34  Termino un argumento: lo valida y genera (param, valor, , parametro)
    void agregarArgumento(int linea);
    // PN35  ")": valida cuantos argumentos llegaron y genera el gosub
    void terminarLlamada(const std::string& nombre, int linea);
    // PN36  Llamada dentro de una expresion: copia el valor que regreso
    //       la funcion a un temporal, que queda como operando
    void agregarResultadoLlamada(const FuncInfo* funcion);
    // PN37  return expresion; genera (return, valor, , dirRetorno)
    void generarReturn(const FuncInfo* funcion, int linea);

    // Numero que va a tener el siguiente cuadruplo (el primero es el 1)
    int siguiente() const;

    // Imprime la fila de cuadruplos (modo --cuadruplos)
    void imprimir() const;

    // La fila completa (la maquina virtual la recorre para ejecutarla)
    const std::vector<Cuadruplo>& lista() const { return fila; }

private:
    // Una llamada que se esta leyendo: f(a, b ...
    struct Llamada {
        const FuncInfo* funcion;   // nullptr si la funcion no existe
        int numArgumentos;         // cuantos argumentos se han leido
    };

    Stack<std::string> pilaOperandos;
    Stack<Tipo> pilaTipos;
    Stack<std::string> pilaOperadores;
    Stack<int> pilaSaltos;
    Stack<Llamada> pilaLlamadas;
    // La fila de cuadruplos. Es un vector porque los saltos se generan
    // incompletos y luego hay que regresar a rellenarlos por su numero.
    std::vector<Cuadruplo> fila;

    void agregarOperando(const std::string& direccion, Tipo tipo);
    void agregar(const std::string& operador, const std::string& izquierdo,
                 const std::string& derecho, const std::string& resultado);
    void rellenar(int numero, int destino);
    bool topeEs(const std::string& a, const std::string& b, const std::string& c = "") const;
    std::string nuevoTemporal(Tipo tipo);
    void generarOperacion();
};

// Instancia unica que usan el parser y main (definida en parser.y)
extern GeneradorCuadruplos cuadruplos;

#endif
