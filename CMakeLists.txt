cmake_minimum_required(VERSION 3.10)
project(CustomGeometryGenerator)

# Definir o padrão C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Definir M_PI
add_definitions(-D_USE_MATH_DEFINES)

# Configurar os diretórios
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

# Adicionar diretórios de include
include_directories(${INCLUDE_DIR})
include_directories(${INCLUDE_DIR}/freetype2)
include_directories(${INCLUDE_DIR}/glad)
include_directories(${INCLUDE_DIR}/GLFW)
include_directories(${INCLUDE_DIR}/glm)
include_directories(${INCLUDE_DIR}/KHR)

# Definir os arquivos fonte
set(SOURCES 
    src/main.cpp
    src/app.cpp
    src/shader.cpp
    src/geometry.cpp
    src/math_utils.cpp
    src/gui.cpp
    src/input.cpp
    src/glad.c
)

# Criar o executável
add_executable(${PROJECT_NAME} ${SOURCES})

# Adicionar diretório de bibliotecas
link_directories(${LIB_DIR})

# Linkar as bibliotecas
target_link_libraries(${PROJECT_NAME} 
    ${LIB_DIR}/libglfw3dll.a
    ${LIB_DIR}/freetype.lib)

# Copiar DLLs para o diretório do executável após o build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/glfw3.dll"
        "${CMAKE_SOURCE_DIR}/freetype.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)