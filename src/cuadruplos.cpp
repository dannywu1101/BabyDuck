#include "cuadruplos.hpp"
#include <cstdio>
#include "cubo_semantico.hpp"
#include "memoria.hpp"
#include "semantica.hpp"

// Marca que se mete a la pila de operadores al abrir un parentesis
static const char* FONDO_FALSO = "(";

// Un salto que todavia no sabe a donde va se genera con este destino
static const char* PENDIENTE = "?";

// Convierte el simbolo de la pila ("+", ">", ...) al Operador que usa el cubo
static Operador aOperador(const std::string& simbolo) {
    if (simbolo == "+")  return Operador::SUMA;
    if (simbolo == "-")  return Operador::RESTA;
    if (simbolo == "*")  return Operador::MULTIPLICACION;
    if (simbolo == "/")  return Operador::DIVISION;
    if (simbolo == ">")  return Operador::MAYOR;
    if (simbolo == "<")  return Operador::MENOR;
    if (simbolo == "!=") return Operador::DIFERENTE;
    return Operador::ASIGNA;
}

// true si el tipo es int o float (no void ni error)
static bool esNumerico(Tipo tipo) {
    return tipo == Tipo::ENTERO || tipo == Tipo::FLOTANTE;
}

// true si a algo de tipo "destino" NO se le puede dar un valor de tipo
// "valor". Lo decide el cubo semantico: int = float no se permite.
// Si alguno ya venia con error o es void, ese error ya se reporto antes.
static bool noSePuedeAsignar(Tipo destino, Tipo valor) {
    return esNumerico(destino) && esNumerico(valor) &&
           cubo::resultado(Operador::ASIGNA, destino, valor) == Tipo::ERROR;
}

// ---------- Ayudas internas ----------

// Mete un operando (su direccion) y su tipo a las pilas
void GeneradorCuadruplos::agregarOperando(const std::string& direccion, Tipo tipo) {
    pilaOperandos.push(direccion);
    pilaTipos.push(tipo);
}

// Agrega un cuadruplo al final de la fila
void GeneradorCuadruplos::agregar(const std::string& operador, const std::string& izquierdo,
                                  const std::string& derecho, const std::string& resultado) {
    fila.push_back(Cuadruplo{ operador, izquierdo, derecho, resultado });
}

// Los cuadruplos se numeran desde 1, y el vector desde 0
int GeneradorCuadruplos::siguiente() const {
    return static_cast<int>(fila.size()) + 1;
}

// Completa un salto pendiente: el cuadruplo "numero" va a saltar a "destino"
void GeneradorCuadruplos::rellenar(int numero, int destino) {
    fila[numero - 1].resultado = std::to_string(destino);
}

// true si el operador del tope de la pila es alguno de los que se piden
bool GeneradorCuadruplos::topeEs(const std::string& a, const std::string& b,
                                 const std::string& c) const {
    if (pilaOperadores.isEmpty()) return false;
    std::string tope = pilaOperadores.peek();
    return tope == a || tope == b || tope == c;
}

// Cada resultado intermedio se guarda en un temporal nuevo.
// Su direccion depende de su tipo: 5000... si es int, 6000... si es float.
std::string GeneradorCuadruplos::nuevoTemporal(Tipo tipo) {
    return std::to_string(memoria.nuevaDireccion(Segmento::TEMPORAL, tipo));
}

// Genera el cuadruplo de una operacion con dos operandos.
// Ejemplo: con + en el tope y los operandos a y b genera (+, a, b, t1).
void GeneradorCuadruplos::generarOperacion() {
    // 1. Saca el operador
    std::string operador = pilaOperadores.pop();

    // 2. Saca los operandos. El derecho sale primero porque entro al ultimo.
    std::string derecho = pilaOperandos.pop();
    Tipo tipoDerecho = pilaTipos.pop();
    std::string izquierdo = pilaOperandos.pop();
    Tipo tipoIzquierdo = pilaTipos.pop();

    // 3. El cubo semantico dice de que tipo es el resultado.
    //    Si un operando ya venia con error (por ejemplo, una variable no
    //    declarada), el resultado es error y no se reporta otra vez.
    Tipo tipoResultado = cubo::resultado(aOperador(operador), tipoIzquierdo, tipoDerecho);

    // 4. Guarda el cuadruplo y deja el temporal como operando para lo que sigue
    std::string temporal = nuevoTemporal(tipoResultado);
    agregar(operador, izquierdo, derecho, temporal);
    agregarOperando(temporal, tipoResultado);
}

