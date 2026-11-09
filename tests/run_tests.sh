#!/bin/bash
# ============================================================
# Test-plan de BabyDuck (Etapas 1 y 2)
#   tests/tokens/    -> salida de --tokens igual a *.esperado
#   tests/validos/   -> el programa debe aceptarse (codigo 0)
#   tests/sintaxis/  -> debe fallar con "Error sintactico"
#   tests/lexico/    -> debe fallar con "Error lexico"
#   tests/semantica/ -> codigo 1 y mensajes iguales a *.esperado
#   tests/tablas/    -> salida de --tablas igual a *.esperado
#   tests/cubo/      -> salida de --cubo igual a cubo.esperado
#   Uso del programa -> argumentos invalidos dan codigo 2
# ============================================================
cd "$(dirname "$0")"
BIN=../babyduck
pass=0
fail=0

ok()   { echo "  [OK]    $1"; pass=$((pass + 1)); }
bad()  { echo "  [FALLA] $1  ($2)"; fail=$((fail + 1)); }

# Compara la salida de "babyduck <opcion> archivo" contra archivo.esperado
expect_output() {
    local dir=$1 option=$2
    for f in "$dir"/*.bd; do
        name=$(basename "$f" .bd)
        if diff <($BIN $option "$f" 2>&1) "$dir/$name.esperado" > /dev/null; then
            ok "$name"
        else
            bad "$name" "salida distinta a lo esperado"
        fi
    done
}

echo "== Scanner: tokens =="
expect_output tokens --tokens

echo "== Programas validos =="
for f in validos/*.bd; do
    name=$(basename "$f" .bd)
    out=$($BIN "$f" 2>&1)
    if [ $? -eq 0 ]; then ok "$name"; else bad "$name" "$out"; fi
done

# Casos que deben fallar: revisa codigo 1 y el tipo de error
expect_error() {
    local dir=$1 kind=$2 forbidden=$3
    for f in "$dir"/*.bd; do
        name=$(basename "$f" .bd)
        out=$($BIN "$f" 2>&1)
        code=$?
        if [ $code -ne 1 ] || ! echo "$out" | grep -q "$kind"; then
            bad "$name" "se esperaba '$kind', salio codigo $code"
        elif [ -n "$forbidden" ] && echo "$out" | grep -q "$forbidden"; then
            bad "$name" "no debe tener '$forbidden'"
        else
            ok "$name -> $(echo "$out" | grep "$kind" | head -1)"
        fi
    done
}

echo "== Errores de sintaxis =="
expect_error sintaxis "Error sintactico" "Error lexico"

echo "== Errores lexicos =="
expect_error lexico "Error lexico"

# Errores semanticos: codigo 1 y los mensajes exactos (texto y linea).
# Los .esperado no tienen errores lexicos ni sintacticos.
echo "== Errores semanticos =="
for f in semantica/*.bd; do
    name=$(basename "$f" .bd)
    out=$($BIN "$f" 2>&1)
    code=$?
    if [ $code -ne 1 ]; then
        bad "$name" "se esperaba codigo 1, salio $code"
    elif [ "$out" != "$(cat "semantica/$name.esperado")" ]; then
        bad "$name" "mensajes distintos a lo esperado"
    else
        ok "$name -> $(echo "$out" | head -1)"
    fi
done

echo "== Directorio de Funciones y Tablas de Variables =="
expect_output tablas --tablas

echo "== Cubo semantico =="
if diff <($BIN --cubo 2>&1) cubo/cubo.esperado > /dev/null; then
    ok "cubo"
else
    bad "cubo" "salida distinta a lo esperado"
fi

# Uso incorrecto del programa: debe terminar con codigo 2
expect_usage() {
    local name=$1
    shift
    $BIN "$@" > /dev/null 2>&1
    local code=$?
    if [ $code -eq 2 ]; then ok "$name"; else bad "$name" "salio codigo $code"; fi
}

echo "== Uso del programa =="
expect_usage "u01_sin_argumentos"
expect_usage "u02_archivo_inexistente" tests/no_existe.bd
expect_usage "u03_tokens_sin_archivo" --tokens
expect_usage "u04_opcion_invalida" --otra validos/v01_minimo.bd
expect_usage "u05_demasiados_argumentos" --tokens validos/v01_minimo.bd extra
expect_usage "u06_tablas_sin_archivo" --tablas
expect_usage "u07_cubo_con_archivo" --cubo validos/v01_minimo.bd

echo
echo "Resultado: $pass pasaron, $fail fallaron"
[ $fail -eq 0 ]
