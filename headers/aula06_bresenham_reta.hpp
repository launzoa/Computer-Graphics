#ifndef AULA06_BRESENHAM_RETA_HPP
#define AULA06_BRESENHAM_RETA_HPP

#include "grid_canvas.hpp"
#include "lab_module.hpp"
#include "ui_widgets.hpp"
#include <vector>

namespace cg {

/**
 * @brief Rasteriza um segmento de reta pelo algoritmo de Bresenham (ponto médio) em todos os 8 octantes.
 *
 * Utiliza exclusivamente aritmética inteira (somas, subtrações e multiplicações por 2),
 * escolhendo o eixo dominante (|dx| >= |dy| ou |dy| > |dx|) e os sentidos incX/incY.
 *
 * @param x0 Coordenada X inicial.
 * @param y0 Coordenada Y inicial.
 * @param x1 Coordenada X final.
 * @param y1 Coordenada Y final.
 * @return Lista de pixels discretos formando a reta contínua.
 */
std::vector<GridPoint> rasterizeLineBresenham(int x0, int y0, int x1, int y1);

/**
 * @brief Classifica o octante (1º ao 8º) ou direção cardinal (horizontal/vertical) da reta.
 * @return String descritiva em português para exibição na interface.
 */
const char* describeLineOctant(int x0, int y0, int x1, int y1);

/**
 * @brief Módulo prático da Aula 06: traçado inteiro de retas com Bresenham nos 8 octantes.
 */
class Aula06BresenhamRetaModule : public LabModule {
public:
    const char* menuLabel() const override { return "Aula 06: Bresenham Retas"; }
    const char* title() const override { return "Aula 06 - Algoritmo de Bresenham (Todos os Octantes)"; }
    const char* subtitle() const override {
        return "Traçado inteiro de retas (sem ponto flutuante) operando nos 8 octantes.";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    struct LineEntry {
        GridPoint p0;
        GridPoint p1;
        Color color;
    };

    void addLine(GridPoint p0, GridPoint p1, Color color);
    void syncInputs();

    GridCanvas grid_{12};
    int cellSize_ = 12;
    bool showGrid_ = true;
    bool showIdeal_ = false;
    bool hasFirstPoint_ = false;
    GridPoint firstPoint_{};
    GridPoint p0_{10, 10};
    GridPoint p1_{35, 22};

    UiTextInput inX0_, inY0_, inX1_, inY1_;
    std::vector<LineEntry> lines_;
    bool initialized_ = false;
};

} // namespace cg

#endif // AULA06_BRESENHAM_RETA_HPP