// ---------- Puntos neuralgicos: expresiones ----------

// PN11
void GeneradorCuadruplos::agregarVariable(const VarInfo* variable) {
    if (variable == nullptr) {
        agregarOperando(PENDIENTE, Tipo::ERROR);   // no existe: ya se reporto el error
        return;
    }
    agregarOperando(std::to_string(variable->direccion), variable->tipo);
}

// PN11
void GeneradorCuadruplos::agregarConstante(const std::string& texto, Tipo tipo) {
    agregarOperando(std::to_string(memoria.direccionConstante(texto, tipo)), tipo);
}

// PN12
void GeneradorCuadruplos::agregarOperador(const std::string& simbolo) {
    pilaOperadores.push(simbolo);
}

// PN13
void GeneradorCuadruplos::resolverSumaResta() {
    if (topeEs("+", "-")) generarOperacion();
}

// PN14
void GeneradorCuadruplos::resolverMultDiv() {
    if (topeEs("*", "/")) generarOperacion();
}

// PN15
void GeneradorCuadruplos::resolverRelacional() {
    if (topeEs(">", "<", "!=")) generarOperacion();
}

// PN16
void GeneradorCuadruplos::abrirParentesis() {
    pilaOperadores.push(FONDO_FALSO);
}

// PN17
void GeneradorCuadruplos::cerrarParentesis() {
    pilaOperadores.pop();
}

// PN18  "-x" se traduce como "0 - x": genera (-, 0, x, t1)
void GeneradorCuadruplos::generarNegativo() {
    std::string operando = pilaOperandos.pop();
    Tipo tipo = pilaTipos.pop();
    Tipo tipoResultado = cubo::resultadoUnario(Operador::RESTA, tipo);

    std::string cero = std::to_string(memoria.direccionConstante("0", Tipo::ENTERO));
    std::string temporal = nuevoTemporal(tipoResultado);
    agregar("-", cero, operando, temporal);
    agregarOperando(temporal, tipoResultado);
}

// ---------- Puntos neuralgicos: estatutos lineales ----------

// PN19  En las pilas estan: la variable (abajo), el "=" y el resultado de la expresion
void GeneradorCuadruplos::generarAsignacion(const std::string& nombre, int linea) {
    std::string operador = pilaOperadores.pop();   // siempre es "="
    std::string valor = pilaOperandos.pop();
    Tipo tipoValor = pilaTipos.pop();
    std::string variable = pilaOperandos.pop();
    Tipo tipoVariable = pilaTipos.pop();

    if (noSePuedeAsignar(tipoVariable, tipoValor)) {
        semantica.error(linea, std::string("no se puede asignar un valor ") + nombreTipo(tipoValor) +
                               " a '" + nombre + "', que es " + nombreTipo(tipoVariable));
    }
    agregar(operador, valor, "", variable);
}

// PN20
void GeneradorCuadruplos::generarPrint() {
    std::string valor = pilaOperandos.pop();
    pilaTipos.pop();
    agregar("print", "", "", valor);
}

// PN21  El letrero no pasa por las pilas porque no es parte de una expresion
void GeneradorCuadruplos::generarPrintLetrero(const std::string& letrero) {
    agregar("print", "", "", std::to_string(memoria.direccionLetrero(letrero)));
}

// PN22
void GeneradorCuadruplos::generarRead(const VarInfo* variable) {
    if (variable == nullptr) return;   // no existe: ya se reporto el error
    agregar("read", "", "", std::to_string(variable->direccion));
}

// ---------- Puntos neuralgicos: programa ----------

