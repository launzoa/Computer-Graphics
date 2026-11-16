#include "aula06_bresenham_reta.hpp"

#include <algorithm>
#include <cstdlib>

namespace cg {

// Algoritmo de Bresenham puramente inteiro para todos os 8 octantes.
std::vector<GridPoint> rasterizeLineBresenham(int x0, int y0, int x1, int y1) {
    std::vector<GridPoint> pts;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int incX = (x0 < x1) ? 1 : -1;
    int incY = (y0 < y1) ? 1 : -1;

    int x = x0;
    int y = y0;
    pts.reserve(static_cast<size_t>(std::max(dx, dy)) + 1);
    pts.push_back({x, y});

    if (dx >= dy) {
        // |inclinação| <= 1: avança sempre em X e decide se mantém Y (E) ou desloca Y (NE).
        // Variável de decisão inicial d = 2*dy - dx e incrementos inteiros dE / dNE.
        int d = 2 * dy - dx;
        int dE = 2 * dy;
        int dNE = 2 * (dy - dx);
        while (x != x1) {
            x += incX;
            if (d < 0) {
                d += dE; // Mantém Y atual (escolhe E)
            } else {
                y += incY;
                d += dNE; // Avança também em Y (escolhe NE)
            }
            pts.push_back({x, y});
        }
    } else {
        // |inclinação| > 1: avança sempre em Y e decide se mantém X (N) ou desloca X (NE).
        // Simetria dos parâmetros trocando os papéis de dx e dy.
        int d = 2 * dx - dy;
        int dN = 2 * dx;
        int dNE = 2 * (dx - dy);
        while (y != y1) {
            y += incY;
            if (d < 0) {
                d += dN; // Mantém X atual
            } else {
                x += incX;
                d += dNE; // Avança também em X
            }
            pts.push_back({x, y});
        }
    }

    return pts;
}

const char* describeLineOctant(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    if (dx == 0 && dy == 0) return "Ponto único";
    if (dy == 0) return (dx > 0) ? "Horizontal (+X)" : "Horizontal (-X)";
    if (dx == 0) return (dy > 0) ? "Vertical (+Y tela)" : "Vertical (-Y tela)";

    // Classificação considerando coordenadas de tela (+X direita, +Y baixo).
    if (std::abs(dx) >= std::abs(dy)) {
        if (dx > 0 && dy < 0) return "Octante 1 (dx>0, dy<0, |dx|>=|dy|)";
        if (dx < 0 && dy < 0) return "Octante 4 (dx<0, dy<0, |dx|>=|dy|)";
        if (dx < 0 && dy > 0) return "Octante 5 (dx<0, dy>0, |dx|>=|dy|)";
        return "Octante 8 (dx>0, dy>0, |dx|>=|dy|)";
    } else {
        if (dx > 0 && dy < 0) return "Octante 2 (dx>0, dy<0, |dy|>|dx|)";
        if (dx < 0 && dy < 0) return "Octante 3 (dx<0, dy<0, |dy|>|dx|)";
        if (dx < 0 && dy > 0) return "Octante 6 (dx<0, dy>0, |dy|>|dx|)";
        return "Octante 7 (dx>0, dy>0, |dy|>|dx|)";
    }
}

void Aula06BresenhamRetaModule::onEnter() {
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
        syncInputs();
        addLine(p0_, p1_, theme::Danger);
        initialized_ = true;
    }
}

void Aula06BresenhamRetaModule::syncInputs() {
    inX0_.setInt(p0_.x);
    inY0_.setInt(p0_.y);
    inX1_.setInt(p1_.x);
    inY1_.setInt(p1_.y);
}

void Aula06BresenhamRetaModule::addLine(GridPoint p0, GridPoint p1, Color color) {
    p0_ = p0;
    p1_ = p1;
    syncInputs();
    lines_.push_back({p0, p1, color});
}

void Aula06BresenhamRetaModule::update(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 295.0f;
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
            syncInputs();
        } else {
            addLine(firstPoint_, gp, theme::Danger);
            hasFirstPoint_ = false;
        }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        hasFirstPoint_ = false;
    }
}

