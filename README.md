# BabyDuck – Compilador (Etapa 2: Semántica)

Danny Wu · A01286362 · TC3002B Compiladores

## Herramientas

- **Flex 2.6.4**: genera el scanner a partir de `src/lexer.l`.
- **GNU Bison 2.3** (parser LALR(1)): genera el parser a partir de `src/parser.y`.
- **g++ con C++17**: compila el código generado y el código propio de `src/`.
- **HashTable y Queue** de la actividad previa (`include/`): base del Directorio de Funciones, las Tablas de Variables y la fila de ids pendientes.

Flex y Bison ya vienen con las Command Line Tools de Xcode en macOS. No hay que instalar nada extra.

## Estructura

```
compilador/
├── include/
│   ├── HashTable.h              # actividad previa (tabla hash)
│   └── Queue.h                  # actividad previa (fila)
├── src/
│   ├── lexer.l                  # expresiones regulares (Flex)
│   ├── parser.y                 # gramática + puntos neurálgicos (Bison)
│   ├── semantica.*              # acciones de los puntos neurálgicos y validaciones
│   ├── directorio_funciones.*   # Directorio de Funciones
│   ├── tabla_variables.*        # Tabla de Variables
│   ├── cubo_semantico.*         # cubo semántico
│   ├── tipos.hpp                # tipos de dato (int, float, void)
│   └── main.cpp                 # programa principal
├── tests/
│   ├── tokens/       # pruebas del scanner (.bd + .esperado)
│   ├── validos/      # programas que deben aceptarse
│   ├── sintaxis/     # programas con error de sintaxis
│   ├── lexico/       # programas con error léxico
│   ├── semantica/    # programas con error semántico (.bd + .esperado)
│   ├── tablas/       # salida de --tablas (.bd + .esperado)
│   ├── cubo/         # salida de --cubo
│   └── run_tests.sh  # corre todo el test-plan
├── Makefile
└── .gitignore
```

## Uso

```bash
make                         # genera parser/scanner y compila ./babyduck
./babyduck archivo.bd        # análisis léxico, sintáctico y semántico
./babyduck --tokens a.bd     # solo muestra los tokens (línea, token, lexema)
./babyduck --tablas a.bd     # muestra el Directorio de Funciones y las Tablas de Variables
./babyduck --cubo            # muestra el cubo semántico
make test                    # corre los 193 test-cases
make clean                   # borra lo generado
```

Códigos de salida:

- `0`: programa válido.
- `1`: hubo un error léxico, sintáctico o semántico.
- `2`: uso incorrecto o el archivo no existe.

La documentación completa está en `../Documentacion_BabyDuck.md`.