// PN23  Las funciones se escriben antes que main, pero el programa empieza
//       en main. Por eso el primer cuadruplo es un salto a main. Todavia no
//       se sabe en que cuadruplo empieza main, asi que queda pendiente.
void GeneradorCuadruplos::iniciarPrograma() {
    agregar("goto", "", "", PENDIENTE);
}

// PN24  Aqui empieza main: se rellena el goto del cuadruplo 1
void GeneradorCuadruplos::iniciarMain() {
    rellenar(1, siguiente());
}

// PN25
void GeneradorCuadruplos::terminarPrograma() {
    agregar("end", "", "", "");
}

// ---------- Puntos neuralgicos: if y while ----------
//
// if (c) A else B;            while (c) do A;
//       c                       inicio: c
//       gotof c -> sino                 gotof c -> fin
//       A                               A
//       goto -> fin                     goto -> inicio
// sino: B                        fin:
// fin:
//
// Cuando se genera un salto hacia adelante todavia no se sabe a donde va.
// Su numero se guarda en la pila de saltos y se rellena despues.

// PN26  La condicion es el operando del tope. Se usa en el if y en el while.
void GeneradorCuadruplos::generarGotoFalso(int linea) {
    std::string condicion = pilaOperandos.pop();
    Tipo tipo = pilaTipos.pop();

    // Una comparacion (a > b) da un int: 1 si es verdadera y 0 si es falsa
    if (tipo == Tipo::FLOTANTE) {
        semantica.error(linea, "la condicion debe ser int (una comparacion), no float");
    }
    pilaSaltos.push(siguiente());   // numero de este gotof, para rellenarlo despues
    agregar("gotof", condicion, "", PENDIENTE);
}

// PN27  Termino la parte verdadera del if y sigue el else
void GeneradorCuadruplos::generarElse() {
    int gotoFalso = pilaSaltos.pop();
    pilaSaltos.push(siguiente());       // este goto se rellena al final del if
    agregar("goto", "", "", PENDIENTE); // la parte verdadera se brinca el else
    rellenar(gotoFalso, siguiente());   // si la condicion es falsa, se llega al else
}

// PN28  El salto pendiente (el gotof, o el goto si hubo else) llega aqui
void GeneradorCuadruplos::terminarIf() {
    rellenar(pilaSaltos.pop(), siguiente());
}

// PN29  La condicion del while se vuelve a evaluar en cada vuelta,
//       asi que se guarda en que cuadruplo empieza
void GeneradorCuadruplos::iniciarWhile() {
    pilaSaltos.push(siguiente());
}

// PN30  En la pila de saltos estan: el inicio (abajo) y el gotof (arriba)
void GeneradorCuadruplos::terminarWhile() {
    int gotoFalso = pilaSaltos.pop();
    int inicio = pilaSaltos.pop();
    agregar("goto", "", "", std::to_string(inicio));   // regresa a evaluar la condicion
    rellenar(gotoFalso, siguiente());                  // si es falsa, sale del ciclo
}

// ---------- Puntos neuralgicos: funciones ----------
//
// Una llamada  f(a, b + 1)  se traduce a:
//     era    _  _  f          aparta la memoria que necesita f
//     param  a  _  p1         copia cada argumento a su parametro
//     +      b  1  t1
//     param  t1 _  p2
//     gosub  f  _  inicio     salta al cuadruplo donde empieza f
//     =      f  _  t2         (solo si f regresa un valor y se usa)

// PN32
void GeneradorCuadruplos::terminarFuncion() {
    agregar("endfunc", "", "", "");
}

// PN33  Si la funcion no existe (nullptr) ya se reporto el error, pero la
//       llamada se registra igual para poder leer sus argumentos.
void GeneradorCuadruplos::iniciarLlamada(const FuncInfo* funcion) {
    if (funcion != nullptr) agregar("era", "", "", funcion->nombre);
    pilaLlamadas.push(Llamada{ funcion, 0 });
    // Fondo falso: los argumentos no deben tocar los operadores de afuera
    pilaOperadores.push(FONDO_FALSO);
}

