#include "tabla_variables.hpp"
#include <cstdio>

// Texto de cada categoria (para imprimir las tablas y para los mensajes de error)
const char* nombreCategoria(Categoria categoria) {
    switch (categoria) {
        case Categoria::GLOBAL:    return "variable global";
        case Categoria::PARAMETRO: return "parametro";
        default:                   return "variable local";
    }
}

// La variable se guarda al final del vector, y en la tabla hash se
// anota en que posicion quedo para encontrarla rapido por su nombre.
bool TablaVariables::agregar(const VarInfo& variable) {
    if (indice.contains(variable.nombre)) return false;
    indice.put(variable.nombre, variables.size());
    variables.push_back(variable);
    return true;
}

// La tabla hash da la posicion y con ella se saca la variable del vector.
// Se revisa contains() primero porque get() falla si el nombre no existe.
const VarInfo* TablaVariables::buscar(const std::string& nombre) const {
    if (!indice.contains(nombre)) return nullptr;
    return &variables[indice.get(nombre)];
}

// Imprime una variable por renglon, en el orden en que se declararon
void TablaVariables::imprimir() const {
    if (variables.empty()) {
        printf("  (sin variables)\n");
        return;
    }
    printf("  %-14s %-6s %-16s %s\n", "nombre", "tipo", "categoria", "linea");
    for (const VarInfo& var : variables) {
        printf("  %-14s %-6s %-16s %d\n", var.nombre.c_str(), nombreTipo(var.tipo),
               nombreCategoria(var.categoria), var.linea);
    }
}
