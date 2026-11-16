#include "ui_widgets.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace cg {

namespace {
Font g_uiFont{};
bool g_hasCustomFont = false;
UiTextInput* g_focusedInput = nullptr;
} // namespace

/**
 * @brief Tenta converter o texto armazenado para double, ignorando estados parciais de digitação.
 */
double UiTextInput::asDouble(double fallback) const {
    if (text.empty() || text == "-" || text == "." || text == "-.") {
        return fallback;
    }
    char* end = nullptr;
    double v = std::strtod(text.c_str(), &end);
    if (end == text.c_str()) {
        return fallback;
    }
    return v;
}

/**
 * @brief Tenta converter o texto armazenado para inteiro na base 10.
 */
int UiTextInput::asInt(int fallback) const {
    if (text.empty() || text == "-") {
        return fallback;
    }
    char* end = nullptr;
    long v = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str()) {
        return fallback;
    }
    return static_cast<int>(v);
}

void UiTextInput::setDouble(double value, int decimals) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, value);
    text = buf;
}

void UiTextInput::setInt(int value) {
    text = std::to_string(value);
}

/**
 * @brief Procura uma fonte TTF do sistema compatível com português e faz fallback para a fonte padrão.
 */
void initUiFonts() {
    // Prioriza a fonte embutida em assets/ (essencial para WebAssembly) e mantém caminhos do sistema Linux.
    const char* candidates[] = {
        "assets/LiberationSans-Regular.ttf",
        "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf",
        "/usr/share/fonts/adwaita-sans-fonts/AdwaitaSans-Regular.ttf"
    };

    // Carrega os glifos ASCII e o bloco Latin-1 (32..255) para exibir acentos e o símbolo de grau (°).
    std::vector<int> codepoints;
    codepoints.reserve(224);
    for (int cp = 32; cp <= 255; ++cp) {
        codepoints.push_back(cp);
    }

    for (const char* path : candidates) {
        if (FileExists(path)) {
            g_uiFont = LoadFontEx(path, 28, codepoints.data(), static_cast<int>(codepoints.size()));
            if (g_uiFont.texture.id != 0) {
                SetTextureFilter(g_uiFont.texture, TEXTURE_FILTER_BILINEAR);
                g_hasCustomFont = true;
                return;
            }
        }
    }

    g_uiFont = GetFontDefault();
    g_hasCustomFont = false;
}

void unloadUiFonts() {
    if (g_hasCustomFont && g_uiFont.texture.id != 0) {
        UnloadFont(g_uiFont);
        g_hasCustomFont = false;
    }
    g_focusedInput = nullptr;
}

const Font& getUiFont() {
    return g_uiFont;
}

bool isAnyTextInputFocused() {
    return g_focusedInput != nullptr && g_focusedInput->focused;
}

void clearTextInputFocus() {
    if (g_focusedInput) {
        g_focusedInput->focused = false;
        g_focusedInput = nullptr;
    }
}

void drawUiText(const char* text, float x, float y, float fontSize, Color color) {
    if (!text) return;
    if (g_hasCustomFont) {
        DrawTextEx(g_uiFont, text, {x, y}, fontSize, 1.0f, color);
    } else {
        DrawText(text, static_cast<int>(x), static_cast<int>(y), static_cast<int>(fontSize), color);
    }
}

float measureUiText(const char* text, float fontSize) {
    if (!text) return 0.0f;
    if (g_hasCustomFont) {
        return MeasureTextEx(g_uiFont, text, fontSize, 1.0f).x;
    }
    return static_cast<float>(MeasureText(text, static_cast<int>(fontSize)));
}

bool uiButton(Rectangle rect, const char* label, bool selected, bool accent) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // Define cores conforme estado: destaque primário, selecionado, hover ou padrão.
    Color bg = WHITE;
    Color border = theme::PanelBorder;
    Color fg = theme::TextDark;

    if (accent) {
        bg = hovered ? theme::AccentHover : theme::Accent;
        border = bg;
        fg = WHITE;
    } else if (selected) {
        bg = {219, 234, 254, 255};
        border = theme::Accent;
        fg = theme::AccentHover;
    } else if (hovered) {
        bg = {241, 245, 249, 255};
        border = {148, 163, 184, 255};
    }

    DrawRectangleRounded(rect, 0.18f, 6, bg);
    DrawRectangleRoundedLinesEx(rect, 0.18f, 6, 1.5f, border);

    // Centraliza o rótulo horizontal e verticalmente dentro do botão.
    const float fontSize = 15.0f;
    float tw = measureUiText(label, fontSize);
    float tx = rect.x + (rect.width - tw) * 0.5f;
    float ty = rect.y + (rect.height - fontSize) * 0.5f;
    drawUiText(label, tx, ty, fontSize, fg);

    if (clicked) {
        clearTextInputFocus();
    }
    return clicked;
}

