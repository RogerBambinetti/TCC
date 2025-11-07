# Custom Geometry Generator

Este projeto se trata de um módulo utilizado para gerar geometrias CICP customizadas para transcodificação de arquivos MPEG-H 3D Audio com o software de referência.

## Pré-requisitos

- CMake (versão 3.10 ou superior)
- Visual Studio 2019 com suporte a C++
- MinGW-w64 com GCC

## Estrutura do Projeto

```
├── include/          # Arquivos de cabeçalho
├── lib/             # Bibliotecas (.lib, .dll)
├── src/             # Código fonte
└── CMakeLists.txt   # Configuração do CMake
```

## Como Construir

1. Clone o repositório:
```bash
git clone https://github.com/RogerBambinetti/TCC.git
cd TCC
```

2. Crie um diretório de build:
```bash
mkdir build
cd build
```

3. Configure o projeto com CMake:
```bash
cmake ..
```

4. Compile o projeto:
```bash
cmake --build .
```

O executável será gerado em `build/Debug/CustomGeometryGenerator.exe`. As DLLs necessárias (glfw3.dll e freetype.dll) serão automaticamente copiadas para o diretório do executável durante o processo de build.

## Arquivos Fonte

- `main.cpp` - Ponto de entrada do programa
- `app.cpp` - Gerenciamento da aplicação
- `shader.cpp` - Gerenciamento de shaders
- `geometry.cpp` - Geração de geometrias
- `math_utils.cpp` - Utilitários matemáticos
- `gui.cpp` - Interface gráfica
- `input.cpp` - Gerenciamento de entrada

## Dependências

- GLFW - Gerenciamento de janelas e contexto OpenGL
- FreeType - Renderização de fontes
- GLM - Biblioteca matemática para OpenGL
- GLAD - Carregamento de funções OpenGL

## Notas de Desenvolvimento

- O projeto utiliza C++17
- As bibliotecas necessárias já estão incluídas no diretório `lib/`
- Os arquivos de cabeçalho das dependências estão no diretório `include/`