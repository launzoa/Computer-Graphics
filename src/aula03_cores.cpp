#include "aula03_cores.hpp"

#include <algorithm>
#include <cstdio>

namespace cg {

namespace {
constexpr int kRgbMax = 255;
constexpr int kHlsMax = 240;
constexpr int kHlsUndefined = (kHlsMax * 2) / 3; // 160 no padrão do MS Paint para tons acromáticos

// Função auxiliar da conversão HSL -> RGB (interpolação linear por setor de matiz).
int hueToRgb(int n1, int n2, int hue) {
    if (hue < 0) hue += kHlsMax;
    if (hue >= kHlsMax) hue -= kHlsMax;

    if (hue < (kHlsMax / 6)) {
        return n1 + (((n2 - n1) * hue + (kHlsMax / 12)) / (kHlsMax / 6));
    }
    if (hue < (kHlsMax / 2)) {
        return n2;
    }
    if (hue < ((kHlsMax * 2) / 3)) {
        return n1 + (((n2 - n1) * (((kHlsMax * 2) / 3) - hue) + (kHlsMax / 12)) / (kHlsMax / 6));
    }
    return n1;
}
} // namespace

// Converte RGB [0..255] para HSL na escala do Paint (H: 0..239, S: 0..240, L: 0..240).
HslPaintColor rgbToHslPaint(RgbColor rgb) {
    int r = std::clamp(rgb.r, 0, kRgbMax);
    int g = std::clamp(rgb.g, 0, kRgbMax);
    int b = std::clamp(rgb.b, 0, kRgbMax);

    int cMax = std::max({r, g, b});
    int cMin = std::min({r, g, b});

    // Calcula a luminosidade L pela média entre o maior e o menor canal RGB.
    int l = (((cMax + cMin) * kHlsMax) + kRgbMax) / (2 * kRgbMax);
    int h = kHlsUndefined;
    int s = 0;

    if (cMax != cMin) {
        // Cor cromática: calcula S conforme a metade da escala de luminosidade (L <= 120 ou > 120).
        int cDelta = cMax - cMin;
        if (l <= (kHlsMax / 2)) {
            s = ((cDelta * kHlsMax) + ((cMax + cMin) / 2)) / (cMax + cMin);
        } else {
            s = ((cDelta * kHlsMax) + ((2 * kRgbMax - cMax - cMin) / 2)) / (2 * kRgbMax - cMax - cMin);
        }

        // Calcula o deslocamento de matiz em cada setor (vermelho = 0, verde = 80, azul = 160).
        int rDelta = (((cMax - r) * (kHlsMax / 6)) + (cDelta / 2)) / cDelta;
        int gDelta = (((cMax - g) * (kHlsMax / 6)) + (cDelta / 2)) / cDelta;
        int bDelta = (((cMax - b) * (kHlsMax / 6)) + (cDelta / 2)) / cDelta;

        if (r == cMax) {
            h = bDelta - gDelta;
        } else if (g == cMax) {
            h = (kHlsMax / 3) + rDelta - bDelta;
        } else {
            h = ((2 * kHlsMax) / 3) + gDelta - rDelta;
        }

        if (h < 0) h += kHlsMax;
        if (h >= kHlsMax) h -= kHlsMax;
    }

    return {std::clamp(h, 0, kHlsMax - 1), std::clamp(s, 0, kHlsMax), std::clamp(l, 0, kHlsMax)};
}

// Converte HSL na escala do Paint (H: 0..239, S: 0..240, L: 0..240) para RGB [0..255].
RgbColor hslPaintToRgb(HslPaintColor hsl) {
    int h = hsl.h % kHlsMax;
    if (h < 0) h += kHlsMax;
    int s = std::clamp(hsl.s, 0, kHlsMax);
    int l = std::clamp(hsl.l, 0, kHlsMax);

    int r = 0;
    int g = 0;
    int b = 0;

    if (s == 0) {
        // Saturação nula gera tom de cinza puro proporcional à luminosidade L.
        r = g = b = ((l * kRgbMax) + (kHlsMax / 2)) / kHlsMax;
    } else {
        // Calcula os limites inferior (magic1) e superior (magic2) e interpola R, G e B.
        int magic2 = (l <= (kHlsMax / 2))
            ? ((l * (kHlsMax + s) + (kHlsMax / 2)) / kHlsMax)
            : (l + s - ((l * s + (kHlsMax / 2)) / kHlsMax));
        int magic1 = 2 * l - magic2;

        r = (hueToRgb(magic1, magic2, h + (kHlsMax / 3)) * kRgbMax + (kHlsMax / 2)) / kHlsMax;
        g = (hueToRgb(magic1, magic2, h) * kRgbMax + (kHlsMax / 2)) / kHlsMax;
        b = (hueToRgb(magic1, magic2, h - (kHlsMax / 3)) * kRgbMax + (kHlsMax / 2)) / kHlsMax;
    }

    return {std::clamp(r, 0, kRgbMax), std::clamp(g, 0, kRgbMax), std::clamp(b, 0, kRgbMax)};
}

Aula03CoresModule::~Aula03CoresModule() {
    if (texturesReady_) {
        if (spectrumTex_.id != 0) UnloadTexture(spectrumTex_);
        if (lumTex_.id != 0) UnloadTexture(lumTex_);
        texturesReady_ = false;
    }
}

void Aula03CoresModule::onEnter() {
    if (!initialized_) {
        auto setupNum = [](UiTextInput& in, int val) {
            in.numericOnly = true;
            in.allowFloat = false;
            in.allowSign = false;
            in.maxChars = 4;
            in.setInt(val);
        };
        setupNum(inputR_, rgb_.r);
        setupNum(inputG_, rgb_.g);
        setupNum(inputB_, rgb_.b);
        setupNum(inputH_, hsl_.h);
        setupNum(inputS_, hsl_.s);
        setupNum(inputL_, hsl_.l);
        initialized_ = true;
    }
    ensureTextures();
    rebuildLumTexture();
}

void Aula03CoresModule::ensureTextures() {
    if (texturesReady_) return;

    const int w = 240;
    const int h = 241;
    Image specImg = GenImageColor(w, h, WHITE);
    for (int y = 0; y < h; ++y) {
        int sat = 240 - y;
        for (int x = 0; x < w; ++x) {
            RgbColor c = hslPaintToRgb({x, sat, 120});
            ImageDrawPixel(&specImg, x, y, Color{
                static_cast<unsigned char>(c.r),
                static_cast<unsigned char>(c.g),
                static_cast<unsigned char>(c.b),
                255
            });
        }
    }
    spectrumTex_ = LoadTextureFromImage(specImg);
    UnloadImage(specImg);

    Image lumImg = GenImageColor(24, 241, WHITE);
    lumTex_ = LoadTextureFromImage(lumImg);
    UnloadImage(lumImg);

    texturesReady_ = true;
    lastLumH_ = -1;
    lastLumS_ = -1;
}

void Aula03CoresModule::rebuildLumTexture() {
    if (!texturesReady_) return;
    if (lastLumH_ == hsl_.h && lastLumS_ == hsl_.s) return;

    const int w = 24;
    const int h = 241;
    Image lumImg = GenImageColor(w, h, WHITE);
    for (int y = 0; y < h; ++y) {
        int lum = 240 - y;
        RgbColor c = hslPaintToRgb({hsl_.h, hsl_.s, lum});
        Color rc{
            static_cast<unsigned char>(c.r),
            static_cast<unsigned char>(c.g),
            static_cast<unsigned char>(c.b),
            255
        };
        for (int x = 0; x < w; ++x) {
            ImageDrawPixel(&lumImg, x, y, rc);
        }
    }
    UpdateTexture(lumTex_, lumImg.data);
    UnloadImage(lumImg);
    lastLumH_ = hsl_.h;
    lastLumS_ = hsl_.s;
}

void Aula03CoresModule::syncInputsFromState() {
    inputR_.setInt(rgb_.r);
    inputG_.setInt(rgb_.g);
    inputB_.setInt(rgb_.b);
    inputH_.setInt(hsl_.h);
    inputS_.setInt(hsl_.s);
    inputL_.setInt(hsl_.l);
}

void Aula03CoresModule::applyRgbToHsl() {
    rgb_.r = std::clamp(rgb_.r, 0, 255);
    rgb_.g = std::clamp(rgb_.g, 0, 255);
    rgb_.b = std::clamp(rgb_.b, 0, 255);
    hsl_ = rgbToHslPaint(rgb_);
    syncInputsFromState();
    rebuildLumTexture();
}

void Aula03CoresModule::applyHslToRgb() {
    hsl_.h = std::clamp(hsl_.h, 0, 239);
    hsl_.s = std::clamp(hsl_.s, 0, 240);
    hsl_.l = std::clamp(hsl_.l, 0, 240);
    rgb_ = hslPaintToRgb(hsl_);
    syncInputsFromState();
    rebuildLumTexture();
}

void Aula03CoresModule::update(Rectangle area) {
    ensureTextures();
    const float pad = 20.0f;
    Rectangle leftCard = {area.x + pad, area.y + pad, std::min(460.0f, area.width * 0.48f), area.height - 2 * pad};
    Rectangle specRect = {leftCard.x + 20.0f, leftCard.y + 52.0f, leftCard.width - 90.0f, 260.0f};
    Rectangle lumRect  = {specRect.x + specRect.width + 18.0f, specRect.y, 28.0f, specRect.height};

    Vector2 mouse = GetMousePosition();
    static int dragTarget = 0; 
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse, specRect)) {
            dragTarget = 1;
            clearTextInputFocus();
        } else if (CheckCollisionPointRec(mouse, lumRect)) {
            dragTarget = 2;
            clearTextInputFocus();
        }
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        dragTarget = 0;
    }

    if (dragTarget == 1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float nx = std::clamp((mouse.x - specRect.x) / specRect.width, 0.0f, 0.999f);
        float ny = std::clamp((mouse.y - specRect.y) / specRect.height, 0.0f, 1.0f);
        hsl_.h = std::clamp(static_cast<int>(nx * 240.0f), 0, 239);
        hsl_.s = std::clamp(static_cast<int>((1.0f - ny) * 240.0f + 0.5f), 0, 240);
        applyHslToRgb();
    } else if (dragTarget == 2 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float ny = std::clamp((mouse.y - lumRect.y) / lumRect.height, 0.0f, 1.0f);
        hsl_.l = std::clamp(static_cast<int>((1.0f - ny) * 240.0f + 0.5f), 0, 240);
        applyHslToRgb();
    }
}

