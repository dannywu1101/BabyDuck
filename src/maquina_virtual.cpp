#include "maquina_virtual.hpp"
#include <cstdio>
#include <cstring>

// Las partes del cuadruplo son texto: "1000" -> 1000
static int numero(const std::string& texto) {
    return std::stoi(texto);
}

MaquinaVirtual::MaquinaVirtual(const std::vector<Cuadruplo>& fila, const Semantica& semantica,
                               const Memoria& compilacion)
    : fila(fila), directorio(semantica.directorio()) {
    memoria.iniciar(*semantica.programa(), compilacion);
}

// El ciclo principal. Los cuadruplos se numeran desde 1 y el vector desde 0.
void MaquinaVirtual::ejecutar() {
    while (true) {
        const Cuadruplo& c = fila[actual - 1];
        const std::string& op = c.operador;

        if (op == "end") {
            return;
        } else if (op == "goto") {
            actual = numero(c.resultado);
        } else if (op == "gotof") {
            // La condicion vale 0 cuando es falsa
            bool falsa = memoria.leer(numero(c.izquierdo)) == 0;
            actual = falsa ? numero(c.resultado) : actual + 1;
        } else if (op == "era") {
            memoria.apartarMarco(*directorio.buscar(c.resultado));
            actual++;
        } else if (op == "param") {
            // El argumento se lee del marco actual y se guarda en el nuevo
            memoria.escribirParametro(numero(c.resultado), memoria.leer(numero(c.izquierdo)));
            actual++;
        } else if (op == "gosub") {
            // Al terminar la funcion se regresa al cuadruplo que sigue
            memoria.activarMarco(actual + 1);
            actual = numero(c.resultado);
        } else if (op == "return") {
            // El valor se deja en la direccion global de la funcion
            memoria.escribir(numero(c.resultado), memoria.leer(numero(c.izquierdo)));
            regresar();
        } else if (op == "endfunc") {
            regresar();
        } else if (op == "print") {
            imprimir(c);
            actual++;
        } else if (op == "read") {
            leer(c);
            actual++;
        } else if (op == "=") {
            memoria.escribir(numero(c.resultado), memoria.leer(numero(c.izquierdo)));
            actual++;
        } else {
            operar(c);   // + - * / > < !=
            actual++;
        }
    }
}

// Operaciones con dos operandos: (op, izquierdo, derecho, resultado)
void MaquinaVirtual::operar(const Cuadruplo& c) {
    double a = memoria.leer(numero(c.izquierdo));
    double b = memoria.leer(numero(c.derecho));
    const std::string& op = c.operador;
    double resultado = 0;

    if (op == "+")       resultado = a + b;
    else if (op == "-")  resultado = a - b;
    else if (op == "*")  resultado = a * b;
    else if (op == "/") {
        if (b == 0) errorDeEjecucion("division entre cero");
        resultado = a / b;
    }
    else if (op == ">")  resultado = a > b ? 1 : 0;
    else if (op == "<")  resultado = a < b ? 1 : 0;
    else if (op == "!=") resultado = a != b ? 1 : 0;
    else errorDeEjecucion("codigo de operacion desconocido: " + op);

    // Si el resultado es int, escribir le quita los decimales (7 / 2 = 3)
    memoria.escribir(numero(c.resultado), resultado);
}

// print: cada valor o letrero sale en su propio renglon
void MaquinaVirtual::imprimir(const Cuadruplo& c) {
    int direccion = numero(c.resultado);
    if (MemoriaEjecucion::esLetrero(direccion)) {
        printf("%s\n", memoria.letrero(direccion).c_str());
    } else if (MemoriaEjecucion::esEntera(direccion)) {
        printf("%d\n", static_cast<int>(memoria.leer(direccion)));
    } else {
        // Un float siempre se muestra con punto decimal: 2.0 y no 2
        char texto[64];
        snprintf(texto, sizeof(texto), "%g", memoria.leer(direccion));
        bool tienePunto = strpbrk(texto, ".en") != nullptr;   // 1.5, 1e+20, inf, nan
        printf(tienePunto ? "%s\n" : "%s.0\n", texto);
    }
}

// read: lee un numero de la entrada y lo guarda en la variable
void MaquinaVirtual::leer(const Cuadruplo& c) {
    fflush(stdout);   // que se vea lo impreso antes de esperar la entrada
    double valor = 0;
    if (scanf("%lf", &valor) != 1) {
        errorDeEjecucion("read esperaba un numero");
    }
    memoria.escribir(numero(c.resultado), valor);
}

// Fin de una funcion (return o endfunc): se quita su marco y se continua
// en el cuadruplo que sigue a la llamada
void MaquinaVirtual::regresar() {
    actual = memoria.quitarMarco();
}
