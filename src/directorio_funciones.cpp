#include "directorio_funciones.hpp"
#include <cstdio>

bool DirectorioFunciones::agregar(const FuncInfo& funcion) {
    if (indice.contains(funcion.nombre)) return false;
    indice.put(funcion.nombre, funciones.size());
    funciones.push_back(funcion);
    return true;
}

const FuncInfo* DirectorioFunciones::buscar(const std::string& nombre) const {
    if (!indice.contains(nombre)) return nullptr;
    return &funciones[indice.get(nombre)];
}

FuncInfo& DirectorioFunciones::obtener(const std::string& nombre) {
    return funciones[indice.get(nombre)];
}

// "(int, float)" para una funcion, "-" para el programa
static std::string firma(const FuncInfo& funcion) {
    if (funcion.esPrograma) return "-";
    std::string texto = "(";
    for (size_t i = 0; i < funcion.parametros.size(); i++) {
        if (i > 0) texto += ", ";
        texto += nombreTipo(funcion.parametros[i]);
    }
    return texto + ")";
}

void DirectorioFunciones::imprimir() const {
    printf("Directorio de Funciones\n");
    printf("  %-14s %-8s %-6s %s\n", "nombre", "tipo", "linea", "parametros");
    for (const FuncInfo& funcion : funciones) {
        const char* tipo = funcion.esPrograma ? "program" : nombreTipo(funcion.tipoRetorno);
        printf("  %-14s %-8s %-6d %s\n", funcion.nombre.c_str(), tipo, funcion.linea,
               firma(funcion).c_str());
    }

    for (const FuncInfo& funcion : funciones) {
        printf("\nTabla de Variables: %s%s\n", funcion.nombre.c_str(),
               funcion.esPrograma ? " (globales)" : "");
        funcion.variables.imprimir();
    }
}
