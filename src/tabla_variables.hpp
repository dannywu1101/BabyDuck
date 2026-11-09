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

enum class Categoria { GLOBAL, PARAMETRO, LOCAL };

const char* nombreCategoria(Categoria categoria);

struct VarInfo {
    std::string nombre;
    Tipo tipo;
    Categoria categoria;
    int linea;   // linea donde se declaro
};

// - HashTable nombre -> posicion: buscar una variable es O(1) promedio.
// - vector: conserva el orden de declaracion (asi se imprime la tabla).
class TablaVariables {
public:
    // Agrega la variable. Regresa false (y no la agrega) si el nombre ya existe.
    bool agregar(const VarInfo& variable);

    // Regresa la variable, o nullptr si no existe en esta tabla
    const VarInfo* buscar(const std::string& nombre) const;

    size_t tamano() const { return variables.size(); }

    void imprimir() const;

private:
    HashTable<std::string, size_t> indice;
    std::vector<VarInfo> variables;
};

#endif