// PN34  El argumento es el operando del tope
void GeneradorCuadruplos::agregarArgumento(int linea) {
    std::string argumento = pilaOperandos.pop();
    Tipo tipoArgumento = pilaTipos.pop();

    // Se cuenta el argumento en la llamada actual
    Llamada llamada = pilaLlamadas.pop();
    int posicion = llamada.numArgumentos;   // 0 es el primer argumento
    llamada.numArgumentos++;
    pilaLlamadas.push(llamada);

    // Si la funcion no existe o sobran argumentos, no hay parametro que llenar
    // (el error de la cantidad se reporta en terminarLlamada)
    const FuncInfo* funcion = llamada.funcion;
    if (funcion == nullptr || posicion >= static_cast<int>(funcion->parametros.size())) return;

    // El argumento se guarda en el parametro, asi que es como una asignacion
    Tipo tipoParametro = funcion->parametros[posicion];
    if (noSePuedeAsignar(tipoParametro, tipoArgumento)) {
        semantica.error(linea, "argumento " + std::to_string(posicion + 1) + " de '" +
                               funcion->nombre + "': no se puede pasar un valor " +
                               nombreTipo(tipoArgumento) + " a un parametro " +
                               nombreTipo(tipoParametro));
    }
    agregar("param", argumento, "", std::to_string(funcion->dirParametros[posicion]));
}

// PN35
void GeneradorCuadruplos::terminarLlamada(const std::string& nombre, int linea) {
    pilaOperadores.pop();   // quita el fondo falso
    Llamada llamada = pilaLlamadas.pop();
    const FuncInfo* funcion = llamada.funcion;
    if (funcion == nullptr) return;

    int esperados = static_cast<int>(funcion->parametros.size());
    if (llamada.numArgumentos != esperados) {
        semantica.error(linea, "la funcion '" + nombre + "' espera " + std::to_string(esperados) +
                               " argumento(s) y recibio " + std::to_string(llamada.numArgumentos));
    }
    agregar("gosub", funcion->nombre, "", std::to_string(funcion->inicio));
}

// PN36  La funcion dejo su valor en su direccion global (dirRetorno).
//       Se copia a un temporal para que otra llamada a la misma funcion
//       no lo borre: en f(1) + f(2) hay dos valores distintos.
void GeneradorCuadruplos::agregarResultadoLlamada(const FuncInfo* funcion) {
    if (funcion == nullptr) {
        agregarOperando(PENDIENTE, Tipo::ERROR);   // no regresa valor: ya se reporto el error
        return;
    }
    std::string temporal = nuevoTemporal(funcion->tipoRetorno);
    agregar("=", std::to_string(funcion->dirRetorno), "", temporal);
    agregarOperando(temporal, funcion->tipoRetorno);
}

// PN37  El valor que se regresa es el operando del tope
void GeneradorCuadruplos::generarReturn(const FuncInfo* funcion, int linea) {
    std::string valor = pilaOperandos.pop();
    Tipo tipoValor = pilaTipos.pop();
    if (funcion == nullptr) return;   // return donde no se puede: ya se reporto el error

    // Regresar un valor es guardarlo en la direccion de la funcion
    if (noSePuedeAsignar(funcion->tipoRetorno, tipoValor)) {
        semantica.error(linea, std::string("la funcion '") + funcion->nombre + "' es " +
                               nombreTipo(funcion->tipoRetorno) + " y no puede regresar un valor " +
                               nombreTipo(tipoValor));
    }
    agregar("return", valor, "", std::to_string(funcion->dirRetorno));
}

// ---------- Despliegue ----------

// Una parte vacia del cuadruplo se muestra como "_"
static const char* parte(const std::string& texto) {
    return texto.empty() ? "_" : texto.c_str();
}

void GeneradorCuadruplos::imprimir() const {
    printf("Fila de cuadruplos\n");
    printf("  %-4s %-9s %-12s %-12s %s\n", "#", "operador", "izquierdo", "derecho", "resultado");
    int numero = 1;
    for (const Cuadruplo& c : fila) {
        printf("  %-4d %-9s %-12s %-12s %s\n", numero, c.operador.c_str(),
               parte(c.izquierdo), parte(c.derecho), parte(c.resultado));
        numero++;
    }
}
