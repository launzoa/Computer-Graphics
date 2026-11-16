#include "aula05_retas_pixels.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace cg {

// Desenho de reta pela equação geral (y = m*x + b ou x = x0 + (y - y0)/m para inclinações íngremes).
// Todos os cálculos são feitos em ponto flutuante (float), arredondando apenas ao gravar o pixel.
std::vector<GridPoint> rasterizeLineGeneralEq(int x0, int y0, int x1, int y1) {
    std::vector<GridPoint> pts;
    float fx0 = static_cast<float>(x0);
    float fy0 = static_cast<float>(y0);
    float fx1 = static_cast<float>(x1);
    float fy1 = static_cast<float>(y1);

    float dx = fx1 - fx0;
    float dy = fy1 - fy0;

    if (dx == 0.0f && dy == 0.0f) {
        // Caso degenerado: ponto único.
        pts.push_back({static_cast<int>(std::round(fx0)), static_cast<int>(std::round(fy0))});
        return pts;
    }

    if (std::fabs(dx) >= std::fabs(dy)) {
        // Inclinação suave (|dy| <= |dx|): percorre X e calcula y = m*x + b em float.
        float m = dy / dx;
        float b = fy0 - m * fx0;
        int step = (x0 <= x1) ? 1 : -1;
        for (int xi = x0;; xi += step) {
            float x = static_cast<float>(xi);
            float y = m * x + b;
            pts.push_back({static_cast<int>(std::round(x)), static_cast<int>(std::round(y))});
            if (xi == x1) break;
        }
    } else {
        // Inclinação íngreme (|dy| > |dx|, incluindo vertical): percorre Y e calcula x(y) para evitar lacunas.
        float invM = dx / dy;
        int step = (y0 <= y1) ? 1 : -1;
        for (int yi = y0;; yi += step) {
            float y = static_cast<float>(yi);
            float x = fx0 + invM * (y - fy0);
            pts.push_back({static_cast<int>(std::round(x)), static_cast<int>(std::round(y))});
            if (yi == y1) break;
        }
    }

    return pts;
}

// Desenho de reta pela equação paramétrica: x(t) = x0 + dx*t, y(t) = y0 + dy*t, com t em [0, 1].
// Os cálculos são mantidos em float e arredondados somente no momento de desenhar o pixel.
std::vector<GridPoint> rasterizeLineParametric(int x0, int y0, int x1, int y1) {
    std::vector<GridPoint> pts;
    float fx0 = static_cast<float>(x0);
    float fy0 = static_cast<float>(y0);
    float fx1 = static_cast<float>(x1);
    float fy1 = static_cast<float>(y1);

    float dx = fx1 - fx0;
    float dy = fy1 - fy0;
    int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0));

    if (steps == 0) {
        pts.push_back({static_cast<int>(std::round(fx0)), static_cast<int>(std::round(fy0))});
        return pts;
    }

    // Quantidade de passos definida pelo maior deslocamento garante ao menos 1 pixel por coluna/linha.
    pts.reserve(static_cast<size_t>(steps) + 1);
    for (int i = 0; i <= steps; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        float x = fx0 + dx * t;
        float y = fy0 + dy * t;
        // Arredonda somente ao gravar o pixel na grade.
        pts.push_back({static_cast<int>(std::round(x)), static_cast<int>(std::round(y))});
    }

    return pts;
}

void Aula05RetasPixelsModule::onEnter() {
    if (!initialized_) {
        auto setup = [](UiTextInput& in) {
            in.numericOnly = true;
            in.allowFloat = false;
            in.allowSign = false;
            in.maxChars = 5;
        };
        setup(inX0_);
        setup(inY0_);
        setup(inX1_);
        setup(inY1_);
        syncInputsFromPoints();
        addSegment(p0_, p1_);
        initialized_ = true;
    }
}

void Aula05RetasPixelsModule::syncInputsFromPoints() {
    inX0_.setInt(p0_.x);
    inY0_.setInt(p0_.y);
    inX1_.setInt(p1_.x);
    inY1_.setInt(p1_.y);
}

void Aula05RetasPixelsModule::addSegment(GridPoint p0, GridPoint p1) {
    p0_ = p0;
    p1_ = p1;
    syncInputsFromPoints();
    segments_.push_back({p0, p1, method_});
}

