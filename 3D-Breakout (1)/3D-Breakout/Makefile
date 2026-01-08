# Nome do executável
TARGET = 3D_Breakout.exe

# Compilador e Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude
# Bibliotecas para Windows (MinGW)
LIBS = -lglfw3 -lglew32 -lopengl32 -lgdi32

# Pastas
SRC_DIR = src
OBJ_DIR = build

# Listar todos os ficheiros .cpp na pasta src (incluindo ImGui)
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
# Gerar nomes para os ficheiros .o dentro da pasta build
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

# Regra principal
all: prepare $(TARGET)

# Criar a pasta build se não existir
prepare:
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

# Linkagem final
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

# Compilação de cada ficheiro .cpp individualmente
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpar ficheiros temporários
clean:
	del /q $(OBJ_DIR)\*.o $(TARGET)

# Atalho para compilar e correr
run: all
	./$(TARGET)