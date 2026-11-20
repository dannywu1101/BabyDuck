#include "semantica.hpp"
#include <cstdio>

// ---------- Ambitos ----------
// BabyDuck no tiene funciones anidadas, asi que solo hay dos niveles:
// el global (programa y main) y el local de la funcion actual.

// La entrada del programa: su tabla guarda las variables globales
FuncInfo& Semantica::global() {
    return directorio_.obtener(nombrePrograma);
}

// La entrada donde se declaran las variables en este momento:
// el programa (si estamos en lo global o en main) o la funcion actual
FuncInfo& Semantica::ambitoActual() {
    return enDescartada ? descartada : directorio_.obtener(nombreAmbito);
}

// true si estamos dentro de una funcion (no en lo global ni en main)
bool Semantica::enFuncion() const {
    return enDescartada || nombreAmbito != nombrePrograma;
}

// ---------- Errores ----------

// Todos los errores semanticos pasan por aqui: se imprimen y se cuentan.
// El analisis NO se detiene, asi se reportan todos los errores del programa.
void Semantica::error(int linea, const std::string& mensaje) {
    fprintf(stderr, "Error semantico (linea %d): %s\n", linea, mensaje.c_str());
    numErrores++;
}

// Mensaje de "doblemente declarado". comoQue dice que era el nombre
// la primera vez (variable global, parametro, funcion, ...).
void Semantica::reportarDuplicado(const std::string& nombre, int linea,
                                  const char* comoQue, int lineaPrevia) {
    error(linea, "'" + nombre + "' doblemente declarado (ya existe como " + comoQue +
                 " en la linea " + std::to_string(lineaPrevia) + ")");
}

// El programa, las funciones y las variables globales comparten el
// ambito global, asi que ningun nombre se puede repetir entre ellos.
// Regresa true (y reporta el error) si el nombre ya esta ocupado.
bool Semantica::yaExisteEnGlobal(const std::string& nombre, int linea) {
    // Ya hay una funcion (o el programa) con ese nombre?
    if (const FuncInfo* funcion = directorio_.buscar(nombre)) {
        reportarDuplicado(nombre, linea, funcion->esPrograma ? "programa" : "funcion",
                          funcion->linea);
        return true;
    }
    // Ya hay una variable global con ese nombre?
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
        // Variable global: no puede repetir ningun nombre global
        if (yaExisteEnGlobal(variable.nombre, variable.linea)) return false;
    } else if (const VarInfo* previa = ambitoActual().variables.buscar(variable.nombre)) {
        // Variable local o parametro: no puede repetirse dentro de su funcion
        reportarDuplicado(variable.nombre, variable.linea,
                          nombreCategoria(previa->categoria), previa->linea);
        return false;
    }
    ambitoActual().variables.agregar(variable);
    return true;
}

// ---------- Puntos neuralgicos: declaraciones ----------

// PN1  El programa es la primera entrada del directorio.
//      El ambito actual es el global hasta que empiece una funcion.
void Semantica::iniciarPrograma(const std::string& nombre, int linea) {
    nombrePrograma = nombre;
    nombreAmbito = nombre;
    directorio_.agregar(FuncInfo{ nombre, Tipo::VACIO, true, linea, {}, {}, {} });
}

// PN2  En "var a, b : int;" los nombres llegan antes que el tipo,
//      asi que se guardan en la fila hasta que se lea el tipo.
void Semantica::agregarIdPendiente(const std::string& nombre, int linea) {
    pendientes.enqueue(IdPendiente{ nombre, linea });
}

// PN3  Ya se leyo el tipo: se sacan los ids de la fila (en el mismo
//      orden en que llegaron) y se declaran con ese tipo.
void Semantica::declararVariables(Tipo tipo) {
    Categoria categoria = enFuncion() ? Categoria::LOCAL : Categoria::GLOBAL;
    Segmento segmento = enFuncion() ? Segmento::LOCAL : Segmento::GLOBAL;
    while (!pendientes.isEmpty()) {
        IdPendiente id = pendientes.dequeue();
        // Cada variable recibe su direccion virtual al declararse
        int direccion = memoria.nuevaDireccion(segmento, tipo);
        declarar(VarInfo{ id.nombre, tipo, categoria, id.linea, direccion });
    }
}

// PN4  Empieza una funcion: se registra y su tabla se vuelve el ambito actual
void Semantica::iniciarFuncion(Tipo tipoRetorno, const std::string& nombre, int linea) {
    // Cada funcion tiene su propia memoria: locales y temporales empiezan de cero
    memoria.reiniciarLocales();

    FuncInfo funcion{ nombre, tipoRetorno, false, linea, {}, {}, {} };
    // Una funcion int o float deja el valor que regresa en una direccion
    // global propia. Si es void, nuevaDireccion regresa -1 (no la necesita).
    funcion.dirRetorno = memoria.nuevaDireccion(Segmento::GLOBAL, tipoRetorno);
    if (yaExisteEnGlobal(nombre, linea)) {
        // Nombre repetido: no entra al directorio, pero su cuerpo si se revisa
        descartada = funcion;
        enDescartada = true;
        return;
    }
    // Se registra antes de su cuerpo para que pueda llamarse a si misma
    directorio_.agregar(funcion);
    nombreAmbito = nombre;
}

