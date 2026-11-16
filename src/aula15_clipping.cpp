#include "aula15_clipping.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace cg {

// Calcula o código de região (outcode) de 4 bits: TOP(8) | BOTTOM(4) | RIGHT(2) | LEFT(1).
int computeCohenSutherlandCode(double x, double y, const ClipRect& win) {
    int code = CLIP_INSIDE;
    if (x < win.xmin)      code |= CLIP_LEFT;
    else if (x > win.xmax) code |= CLIP_RIGHT;

    if (y < win.ymin)      code |= CLIP_TOP;
    else if (y > win.ymax) code |= CLIP_BOTTOM;
    return code;
}

// Recorta o segmento (a, b) contra a janela win pelo algoritmo iterativo de Cohen-Sutherland.
ClipResult cohenSutherlandClip(Vector2 a, Vector2 b, const ClipRect& win) {
    ClipRect normWin{
        std::min(win.xmin, win.xmax),
        std::min(win.ymin, win.ymax),
        std::max(win.xmin, win.xmax),
        std::max(win.ymin, win.ymax)
    };

    double x0 = a.x, y0 = a.y;
    double x1 = b.x, y1 = b.y;

    int code0 = computeCohenSutherlandCode(x0, y0, normWin);
    int code1 = computeCohenSutherlandCode(x1, y1, normWin);

    ClipResult result{};
    result.initialCode0 = code0;
    result.initialCode1 = code1;

    while (true) {
        if ((code0 | code1) == 0) {
            // Aceite trivial: ambos os extremos estão dentro da janela.
            result.accepted = true;
            break;
        }
        if ((code0 & code1) != 0) {
            // Rejeição trivial: ambos os extremos compartilham uma região externa.
            result.accepted = false;
            break;
        }

        // Seleciona um ponto extremo fora da janela para calcular a interseção com a borda.
        int codeOut = (code0 != 0) ? code0 : code1;
        double x = 0.0;
        double y = 0.0;

        // Interseção com uma das quatro retas limites da janela (evitando divisão por zero).
        if (codeOut & CLIP_TOP) {
            x = (std::fabs(y1 - y0) > 1e-9) ? x0 + (x1 - x0) * (normWin.ymin - y0) / (y1 - y0) : x0;
            y = normWin.ymin;
        } else if (codeOut & CLIP_BOTTOM) {
            x = (std::fabs(y1 - y0) > 1e-9) ? x0 + (x1 - x0) * (normWin.ymax - y0) / (y1 - y0) : x0;
            y = normWin.ymax;
        } else if (codeOut & CLIP_RIGHT) {
            y = (std::fabs(x1 - x0) > 1e-9) ? y0 + (y1 - y0) * (normWin.xmax - x0) / (x1 - x0) : y0;
            x = normWin.xmax;
        } else if (codeOut & CLIP_LEFT) {
            y = (std::fabs(x1 - x0) > 1e-9) ? y0 + (y1 - y0) * (normWin.xmin - x0) / (x1 - x0) : y0;
            x = normWin.xmin;
        }

        // Substitui o extremo externo pelo ponto de interseção e recalcula o seu outcode.
        if (codeOut == code0) {
            x0 = x;
            y0 = y;
            code0 = computeCohenSutherlandCode(x0, y0, normWin);
        } else {
            x1 = x;
            y1 = y;
            code1 = computeCohenSutherlandCode(x1, y1, normWin);
        }
    }

    result.p0 = {static_cast<float>(x0), static_cast<float>(y0)};
    result.p1 = {static_cast<float>(x1), static_cast<float>(y1)};
    return result;
}

std::string formatOutCodeBinary(int code) {
    std::string s(4, '0');
    s[0] = (code & CLIP_TOP) ? '1' : '0';
    s[1] = (code & CLIP_BOTTOM) ? '1' : '0';
    s[2] = (code & CLIP_RIGHT) ? '1' : '0';
    s[3] = (code & CLIP_LEFT) ? '1' : '0';
    return s;
}

void Aula15ClippingModule::onEnter() {
    if (!initialized_) {
        stage_ = Stage::PickWinFirst;
        initialized_ = true;
    }
}

void Aula15ClippingModule::drawDashedLine(Vector2 a, Vector2 b, float dashLen, float gapLen, Color color) const {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float totalLen = std::sqrt(dx * dx + dy * dy);
    if (totalLen < 1e-3f) return;

    float ux = dx / totalLen;
    float uy = dy / totalLen;
    float dist = 0.0f;
    while (dist < totalLen) {
        float segEnd = std::min(totalLen, dist + dashLen);
        DrawLineEx(
            {a.x + ux * dist, a.y + uy * dist},
            {a.x + ux * segEnd, a.y + uy * segEnd},
            1.6f, color
        );
        dist += dashLen + gapLen;
    }
}

