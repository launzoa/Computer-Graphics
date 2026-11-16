#ifndef AULA15_CLIPPING_HPP
#define AULA15_CLIPPING_HPP

#include "grid_canvas.hpp"
#include "lab_module.hpp"
#include "ui_widgets.hpp"
#include <string>
#include <vector>

namespace cg {

/** @brief Retângulo delimitador da janela de recorte (clipping window). */
struct ClipRect {
    double xmin = 0.0;
    double ymin = 0.0;
    double xmax = 0.0;
    double ymax = 0.0;
};

/** @brief Resultado do recorte de reta pelo algoritmo de Cohen-Sutherland. */
struct ClipResult {
    bool accepted = false;  ///< True se houver ao menos um trecho visível dentro da janela.
    Vector2 p0{};           ///< Ponto inicial recortado.
    Vector2 p1{};           ///< Ponto final recortado.
    int initialCode0 = 0;   ///< Outcode original de P0 antes do recorte.
    int initialCode1 = 0;   ///< Outcode original de P1 antes do recorte.
};

/** @brief Máscaras de bits dos códigos de região (outcodes) do Cohen-Sutherland. */
enum OutCodeBits {
    CLIP_INSIDE = 0, ///< 0000: ponto dentro da janela.
    CLIP_LEFT   = 1, ///< 0001: à esquerda de xmin.
    CLIP_RIGHT  = 2, ///< 0010: à direita de xmax.
    CLIP_BOTTOM = 4, ///< 0100: abaixo de ymax.
    CLIP_TOP    = 8  ///< 1000: acima de ymin.
};

/**
 * @brief Calcula o código de região (outcode) de 4 bits de um ponto (x, y) em relação a win.
 * @return Combinação OR dos bits OutCodeBits.
 */
int computeCohenSutherlandCode(double x, double y, const ClipRect& win);

/**
 * @brief Recorta o segmento de reta (a, b) contra a janela win usando Cohen-Sutherland.
 * @param a Extremo inicial da reta original.
 * @param b Extremo final da reta original.
 * @param win Janela retangular de visualização.
 * @return Estrutura ClipResult contendo aceitação, extremos recortados e outcodes iniciais.
 */
ClipResult cohenSutherlandClip(Vector2 a, Vector2 b, const ClipRect& win);

/** @brief Formata um outcode inteiro como string binária de 4 bits (ex.: "1001"). */
std::string formatOutCodeBinary(int code);

/**
 * @brief Módulo prático da Aula 15c: definição de janela com 2 cliques e recorte de retas por arraste.
 */
class Aula15ClippingModule : public LabModule {
public:
    const char* menuLabel() const override { return "Aula 15c: Recorte Cohen-Sutherland"; }
    const char* title() const override { return "Aula 15c - Recorte de Retas (Clipping Cohen-Sutherland)"; }
    const char* subtitle() const override {
        return "Clique 2 pontos para definir a janela; depois clique, arraste e solte para traçar retas recortadas.";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    enum class Stage {
        PickWinFirst = 0,
        PickWinSecond,
        DrawLines
    };

    struct ClippedLineRecord {
        Vector2 raw0;
        Vector2 raw1;
        ClipResult res;
    };

    void drawDashedLine(Vector2 a, Vector2 b, float dashLen, float gapLen, Color color) const;

    GridCanvas grid_{16};
    Stage stage_ = Stage::PickWinFirst;
    Vector2 winPt0_{};
    Vector2 winPt1_{};
    ClipRect clipWin_{};

    bool draggingLine_ = false;
    Vector2 lineStart_{};
    Vector2 lineEnd_{};

    bool showGrid_ = true;
    bool showDiscardedDashed_ = false;
    std::vector<ClippedLineRecord> lines_;
    bool initialized_ = false;
};

} // namespace cg

#endif // AULA15_CLIPPING_HPP
