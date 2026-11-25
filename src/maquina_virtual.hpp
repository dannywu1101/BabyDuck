// ============================================================
// BabyDuck - Maquina virtual (Etapa 5)
//
// Ejecuta la fila de cuadruplos que genero el compilador.
//
// Funciona como un ciclo:
//     1. toma el cuadruplo numero "actual"
//     2. hace lo que dice su operador (su codigo de operacion)
//     3. pasa al siguiente, o salta a otro si fue un goto, gosub, etc.
// y termina cuando llega al cuadruplo "end".
//
// Que hace cada codigo de operacion:
//     + - * /      resultado = izquierdo (operacion) derecho
//     > < !=       resultado = 1 si se cumple, 0 si no
//     =            resultado = izquierdo
//     print        escribe el valor (o el letrero) y un salto de linea
//     read         lee un numero del teclado y lo guarda
//     goto         salta al cuadruplo "resultado"
//     gotof        salta si el izquierdo vale 0 (falso)
//     era          crea el marco (la memoria) de la funcion
//     param        copia el argumento al parametro del marco nuevo
//     gosub        activa el marco nuevo y salta al inicio de la funcion
//     return       guarda el valor en la direccion de la funcion y regresa
//     endfunc      quita el marco y regresa a donde se llamo
//     end          termina el programa
// ============================================================
#ifndef MAQUINA_VIRTUAL_HPP
#define MAQUINA_VIRTUAL_HPP

#include <vector>
#include "cuadruplos.hpp"
#include "memoria_ejecucion.hpp"
#include "semantica.hpp"

class MaquinaVirtual {
public:
    // Recibe lo que dejo el compilador: los cuadruplos, el directorio de
    // funciones (dentro de la semantica) y la tabla de constantes
    MaquinaVirtual(const std::vector<Cuadruplo>& fila, const Semantica& semantica,
                   const Memoria& compilacion);

    // Ejecuta el programa completo
    void ejecutar();

private:
    const std::vector<Cuadruplo>& fila;
    const DirectorioFunciones& directorio;
    MemoriaEjecucion memoria;
    int actual = 1;   // numero del cuadruplo que se esta ejecutando

    // Cada una ejecuta una clase de cuadruplo
    void operar(const Cuadruplo& c);
    void imprimir(const Cuadruplo& c);
    void leer(const Cuadruplo& c);
    void regresar();
};

#endif
