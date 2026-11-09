# BabyDuck - Etapa 2 (Flex + Bison + C++)
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-function -Wno-sign-compare -Iinclude -Isrc -Ibuild
BUILD    = build
TARGET   = babyduck

# Codigo propio (el parser y el scanner los generan Bison y Flex)
SOURCES  = src/main.cpp src/semantica.cpp src/directorio_funciones.cpp \
           src/tabla_variables.cpp src/cubo_semantico.cpp
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

$(TARGET): $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp $(SOURCES)

test: $(TARGET)
	@bash tests/run_tests.sh

clean:
	rm -rf $(BUILD) $(TARGET)

.PHONY: all test clean