void Aula03CoresModule::draw(Rectangle area) {
    const float pad = 20.0f;
    float leftW = std::min(460.0f, area.width * 0.48f);
    Rectangle leftCard = {area.x + pad, area.y + pad, leftW, area.height - 2 * pad};
    Rectangle rightCard = {leftCard.x + leftW + pad, area.y + pad, area.width - leftW - 3 * pad, area.height - 2 * pad};

    drawPanelCard(leftCard, "Seletor Visual (Estilo Paint)");
    drawPanelCard(rightCard, "Conversão Bidirecional RGB <-> HSL");

    // Espectro de Matiz x Saturação e barra vertical de Luminosidade
    Rectangle specRect = {leftCard.x + 20.0f, leftCard.y + 52.0f, leftCard.width - 90.0f, 260.0f};
    Rectangle lumRect  = {specRect.x + specRect.width + 18.0f, specRect.y, 28.0f, specRect.height};

    DrawTexturePro(spectrumTex_,
                   {0, 0, static_cast<float>(spectrumTex_.width), static_cast<float>(spectrumTex_.height)},
                   specRect, {0, 0}, 0.0f, WHITE);
    DrawRectangleLinesEx(specRect, 1.5f, theme::PanelBorder);

    // Mira indicando a posição (H, S) atual sobre o espectro
    float cx = specRect.x + (static_cast<float>(hsl_.h) / 240.0f) * specRect.width;
    float cy = specRect.y + (1.0f - static_cast<float>(hsl_.s) / 240.0f) * specRect.height;
    DrawLineEx({cx - 8.0f, cy}, {cx - 3.0f, cy}, 2.0f, BLACK);
    DrawLineEx({cx + 3.0f, cy}, {cx + 8.0f, cy}, 2.0f, BLACK);
    DrawLineEx({cx, cy - 8.0f}, {cx, cy - 3.0f}, 2.0f, BLACK);
    DrawLineEx({cx, cy + 3.0f}, {cx, cy + 8.0f}, 2.0f, BLACK);

    DrawTexturePro(lumTex_,
                   {0, 0, static_cast<float>(lumTex_.width), static_cast<float>(lumTex_.height)},
                   lumRect, {0, 0}, 0.0f, WHITE);
    DrawRectangleLinesEx(lumRect, 1.5f, theme::PanelBorder);

    // Seta triangular indicando o nível de luminosidade (L) atual
    float ly = lumRect.y + (1.0f - static_cast<float>(hsl_.l) / 240.0f) * lumRect.height;
    DrawTriangle(
        {lumRect.x + lumRect.width + 2.0f, ly},
        {lumRect.x + lumRect.width + 11.0f, ly - 5.0f},
        {lumRect.x + lumRect.width + 11.0f, ly + 5.0f},
        theme::TextDark
    );

    // Amostra visual da cor selecionada
    Color currentCol = {
        static_cast<unsigned char>(rgb_.r),
        static_cast<unsigned char>(rgb_.g),
        static_cast<unsigned char>(rgb_.b),
        255
    };
    Rectangle previewBox = {leftCard.x + 20.0f, specRect.y + specRect.height + 18.0f, 110.0f, 68.0f};
    DrawRectangleRec(previewBox, currentCol);
    DrawRectangleLinesEx(previewBox, 2.0f, theme::TextDark);
    drawUiText("Cor | Sólida", previewBox.x + 12.0f, previewBox.y + previewBox.height + 6.0f, 13.0f, theme::TextMuted);

    // Informações complementares (Hexadecimal, graus/porcentagem e luminância)
    char infoBuf[128];
    std::snprintf(infoBuf, sizeof(infoBuf), "Hex: #%02X%02X%02X", rgb_.r, rgb_.g, rgb_.b);
    drawUiText(infoBuf, previewBox.x + 126.0f, previewBox.y + 4.0f, 16.0f, theme::TextDark);

    float degH = (static_cast<float>(hsl_.h) / 240.0f) * 360.0f;
    float pctS = (static_cast<float>(hsl_.s) / 240.0f) * 100.0f;
    float pctL = (static_cast<float>(hsl_.l) / 240.0f) * 100.0f;
    std::snprintf(infoBuf, sizeof(infoBuf), "H: %.1f°  S: %.1f%%  L: %.1f%%", degH, pctS, pctL);
    drawUiText(infoBuf, previewBox.x + 126.0f, previewBox.y + 28.0f, 14.0f, theme::TextMuted);

    double lumY = 0.299 * rgb_.r + 0.587 * rgb_.g + 0.114 * rgb_.b;
    std::snprintf(infoBuf, sizeof(infoBuf), "Luminância (I): %.1f", lumY);
    drawUiText(infoBuf, previewBox.x + 126.0f, previewBox.y + 50.0f, 14.0f, theme::TextMuted);

    // Paleta de cores básicas inspirada no seletor do MS Paint
    drawUiText("Cores Básicas:", leftCard.x + 20.0f, previewBox.y + previewBox.height + 34.0f, 14.0f, theme::TextDark);
    static const RgbColor palette[] = {
        {255, 128, 128}, {255, 255, 128}, {128, 255, 128}, {0, 255, 128}, {128, 255, 255}, {0, 128, 255}, {255, 128, 192}, {255, 128, 255},
        {255, 0, 0},     {255, 255, 0},   {128, 255, 0},   {0, 255, 64},  {0, 255, 255},   {0, 128, 192}, {128, 128, 192}, {255, 0, 255},
        {128, 64, 64},   {255, 128, 64},  {0, 255, 0},     {0, 128, 128}, {0, 64, 128},    {128, 128, 255},{128, 0, 64},   {255, 0, 128},
        {128, 0, 0},     {255, 128, 0},   {0, 128, 0},     {0, 128, 64},  {0, 0, 255},     {0, 0, 160},   {128, 0, 128},   {128, 0, 255},
        {0, 0, 0},       {128, 128, 0},   {128, 128, 64},  {128, 128, 128},{64, 128, 128}, {192, 192, 192},{64, 0, 64},    {255, 255, 255}
    };
    float swY = previewBox.y + previewBox.height + 56.0f;
    float swSize = 26.0f;
    float swGap = 6.0f;
    for (int i = 0; i < 40; ++i) {
        int row = i / 8;
        int col = i % 8;
        Rectangle sw = {leftCard.x + 20.0f + col * (swSize + swGap), swY + row * (swSize + swGap), swSize, swSize};
        Color sc = {
            static_cast<unsigned char>(palette[i].r),
            static_cast<unsigned char>(palette[i].g),
            static_cast<unsigned char>(palette[i].b),
            255
        };
        DrawRectangleRec(sw, sc);
        DrawRectangleLinesEx(sw, 1.0f, theme::GridAxis);
        if (CheckCollisionPointRec(GetMousePosition(), sw) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            rgb_ = palette[i];
            applyRgbToHsl();
        }
    }

    // Painel direito: sliders e caixas numéricas para RGB e HSL (escala Paint).
    float rx = rightCard.x + 24.0f;
    float rw = rightCard.width - 48.0f;
    float sliderW = std::max(140.0f, rw - 115.0f);
    float ry = rightCard.y + 54.0f;

    drawUiText("Sistema RGB (0 a 255)", rx, ry, 17.0f, theme::AccentHover);
    ry += 28.0f;

    bool rgbEdited = false;
    if (uiSliderInt({rx, ry, sliderW, 30.0f}, "Red (R)", &rgb_.r, 0, 255)) rgbEdited = true;
    if (uiTextInput({rx + sliderW + 18.0f, ry + 4.0f, 82.0f, 30.0f}, inputR_)) {
        rgb_.r = std::clamp(inputR_.asInt(rgb_.r), 0, 255);
        rgbEdited = true;
    }
    ry += 46.0f;

    if (uiSliderInt({rx, ry, sliderW, 30.0f}, "Green (G)", &rgb_.g, 0, 255)) rgbEdited = true;
    if (uiTextInput({rx + sliderW + 18.0f, ry + 4.0f, 82.0f, 30.0f}, inputG_)) {
        rgb_.g = std::clamp(inputG_.asInt(rgb_.g), 0, 255);
        rgbEdited = true;
    }
    ry += 46.0f;

    if (uiSliderInt({rx, ry, sliderW, 30.0f}, "Blue (B)", &rgb_.b, 0, 255)) rgbEdited = true;
    if (uiTextInput({rx + sliderW + 18.0f, ry + 4.0f, 82.0f, 30.0f}, inputB_)) {
        rgb_.b = std::clamp(inputB_.asInt(rgb_.b), 0, 255);
        rgbEdited = true;
    }
    ry += 46.0f;

    if (uiButton({rx, ry, 210.0f, 34.0f}, "Converter RGB -> HSL", false, true) || rgbEdited) {
        rgb_.r = std::clamp(inputR_.asInt(rgb_.r), 0, 255);
        rgb_.g = std::clamp(inputG_.asInt(rgb_.g), 0, 255);
        rgb_.b = std::clamp(inputB_.asInt(rgb_.b), 0, 255);
        applyRgbToHsl();
    }

    ry += 54.0f;
    DrawLineEx({rx, ry}, {rx + rw, ry}, 1.0f, theme::GridMinor);
    ry += 18.0f;

    drawUiText("Sistema HSL - Paint (H: 0..239, S: 0..240, L: 0..240)", rx, ry, 17.0f, theme::AccentHover);
    ry += 28.0f;

    bool hslEdited = false;
    if (uiSliderInt({rx, ry, sliderW, 30.0f}, "Hue (Matiz)", &hsl_.h, 0, 239)) hslEdited = true;
    if (uiTextInput({rx + sliderW + 18.0f, ry + 4.0f, 82.0f, 30.0f}, inputH_)) {
        hsl_.h = std::clamp(inputH_.asInt(hsl_.h), 0, 239);
        hslEdited = true;
    }
    ry += 46.0f;

    if (uiSliderInt({rx, ry, sliderW, 30.0f}, "Sat (Saturação)", &hsl_.s, 0, 240)) hslEdited = true;
    if (uiTextInput({rx + sliderW + 18.0f, ry + 4.0f, 82.0f, 30.0f}, inputS_)) {
        hsl_.s = std::clamp(inputS_.asInt(hsl_.s), 0, 240);
        hslEdited = true;
    }
    ry += 46.0f;

    if (uiSliderInt({rx, ry, sliderW, 30.0f}, "Lum (Luminosidade)", &hsl_.l, 0, 240)) hslEdited = true;
    if (uiTextInput({rx + sliderW + 18.0f, ry + 4.0f, 82.0f, 30.0f}, inputL_)) {
        hsl_.l = std::clamp(inputL_.asInt(hsl_.l), 0, 240);
        hslEdited = true;
    }
    ry += 46.0f;

    if (uiButton({rx, ry, 210.0f, 34.0f}, "Converter HSL -> RGB", false, true) || hslEdited) {
        hsl_.h = std::clamp(inputH_.asInt(hsl_.h), 0, 239);
        hsl_.s = std::clamp(inputS_.asInt(hsl_.s), 0, 240);
        hsl_.l = std::clamp(inputL_.asInt(hsl_.l), 0, 240);
        applyHslToRgb();
    }
}

} // namespace cg
