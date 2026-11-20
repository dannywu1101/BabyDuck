# BabyDuck - Etapa 4 (Flex + Bison + C++)
#
#   make        genera el parser y el scanner, y compila ./babyduck
#   make test   corre todo el test-plan
#   make clean  borra lo generado

# Compilador y opciones: C++17, con avisos, y las carpetas donde buscar los .h
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-function -Wno-sign-compare -Iinclude -Isrc -Ibuild
BUILD    = build
TARGET   = babyduck

# Codigo propio (el parser y el scanner los generan Bison y Flex)
SOURCES  = src/main.cpp src/semantica.cpp src/directorio_funciones.cpp \
           src/tabla_variables.cpp src/cubo_semantico.cpp src/cuadruplos.cpp src/memoria.cpp
# Si cambia cualquier header, se vuelve a compilar
HEADERS  = $(wildcard src/*.hpp include/*.h)

all: $(TARGET)

# Bison genera el parser (.cpp) y el header con los tokens (.hpp).
# -v genera parser.output: ahi se revisa que haya 0 conflictos.
$(BUILD)/parser.tab.cpp $(BUILD)/parser.tab.hpp: src/parser.y
	@mkdir -p $(BUILD)
	bison -d -v -o $(BUILD)/parser.tab.cpp src/parser.y

# Flex genera el scanner (.cpp)
$(BUILD)/lexer.yy.cpp: src/lexer.l $(BUILD)/parser.tab.hpp
	flex -o $(BUILD)/lexer.yy.cpp src/lexer.l

# Compila todo junto en el ejecutable ./babyduck
$(TARGET): $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp $(SOURCES)

# Corre las pruebas (primero compila si hace falta)
test: $(TARGET)
	@bash tests/run_tests.sh

# Borra la carpeta build y el ejecutable
clean:
	rm -rf $(BUILD) $(TARGET)

.PHONY: all test clean
