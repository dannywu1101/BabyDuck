#include "cuadruplos.hpp"
#include <cstdio>
#include "cubo_semantico.hpp"
#include "semantica.hpp"

// Marca que se mete a la pila de operadores al abrir un parentesis
static const char* FONDO_FALSO = "(";

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

// ---------- Ayudas internas ----------

// true si el operador del tope de la pila es alguno de los que se piden
bool GeneradorCuadruplos::topeEs(const std::string& a, const std::string& b,
                                 const std::string& c) const {
    if (pilaOperadores.isEmpty()) return false;
    std::string tope = pilaOperadores.peek();
    return tope == a || tope == b || tope == c;
}

// Cada resultado intermedio se guarda en un temporal nuevo: t1, t2, t3, ...
std::string GeneradorCuadruplos::nuevoTemporal() {
    numTemporales++;
    return "t" + std::to_string(numTemporales);
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
    std::string temporal = nuevoTemporal();
    fila.enqueue(Cuadruplo{ operador, izquierdo, derecho, temporal });
    pilaOperandos.push(temporal);
    pilaTipos.push(tipoResultado);
}

// ---------- Puntos neuralgicos: expresiones ----------

// PN11
void GeneradorCuadruplos::agregarOperando(const std::string& nombre, Tipo tipo) {
    pilaOperandos.push(nombre);
    pilaTipos.push(tipo);
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

    std::string temporal = nuevoTemporal();
    fila.enqueue(Cuadruplo{ "-", "0", operando, temporal });
    pilaOperandos.push(temporal);
    pilaTipos.push(cubo::resultadoUnario(Operador::RESTA, tipo));
}

// ---------- Puntos neuralgicos: estatutos lineales ----------

// PN19  En las pilas estan: la variable (abajo), el "=" y el resultado de la expresion
void GeneradorCuadruplos::generarAsignacion(int linea) {
    std::string operador = pilaOperadores.pop();   // siempre es "="
    std::string valor = pilaOperandos.pop();
    Tipo tipoValor = pilaTipos.pop();
    std::string variable = pilaOperandos.pop();
    Tipo tipoVariable = pilaTipos.pop();

    // El cubo valida la asignacion: int = float no se permite.
    // Si alguno ya venia con error o es void, ese error ya se reporto antes.
    bool ambosNumericos = (tipoVariable == Tipo::ENTERO || tipoVariable == Tipo::FLOTANTE) &&
                          (tipoValor == Tipo::ENTERO || tipoValor == Tipo::FLOTANTE);
    if (ambosNumericos && cubo::resultado(Operador::ASIGNA, tipoVariable, tipoValor) == Tipo::ERROR) {
        semantica.error(linea, std::string("no se puede asignar un valor ") + nombreTipo(tipoValor) +
                               " a '" + variable + "', que es " + nombreTipo(tipoVariable));
    }

    fila.enqueue(Cuadruplo{ operador, valor, "", variable });
}

// PN20
void GeneradorCuadruplos::generarPrint() {
    std::string valor = pilaOperandos.pop();
    pilaTipos.pop();
    fila.enqueue(Cuadruplo{ "print", "", "", valor });
}

// PN21  El letrero no pasa por las pilas porque no es parte de una expresion
void GeneradorCuadruplos::generarPrintLetrero(const std::string& letrero) {
    fila.enqueue(Cuadruplo{ "print", "", "", letrero });
}

// PN22
void GeneradorCuadruplos::generarRead(const std::string& nombre) {
    fila.enqueue(Cuadruplo{ "read", "", "", nombre });
}

// ---------- Pendiente para la Etapa 4 ----------

void GeneradorCuadruplos::descartarOperando() {
    pilaOperandos.pop();
    pilaTipos.pop();
}

// ---------- Despliegue ----------

// Una parte vacia del cuadruplo se muestra como "_"
static const char* parte(const std::string& texto) {
    return texto.empty() ? "_" : texto.c_str();
}

void GeneradorCuadruplos::imprimir() const {
    printf("Fila de cuadruplos\n");
    if (fila.isEmpty()) {
        printf("  (sin cuadruplos)\n");
        return;
    }
    printf("  %-4s %-9s %-12s %-12s %s\n", "#", "operador", "izquierdo", "derecho", "resultado");

    // La fila solo deja sacar del frente, asi que se recorre una copia
    Queue<Cuadruplo> copia = fila;
    int numero = 1;
    while (!copia.isEmpty()) {
        Cuadruplo c = copia.dequeue();
        printf("  %-4d %-9s %-12s %-12s %s\n", numero, c.operador.c_str(),
               parte(c.izquierdo), parte(c.derecho), parte(c.resultado));
        numero++;
    }
}
