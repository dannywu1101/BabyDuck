#include "semantica.hpp"
#include <cstdio>

// ---------- Ambitos ----------
// BabyDuck no tiene funciones anidadas, asi que solo hay dos niveles:
// el global (programa y main) y el local de la funcion actual.

FuncInfo& Semantica::global() {
    return directorio_.obtener(nombrePrograma);
}

FuncInfo& Semantica::ambitoActual() {
    return enDescartada ? descartada : directorio_.obtener(nombreAmbito);
}

bool Semantica::enFuncion() const {
    return enDescartada || nombreAmbito != nombrePrograma;
}

// ---------- Errores ----------

void Semantica::error(int linea, const std::string& mensaje) {
    fprintf(stderr, "Error semantico (linea %d): %s\n", linea, mensaje.c_str());
    numErrores++;
}

void Semantica::reportarDuplicado(const std::string& nombre, int linea,
                                  const char* comoQue, int lineaPrevia) {
    error(linea, "'" + nombre + "' doblemente declarado (ya existe como " + comoQue +
                 " en la linea " + std::to_string(lineaPrevia) + ")");
}

// El programa, las funciones y las variables globales comparten el
// ambito global, asi que ningun nombre se puede repetir entre ellos.
bool Semantica::yaExisteEnGlobal(const std::string& nombre, int linea) {
    if (const FuncInfo* funcion = directorio_.buscar(nombre)) {
        reportarDuplicado(nombre, linea, funcion->esPrograma ? "programa" : "funcion",
                          funcion->linea);
        return true;
    }
    if (const VarInfo* variable = global().variables.buscar(nombre)) {
        reportarDuplicado(nombre, linea, nombreCategoria(variable->categoria),
                          variable->linea);
        return true;
    }
    return false;
}

// Agrega la variable al ambito actual si su nombre no se repite.
// Dentro de una funcion solo se revisa su propia tabla: un parametro o una
// variable local puede tener el mismo nombre que algo global (lo oculta).
bool Semantica::declarar(const VarInfo& variable) {
    if (!enFuncion()) {
        if (yaExisteEnGlobal(variable.nombre, variable.linea)) return false;
    } else if (const VarInfo* previa = ambitoActual().variables.buscar(variable.nombre)) {
        reportarDuplicado(variable.nombre, variable.linea,
                          nombreCategoria(previa->categoria), previa->linea);
        return false;
    }
    ambitoActual().variables.agregar(variable);
    return true;
}

// ---------- Puntos neuralgicos: declaraciones ----------

void Semantica::iniciarPrograma(const std::string& nombre, int linea) {
    nombrePrograma = nombre;
    nombreAmbito = nombre;
    directorio_.agregar(FuncInfo{ nombre, Tipo::VACIO, true, linea, {}, {} });
}

void Semantica::agregarIdPendiente(const std::string& nombre, int linea) {
    pendientes.enqueue(IdPendiente{ nombre, linea });
}

void Semantica::declararVariables(Tipo tipo) {
    Categoria categoria = enFuncion() ? Categoria::LOCAL : Categoria::GLOBAL;
    while (!pendientes.isEmpty()) {
        IdPendiente id = pendientes.dequeue();
        declarar(VarInfo{ id.nombre, tipo, categoria, id.linea });
    }
}

void Semantica::iniciarFuncion(Tipo tipoRetorno, const std::string& nombre, int linea) {
    FuncInfo funcion{ nombre, tipoRetorno, false, linea, {}, {} };
    if (yaExisteEnGlobal(nombre, linea)) {
        descartada = funcion;
        enDescartada = true;
        return;
    }
    // Se registra antes de su cuerpo para que pueda llamarse a si misma
    directorio_.agregar(funcion);
    nombreAmbito = nombre;
}

void Semantica::agregarParametro(const std::string& nombre, Tipo tipo, int linea) {
    if (declarar(VarInfo{ nombre, tipo, Categoria::PARAMETRO, linea })) {
        ambitoActual().parametros.push_back(tipo);
    }
}

void Semantica::terminarFuncion() {
    nombreAmbito = nombrePrograma;
    enDescartada = false;
}

// ---------- Puntos neuralgicos: usos ----------

// Busca primero en el ambito actual y luego en el global
void Semantica::usarVariable(const std::string& nombre, int linea) {
    if (ambitoActual().variables.buscar(nombre) || global().variables.buscar(nombre)) return;

    if (const FuncInfo* funcion = directorio_.buscar(nombre)) {
        error(linea, "'" + nombre + "' es " +
                     (funcion->esPrograma ? "el nombre del programa" : "una funcion") +
                     ", no una variable");
        return;
    }
    error(linea, "variable '" + nombre + "' no declarada");
}

void Semantica::llamarFuncion(const std::string& nombre, int linea) {
    const FuncInfo* funcion = directorio_.buscar(nombre);
    if (funcion && !funcion->esPrograma) return;

    if (funcion) {
        error(linea, "'" + nombre + "' es el nombre del programa, no una funcion");
    } else if (ambitoActual().variables.buscar(nombre) || global().variables.buscar(nombre)) {
        error(linea, "'" + nombre + "' es una variable, no una funcion");
    } else {
        error(linea, "funcion '" + nombre + "' no declarada");
    }
}

void Semantica::usarLlamadaComoValor(const std::string& nombre, int linea) {
    // Si la funcion no existe, llamarFuncion ya reporto el error
    const FuncInfo* funcion = directorio_.buscar(nombre);
    if (funcion && !funcion->esPrograma && funcion->tipoRetorno == Tipo::VACIO) {
        error(linea, "la funcion '" + nombre + "' es void y no regresa un valor");
    }
}

void Semantica::validarReturn(int linea) {
    if (!enFuncion()) {
        error(linea, "return no se permite en main, solo dentro de una funcion");
        return;
    }
    const FuncInfo& funcion = ambitoActual();
    if (funcion.tipoRetorno == Tipo::VACIO) {
        error(linea, "la funcion '" + funcion.nombre + "' es void y no puede regresar un valor");
    }
}
