// ============================================================
// BabyDuck - Analizador semantico (Etapas 2 y 4)
//
// El parser solo revisa que el programa este bien escrito. Esta
// clase revisa que tenga sentido: que no se declare un nombre dos
// veces, que no se use una variable que no existe, etc.
//
// Las acciones del parser (parser.y) llaman a estos metodos en los
// puntos neuralgicos PN1 a PN10. Con ellos se crea y se llena el
// Directorio de Funciones y las Tablas de Variables.
//
// Etapa 4: al declarar una variable se le da su direccion virtual, y
// de cada funcion se guarda donde empieza y cuanta memoria ocupa.
// ============================================================
#ifndef SEMANTICA_HPP
#define SEMANTICA_HPP

#include <string>
#include "Queue.h"
#include "directorio_funciones.hpp"
#include "memoria.hpp"

class Semantica {
public:
    // ----- Declaraciones (crean y llenan las tablas) -----
    // PN1  program id ;      -> crea la entrada del programa (ambito global)
    void iniciarPrograma(const std::string& nombre, int linea);
    // PN2  cada id de "var a, b : tipo;" espera su tipo en una fila
    void agregarIdPendiente(const std::string& nombre, int linea);
    // PN3  ... : tipo ;      -> declara todos los ids pendientes con ese tipo
    void declararVariables(Tipo tipo);
    // PN4  tipo id (         -> registra la funcion y abre su ambito local
    void iniciarFuncion(Tipo tipoRetorno, const std::string& nombre, int linea);
    // PN5  id : tipo         -> agrega un parametro a la tabla y a la firma
    void agregarParametro(const std::string& nombre, Tipo tipo, int linea);
    // PN6  ] ;               -> cierra la funcion y regresa al ambito global
    void terminarFuncion();

    // ----- Etapa 4: datos de cada funcion para poder llamarla -----
    // PN31 antes del cuerpo de una funcion: guarda en que cuadruplo empieza
    void fijarInicioFuncion(int cuadruplo);
    // PN24 al leer "main": guarda donde empieza main y reinicia los temporales
    void iniciarMain(int cuadruplo);
    // PN25 al leer "end": guarda cuanta memoria ocupa el programa
    void terminarPrograma();

    // ----- Usos (validan contra las tablas) -----
    // Los cuatro regresan un puntero a la tabla, o nullptr si hubo error.
    // El puntero solo se usa en ese momento (no se guarda para despues).
    // PN7  id en una asignacion, read o expresion: debe ser una variable visible.
    //      Regresa la variable (con su tipo y su direccion).
    const VarInfo* usarVariable(const std::string& nombre, int linea);
    // PN8  id ( args )       -> debe ser una funcion declarada. Regresa la funcion.
    const FuncInfo* llamarFuncion(const std::string& nombre, int linea);
    // PN9  llamada dentro de una expresion: la funcion no puede ser void.
    //      Regresa la funcion.
    const FuncInfo* usarLlamadaComoValor(const std::string& nombre, int linea);
    // PN10 return            -> solo en funciones que regresan int o float.
    //      Regresa la funcion en la que estamos.
    const FuncInfo* validarReturn(int linea);

    // Imprime "Error semantico (linea N): mensaje" y lo cuenta.
    // Tambien lo usa el generador de cuadruplos (errores de tipos).
    void error(int linea, const std::string& mensaje);

    // Cuantos errores semanticos se han reportado
    int errores() const { return numErrores; }

    // Acceso de solo lectura al directorio (para imprimirlo con --tablas)
    const DirectorioFunciones& directorio() const { return directorio_; }

private:
    // Un id de "var a, b : int;" que todavia no sabe su tipo
    struct IdPendiente {
        std::string nombre;
        int linea;
    };

    DirectorioFunciones directorio_;   // todas las funciones y sus tablas de variables
    Queue<IdPendiente> pendientes;     // ids que esperan su tipo (PN2 -> PN3)
    std::string nombrePrograma;        // nombre del programa: su tabla guarda las globales
    std::string nombreAmbito;          // donde estamos: el programa (global/main) o una funcion
    // Una funcion doblemente declarada no entra al directorio, pero su
    // cuerpo se sigue revisando en este ambito aparte.
    FuncInfo descartada;
    bool enDescartada = false;
    int numErrores = 0;

    // Ayudas internas (explicadas en semantica.cpp)
    FuncInfo& global();
    FuncInfo& ambitoActual();
    bool enFuncion() const;
    bool declarar(const VarInfo& variable);
    void guardarMemoria(FuncInfo& funcion, Segmento variables);
    bool yaExisteEnGlobal(const std::string& nombre, int linea);
    void reportarDuplicado(const std::string& nombre, int linea,
                           const char* comoQue, int lineaPrevia);
};

// Instancia unica que usan el parser y main (definida en parser.y)
extern Semantica semantica;

#endif
