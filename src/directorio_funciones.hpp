// ============================================================
// BabyDuck - Directorio de Funciones
// Una entrada para el programa (guarda las variables globales)
// y una por cada funcion declarada.
// ============================================================
#ifndef DIRECTORIO_FUNCIONES_HPP
#define DIRECTORIO_FUNCIONES_HPP

#include <string>
#include <vector>
#include "HashTable.h"
#include "tabla_variables.hpp"

struct FuncInfo {
    std::string nombre;
    Tipo tipoRetorno;               // void, int o float
    bool esPrograma;                // true solo en la entrada del programa
    int linea;                      // linea donde se declaro
    std::vector<Tipo> parametros;   // firma: tipos de los parametros, en orden
    TablaVariables variables;       // parametros + locales (o las globales)
};

// - HashTable nombre -> posicion: buscar una funcion es O(1) promedio.
// - vector: conserva el orden de declaracion.
class DirectorioFunciones {
public:
    // Agrega la funcion. Regresa false (y no la agrega) si el nombre ya existe.
    bool agregar(const FuncInfo& funcion);

    // Regresa la funcion, o nullptr si no existe
    const FuncInfo* buscar(const std::string& nombre) const;

    // Acceso para llenar la funcion (sus parametros y variables). Debe existir.
    FuncInfo& obtener(const std::string& nombre);

    // Imprime el directorio y la tabla de variables de cada entrada (modo --tablas)
    void imprimir() const;

private:
    HashTable<std::string, size_t> indice;
    std::vector<FuncInfo> funciones;
};

#endif
