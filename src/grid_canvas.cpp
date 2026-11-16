#include "grid_canvas.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace cg {

GridCanvas::GridCanvas(int cellSize) : cellSize_(std::clamp(cellSize, 1, 40)) {}

void GridCanvas::setCellSize(int size) {
    cellSize_ = std::clamp(size, 1, 40);
}

int GridCanvas::cols(Rectangle area) const {
    return std::max(1, static_cast<int>(area.width) / cellSize_);
}

int GridCanvas::rows(Rectangle area) const {
    return std::max(1, static_cast<int>(area.height) / cellSize_);
}

bool GridCanvas::containsScreenPoint(Vector2 screenPos, Rectangle area) const {
    return CheckCollisionPointRec(screenPos, area);
}

GridPoint GridCanvas::screenToGrid(Vector2 screenPos, Rectangle area) const {
    // Converte deslocamento relativo ao canto da grade em índice inteiro de célula.
    int gx = static_cast<int>(std::floor((screenPos.x - area.x) / static_cast<float>(cellSize_)));
    int gy = static_cast<int>(std::floor((screenPos.y - area.y) / static_cast<float>(cellSize_)));
    gx = std::clamp(gx, 0, cols(area) - 1);
    gy = std::clamp(gy, 0, rows(area) - 1);
    return {gx, gy};
}

Vector2 GridCanvas::gridToScreenCenter(int gx, int gy, Rectangle area) const {
    float half = cellSize_ * 0.5f;
    return {
        area.x + static_cast<float>(gx * cellSize_) + half,
        area.y + static_cast<float>(gy * cellSize_) + half
    };
}

Rectangle GridCanvas::gridToScreenRect(int gx, int gy, Rectangle area) const {
    return {
        area.x + static_cast<float>(gx * cellSize_),
        area.y + static_cast<float>(gy * cellSize_),
        static_cast<float>(cellSize_),
        static_cast<float>(cellSize_)
    };
}

bool GridCanvas::isInside(int gx, int gy, Rectangle area) const {
    return gx >= 0 && gy >= 0 && gx < cols(area) && gy < rows(area);
}

void GridCanvas::drawBackgroundAndGrid(Rectangle area, bool showGrid, int majorStep) const {
    DrawRectangleRec(area, theme::CanvasBg);

    // Recorta o desenho para impedir que linhas ou pixels vazem além da borda da grade.
    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y),
                     static_cast<int>(area.width), static_cast<int>(area.height));

    if (showGrid && cellSize_ >= 4) {
        // Desenha linhas verticais e horizontais destacando os múltiplos de majorStep.
        int maxC = cols(area);
        int maxR = rows(area);
        for (int c = 0; c <= maxC; ++c) {
            float x = area.x + static_cast<float>(c * cellSize_);
            bool major = (majorStep > 0 && c % majorStep == 0);
            DrawLineV({x, area.y}, {x, area.y + area.height}, major ? theme::GridMajor : theme::GridMinor);
        }
        for (int r = 0; r <= maxR; ++r) {
            float y = area.y + static_cast<float>(r * cellSize_);
            bool major = (majorStep > 0 && r % majorStep == 0);
            DrawLineV({area.x, y}, {area.x + area.width, y}, major ? theme::GridMajor : theme::GridMinor);
        }
    } else if (showGrid && cellSize_ < 4) {
        // No modo de pixel fino (célula < 4px), desenha linhas de referência a cada 25 px para evitar poluição visual.
        int step = 25;
        for (int x = step; x < static_cast<int>(area.width); x += step) {
            DrawLineV({area.x + x, area.y}, {area.x + x, area.y + area.height}, theme::GridMinor);
        }
        for (int y = step; y < static_cast<int>(area.height); y += step) {
            DrawLineV({area.x, area.y + y}, {area.x + area.width, area.y + y}, theme::GridMinor);
        }
    }

    EndScissorMode();
    DrawRectangleLinesEx(area, 1.5f, theme::PanelBorder);
}

// Desenha um pixel discreto na célula (gx, gy), respeitando os limites visíveis da área.
void GridCanvas::drawPixel(Rectangle area, int gx, int gy, Color color) const {
    if (!isInside(gx, gy, area)) return;
    if (cellSize_ == 1) {
        DrawPixel(static_cast<int>(area.x) + gx, static_cast<int>(area.y) + gy, color);
    } else {
        // Deixa 1 px de folga quando ampliado para destacar a separação entre células da grade.
        float pad = (cellSize_ >= 8) ? 1.0f : 0.0f;
        DrawRectangle(
            static_cast<int>(area.x) + gx * cellSize_,
            static_cast<int>(area.y) + gy * cellSize_,
            std::max(1, cellSize_ - static_cast<int>(pad)),
            std::max(1, cellSize_ - static_cast<int>(pad)),
            color
        );
    }
}

void GridCanvas::drawPixels(Rectangle area, const std::vector<GridPixel>& pixels) const {
    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y),
                     static_cast<int>(area.width), static_cast<int>(area.height));
    for (const auto& p : pixels) {
        drawPixel(area, p.x, p.y, p.color);
    }
    EndScissorMode();
}

void GridCanvas::drawHoverIndicator(Rectangle area, bool showLabel) const {
    Vector2 mouse = GetMousePosition();
    if (!containsScreenPoint(mouse, area)) return;

    GridPoint gp = screenToGrid(mouse, area);
    if (cellSize_ >= 4) {
        Rectangle cell = gridToScreenRect(gp.x, gp.y, area);
        DrawRectangleLinesEx(cell, 1.5f, theme::Accent);
    }

    if (showLabel) {
        char buf[48];
        std::snprintf(buf, sizeof(buf), "(%d, %d)", gp.x, gp.y);
        float tw = measureUiText(buf, 14.0f);
        Rectangle badge = {area.x + area.width - tw - 22.0f, area.y + 8.0f, tw + 14.0f, 24.0f};
        DrawRectangleRounded(badge, 0.25f, 4, {15, 23, 42, 200});
        drawUiText(buf, badge.x + 7.0f, badge.y + 4.0f, 14.0f, WHITE);
    }
}

} // namespace cg
