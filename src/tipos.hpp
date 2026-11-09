// ============================================================
// BabyDuck - Tipos de dato
// ============================================================
#ifndef TIPOS_HPP
#define TIPOS_HPP

// Nombres en espanol porque Bison ya define INT, FLOAT y VOID como tokens.
// int y float se pueden declarar en variables y parametros.
// void solo aplica al tipo de regreso de una funcion.
// ERROR es el resultado del cubo semantico cuando una operacion no es valida.
enum class Tipo { ENTERO, FLOTANTE, VACIO, ERROR };

inline const char* nombreTipo(Tipo tipo) {
    switch (tipo) {
        case Tipo::ENTERO:   return "int";
        case Tipo::FLOTANTE: return "float";
        case Tipo::VACIO:    return "void";
        default:             return "error";
    }
}

#endif