void Aula15ClippingModule::update(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 305.0f;
    Rectangle canvasRect = {area.x + pad + 8.0f, area.y + pad + 40.0f,
                            area.width - ctrlW - 3 * pad - 16.0f, area.height - 2 * pad - 52.0f};

    Vector2 mouse = GetMousePosition();
    bool inside = CheckCollisionPointRec(mouse, canvasRect);

    // Armazena coordenadas relativas ao canto superior esquerdo da área de desenho para suportar redimensionamento.
    Vector2 localPos = {
        std::clamp(mouse.x - canvasRect.x, 0.0f, canvasRect.width),
        std::clamp(mouse.y - canvasRect.y, 0.0f, canvasRect.height)
    };

    if (stage_ == Stage::PickWinFirst) {
        if (inside && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            clearTextInputFocus();
            winPt0_ = localPos;
            winPt1_ = localPos;
            stage_ = Stage::PickWinSecond;
        }
    } else if (stage_ == Stage::PickWinSecond) {
        if (inside) {
            winPt1_ = localPos;
        }
        if (inside && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            winPt1_ = localPos;
            clipWin_ = {
                std::min(winPt0_.x, winPt1_.x),
                std::min(winPt0_.y, winPt1_.y),
                std::max(winPt0_.x, winPt1_.x),
                std::max(winPt0_.y, winPt1_.y)
            };
            // Garante tamanho mínimo não degenerado para a janela de recorte.
            if (clipWin_.xmax - clipWin_.xmin < 8.0) clipWin_.xmax = clipWin_.xmin + 8.0;
            if (clipWin_.ymax - clipWin_.ymin < 8.0) clipWin_.ymax = clipWin_.ymin + 8.0;
            stage_ = Stage::DrawLines;
        }
    } else if (stage_ == Stage::DrawLines) {
        if (inside && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            clearTextInputFocus();
            draggingLine_ = true;
            lineStart_ = localPos;
            lineEnd_ = localPos;
        }
        if (draggingLine_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            lineEnd_ = localPos;
        }
        if (draggingLine_ && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            lineEnd_ = localPos;
            ClipResult res = cohenSutherlandClip(lineStart_, lineEnd_, clipWin_);
            lines_.push_back({lineStart_, lineEnd_, res});
            draggingLine_ = false;
        }
    }
}

