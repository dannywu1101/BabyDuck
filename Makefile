# BabyDuck - Etapa 1 (Flex + Bison + C++)
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-unused-function -Wno-sign-compare -Ibuild
BUILD    = build
TARGET   = babyduck

all: $(TARGET)

# Bison genera el parser (.cpp) y el header con los tokens (.hpp).
# -v genera parser.output: ahi se revisa que haya 0 conflictos.
$(BUILD)/parser.tab.cpp $(BUILD)/parser.tab.hpp: src/parser.y
	@mkdir -p $(BUILD)
	bison -d -v -o $(BUILD)/parser.tab.cpp src/parser.y

# Flex genera el scanner (.cpp)
$(BUILD)/lexer.yy.cpp: src/lexer.l $(BUILD)/parser.tab.hpp
	flex -o $(BUILD)/lexer.yy.cpp src/lexer.l

$(TARGET): $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp src/main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(BUILD)/parser.tab.cpp $(BUILD)/lexer.yy.cpp src/main.cpp

test: $(TARGET)
	@bash tests/run_tests.sh

clean:
	rm -rf $(BUILD) $(TARGET)

.PHONY: all test clean
