# BabyDuck – Compilador (Etapa 1: Scanner y Parser)

Danny Wu · A01286362 · TC3002B Compiladores

## Herramientas

- **Flex 2.6.4**: genera el scanner a partir de `src/lexer.l`.
- **GNU Bison 2.3** (parser LALR(1)): genera el parser a partir de `src/parser.y`.
- **g++ con C++17**: compila el código generado y `src/main.cpp`.

Flex y Bison ya vienen con las Command Line Tools de Xcode en macOS. No hay que instalar nada extra.

## Estructura

```
compilador/
├── src/
│   ├── lexer.l       # expresiones regulares (Flex)
│   ├── parser.y      # gramática / CFG (Bison)
│   └── main.cpp      # programa principal
├── tests/
│   ├── tokens/       # pruebas del scanner (.bd + .esperado)
│   ├── validos/      # programas que deben aceptarse
│   ├── sintaxis/     # programas con error de sintaxis
│   ├── lexico/       # programas con error léxico
│   └── run_tests.sh  # corre todo el test-plan
├── Makefile
└── .gitignore
```

## Uso

```bash
make                       # genera parser/scanner y compila ./babyduck
./babyduck archivo.bd      # analiza el programa
./babyduck --tokens a.bd   # solo muestra los tokens (línea, token, lexema)
make test                  # corre los 147 test-cases
make clean                 # borra lo generado
```

Códigos de salida:

- `0`: programa válido.
- `1`: hubo un error léxico o sintáctico.
- `2`: uso incorrecto o el archivo no existe.

La documentación completa está en `../Documentacion_BabyDuck.md`.
