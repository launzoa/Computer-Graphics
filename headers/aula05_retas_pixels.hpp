#ifndef AULA05_RETAS_PIXELS_HPP
#define AULA05_RETAS_PIXELS_HPP

#include "grid_canvas.hpp"
#include "lab_module.hpp"
#include "ui_widgets.hpp"
#include <vector>

namespace cg {

/**
 * @brief Rasteriza um segmento de reta usando a equação geral da reta (y = m*x + b).
 *
 * Todos os cálculos intermediários são efetuados em ponto flutuante (float)
 * e arredondados somente no instante de gravar cada pixel discreto.
 *
 * @param x0 Coordenada X inicial.
 * @param y0 Coordenada Y inicial.
 * @param x1 Coordenada X final.
 * @param y1 Coordenada Y final.
 * @return Lista ordenada de pixels discretos da reta.
 */
std::vector<GridPoint> rasterizeLineGeneralEq(int x0, int y0, int x1, int y1);

/**
 * @brief Rasteriza um segmento de reta usando a equação paramétrica (P(t) = P0 + t*(P1 - P0)).
 *
 * Todos os cálculos são feitos em float com parâmetro t em [0, 1],
 * aplicando arredondamento apenas ao gerar cada pixel da grade.
 *
 * @param x0 Coordenada X inicial.
 * @param y0 Coordenada Y inicial.
 * @param x1 Coordenada X final.
 * @param y1 Coordenada Y final.
 * @return Lista ordenada de pixels discretos da reta.
 */
std::vector<GridPoint> rasterizeLineParametric(int x0, int y0, int x1, int y1);

/**
 * @brief Módulo prático da Aula 05-A: traçado de linhas retas por equação geral e paramétrica.
 */
class Aula05RetasPixelsModule : public LabModule {
public:
    const char* menuLabel() const override { return "Aula 05A: Retas com Pixels"; }
    const char* title() const override { return "Aula 05-A - Desenho de Linhas Retas usando Pixels"; }
    const char* subtitle() const override {
        return "A) Equação Geral   B) Equação Paramétrica   (cálculos em float, arredondando ao desenhar o pixel)";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    enum class Method {
        GeneralEq = 0,
        Parametric,
        Both
    };

    struct DrawnSegment {
        GridPoint p0;
        GridPoint p1;
        Method method;
    };

    void addSegment(GridPoint p0, GridPoint p1);
    void syncInputsFromPoints();

    GridCanvas grid_{12};
    Method method_ = Method::GeneralEq;
    bool showIdealLine_ = true;
    bool showGrid_ = true;
    int cellSize_ = 12;

    bool hasFirstPoint_ = false;
    GridPoint firstPoint_{};
    GridPoint p0_{5, 25};
    GridPoint p1_{35, 10};

    UiTextInput inX0_, inY0_, inX1_, inY1_;
    std::vector<DrawnSegment> segments_;
    bool initialized_ = false;
};

} // namespace cg

#endif // AULA05_RETAS_PIXELS_HPP