void Aula15ClippingModule::draw(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 305.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    Rectangle ctrlCard   = {canvasCard.x + canvasCard.width + pad, area.y + pad, ctrlW, area.height - 2 * pad};

    drawPanelCard(canvasCard, "Área de Desenho (Recorte Cohen-Sutherland)");
    drawPanelCard(ctrlCard, "Controles - Aula 15c");

    Rectangle canvasRect = {canvasCard.x + 8.0f, canvasCard.y + 40.0f, canvasCard.width - 16.0f, canvasCard.height - 52.0f};
    grid_.drawBackgroundAndGrid(canvasRect, showGrid_, 5);

    auto toScreen = [&](Vector2 local) -> Vector2 {
        return {canvasRect.x + local.x, canvasRect.y + local.y};
    };

    BeginScissorMode(static_cast<int>(canvasRect.x), static_cast<int>(canvasRect.y),
                     static_cast<int>(canvasRect.width), static_cast<int>(canvasRect.height));

    // Desenha a janela de recorte e as retas já processadas
    if (stage_ == Stage::PickWinSecond) {
        Vector2 a = toScreen(winPt0_);
        Vector2 b = toScreen(winPt1_);
        Rectangle preview = {std::min(a.x, b.x), std::min(a.y, b.y), std::fabs(b.x - a.x), std::fabs(b.y - a.y)};
        DrawRectangleRec(preview, Color{37, 99, 235, 22});
        DrawRectangleLinesEx(preview, 2.0f, theme::Accent);
        DrawCircleV(a, 4.5f, theme::Accent);
    } else if (stage_ == Stage::DrawLines) {
        Rectangle winRect = {
            canvasRect.x + static_cast<float>(clipWin_.xmin),
            canvasRect.y + static_cast<float>(clipWin_.ymin),
            static_cast<float>(clipWin_.xmax - clipWin_.xmin),
            static_cast<float>(clipWin_.ymax - clipWin_.ymin)
        };
        DrawRectangleRec(winRect, Color{255, 255, 255, 190});
        DrawRectangleLinesEx(winRect, 2.2f, theme::TextDark);

        for (const auto& rec : lines_) {
            if (showDiscardedDashed_) {
                drawDashedLine(toScreen(rec.raw0), toScreen(rec.raw1), 7.0f, 5.0f, theme::GridAxis);
            }
            if (rec.res.accepted) {
                DrawLineEx(toScreen(rec.res.p0), toScreen(rec.res.p1), 2.5f, theme::Accent);
            }
        }

        if (draggingLine_) {
            DrawLineEx(toScreen(lineStart_), toScreen(lineEnd_), 1.5f, theme::Warning);
            DrawCircleV(toScreen(lineStart_), 4.0f, theme::Warning);
            DrawCircleV(toScreen(lineEnd_), 4.0f, theme::Warning);
        }
    }

    EndScissorMode();
    DrawRectangleLinesEx(canvasRect, 1.5f, theme::PanelBorder);

    // Faixa superior com instrução da etapa atual
    const char* prompt = "";
    Color promptCol = theme::Accent;
    if (stage_ == Stage::PickWinFirst) {
        prompt = "Passo 1: Clique no 1º ponto da janela de visualização";
    } else if (stage_ == Stage::PickWinSecond) {
        prompt = "Passo 2: Clique no 2º ponto para fechar a janela";
        promptCol = theme::Warning;
    } else {
        prompt = "Passo 3: Clique, segure, arraste e solte para traçar uma reta";
        promptCol = theme::Success;
    }
    float tw = measureUiText(prompt, 14.0f);
    DrawRectangleRounded({canvasRect.x + 10.0f, canvasRect.y + 8.0f, tw + 18.0f, 26.0f}, 0.2f, 4, {15, 23, 42, 210});
    drawUiText(prompt, canvasRect.x + 19.0f, canvasRect.y + 13.0f, 14.0f, WHITE);
    (void)promptCol;

    // Painel lateral de controles
    float cx = ctrlCard.x + 14.0f;
    float cw = ctrlCard.width - 28.0f;
    float cy = ctrlCard.y + 44.0f;

    drawUiText("Fluxo de Uso:", cx, cy, 14.0f, theme::TextDark);
    cy += 22.0f;
    drawUiText("1. Clique em 2 pontos na grade", cx, cy, 13.0f, theme::TextMuted);
    drawUiText("   para definir a janela de recorte.", cx, cy + 16.0f, 13.0f, theme::TextMuted);
    drawUiText("2. Arraste e solte o mouse para", cx, cy + 34.0f, 13.0f, theme::TextMuted);
    drawUiText("   desenhar uma reta recortada.", cx, cy + 50.0f, 13.0f, theme::TextMuted);
    cy += 78.0f;

    if (uiButton({cx, cy, cw, 32.0f}, "Redefinir Janela (2 cliques)", false, true)) {
        stage_ = Stage::PickWinFirst;
        lines_.clear();
        draggingLine_ = false;
    }
    cy += 40.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Demo Janela + Retas Exemplo")) {
        float w = canvasRect.width;
        float h = canvasRect.height;
        clipWin_ = {w * 0.25, h * 0.25, w * 0.75, h * 0.75};
        stage_ = Stage::DrawLines;
        lines_.clear();
        Vector2 demos[][2] = {
            {{w * 0.10f, h * 0.60f}, {w * 0.35f, h * 0.50f}}, // parcialmente dentro
            {{w * 0.35f, h * 0.40f}, {w * 0.65f, h * 0.35f}}, // totalmente dentro
            {{w * 0.45f, h * 0.85f}, {w * 0.85f, h * 0.45f}}, // diagonal cruzando 2 bordas
            {{w * 0.10f, h * 0.15f}, {w * 0.30f, h * 0.20f}}, // fora (canto superior esquerdo)
            {{w * 0.60f, h * 0.12f}, {w * 0.90f, h * 0.18f}}  // totalmente fora (acima)
        };
        for (const auto& d : demos) {
            lines_.push_back({d[0], d[1], cohenSutherlandClip(d[0], d[1], clipWin_)});
        }
    }
    cy += 38.0f;

    if (uiButton({cx, cy, cw, 30.0f}, "Limpar Retas")) {
        lines_.clear();
    }
    cy += 40.0f;

    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Mostrar parte recortada (tracejada)", &showDiscardedDashed_);
    cy += 26.0f;
    uiCheckBox({cx, cy, 18.0f, 18.0f}, "Mostrar Grade de Fundo", &showGrid_);
    cy += 38.0f;

    drawUiText("Última Reta (Outcodes):", cx, cy, 14.0f, theme::TextDark);
    cy += 22.0f;
    if (!lines_.empty()) {
        const auto& last = lines_.back();
        drawUiText(TextFormat("Código P0: %s (%d)", formatOutCodeBinary(last.res.initialCode0).c_str(), last.res.initialCode0),
                   cx, cy, 13.0f, theme::TextMuted);
        drawUiText(TextFormat("Código P1: %s (%d)", formatOutCodeBinary(last.res.initialCode1).c_str(), last.res.initialCode1),
                   cx, cy + 18.0f, 13.0f, theme::TextMuted);
        drawUiText(last.res.accepted ? "Status: ACEITA (visível)" : "Status: REJEITADA (fora)",
                   cx, cy + 38.0f, 14.0f, last.res.accepted ? theme::Success : theme::Danger);
    } else {
        drawUiText("Nenhuma reta traçada ainda.", cx, cy, 13.0f, theme::TextMuted);
    }
}

} // namespace cg
