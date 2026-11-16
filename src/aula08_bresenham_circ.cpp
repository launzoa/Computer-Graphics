#include "aula08_bresenham_circ.hpp"

#include <algorithm>
#include <cmath>

namespace cg {

namespace {
// Espelha o ponto (x, y) do segundo octante para os outros 7 octantes (simetria-8).
void append8Sym(std::vector<CircleOctantPixel>& out, int xc, int yc, int x, int y) {
    out.push_back({xc + x, yc + y, 0});
    out.push_back({xc + x, yc - y, 1});
    out.push_back({xc - x, yc + y, 2});
    out.push_back({xc - x, yc - y, 3});
    out.push_back({xc + y, yc + x, 4});
    out.push_back({xc + y, yc - x, 5});
    out.push_back({xc - y, yc + x, 6});
    out.push_back({xc - y, yc - x, 7});
}
} // namespace

// Algoritmo de Bresenham para circunferências usando diferenças de segunda ordem (somente inteiros).
std::vector<CircleOctantPixel> rasterizeCircleBresenham8Sym(int xc, int yc, int radius) {
    std::vector<CircleOctantPixel> pts;
    if (radius < 0) return pts;
    if (radius == 0) {
        pts.push_back({xc, yc, 0});
        return pts;
    }

    // Condições iniciais da versão com diferenças de 2ª ordem (ponto inicial em (0, R)).
    int x = 0;
    int y = radius;
    int h = 1 - radius;
    int dE = 3;
    int dSE = -2 * radius + 5;

    append8Sym(pts, xc, yc, x, y);
    while (x < y) {
        if (h < 0) { // Seleciona E: o ponto médio está dentro da circunferência
            h += dE;
            dE += 2;
            dSE += 2;
        } else { // Seleciona SE: o ponto médio está sobre ou fora da circunferência
            h += dSE;
            dE += 2;
            dSE += 4;
            y -= 1;
        }
        x += 1;
        append8Sym(pts, xc, yc, x, y);
    }

    return pts;
}

void Aula08BresenhamCircModule::onEnter() {
    if (!initialized_) {
        auto setup = [](UiTextInput& in) {
            in.numericOnly = true;
            in.allowFloat = false;
            in.allowSign = false;
            in.maxChars = 5;
        };
        setup(inXc_);
        setup(inYc_);
        setup(inR_);
        syncInputs();
        addCircle(xc_, yc_, radius_);
        initialized_ = true;
    }
}

void Aula08BresenhamCircModule::syncInputs() {
    inXc_.setInt(xc_);
    inYc_.setInt(yc_);
    inR_.setInt(radius_);
}

void Aula08BresenhamCircModule::addCircle(int xc, int yc, int radius) {
    xc_ = xc;
    yc_ = yc;
    radius_ = std::max(0, radius);
    syncInputs();
    circles_.push_back({xc_, yc_, radius_});
}

void Aula08BresenhamCircModule::update(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 295.0f;
    Rectangle gridRect = {area.x + pad + 8.0f, area.y + pad + 40.0f,
                          area.width - ctrlW - 3 * pad - 16.0f, area.height - 2 * pad - 52.0f};

    Vector2 mouse = GetMousePosition();
    if (grid_.containsScreenPoint(mouse, gridRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        clearTextInputFocus();
        dragCenter_ = grid_.screenToGrid(mouse, gridRect);
        draggingRadius_ = true;
    }
    if (draggingRadius_ && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        GridPoint edge = grid_.screenToGrid(mouse, gridRect);
        float dx = static_cast<float>(edge.x - dragCenter_.x);
        float dy = static_cast<float>(edge.y - dragCenter_.y);
        int r = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));
        if (r == 0) r = radius_;
        addCircle(dragCenter_.x, dragCenter_.y, r);
        draggingRadius_ = false;
    }
}