void Aula06BresenhamRetaModule::draw(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 295.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    Rectangle ctrlCard   = {canvasCard.x + canvasCard.width + pad, area.y + pad, ctrlW, area.height - 2 * pad};

    drawPanelCard(canvasCard, "Grade de Desenho - Bresenham Retas (8 Octantes)");
    drawPanelCard(ctrlCard, "Controles - Aula 06");

    Rectangle gridRect = {canvasCard.x + 8.0f, canvasCard.y + 40.0f, canvasCard.width - 16.0f, canvasCard.height - 52.0f};
    grid_.setCellSize(cellSize_);
    grid_.drawBackgroundAndGrid(gridRect, showGrid_, 5);

    BeginScissorMode(static_cast<int>(gridRect.x), static_cast<int>(gridRect.y),
                     static_cast<int>(gridRect.width), static_cast<int>(gridRect.height));

    for (const auto& line : lines_) {
        auto pts = rasterizeLineBresenham(line.p0.x, line.p0.y, line.p1.x, line.p1.y);
        for (const auto& pt : pts) {
            grid_.drawPixel(gridRect, pt.x, pt.y, line.color);
        }
        if (showIdeal_) {
            DrawLineEx(grid_.gridToScreenCenter(line.p0.x, line.p0.y, gridRect),
                       grid_.gridToScreenCenter(line.p1.x, line.p1.y, gridRect), 1.5f, BLACK);
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

    // Painel lateral de controles e classificação do octante.
    float cx = ctrlCard.x + 14.0f;
    float cw = ctrlCard.width - 28.0f;
    float cy = ctrlCard.y + 44.0f;

    drawUiText("Coordenadas (Inteiros):", cx, cy, 14.0f, theme::TextDark);
    cy += 34.0f;
    float iw = (cw - 14.0f) * 0.5f;
    uiTextInput({cx, cy, iw, 28.0f}, inX0_, "x0");
    uiTextInput({cx + iw + 14.0f, cy, iw, 28.0f}, inY0_, "y0");
    cy += 48.0f;
    uiTextInput({cx, cy, iw, 28.0f}, inX1_, "x1");
    uiTextInput({cx + iw + 14.0f, cy, iw, 28.0f}, inY1_, "y1");
    cy += 38.0f;

    if (uiButton({cx, cy, cw, 32.0f}, "Traçar Reta (Bresenham)", false, true)) {
        addLine({inX0_.asInt(p0_.x), inY0_.asInt(p0_.y)},
                {inX1_.asInt(p1_.x), inY1_.asInt(p1_.y)}, theme::Danger);
    }
    cy += 40.0f;

    if (uiButton({cx, cy, cw, 32.0f}, "Demo 8 Octantes (Estrela)")) {
        lines_.clear();
        int cx0 = grid_.cols(gridRect) / 2;
        int cy0 = grid_.rows(gridRect) / 2;
        int rLong = std::max(8, std::min(cx0, cy0) - 4);
        int rShort = std::max(4, rLong / 2);

        const GridPoint targets[8] = {
            {cx0 + rLong, cy0 - rShort}, // Octante 1
            {cx0 + rShort, cy0 - rLong}, // Octante 2
            {cx0 - rShort, cy0 - rLong}, // Octante 3
            {cx0 - rLong, cy0 - rShort}, // Octante 4
            {cx0 - rLong, cy0 + rShort}, // Octante 5
            {cx0 - rShort, cy0 + rLong}, // Octante 6
            {cx0 + rShort, cy0 + rLong}, // Octante 7
            {cx0 + rLong, cy0 + rShort}  // Octante 8
        };
        const Color octColors[8] = {
            RED, ORANGE, GREEN, SKYBLUE, BLUE, PURPLE, MAGENTA, DARKGREEN
        };
        for (int i = 0; i < 8; ++i) {
            addLine({cx0, cy0}, targets[i], octColors[i]);
        }
        // Eixos cardinais (horizontal e vertical) para verificação completa.
        addLine({cx0, cy0}, {cx0 + rLong, cy0}, BLACK);
        addLine({cx0, cy0}, {cx0 - rLong, cy0}, BLACK);
        addLine({cx0, cy0}, {cx0, cy0 + rLong}, BLACK);
        addLine({cx0, cy0}, {cx0, cy0 - rLong}, BLACK);
    }
    cy += 42.0f;

    uiSliderInt({cx, cy, cw, 30.0f}, "Tamanho da Célula (px)", &cellSize_, 1, 24);
    cy += 42.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Mostrar Grade", &showGrid_);
    cy += 26.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Sobrepor Reta Ideal", &showIdeal_);
    cy += 34.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Limpar Tela")) {
        lines_.clear();
        hasFirstPoint_ = false;
    }
    cy += 44.0f;

    drawUiText("Última Reta:", cx, cy, 14.0f, theme::TextDark);
    cy += 20.0f;
    drawUiText(describeLineOctant(p0_.x, p0_.y, p1_.x, p1_.y), cx, cy, 13.0f, theme::AccentHover);
    cy += 20.0f;
    drawUiText(TextFormat("dx = %d, dy = %d", std::abs(p1_.x - p0_.x), std::abs(p1_.y - p0_.y)),
               cx, cy, 13.0f, theme::TextMuted);
    cy += 18.0f;
    drawUiText(TextFormat("Total de retas: %zu", lines_.size()), cx, cy, 13.0f, theme::TextMuted);
}

} // namespace cg
