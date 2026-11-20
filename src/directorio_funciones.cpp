#include "directorio_funciones.hpp"
#include <cstdio>

// Igual que en la tabla de variables: la funcion va al final del vector
// y la tabla hash guarda su posicion.
bool DirectorioFunciones::agregar(const FuncInfo& funcion) {
    if (indice.contains(funcion.nombre)) return false;
    indice.put(funcion.nombre, funciones.size());
    funciones.push_back(funcion);
    return true;
}

// Solo para consultar: regresa nullptr si el nombre no es una funcion
const FuncInfo* DirectorioFunciones::buscar(const std::string& nombre) const {
    if (!indice.contains(nombre)) return nullptr;
    return &funciones[indice.get(nombre)];
}

// Para modificar una funcion que ya existe (agregarle parametros y variables)
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
    // Primero la lista de funciones con su tipo, su firma, el cuadruplo
    // donde empiezan y la direccion donde dejan el valor que regresan
    printf("Directorio de Funciones\n");
    printf("  %-14s %-8s %-6s %-16s %-7s %s\n", "nombre", "tipo", "linea", "parametros",
           "inicio", "retorno");
    for (const FuncInfo& funcion : funciones) {
        const char* tipo = funcion.esPrograma ? "program" : nombreTipo(funcion.tipoRetorno);
        std::string retorno = funcion.dirRetorno < 0 ? "-" : std::to_string(funcion.dirRetorno);
        printf("  %-14s %-8s %-6d %-16s %-7d %s\n", funcion.nombre.c_str(), tipo, funcion.linea,
               firma(funcion).c_str(), funcion.inicio, retorno.c_str());
    }

    // Despues, la tabla de variables de cada una
    for (const FuncInfo& funcion : funciones) {
        printf("\nTabla de Variables: %s%s\n", funcion.nombre.c_str(),
               funcion.esPrograma ? " (globales)" : "");
        funcion.variables.imprimir();
        printf("  Memoria: variables int=%d float=%d, temporales int=%d float=%d\n",
               funcion.variablesInt, funcion.variablesFloat,
               funcion.temporalesInt, funcion.temporalesFloat);
    }
}
