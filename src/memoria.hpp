// ============================================================
// BabyDuck - Direcciones virtuales (Etapas 4 y 5)
//
// En los cuadruplos ya no se usan nombres (a, b, t1), sino numeros
// llamados DIRECCIONES VIRTUALES. Cada variable, temporal y constante
// recibe una direccion, y el numero dice de que clase y tipo es:
//
//     segmento      int           float         letrero
//     Globales      1000 - 1999   2000 - 2999
//     Locales       3000 - 3999   4000 - 4999
//     Temporales    5000 - 5999   6000 - 6999
//     Constantes    7000 - 7999   8000 - 8999   9000 - 9999
//
// Cada bloque tiene 1000 direcciones. Ejemplo: la primera variable
// global int es la 1000, la segunda la 1001; el primer temporal float
// es el 6000.
//
// Las locales y los temporales se vuelven a contar desde el inicio en
// cada funcion (y en main), porque cada funcion tiene su propia memoria.
//
// Esta clase solo REPARTE direcciones mientras se compila. Los valores
// se guardan al ejecutar, en memoria_ejecucion.hpp (Etapa 5).
// ============================================================
#ifndef MEMORIA_HPP
#define MEMORIA_HPP

#include <string>
#include <vector>
#include "HashTable.h"
#include "tipos.hpp"

// Las 4 clases de direcciones (el orden es el orden de los bloques)
enum class Segmento { GLOBAL, LOCAL, TEMPORAL, CONSTANTE };

const int DIRECCION_BASE = 1000;   // primera direccion que se reparte
const int TAMANO_BLOQUE  = 1000;   // direcciones que tiene cada bloque
const int NUM_BLOQUES    = 9;      // 4 segmentos x 2 tipos + los letreros
const int SIN_DIRECCION  = -1;     // cuando el tipo es invalido (hubo un error antes)

class Memoria {
public:
    // Reparte la siguiente direccion libre de ese segmento y tipo.
    // Regresa SIN_DIRECCION si el tipo no es int ni float.
    int nuevaDireccion(Segmento segmento, Tipo tipo);

    // Direccion de una constante (5, 3.14). Si la constante ya habia
    // salido antes, regresa la misma direccion: no se repiten.
    int direccionConstante(const std::string& texto, Tipo tipo);

    // Igual que la anterior, para los letreros de print ("hola")
    int direccionLetrero(const std::string& texto);

    // Empieza una funcion (o main): locales y temporales vuelven a cero
    void reiniciarLocales();

    // Cuantas direcciones se han repartido de ese segmento y tipo
    int usadas(Segmento segmento, Tipo tipo) const;

    // Imprime la tabla de constantes (modo --cuadruplos)
    void imprimirConstantes() const;

    // Una constante con la direccion que le toco
    struct Constante {
        std::string texto;   // como se escribio en el programa
        std::string tipo;    // int, float o letrero
        int direccion;
    };

    // Las constantes del programa (la maquina virtual las carga a su memoria)
    const std::vector<Constante>& listaConstantes() const { return constantes; }

private:
    int usados[NUM_BLOQUES] = {};            // direcciones repartidas de cada bloque
    HashTable<std::string, int> indice;      // texto de la constante -> direccion
    std::vector<Constante> constantes;       // las constantes, en orden de aparicion

    int tomar(int bloque);
    int constante(const std::string& texto, int bloque, const char* tipo);
};

// Instancia unica (definida en parser.y)
extern Memoria memoria;

#endif
