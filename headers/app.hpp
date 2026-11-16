#ifndef APP_HPP
#define APP_HPP

#include "grid_canvas.hpp"
#include "lab_module.hpp"
#include <memory>
#include <vector>

namespace cg {

/**
 * @brief Módulo da tela inicial: exibe uma grade interativa livre para desenho.
 */
class HomeGridModule : public LabModule {
public:
    const char* menuLabel() const override { return "Tela Inicial (Grade)"; }
    const char* title() const override { return "Computação Gráfica - Grade para Desenho"; }
    const char* subtitle() const override {
        return "Selecione uma prática no menu à esquerda ou desenhe livremente na grade abaixo (Esq: pintar | Dir: apagar).";
    }

    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    /** @brief Insere/atualiza o pixel (gx, gy) com a cor ativa ou o remove se erase for true. */
    void setOrErasePixel(int gx, int gy, bool erase);

    GridCanvas grid_{16};
    int cellSize_ = 16;
    bool showGrid_ = true;
    int colorIdx_ = 0;
    std::vector<GridPixel> pixels_;
};

/**
 * @brief Controlador principal da aplicação: gerencia janela, menu lateral e módulos das aulas.
 */
class App {
public:
    /** @brief Inicializa a janela raylib, fontes e todos os módulos práticos. */
    App();
    /** @brief Libera módulos, fontes e fecha a janela raylib. */
    ~App();

    /** @brief Executa o loop principal da aplicação (nativo ou via emscripten_set_main_loop). */
    void run();

private:
    /** @brief Executa um único quadro de atualização e renderização. */
    void tick();
    /** @brief Alterna o módulo ativo pelo índice e dispara onEnter(). */
    void selectModule(int index);
    /** @brief Processa atalhos globais de navegação e delega update() ao módulo ativo. */
    void update();
    /** @brief Renderiza barra lateral, cabeçalho e o conteúdo do módulo ativo. */
    void draw();

    std::vector<std::unique_ptr<LabModule>> modules_;
    int activeIndex_ = 0;
};

} // namespace cg

#endif // APP_HPP