void Aula05RetasPixelsModule::update(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 290.0f;
    Rectangle gridRect = {area.x + pad + 8.0f, area.y + pad + 40.0f,
                          area.width - ctrlW - 3 * pad - 16.0f, area.height - 2 * pad - 52.0f};

    Vector2 mouse = GetMousePosition();
    if (grid_.containsScreenPoint(mouse, gridRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        clearTextInputFocus();
        GridPoint gp = grid_.screenToGrid(mouse, gridRect);
        if (!hasFirstPoint_) {
            firstPoint_ = gp;
            hasFirstPoint_ = true;
            p0_ = gp;
            syncInputsFromPoints();
        } else {
            addSegment(firstPoint_, gp);
            hasFirstPoint_ = false;
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        hasFirstPoint_ = false;
    }
}

void Aula05RetasPixelsModule::draw(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 290.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    Rectangle ctrlCard   = {canvasCard.x + canvasCard.width + pad, area.y + pad, ctrlW, area.height - 2 * pad};

    drawPanelCard(canvasCard, "Grade de Desenho (Clique em 2 pontos para traçar)");
    drawPanelCard(ctrlCard, "Parâmetros - Aula 05A");

    Rectangle gridRect = {canvasCard.x + 8.0f, canvasCard.y + 40.0f, canvasCard.width - 16.0f, canvasCard.height - 52.0f};
    grid_.setCellSize(cellSize_);
    grid_.drawBackgroundAndGrid(gridRect, showGrid_, 5);

    BeginScissorMode(static_cast<int>(gridRect.x), static_cast<int>(gridRect.y),
                     static_cast<int>(gridRect.width), static_cast<int>(gridRect.height));

    for (const auto& seg : segments_) {
        if (seg.method == Method::GeneralEq || seg.method == Method::Both) {
            auto pts = rasterizeLineGeneralEq(seg.p0.x, seg.p0.y, seg.p1.x, seg.p1.y);
            for (const auto& pt : pts) {
                grid_.drawPixel(gridRect, pt.x, pt.y, theme::Accent);
            }
        }
        if (seg.method == Method::Parametric || seg.method == Method::Both) {
            auto pts = rasterizeLineParametric(seg.p0.x, seg.p0.y, seg.p1.x, seg.p1.y);
            Color col = (seg.method == Method::Both) ? Color{220, 38, 38, 175} : theme::Success;
            for (const auto& pt : pts) {
                grid_.drawPixel(gridRect, pt.x, pt.y, col);
            }
        }
        if (showIdealLine_) {
            DrawLineEx(grid_.gridToScreenCenter(seg.p0.x, seg.p0.y, gridRect),
                       grid_.gridToScreenCenter(seg.p1.x, seg.p1.y, gridRect), 1.5f, Color{15, 23, 42, 180});
        }
    }

    if (hasFirstPoint_) {
        grid_.drawPixel(gridRect, firstPoint_.x, firstPoint_.y, theme::Warning);
        Vector2 mouse = GetMousePosition();
        if (grid_.containsScreenPoint(mouse, gridRect)) {
            GridPoint gp = grid_.screenToGrid(mouse, gridRect);
            DrawLineEx(grid_.gridToScreenCenter(firstPoint_.x, firstPoint_.y, gridRect),
                       grid_.gridToScreenCenter(gp.x, gp.y, gridRect), 1.5f, theme::Warning);
        }
    }

    EndScissorMode();
    grid_.drawHoverIndicator(gridRect, true);

    // Painel lateral de parâmetros e seleção do método.
    float cx = ctrlCard.x + 14.0f;
    float cw = ctrlCard.width - 28.0f;
    float cy = ctrlCard.y + 44.0f;

    drawUiText("Método de Rasterização:", cx, cy, 14.0f, theme::TextDark);
    cy += 24.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "A) Equação Geral (Azul)", method_ == Method::GeneralEq)) {
        method_ = Method::GeneralEq;
    }
    cy += 26.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "B) Equação Paramétrica (Verde)", method_ == Method::Parametric)) {
        method_ = Method::Parametric;
    }
    cy += 26.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "Ambas (Geral + Paramétrica)", method_ == Method::Both)) {
        method_ = Method::Both;
    }
    cy += 34.0f;

    drawUiText("Coordenadas Manuais (Grid):", cx, cy, 14.0f, theme::TextDark);
    cy += 34.0f;
    float iw = (cw - 14.0f) * 0.5f;
    uiTextInput({cx, cy, iw, 28.0f}, inX0_, "x0");
    uiTextInput({cx + iw + 14.0f, cy, iw, 28.0f}, inY0_, "y0");
    cy += 48.0f;
    uiTextInput({cx, cy, iw, 28.0f}, inX1_, "x1");
    uiTextInput({cx + iw + 14.0f, cy, iw, 28.0f}, inY1_, "y1");
    cy += 38.0f;

    if (uiButton({cx, cy, cw, 32.0f}, "Desenhar Reta", false, true)) {
        GridPoint a{inX0_.asInt(p0_.x), inY0_.asInt(p0_.y)};
        GridPoint b{inX1_.asInt(p1_.x), inY1_.asInt(p1_.y)};
        addSegment(a, b);
    }
    cy += 40.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Demo (Horiz, Vert, Diag, Íngreme)")) {
        int maxC = std::max(10, grid_.cols(gridRect) - 4);
        int maxR = std::max(10, grid_.rows(gridRect) - 4);
        addSegment({3, 4}, {maxC, 4});
        addSegment({4, 3}, {4, maxR});
        addSegment({6, 6}, {maxC, maxR / 2});
        addSegment({8, maxR}, {18, 6});
    }
    cy += 38.0f;

    uiSliderInt({cx, cy, cw, 30.0f}, "Tamanho da Célula (px)", &cellSize_, 1, 24);
    cy += 42.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Mostrar Grade", &showGrid_);
    cy += 26.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Mostrar Reta Ideal (Referência)", &showIdealLine_);
    cy += 34.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Limpar Tela")) {
        segments_.clear();
        hasFirstPoint_ = false;
    }
    cy += 40.0f;

    drawUiText(TextFormat("Segmentos desenhados: %zu", segments_.size()), cx, cy, 13.0f, theme::TextMuted);
    cy += 20.0f;
    drawUiText("Dica: clique esquerdo 2x na grade", cx, cy, 13.0f, theme::TextMuted);
    drawUiText("ou botão direito para cancelar.", cx, cy + 16.0f, 13.0f, theme::TextMuted);
}

} // namespace cg