// PN5  Un parametro es una variable de la funcion, y ademas su tipo
//      se agrega a la firma (la lista de tipos de los parametros)
void Semantica::agregarParametro(const std::string& nombre, Tipo tipo, int linea) {
    int direccion = memoria.nuevaDireccion(Segmento::LOCAL, tipo);
    if (declarar(VarInfo{ nombre, tipo, Categoria::PARAMETRO, linea, direccion })) {
        ambitoActual().parametros.push_back(tipo);
        ambitoActual().dirParametros.push_back(direccion);
    }
}

// PN6  Termina la funcion: se guarda cuanta memoria ocupo y se regresa
//      al ambito global
void Semantica::terminarFuncion() {
    guardarMemoria(ambitoActual(), Segmento::LOCAL);
    nombreAmbito = nombrePrograma;
    enDescartada = false;
}

// ---------- Etapa 4: datos de cada funcion ----------

// Anota en la funcion cuantas variables y temporales de cada tipo uso.
// Con esto, al llamarla se sabe cuanta memoria hay que apartarle.
void Semantica::guardarMemoria(FuncInfo& funcion, Segmento variables) {
    funcion.variablesInt    = memoria.usadas(variables, Tipo::ENTERO);
    funcion.variablesFloat  = memoria.usadas(variables, Tipo::FLOTANTE);
    funcion.temporalesInt   = memoria.usadas(Segmento::TEMPORAL, Tipo::ENTERO);
    funcion.temporalesFloat = memoria.usadas(Segmento::TEMPORAL, Tipo::FLOTANTE);
}

// PN31 Ya se leyeron los parametros y las variables: aqui empieza el codigo
void Semantica::fijarInicioFuncion(int cuadruplo) {
    ambitoActual().inicio = cuadruplo;
}

// PN24 main usa las variables globales, pero tiene sus propios temporales
void Semantica::iniciarMain(int cuadruplo) {
    memoria.reiniciarLocales();
    global().inicio = cuadruplo;
}

// PN25 Las "variables" del programa son las globales
void Semantica::terminarPrograma() {
    guardarMemoria(global(), Segmento::GLOBAL);
}

// ---------- Puntos neuralgicos: usos ----------

// PN7  Busca primero en el ambito actual y luego en el global
const VarInfo* Semantica::usarVariable(const std::string& nombre, int linea) {
    if (const VarInfo* variable = ambitoActual().variables.buscar(nombre)) return variable;
    if (const VarInfo* variable = global().variables.buscar(nombre)) return variable;

    // No es una variable. Si es una funcion, el mensaje lo dice.
    if (const FuncInfo* funcion = directorio_.buscar(nombre)) {
        error(linea, "'" + nombre + "' es " +
                     (funcion->esPrograma ? "el nombre del programa" : "una funcion") +
                     ", no una variable");
    } else {
        error(linea, "variable '" + nombre + "' no declarada");
    }
    return nullptr;
}

// PN8  El nombre debe ser una funcion del directorio
const FuncInfo* Semantica::llamarFuncion(const std::string& nombre, int linea) {
    const FuncInfo* funcion = directorio_.buscar(nombre);
    if (funcion && !funcion->esPrograma) return funcion;   // todo bien

    // No es una funcion: se elige el mensaje segun lo que si es
    if (funcion) {
        error(linea, "'" + nombre + "' es el nombre del programa, no una funcion");
    } else if (ambitoActual().variables.buscar(nombre) || global().variables.buscar(nombre)) {
        error(linea, "'" + nombre + "' es una variable, no una funcion");
    } else {
        error(linea, "funcion '" + nombre + "' no declarada");
    }
    return nullptr;
}

// PN9  Una llamada usada como valor (a = f(1) + 2;) necesita que f regrese algo
const FuncInfo* Semantica::usarLlamadaComoValor(const std::string& nombre, int linea) {
    // Si la funcion no existe, llamarFuncion (PN8) ya reporto el error
    const FuncInfo* funcion = directorio_.buscar(nombre);
    if (!funcion || funcion->esPrograma) return nullptr;

    if (funcion->tipoRetorno == Tipo::VACIO) {
        error(linea, "la funcion '" + nombre + "' es void y no regresa un valor");
        return nullptr;
    }
    return funcion;
}

// PN10 return solo tiene sentido en una funcion que regresa int o float
const FuncInfo* Semantica::validarReturn(int linea) {
    if (!enFuncion()) {
        error(linea, "return no se permite en main, solo dentro de una funcion");
        return nullptr;
    }
    const FuncInfo& funcion = ambitoActual();
    if (funcion.tipoRetorno == Tipo::VACIO) {
        error(linea, "la funcion '" + funcion.nombre + "' es void y no puede regresar un valor");
        return nullptr;
    }
    return &funcion;
}
