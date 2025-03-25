# MarioFanGame

Este é um jogo desenvolvido utilizando OpenGL, GLFW e GLEW. Abaixo estão as instruções para compilar e rodar o jogo em macOS e Linux.

## Requisitos

Antes de compilar, você precisa garantir que as bibliotecas necessárias estão instaladas:

- **GLEW** (para extensão OpenGL)
- **GLFW** (para gerenciamento de janela e entrada)
- **OpenGL** (para gráficos)
- **C++11** (ou superior)

### macOS
1. **Instale o GLEW e o GLFW:**
 ```bash
brew install glew glfw
```

2. **Compilação:**
```bash
g++ -std=c++11 -Wall -Wextra -g \
    main.cpp Shader.cpp Geometry.cpp Character.cpp Mario.cpp \
    -o MarioFanGame \
    -framework OpenGL -lGLEW -lglfw -lm \
    -I.
```
3. **Execução:**
 ```bash
./MarioFanGame
```

### Linux
1. **Instale o GLEW e o GLFW:**
 ```bash
sudo apt-get update
sudo apt-get install build-essential libglew-dev libglfw3-dev libglm-dev
```

2. **Compilação:**
```bash
g++ -std=c++11 -Wall -Wextra -g \
    main.cpp Shader.cpp Geometry.cpp Character.cpp Mario.cpp \
    -o MarioFanGame \
    -lGL -lGLEW -lglfw -lm \
    -I.
```

3. **Execução:**
 ```bash
./MarioFanGame
```
