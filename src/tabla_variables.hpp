// ============================================================
// BabyDuck - Tabla de Variables
// Una tabla por ambito: la global (del programa) y una por funcion.
// ============================================================
#ifndef TABLA_VARIABLES_HPP
#define TABLA_VARIABLES_HPP

#include <string>
#include <vector>
#include "HashTable.h"
#include "tipos.hpp"

// Donde se declaro la variable
enum class Categoria { GLOBAL, PARAMETRO, LOCAL };

const char* nombreCategoria(Categoria categoria);

// Lo que se guarda de cada variable
struct VarInfo {
    std::string nombre;
    Tipo tipo;             // int o float
    Categoria categoria;   // global, parametro o local
    int linea;             // linea donde se declaro
    int direccion;         // direccion virtual (Etapa 4, ver memoria.hpp)
};

// - HashTable nombre -> posicion: buscar una variable es O(1) promedio.
// - vector: conserva el orden de declaracion (asi se imprime la tabla).
class TablaVariables {
public:
    // Agrega la variable. Regresa false (y no la agrega) si el nombre ya existe.
    bool agregar(const VarInfo& variable);

    // Regresa la variable, o nullptr si no existe en esta tabla
    const VarInfo* buscar(const std::string& nombre) const;

    // Cuantas variables tiene la tabla
    size_t tamano() const { return variables.size(); }

    // Imprime la tabla en orden de declaracion (modo --tablas)
    void imprimir() const;

private:
    HashTable<std::string, size_t> indice;   // nombre -> posicion en el vector
    std::vector<VarInfo> variables;          // las variables, en orden de declaracion
};

#endif
