#include "memoria_ejecucion.hpp"
#include <climits>
#include <cstdio>
#include <cstdlib>

// Un error al ejecutar no se puede recuperar: se avisa y se termina
void errorDeEjecucion(const std::string& mensaje) {
    fflush(stdout);   // primero sale lo que el programa ya habia impreso
    fprintf(stderr, "Error de ejecucion: %s\n", mensaje.c_str());
    exit(3);
}

// ---------- De la direccion a la casilla ----------

// Bloque de la direccion. Los bloques van en el mismo orden que al compilar:
//   0 global int    1 global float    2 local int      3 local float
//   4 temporal int  5 temporal float  6 constante int  7 constante float
//   8 letreros
static int bloqueDe(int direccion) {
    return (direccion - DIRECCION_BASE) / TAMANO_BLOQUE;
}

// Casilla dentro de su bloque
static int posicionDe(int direccion) {
    return (direccion - DIRECCION_BASE) % TAMANO_BLOQUE;
}

static const int BLOQUE_LETREROS = NUM_BLOQUES - 1;

bool MemoriaEjecucion::esLetrero(int direccion) {
    return bloqueDe(direccion) == BLOQUE_LETREROS;
}

// Los bloques pares son int y los nones float
bool MemoriaEjecucion::esEntera(int direccion) {
    return !esLetrero(direccion) && bloqueDe(direccion) % 2 == 0;
}

// El segmento dice en cual grupo de casillas esta la direccion.
// Las locales y los temporales se buscan en el marco que se recibe.
Casillas& MemoriaEjecucion::casillasDe(int direccion, Marco& marco) {
    switch (static_cast<Segmento>(bloqueDe(direccion) / 2)) {
        case Segmento::GLOBAL:   return globales;
        case Segmento::LOCAL:    return marco.locales;
        case Segmento::TEMPORAL: return marco.temporales;
        default:                 return constantes;
    }
}

// Las dos funciones que siguen regresan la casilla de una direccion.
// Nunca deberian fallar: si fallan, el compilador genero mal una direccion.
int& MemoriaEjecucion::casillaEntera(int direccion, Marco& marco) {
    std::vector<int>& casillas = casillasDe(direccion, marco).enteros;
    int posicion = posicionDe(direccion);
    if (posicion >= static_cast<int>(casillas.size())) {
        errorDeEjecucion("la direccion " + std::to_string(direccion) + " no existe");
    }
    return casillas[posicion];
}

double& MemoriaEjecucion::casillaFlotante(int direccion, Marco& marco) {
    std::vector<double>& casillas = casillasDe(direccion, marco).flotantes;
    int posicion = posicionDe(direccion);
    if (posicion >= static_cast<int>(casillas.size())) {
        errorDeEjecucion("la direccion " + std::to_string(direccion) + " no existe");
    }
    return casillas[posicion];
}

// ---------- Leer y escribir ----------

double MemoriaEjecucion::leer(int direccion) {
    Marco& actual = pilaMarcos.back();
    if (esEntera(direccion)) return casillaEntera(direccion, actual);
    return casillaFlotante(direccion, actual);
}

void MemoriaEjecucion::escribir(int direccion, double valor) {
    escribirEn(pilaMarcos.back(), direccion, valor);
}

void MemoriaEjecucion::escribirEn(Marco& marco, int direccion, double valor) {
    if (!esEntera(direccion)) {
        casillaFlotante(direccion, marco) = valor;
        return;
    }
    // Un int tiene limite: una operacion muy grande no se puede guardar
    if (valor > INT_MAX || valor < INT_MIN) {
        errorDeEjecucion("el resultado no cabe en un int");
    }
    casillaEntera(direccion, marco) = static_cast<int>(valor);   // quita los decimales
}

const std::string& MemoriaEjecucion::letrero(int direccion) const {
    return letreros[posicionDe(direccion)];
}

// ---------- Inicio ----------

void MemoriaEjecucion::iniciar(const FuncInfo& programa, const Memoria& compilacion) {
    // Globales: las variables globales y el valor de regreso de cada funcion
    globales = Casillas(programa.variablesInt, programa.variablesFloat);

    // main no tiene locales, solo temporales. Su marco es el fondo de la pila.
    Marco marcoMain;
    marcoMain.temporales = Casillas(programa.temporalesInt, programa.temporalesFloat);
    pilaMarcos.push_back(marcoMain);

    // Constantes: cada una se guarda en la casilla de su direccion
    constantes = Casillas(compilacion.usadas(Segmento::CONSTANTE, Tipo::ENTERO),
                          compilacion.usadas(Segmento::CONSTANTE, Tipo::FLOTANTE));
    for (const Memoria::Constante& c : compilacion.listaConstantes()) {
        if (esLetrero(c.direccion)) {
            // Se guarda sin las comillas. Los letreros estan en orden de direccion.
            letreros.push_back(c.texto.substr(1, c.texto.size() - 2));
        } else {
            double valor = strtod(c.texto.c_str(), nullptr);
            if (esEntera(c.direccion) && valor > INT_MAX) {
                errorDeEjecucion("la constante " + c.texto + " no cabe en un int");
            }
            escribir(c.direccion, valor);
        }
    }
}

// ---------- Llamadas a funcion ----------

// era: la funcion dice cuantas casillas de cada tipo necesita
void MemoriaEjecucion::apartarMarco(const FuncInfo& funcion) {
    Marco marco;
    marco.locales = Casillas(funcion.variablesInt, funcion.variablesFloat);
    marco.temporales = Casillas(funcion.temporalesInt, funcion.temporalesFloat);
    marcosApartados.push_back(marco);
}

// param: el parametro vive en el marco nuevo, no en el actual
void MemoriaEjecucion::escribirParametro(int direccion, double valor) {
    escribirEn(marcosApartados.back(), direccion, valor);
}

// gosub: el marco pasa de "apartado" a la pila de marcos
void MemoriaEjecucion::activarMarco(int regreso) {
    if (static_cast<int>(pilaMarcos.size()) == MAX_MARCOS) {
        errorDeEjecucion("demasiadas llamadas una dentro de otra (recursion sin fin?)");
    }
    marcosApartados.back().regreso = regreso;
    pilaMarcos.push_back(marcosApartados.back());
    marcosApartados.pop_back();
}

// endfunc: la memoria de la llamada desaparece
int MemoriaEjecucion::quitarMarco() {
    int regreso = pilaMarcos.back().regreso;
    pilaMarcos.pop_back();
    return regreso;
}
