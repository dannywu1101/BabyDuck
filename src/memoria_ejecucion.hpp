// ============================================================
// BabyDuck - Memoria de ejecucion (Etapa 5)
//
// Al compilar, cada variable, temporal y constante recibio una
// DIRECCION VIRTUAL (memoria.hpp). Al ejecutar, esta clase guarda el
// VALOR que hay en cada direccion.
//
// Mapa de memoria:
//
//     Globales     1000 - 2999   una sola copia para todo el programa
//     Locales      3000 - 4999   una copia por cada llamada (en su Marco)
//     Temporales   5000 - 6999   una copia por cada llamada (en su Marco)
//     Constantes   7000 - 9999   una sola copia, se cargan antes de empezar
//
// La direccion sirve de indice. Con dos cuentas se sabe donde esta el valor:
//
//     bloque   = (direccion - 1000) / 1000     que segmento y que tipo
//     posicion = (direccion - 1000) % 1000     que casilla dentro del bloque
//
// Ejemplo: 4002 -> bloque 3 (local float), posicion 2
//          -> es la casilla flotantes[2] de las locales del marco actual.
//
// Cada llamada a una funcion tiene su propio Marco (sus locales y sus
// temporales). Los marcos se guardan en una pila: asi la recursion
// funciona, porque cada llamada tiene sus propios valores.
// ============================================================
#ifndef MEMORIA_EJECUCION_HPP
#define MEMORIA_EJECUCION_HPP

#include <string>
#include <vector>
#include "directorio_funciones.hpp"
#include "memoria.hpp"

// Un grupo de casillas: las int en un vector y las float en otro.
// Todas empiezan en 0.
struct Casillas {
    std::vector<int> enteros;
    std::vector<double> flotantes;

    Casillas(int numEnteros = 0, int numFlotantes = 0)
        : enteros(numEnteros, 0), flotantes(numFlotantes, 0.0) {}
};

// La memoria de UNA llamada a una funcion (registro de activacion)
struct Marco {
    Casillas locales;      // parametros y variables locales
    Casillas temporales;   // resultados intermedios
    int regreso = 0;       // cuadruplo al que se regresa cuando termina
};

// Limite de llamadas una dentro de otra (detecta la recursion sin fin)
const int MAX_MARCOS = 10000;

class MemoriaEjecucion {
public:
    // Prepara la memoria antes de ejecutar: aparta las globales, carga las
    // constantes y crea el marco de main
    void iniciar(const FuncInfo& programa, const Memoria& compilacion);

    // Valor guardado en una direccion (int o float; un int cabe en un double)
    double leer(int direccion);
    // Guarda un valor en una direccion. Si la direccion es int, se le
    // quitan los decimales (asi 7 / 2 da 3).
    void escribir(int direccion, double valor);
    // Texto de un letrero (direcciones 9000 - 9999)
    const std::string& letrero(int direccion) const;

    // ----- Llamadas a funcion -----
    // era:     crea el marco de la funcion. Todavia no es el actual.
    void apartarMarco(const FuncInfo& funcion);
    // param:   guarda un argumento en un parametro del marco apartado
    void escribirParametro(int direccion, double valor);
    // gosub:   el marco apartado se vuelve el actual
    void activarMarco(int regreso);
    // endfunc: quita el marco actual y dice a que cuadruplo regresar
    int quitarMarco();

    // true si la direccion guarda un int / un letrero
    static bool esEntera(int direccion);
    static bool esLetrero(int direccion);

private:
    Casillas globales;
    Casillas constantes;
    std::vector<std::string> letreros;

    // Pila de marcos: el ultimo es el de la funcion que se esta ejecutando
    // y el primero es el de main. Es un vector usado como pila (solo se
    // agrega y se quita al final) porque hay que leer y escribir en el
    // tope sin sacarlo.
    std::vector<Marco> pilaMarcos;
    // Marcos creados con "era" que todavia esperan su "gosub". Es una pila
    // porque un argumento puede ser otra llamada: f(g(1)).
    std::vector<Marco> marcosApartados;

    double& casillaFlotante(int direccion, Marco& marco);
    int& casillaEntera(int direccion, Marco& marco);
    Casillas& casillasDe(int direccion, Marco& marco);
    void escribirEn(Marco& marco, int direccion, double valor);
};

// Reporta un error que ocurre al ejecutar (division entre cero, etc.)
// y termina el programa con codigo 3
[[noreturn]] void errorDeEjecucion(const std::string& mensaje);

#endif
