# BabyDuck – Compilador (Etapa 5: Memoria de ejecución y máquina virtual)

Danny Wu · A01286362 · TC3002B Compiladores

## Herramientas

- **Flex 2.6.4**: genera el scanner a partir de `src/lexer.l`.
- **GNU Bison 2.3** (parser LALR(1)): genera el parser a partir de `src/parser.y`.
- **g++ con C++17**: compila el código generado y el código propio de `src/`.
- **HashTable, Queue y Stack** de la actividad previa (`include/`): base del Directorio de Funciones, las Tablas de Variables, las pilas de operandos, tipos, operadores, saltos y llamadas, y la tabla de constantes.

Flex y Bison ya vienen con las Command Line Tools de Xcode en macOS. No hay que instalar nada extra.

## Estructura

```
compilador/
├── include/
│   ├── HashTable.h              # actividad previa (tabla hash)
│   ├── Queue.h                  # actividad previa (fila)
│   └── Stack.h                  # actividad previa (pila)
├── src/
│   ├── lexer.l                  # expresiones regulares (Flex)
│   ├── parser.y                 # gramática + puntos neurálgicos (Bison)
│   ├── semantica.*              # acciones de los puntos neurálgicos y validaciones
│   ├── directorio_funciones.*   # Directorio de Funciones
│   ├── tabla_variables.*        # Tabla de Variables
│   ├── cuadruplos.*             # pilas, fila de cuádruplos y traducción (Etapas 3 y 4)
│   ├── memoria.*                # direcciones virtuales y tabla de constantes (Etapa 4)
│   ├── memoria_ejecucion.*      # memoria al ejecutar: globales, constantes y pila de marcos (Etapa 5)
│   ├── maquina_virtual.*        # ejecuta los cuádruplos (Etapa 5)
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
│   ├── cuadruplos/   # salida de --cuadruplos (.bd + .esperado)
│   ├── ejecucion/    # salida de --ejecutar (.bd + .esperado, y .entrada si usa read)
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
./babyduck --cuadruplos a.bd # muestra la fila de cuádruplos y la tabla de constantes
./babyduck --ejecutar a.bd   # compila y ejecuta el programa en la máquina virtual
./babyduck --cubo            # muestra el cubo semántico
make test                    # corre los 263 test-cases
make clean                   # borra lo generado
```

Códigos de salida:

- `0`: programa válido.
- `1`: hubo un error léxico, sintáctico o semántico.
- `2`: uso incorrecto o el archivo no existe.
- `3`: error de ejecución (división entre cero, `read` sin número, recursión sin fin).

## Direcciones virtuales

| Segmento | int | float | letrero |
|---|---|---|---|
| Globales | 1000 – 1999 | 2000 – 2999 | |
| Locales | 3000 – 3999 | 4000 – 4999 | |
| Temporales | 5000 – 5999 | 6000 – 6999 | |
| Constantes | 7000 – 7999 | 8000 – 8999 | 9000 – 9999 |

## Memoria de ejecución

- La dirección virtual es el índice: `bloque = (dirección - 1000) / 1000` dice el segmento y el tipo, y `posición = (dirección - 1000) % 1000` dice la casilla.
- Globales y constantes existen una sola vez.
- Locales y temporales viven en un **marco**. Cada llamada a una función tiene su marco, y los marcos se guardan en una pila.

La documentación completa está en `../Documentacion_BabyDuck.md`.