bool uiRadioButton(Vector2 center, float radius, const char* label, bool selected) {
    const float fontSize = 15.0f;
    float tw = measureUiText(label, fontSize);
    // Área de clique inclui tanto o círculo quanto o rótulo de texto ao lado.
    Rectangle hit = {
        center.x - radius - 4.0f,
        center.y - radius - 4.0f,
        radius * 2.0f + 12.0f + tw,
        radius * 2.0f + 8.0f
    };

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, hit);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    DrawCircleV(center, radius, WHITE);
    DrawCircleLinesV(center, radius, selected ? theme::Accent : (hovered ? theme::TextDark : theme::GridAxis));
    if (selected) {
        DrawCircleV(center, radius * 0.55f, theme::Accent);
    }

    drawUiText(label, center.x + radius + 8.0f, center.y - fontSize * 0.5f, fontSize, theme::TextDark);
    if (clicked) {
        clearTextInputFocus();
    }
    return clicked;
}

bool uiCheckBox(Rectangle box, const char* label, bool* checked) {
    const float fontSize = 15.0f;
    float tw = measureUiText(label, fontSize);
    Rectangle hit = {box.x, box.y, box.width + 10.0f + tw, std::max(box.height, fontSize + 4.0f)};

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, hit);
    bool toggled = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (toggled && checked) {
        *checked = !(*checked);
        clearTextInputFocus();
    }

    bool state = checked ? *checked : false;
    DrawRectangleRounded(box, 0.2f, 4, state ? theme::Accent : WHITE);
    DrawRectangleRoundedLinesEx(box, 0.2f, 4, 1.5f, state ? theme::Accent : (hovered ? theme::TextDark : theme::GridAxis));

    // Desenha o sinal de verificação (check) quando marcado.
    if (state) {
        Vector2 p1 = {box.x + box.width * 0.22f, box.y + box.height * 0.52f};
        Vector2 p2 = {box.x + box.width * 0.44f, box.y + box.height * 0.74f};
        Vector2 p3 = {box.x + box.width * 0.80f, box.y + box.height * 0.26f};
        DrawLineEx(p1, p2, 2.2f, WHITE);
        DrawLineEx(p2, p3, 2.2f, WHITE);
    }

    drawUiText(label, box.x + box.width + 8.0f, box.y + (box.height - fontSize) * 0.5f, fontSize, theme::TextDark);
    return toggled;
}

bool uiSliderInt(Rectangle rect, const char* label, int* value, int minVal, int maxVal) {
    if (!value || maxVal <= minVal) return false;
    int oldVal = *value;

    const float labelSize = 14.0f;
    char valBuf[64];
    std::snprintf(valBuf, sizeof(valBuf), "%s: %d", label, *value);
    drawUiText(valBuf, rect.x, rect.y, labelSize, theme::TextDark);

    Rectangle track = {rect.x, rect.y + 18.0f, rect.width, 10.0f};
    Rectangle hit = {rect.x - 6.0f, rect.y + 12.0f, rect.width + 12.0f, 22.0f};
    Vector2 mouse = GetMousePosition();

    // Ponteiro estático garante que apenas o slider capturado no clique acompanhe o arraste.
    static const int* activeSlider = nullptr;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, hit)) {
        activeSlider = value;
        clearTextInputFocus();
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (activeSlider == value) {
            activeSlider = nullptr;
        }
    }

    if (activeSlider == value && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float t = (mouse.x - track.x) / track.width;
        t = std::clamp(t, 0.0f, 1.0f);
        *value = minVal + static_cast<int>(std::round(t * (maxVal - minVal)));
    }

    float norm = static_cast<float>(*value - minVal) / static_cast<float>(maxVal - minVal);
    DrawRectangleRounded(track, 0.5f, 4, theme::GridMinor);
    Rectangle fill = {track.x, track.y, track.width * norm, track.height};
    if (fill.width > 1.0f) {
        DrawRectangleRounded(fill, 0.5f, 4, theme::Accent);
    }
    Vector2 knob = {track.x + track.width * norm, track.y + track.height * 0.5f};
    DrawCircleV(knob, 7.5f, WHITE);
    DrawCircleLinesV(knob, 7.5f, theme::Accent);

    return *value != oldVal;
}

