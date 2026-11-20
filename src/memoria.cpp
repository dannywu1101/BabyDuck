#include "memoria.hpp"
#include <cstdio>
#include <cstdlib>

// Bloque de los letreros: es el ultimo (9000 - 9999)
static const int BLOQUE_LETREROS = NUM_BLOQUES - 1;

// Numero de bloque de un segmento y tipo. Los bloques van en este orden:
//   0 global int    1 global float    2 local int      3 local float
//   4 temporal int  5 temporal float  6 constante int  7 constante float
static int bloqueDe(Segmento segmento, Tipo tipo) {
    return static_cast<int>(segmento) * 2 + (tipo == Tipo::FLOTANTE ? 1 : 0);
}

// Toma la siguiente direccion libre del bloque.
// Ejemplo: bloque 1 (global float) con 2 usadas -> 1000 + 1*1000 + 2 = 2002
int Memoria::tomar(int bloque) {
    if (usados[bloque] == TAMANO_BLOQUE) {
        // Un programa con mas de 1000 datos de la misma clase no cabe
        fprintf(stderr, "Error de memoria: se acabaron las direcciones del bloque %d - %d\n",
                DIRECCION_BASE + bloque * TAMANO_BLOQUE,
                DIRECCION_BASE + (bloque + 1) * TAMANO_BLOQUE - 1);
        exit(1);
    }
    int direccion = DIRECCION_BASE + bloque * TAMANO_BLOQUE + usados[bloque];
    usados[bloque]++;
    return direccion;
}

int Memoria::nuevaDireccion(Segmento segmento, Tipo tipo) {
    if (tipo != Tipo::ENTERO && tipo != Tipo::FLOTANTE) return SIN_DIRECCION;
    return tomar(bloqueDe(segmento, tipo));
}

// Busca la constante; si es nueva le da una direccion y la guarda.
// Un int nunca se confunde con un float ni con un letrero porque su
// texto es distinto: 5, 5.0 y "5".
int Memoria::constante(const std::string& texto, int bloque, const char* tipo) {
    if (indice.contains(texto)) return indice.get(texto);

    int direccion = tomar(bloque);
    indice.put(texto, direccion);
    constantes.push_back(Constante{ texto, tipo, direccion });
    return direccion;
}

int Memoria::direccionConstante(const std::string& texto, Tipo tipo) {
    return constante(texto, bloqueDe(Segmento::CONSTANTE, tipo), nombreTipo(tipo));
}

int Memoria::direccionLetrero(const std::string& texto) {
    return constante(texto, BLOQUE_LETREROS, "letrero");
}

void Memoria::reiniciarLocales() {
    usados[bloqueDe(Segmento::LOCAL, Tipo::ENTERO)] = 0;
    usados[bloqueDe(Segmento::LOCAL, Tipo::FLOTANTE)] = 0;
    usados[bloqueDe(Segmento::TEMPORAL, Tipo::ENTERO)] = 0;
    usados[bloqueDe(Segmento::TEMPORAL, Tipo::FLOTANTE)] = 0;
}

int Memoria::usadas(Segmento segmento, Tipo tipo) const {
    return usados[bloqueDe(segmento, tipo)];
}

void Memoria::imprimirConstantes() const {
    printf("Tabla de constantes\n");
    if (constantes.empty()) {
        printf("  (sin constantes)\n");
        return;
    }
    printf("  %-10s %-8s %s\n", "direccion", "tipo", "valor");
    for (const Constante& c : constantes) {
        printf("  %-10d %-8s %s\n", c.direccion, c.tipo.c_str(), c.texto.c_str());
    }
}
