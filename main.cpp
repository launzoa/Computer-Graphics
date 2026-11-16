#include "app.hpp"

/**
 * @file main.cpp
 * @brief Ponto de entrada da aplicação de Computação Gráfica em C++ e raylib.
 *
 * Para compilar: cmake -B build && cmake --build build
 * Para executar: ./build/app
 */

/**
 * @brief Função principal do programa.
 * @return Código de status de saída do processo (0 em execução normal).
 */
int main() {
    // Instancia o controlador da aplicação, abre a janela principal e inicia na grade de desenho.
    cg::App app;
    app.run();
    return 0;
}