void Aula08BresenhamCircModule::draw(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 295.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    Rectangle ctrlCard   = {canvasCard.x + canvasCard.width + pad, area.y + pad, ctrlW, area.height - 2 * pad};

    drawPanelCard(canvasCard, "Grade de Desenho - Bresenham Circunferência (Simetria-8)");
    drawPanelCard(ctrlCard, "Controles - Aula 08");

    Rectangle gridRect = {canvasCard.x + 8.0f, canvasCard.y + 40.0f, canvasCard.width - 16.0f, canvasCard.height - 52.0f};
    grid_.setCellSize(cellSize_);
    grid_.drawBackgroundAndGrid(gridRect, showGrid_, 5);

    static const Color octColors[8] = {
        {220, 38, 38, 255},
        {234, 88, 12, 255},
        {22, 163, 74, 255},
        {13, 148, 136, 255},
        {37, 99, 235, 255},
        {79, 70, 229, 255},
        {147, 51, 234, 255},
        {219, 39, 119, 255}
    };

    BeginScissorMode(static_cast<int>(gridRect.x), static_cast<int>(gridRect.y),
                     static_cast<int>(gridRect.width), static_cast<int>(gridRect.height));

    for (const auto& c : circles_) {
        auto pts = rasterizeCircleBresenham8Sym(c.xc, c.yc, c.radius);
        for (const auto& p : pts) {
            Color col = colorizeOctants_ ? octColors[p.octant % 8] : theme::Accent;
            grid_.drawPixel(gridRect, p.x, p.y, col);
        }
        grid_.drawPixel(gridRect, c.xc, c.yc, BLACK);
    }

    if (draggingRadius_) {
        Vector2 mouse = GetMousePosition();
        GridPoint edge = grid_.screenToGrid(mouse, gridRect);
        float dx = static_cast<float>(edge.x - dragCenter_.x);
        float dy = static_cast<float>(edge.y - dragCenter_.y);
        int r = std::max(1, static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy))));
        Vector2 centerPx = grid_.gridToScreenCenter(dragCenter_.x, dragCenter_.y, gridRect);
        DrawCircleLinesV(centerPx, static_cast<float>(r * cellSize_), theme::Warning);
        grid_.drawPixel(gridRect, dragCenter_.x, dragCenter_.y, theme::Warning);
    }

    EndScissorMode();
    grid_.drawHoverIndicator(gridRect, true);

    // Painel lateral de parâmetros e opções de visualização.
    float cx = ctrlCard.x + 14.0f;
    float cw = ctrlCard.width - 28.0f;
    float cy = ctrlCard.y + 44.0f;

    drawUiText("Centro (xc, yc) e Raio (R):", cx, cy, 14.0f, theme::TextDark);
    cy += 32.0f;
    float iw3 = (cw - 16.0f) / 3.0f;
    uiTextInput({cx, cy, iw3, 28.0f}, inXc_, "xc");
    uiTextInput({cx + iw3 + 8.0f, cy, iw3, 28.0f}, inYc_, "yc");
    uiTextInput({cx + 2 * (iw3 + 8.0f), cy, iw3, 28.0f}, inR_, "Raio");
    cy += 38.0f;

    if (uiButton({cx, cy, cw, 32.0f}, "Desenhar Circunferência", false, true)) {
        addCircle(inXc_.asInt(xc_), inYc_.asInt(yc_), inR_.asInt(radius_));
    }
    cy += 40.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Demo Concêntricas")) {
        circles_.clear();
        int cx0 = grid_.cols(gridRect) / 2;
        int cy0 = grid_.rows(gridRect) / 2;
        int maxR = std::max(6, std::min(cx0, cy0) - 2);
        for (int r = 4; r <= maxR; r += 4) {
            addCircle(cx0, cy0, r);
        }
    }
    cy += 42.0f;

    uiSliderInt({cx, cy, cw, 30.0f}, "Tamanho da Célula (px)", &cellSize_, 1, 24);
    cy += 42.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Mostrar Grade", &showGrid_);
    cy += 26.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Colorir 8 Octantes (Simetria-8)", &colorizeOctants_);
    cy += 34.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Limpar Tela")) {
        circles_.clear();
    }
    cy += 44.0f;

    drawUiText("Algoritmo (2ª Ordem):", cx, cy, 14.0f, theme::TextDark);
    cy += 20.0f;
    drawUiText("h = 1 - R, dE = 3, dSE = -2R + 5", cx, cy, 13.0f, theme::TextMuted);
    drawUiText("Somente inteiros, sem buracos!", cx, cy + 18.0f, 13.0f, theme::TextMuted);
}

} // namespace cg