bool uiSliderFloat(Rectangle rect, const char* label, float* value, float minVal, float maxVal, const char* fmt) {
    if (!value || maxVal <= minVal) return false;
    float oldVal = *value;

    const float labelSize = 14.0f;
    char numBuf[32];
    std::snprintf(numBuf, sizeof(numBuf), fmt, *value);
    char valBuf[96];
    std::snprintf(valBuf, sizeof(valBuf), "%s: %s", label, numBuf);
    drawUiText(valBuf, rect.x, rect.y, labelSize, theme::TextDark);

    Rectangle track = {rect.x, rect.y + 18.0f, rect.width, 10.0f};
    Rectangle hit = {rect.x - 6.0f, rect.y + 12.0f, rect.width + 12.0f, 22.0f};
    Vector2 mouse = GetMousePosition();

    static const float* activeSlider = nullptr;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, hit)) {
        activeSlider = value;
        clearTextInputFocus();
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (activeSlider == value) {
            activeSlider = nullptr;
        }
    }

    if (activeSlider == value && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float t = (mouse.x - track.x) / track.width;
        t = std::clamp(t, 0.0f, 1.0f);
        *value = minVal + t * (maxVal - minVal);
    }

    float norm = (*value - minVal) / (maxVal - minVal);
    norm = std::clamp(norm, 0.0f, 1.0f);
    DrawRectangleRounded(track, 0.5f, 4, theme::GridMinor);
    Rectangle fill = {track.x, track.y, track.width * norm, track.height};
    if (fill.width > 1.0f) {
        DrawRectangleRounded(fill, 0.5f, 4, theme::Accent);
    }
    Vector2 knob = {track.x + track.width * norm, track.y + track.height * 0.5f};
    DrawCircleV(knob, 7.5f, WHITE);
    DrawCircleLinesV(knob, 7.5f, theme::Accent);

    return std::fabs(*value - oldVal) > 1e-6f;
}

// Controla foco, captura de teclado e renderização de uma caixa de texto/número.
bool uiTextInput(Rectangle rect, UiTextInput& input, const char* label) {
    bool changed = false;
    const float fontSize = 15.0f;
    if (label && label[0] != '\0') {
        drawUiText(label, rect.x, rect.y - 18.0f, 13.0f, theme::TextMuted);
    }

    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (hovered) {
            if (g_focusedInput && g_focusedInput != &input) {
                g_focusedInput->focused = false;
            }
            input.focused = true;
            g_focusedInput = &input;
        } else if (input.focused) {
            input.focused = false;
            if (g_focusedInput == &input) {
                g_focusedInput = nullptr;
            }
        }
    }

    if (input.focused) {
        int key = GetCharPressed();
        while (key > 0) {
            bool accept = false;
            if (key >= 32 && key <= 126) {
                if (input.numericOnly) {
                    if (key >= '0' && key <= '9') accept = true;
                    else if (input.allowSign && (key == '-' || key == '+') && input.text.empty()) accept = true;
                    else if (input.allowFloat && key == '.' && input.text.find('.') == std::string::npos) accept = true;
                } else {
                    accept = true;
                }
            }
            if (accept && static_cast<int>(input.text.size()) < input.maxChars) {
                input.text.push_back(static_cast<char>(key));
                changed = true;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
            if (!input.text.empty()) {
                input.text.pop_back();
                changed = true;
            }
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
            input.focused = false;
            if (g_focusedInput == &input) {
                g_focusedInput = nullptr;
            }
        }
    }

    DrawRectangleRounded(rect, 0.14f, 4, WHITE);
    DrawRectangleRoundedLinesEx(rect, 0.14f, 4, 1.5f,
                                input.focused ? theme::Accent : (hovered ? theme::GridAxis : theme::PanelBorder));

    BeginScissorMode(static_cast<int>(rect.x + 4), static_cast<int>(rect.y + 2),
                     std::max(1, static_cast<int>(rect.width - 8)),
                     std::max(1, static_cast<int>(rect.height - 4)));

    float ty = rect.y + (rect.height - fontSize) * 0.5f;
    float tw = measureUiText(input.text.c_str(), fontSize);
    float tx = rect.x + 7.0f;
    if (tw > rect.width - 14.0f) {
        tx = rect.x + rect.width - 7.0f - tw;
    }
    drawUiText(input.text.c_str(), tx, ty, fontSize, theme::TextDark);

    if (input.focused && (static_cast<int>(GetTime() * 2.0) % 2 == 0)) {
        float cx = tx + tw + 1.0f;
        DrawLineEx({cx, rect.y + 5.0f}, {cx, rect.y + rect.height - 5.0f}, 1.5f, theme::Accent);
    }

    EndScissorMode();
    return changed;
}

void drawPanelCard(Rectangle rect, const char* title) {
    DrawRectangleRounded(rect, 0.03f, 6, WHITE);
    DrawRectangleRoundedLinesEx(rect, 0.03f, 6, 1.0f, theme::PanelBorder);
    if (title && title[0] != '\0') {
        drawUiText(title, rect.x + 12.0f, rect.y + 10.0f, 16.0f, theme::TextDark);
        DrawLineEx({rect.x + 10.0f, rect.y + 32.0f},
                   {rect.x + rect.width - 10.0f, rect.y + 32.0f}, 1.0f, theme::GridMinor);
    }
}

} // namespace cg
