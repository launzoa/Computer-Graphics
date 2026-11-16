#ifndef AULA07_CIRC_PIXELS_HPP
#define AULA07_CIRC_PIXELS_HPP

#include "grid_canvas.hpp"
#include "lab_module.hpp"
#include "ui_widgets.hpp"
#include <vector>

namespace cg {

/**
 * @brief Método 1: Traça uma circunferência usando a equação cartesiana y = sqrt(R² - x²).
 * @param xc Coordenada X do centro.
 * @param yc Coordenada Y do centro.
 * @param radius Raio inteiro da circunferência.
 * @return Lista de pixels gerados (metade superior e inferior por simetria em Y).
 */
std::vector<GridPoint> rasterizeCircleEquation(int xc, int yc, int radius);

/**
 * @brief Método 2: Traça uma circunferência usando a equação paramétrica polar.
 * @param xc Coordenada X do centro.
 * @param yc Coordenada Y do centro.
 * @param radius Raio inteiro da circunferência.
 * @param angleStepRad Incremento angular em radianos ao percorrer [0, 2*PI].
 * @return Lista de pixels gerados.
 */
std::vector<GridPoint> rasterizeCircleParametric(int xc, int yc, int radius, float angleStepRad = 0.01f);

/**
 * @brief Método 3: Traça uma circunferência aplicando rotações incrementais de 1 grau (360 passos).
 * @param xc Coordenada X do centro.
 * @param yc Coordenada Y do centro.
 * @param radius Raio inteiro da circunferência.
 * @return Lista de pixels gerados ao longo dos 360 graus.
 */
std::vector<GridPoint> rasterizeCircleIncrementalRotation(int xc, int yc, int radius);

/**
 * @brief Módulo prático da Aula 07: comparação dos três métodos de traçado de circunferência com pixels.
 */
class Aula07CircPixelsModule : public LabModule {
public:
    const char* menuLabel() const override { return "Aula 07: Circunferências Pixels"; }
    const char* title() const override { return "Aula 07 - Traçado de Circunferências usando Pixels"; }
    const char* subtitle() const override {
        return "1) Equação (sqrt)   2) Equação Paramétrica (cos/sin)   3) Rotação Incremental de 1°";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    enum class Method {
        Equation = 0,
        Parametric,
        Rotation1Deg,
        CompareAll
    };

    struct CircleEntry {
        int xc;
        int yc;
        int radius;
        Method method;
        float paramStep;
    };

    void addCircle(int xc, int yc, int radius);
    void syncInputs();

    GridCanvas grid_{10};
    int cellSize_ = 10;
    bool showGrid_ = true;
    Method method_ = Method::Equation;
    int xc_ = 28;
    int yc_ = 22;
    int radius_ = 15;
    float paramStep_ = 0.02f;

    bool draggingRadius_ = false;
    GridPoint dragCenter_{};

    UiTextInput inXc_, inYc_, inR_;
    std::vector<CircleEntry> circles_;
    bool initialized_ = false;
};

} // namespace cg

#endif // AULA07_CIRC_PIXELS_HPP
