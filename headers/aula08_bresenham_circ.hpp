#ifndef AULA08_BRESENHAM_CIRC_HPP
#define AULA08_BRESENHAM_CIRC_HPP

#include "grid_canvas.hpp"
#include "lab_module.hpp"
#include "ui_widgets.hpp"
#include <vector>

namespace cg {

/** @brief Pixel da circunferência acompanhado do índice do octante (0..7) da simetria-8. */
struct CircleOctantPixel {
    int x = 0;      ///< Coordenada X na grade.
    int y = 0;      ///< Coordenada Y na grade.
    int octant = 0; ///< Índice do octante simétrico (0 a 7).
};

/**
 * @brief Traça uma circunferência com o algoritmo inteiro de Bresenham (2ª ordem) e simetria-8.
 *
 * Calcula apenas os pontos do 2º octante (0 a 45°, x < y) usando diferenças de
 * segunda ordem (h = 1 - R, dE = 3, dSE = -2R + 5) e espelha nos outros 7 octantes.
 *
 * @param xc Coordenada X do centro.
 * @param yc Coordenada Y do centro.
 * @param radius Raio inteiro da circunferência.
 * @return Vetor de pixels com a indicação do octante correspondente.
 */
std::vector<CircleOctantPixel> rasterizeCircleBresenham8Sym(int xc, int yc, int radius);

/**
 * @brief Módulo prático da Aula 08: circunferência por Bresenham (diferenças de 2ª ordem + simetria-8).
 */
class Aula08BresenhamCircModule : public LabModule {
public:
    const char* menuLabel() const override { return "Aula 08: Bresenham Circunferência"; }
    const char* title() const override { return "Aula 08 - Circunferência de Bresenham (2ª Ordem + Simetria-8)"; }
    const char* subtitle() const override {
        return "Versão com diferenças de segunda ordem (dE = 3, dSE = -2R + 5) e espelhamento nos 8 octantes.";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    struct CircleEntry {
        int xc;
        int yc;
        int radius;
    };

    void addCircle(int xc, int yc, int radius);
    void syncInputs();

    GridCanvas grid_{12};
    int cellSize_ = 12;
    bool showGrid_ = true;
    bool colorizeOctants_ = true;
    int xc_ = 25;
    int yc_ = 20;
    int radius_ = 14;

    bool draggingRadius_ = false;
    GridPoint dragCenter_{};

    UiTextInput inXc_, inYc_, inR_;
    std::vector<CircleEntry> circles_;
    bool initialized_ = false;
};

} // namespace cg

#endif // AULA08_BRESENHAM_CIRC_HPP
