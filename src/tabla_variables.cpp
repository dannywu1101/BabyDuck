#include "tabla_variables.hpp"
#include <cstdio>

const char* nombreCategoria(Categoria categoria) {
    switch (categoria) {
        case Categoria::GLOBAL:    return "variable global";
        case Categoria::PARAMETRO: return "parametro";
        default:                   return "variable local";
    }
}

bool TablaVariables::agregar(const VarInfo& variable) {
    if (indice.contains(variable.nombre)) return false;
    indice.put(variable.nombre, variables.size());
    variables.push_back(variable);
    return true;
}

const VarInfo* TablaVariables::buscar(const std::string& nombre) const {
    if (!indice.contains(nombre)) return nullptr;
    return &variables[indice.get(nombre)];
}

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
