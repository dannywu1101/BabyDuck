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

// Lo que se guarda de cada funcion (y del programa)
struct FuncInfo {
    std::string nombre;
    Tipo tipoRetorno;               // void, int o float
    bool esPrograma;                // true solo en la entrada del programa
    int linea;                      // linea donde se declaro
    std::vector<Tipo> parametros;   // firma: tipos de los parametros, en orden
    TablaVariables variables;       // parametros + locales (o las globales)

    // ----- Etapa 4: lo que se necesita para llamar a la funcion -----
    std::vector<int> dirParametros; // direccion de cada parametro, en orden
    int inicio = 0;                 // numero del cuadruplo donde empieza su codigo
    int dirRetorno = -1;            // direccion global donde deja el valor que regresa
                                    // (-1 si es void o es el programa)
    // Memoria que ocupa: cuantas variables y temporales de cada tipo.
    // En el programa, "variables" son las globales y los temporales son los de main.
    int variablesInt = 0, variablesFloat = 0;
    int temporalesInt = 0, temporalesFloat = 0;
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
    HashTable<std::string, size_t> indice;   // nombre -> posicion en el vector
    std::vector<FuncInfo> funciones;         // las funciones, en orden de declaracion
};

#endif
