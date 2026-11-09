// ============================================================
// BabyDuck - Analizador semantico (Etapa 2)
// Las acciones del parser (parser.y) llaman a estos metodos en los
// puntos neuralgicos (PN) para crear y llenar el Directorio de
// Funciones y las Tablas de Variables, y para validar los usos.
// ============================================================
#ifndef SEMANTICA_HPP
#define SEMANTICA_HPP

#include <string>
#include "Queue.h"
#include "directorio_funciones.hpp"

class Semantica {
public:
    // ----- Declaraciones -----
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

    // ----- Usos -----
    // PN7  id en una asignacion, read o expresion: debe ser una variable visible
    void usarVariable(const std::string& nombre, int linea);
    // PN8  id ( args )       -> debe ser una funcion declarada
    void llamarFuncion(const std::string& nombre, int linea);
    // PN9  llamada dentro de una expresion: la funcion no puede ser void
    void usarLlamadaComoValor(const std::string& nombre, int linea);
    // PN10 return            -> solo en funciones que regresan int o float
    void validarReturn(int linea);

    int errores() const { return numErrores; }
    const DirectorioFunciones& directorio() const { return directorio_; }

private:
    struct IdPendiente {
        std::string nombre;
        int linea;
    };

    DirectorioFunciones directorio_;
    Queue<IdPendiente> pendientes;
    std::string nombrePrograma;
    std::string nombreAmbito;      // programa (global/main) o funcion actual
    // Una funcion doblemente declarada no entra al directorio, pero su
    // cuerpo se sigue revisando en este ambito aparte.
    FuncInfo descartada;
    bool enDescartada = false;
    int numErrores = 0;

    FuncInfo& global();
    FuncInfo& ambitoActual();
    bool enFuncion() const;
    bool declarar(const VarInfo& variable);
    bool yaExisteEnGlobal(const std::string& nombre, int linea);
    void reportarDuplicado(const std::string& nombre, int linea,
                           const char* comoQue, int lineaPrevia);
    void error(int linea, const std::string& mensaje);
};

// Instancia unica que usan el parser y main (definida en parser.y)
extern Semantica semantica;

#endif
