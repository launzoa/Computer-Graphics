#ifndef AULA03_CORES_HPP
#define AULA03_CORES_HPP

#include "lab_module.hpp"
#include "ui_widgets.hpp"

namespace cg {

/** @brief Representa uma cor no sistema RGB com canais inteiros de 8 bits (0 a 255). */
struct RgbColor {
    int r = 0; ///< Canal vermelho (0..255).
    int g = 0; ///< Canal verde (0..255).
    int b = 0; ///< Canal azul (0..255).
};

/** @brief Representa uma cor no espaço HSL na escala do Microsoft Paint. */
struct HslPaintColor {
    int h = 160; ///< Matiz (Hue): 0..239 (160 quando acromático, padrão do MS Paint).
    int s = 0;   ///< Saturação (Sat): 0..240.
    int l = 0;   ///< Luminosidade (Lum): 0..240.
};

/**
 * @brief Converte uma cor RGB (0..255) para HSL na escala do Paint (H: 0..239, S/L: 0..240).
 * @param rgb Cor de entrada no espaço RGB.
 * @return Cor equivalente no espaço HSL padrão Windows Paint.
 */
HslPaintColor rgbToHslPaint(RgbColor rgb);

/**
 * @brief Converte uma cor HSL na escala do Paint (H: 0..239, S/L: 0..240) para RGB (0..255).
 * @param hsl Cor de entrada no espaço HSL do Paint.
 * @return Cor equivalente no espaço RGB de 8 bits.
 */
RgbColor hslPaintToRgb(HslPaintColor hsl);

/**
 * @brief Módulo prático da Aula 03: conversão bidirecional RGB <-> HSL com seletor estilo Paint.
 */
class Aula03CoresModule : public LabModule {
public:
    Aula03CoresModule() = default;
    ~Aula03CoresModule() override;

    const char* menuLabel() const override { return "Aula 03: Cores (RGB <-> HSL)"; }
    const char* title() const override { return "Aula 03 - Conversão de Cores RGB <-> HSL (Padrão Paint)"; }
    const char* subtitle() const override {
        return "Escala MS Paint: R,G,B em [0..255] | Hue em [0..239], Sat em [0..240], Lum em [0..240]";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    void syncInputsFromState();
    void applyRgbToHsl();
    void applyHslToRgb();
    void ensureTextures();
    void rebuildLumTexture();

    RgbColor rgb_{255, 0, 0};
    HslPaintColor hsl_{0, 240, 120};

    UiTextInput inputR_;
    UiTextInput inputG_;
    UiTextInput inputB_;
    UiTextInput inputH_;
    UiTextInput inputS_;
    UiTextInput inputL_;

    Texture2D spectrumTex_{};
    Texture2D lumTex_{};
    bool texturesReady_ = false;
    int lastLumH_ = -1;
    int lastLumS_ = -1;
    bool initialized_ = false;
};

} // namespace cg

#endif // AULA03_CORES_HPP
