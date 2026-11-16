#include "aula07_circ_pixels.hpp"

#include <algorithm>
#include <cmath>

namespace cg {

// 1) Equação cartesiana: percorre x de -R até R e calcula y = sqrt(R² - x²) espelhando (+y, -y).
std::vector<GridPoint> rasterizeCircleEquation(int xc, int yc, int radius) {
    std::vector<GridPoint> pts;
    if (radius < 0) return pts;
    if (radius == 0) {
        pts.push_back({xc, yc});
        return pts;
    }

    // Percorre o intervalo [-R, R] e calcula a ordenada correspondente; produz lacunas nas bordas |x| ~ R.
    pts.reserve(static_cast<size_t>(4 * radius + 2));
    for (int x = -radius; x <= radius; ++x) {
        float y = std::sqrt(static_cast<float>(radius * radius - x * x));
        int yi = static_cast<int>(std::round(y));
        pts.push_back({xc + x, yc + yi});
        pts.push_back({xc + x, yc - yi});
    }
    return pts;
}

// 2) Equação paramétrica: x = R*cos(a), y = R*sin(a) com ângulo variando de 0 até 2*PI.
std::vector<GridPoint> rasterizeCircleParametric(int xc, int yc, int radius, float angleStepRad) {
    std::vector<GridPoint> pts;
    if (radius < 0) return pts;
    if (radius == 0) {
        pts.push_back({xc, yc});
        return pts;
    }

    float step = std::clamp(angleStepRad, 0.002f, 1.0f);
    for (float a = 0.0f; a <= 2.0f * PI + 1e-4f; a += step) {
        float x = static_cast<float>(radius) * std::cos(a);
        float y = static_cast<float>(radius) * std::sin(a);
        pts.push_back({xc + static_cast<int>(std::round(x)), yc + static_cast<int>(std::round(y))});
    }
    return pts;
}

// 3) Rotação incremental de 1 grau: parte de (R, 0) e aplica matriz de rotação 360 vezes.
std::vector<GridPoint> rasterizeCircleIncrementalRotation(int xc, int yc, int radius) {
    std::vector<GridPoint> pts;
    if (radius < 0) return pts;
    if (radius == 0) {
        pts.push_back({xc, yc});
        return pts;
    }

    // Inicia no ponto (R, 0) e gira 1 grau por iteração usando cos(1°) e sen(1°).
    float x = static_cast<float>(radius);
    float y = 0.0f;
    const float cos1 = std::cos(DEG2RAD * 1.0f);
    const float sin1 = std::sin(DEG2RAD * 1.0f);

    pts.reserve(361);
    pts.push_back({xc + static_cast<int>(std::round(x)), yc + static_cast<int>(std::round(y))});
    for (int i = 1; i <= 360; ++i) {
        // Matriz de rotação 2D: x' = x*cos - y*sen; y' = x*sen + y*cos.
        float xn = x * cos1 - y * sin1;
        y = x * sin1 + y * cos1;
        x = xn;
        pts.push_back({xc + static_cast<int>(std::round(x)), yc + static_cast<int>(std::round(y))});
    }
    return pts;
}

void Aula07CircPixelsModule::onEnter() {
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

void Aula07CircPixelsModule::syncInputs() {
    inXc_.setInt(xc_);
    inYc_.setInt(yc_);
    inR_.setInt(radius_);
}

void Aula07CircPixelsModule::addCircle(int xc, int yc, int radius) {
    xc_ = xc;
    yc_ = yc;
    radius_ = std::max(0, radius);
    syncInputs();
    circles_.push_back({xc_, yc_, radius_, method_, paramStep_});
}

void Aula07CircPixelsModule::update(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 305.0f;
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

void Aula07CircPixelsModule::draw(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 305.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    Rectangle ctrlCard   = {canvasCard.x + canvasCard.width + pad, area.y + pad, ctrlW, area.height - 2 * pad};

    drawPanelCard(canvasCard, "Grade de Desenho (Clique ou arraste centro -> raio)");
    drawPanelCard(ctrlCard, "Controles - Aula 07");

    Rectangle gridRect = {canvasCard.x + 8.0f, canvasCard.y + 40.0f, canvasCard.width - 16.0f, canvasCard.height - 52.0f};
    grid_.setCellSize(cellSize_);
    grid_.drawBackgroundAndGrid(gridRect, showGrid_, 5);

    BeginScissorMode(static_cast<int>(gridRect.x), static_cast<int>(gridRect.y),
                     static_cast<int>(gridRect.width), static_cast<int>(gridRect.height));

    for (const auto& c : circles_) {
        if (c.method == Method::Equation || c.method == Method::CompareAll) {
            auto pts = rasterizeCircleEquation(c.xc, c.yc, c.radius);
            for (const auto& p : pts) grid_.drawPixel(gridRect, p.x, p.y, theme::Accent);
        }
        if (c.method == Method::Parametric || c.method == Method::CompareAll) {
            auto pts = rasterizeCircleParametric(c.xc, c.yc, c.radius, c.paramStep);
            Color col = (c.method == Method::CompareAll) ? Color{22, 163, 74, 180} : theme::Success;
            for (const auto& p : pts) grid_.drawPixel(gridRect, p.x, p.y, col);
        }
        if (c.method == Method::Rotation1Deg || c.method == Method::CompareAll) {
            auto pts = rasterizeCircleIncrementalRotation(c.xc, c.yc, c.radius);
            Color col = (c.method == Method::CompareAll) ? Color{220, 38, 38, 180} : theme::Danger;
            for (const auto& p : pts) grid_.drawPixel(gridRect, p.x, p.y, col);
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

    // Painel lateral de parâmetros e seleção do método de circunferência.
    float cx = ctrlCard.x + 14.0f;
    float cw = ctrlCard.width - 28.0f;
    float cy = ctrlCard.y + 44.0f;

    drawUiText("Método:", cx, cy, 14.0f, theme::TextDark);
    cy += 24.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "1) Equação y = sqrt(R² - x²) (Azul)", method_ == Method::Equation)) {
        method_ = Method::Equation;
    }
    cy += 26.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "2) Paramétrica cos/sin (Verde)", method_ == Method::Parametric)) {
        method_ = Method::Parametric;
    }
    cy += 26.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "3) Rotação Incremental 1° (Vermelho)", method_ == Method::Rotation1Deg)) {
        method_ = Method::Rotation1Deg;
    }
    cy += 26.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "Comparar Todos (Sobrepostos)", method_ == Method::CompareAll)) {
        method_ = Method::CompareAll;
    }
    cy += 34.0f;

    uiSliderFloat({cx, cy, cw, 30.0f}, "Passo Paramétrico (rad)", &paramStep_, 0.005f, 0.25f, "%.3f");
    cy += 48.0f;

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
    cy += 42.0f;

    uiSliderInt({cx, cy, cw, 30.0f}, "Tamanho da Célula (px)", &cellSize_, 1, 24);
    cy += 42.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Mostrar Grade", &showGrid_);
    cy += 32.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Limpar Tela")) {
        circles_.clear();
    }
    cy += 40.0f;

    drawUiText("Obs.: Na Equação (1), note as", cx, cy, 13.0f, theme::TextMuted);
    drawUiText("falhas laterais perto de |x| = R!", cx, cy + 16.0f, 13.0f, theme::TextMuted);
}

} // namespace cg
