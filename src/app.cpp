#include "app.hpp"

#include "aula03_cores.hpp"
#include "aula04_pixels.hpp"
#include "aula05_retas_pixels.hpp"
#include "aula06_bresenham_reta.hpp"
#include "aula07_circ_pixels.hpp"
#include "aula08_bresenham_circ.hpp"
#include "aula12_projecoes.hpp"
#include "aula15_clipping.hpp"
#include "ui_widgets.hpp"

#include <algorithm>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace cg {

namespace {
const Color kHomePalette[] = {
    {37, 99, 235, 255},
    {220, 38, 38, 255},
    {22, 163, 74, 255},
    {217, 119, 6, 255},
    {15, 23, 42, 255},
    {147, 51, 234, 255}
};
} // namespace

void HomeGridModule::setOrErasePixel(int gx, int gy, bool erase) {
    // Procura se já existe um pixel pintado nessa coordenada da grade.
    auto it = std::find_if(pixels_.begin(), pixels_.end(), [gx, gy](const GridPixel& p) {
        return p.x == gx && p.y == gy;
    });

    if (erase) {
        if (it != pixels_.end()) {
            pixels_.erase(it);
        }
        return;
    }

    Color col = kHomePalette[colorIdx_];
    if (it != pixels_.end()) {
        it->color = col;
    } else {
        pixels_.push_back({gx, gy, col});
    }
}

void HomeGridModule::update(Rectangle area) {
    float pad = 16.0f;
    Rectangle gridRect = {area.x + pad + 8.0f, area.y + pad + 52.0f,
                          area.width - 2 * pad - 16.0f, area.height - 2 * pad - 62.0f};

    Vector2 mouse = GetMousePosition();
    if (!grid_.containsScreenPoint(mouse, gridRect)) return;

    // Botão esquerdo pinta a célula sob o cursor; botão direito apaga.
    GridPoint gp = grid_.screenToGrid(mouse, gridRect);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        setOrErasePixel(gp.x, gp.y, false);
    } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        setOrErasePixel(gp.x, gp.y, true);
    }
}

void HomeGridModule::draw(Rectangle area) {
    float pad = 16.0f;
    Rectangle card = {area.x + pad, area.y + pad, area.width - 2 * pad, area.height - 2 * pad};
    drawPanelCard(card, nullptr);

    // Barra de ferramentas superior dentro do cartão da grade inicial.
    float tx = card.x + 14.0f;
    float ty = card.y + 12.0f;

    drawUiText("Cor:", tx, ty + 6.0f, 14.0f, theme::TextDark);
    tx += 38.0f;
    for (int i = 0; i < 6; ++i) {
        Rectangle sw = {tx + i * 32.0f, ty + 2.0f, 24.0f, 24.0f};
        DrawRectangleRec(sw, kHomePalette[i]);
        DrawRectangleLinesEx(sw, i == colorIdx_ ? 2.5f : 1.0f, i == colorIdx_ ? theme::TextDark : theme::GridAxis);
        if (CheckCollisionPointRec(GetMousePosition(), sw) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            colorIdx_ = i;
        }
    }
    tx += 6 * 32.0f + 20.0f;

    uiSliderInt({tx, ty - 2.0f, 175.0f, 30.0f}, "Célula (px)", &cellSize_, 4, 28);
    tx += 195.0f;

    uiCheckBox({tx, ty + 5.0f, 18.0f, 18.0f}, "Mostrar Grade", &showGrid_);
    tx += 140.0f;

    if (uiButton({tx, ty, 110.0f, 28.0f}, "Limpar Grade")) {
        pixels_.clear();
    }

    Rectangle gridRect = {card.x + 8.0f, card.y + 52.0f, card.width - 16.0f, card.height - 62.0f};
    grid_.setCellSize(cellSize_);
    grid_.drawBackgroundAndGrid(gridRect, showGrid_, 5);
    grid_.drawPixels(gridRect, pixels_);
    grid_.drawHoverIndicator(gridRect, true);
}

App::App() {
    // Configura janela redimensionável (1280x760 para caber bem no navegador) e impede que ESC feche o app.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 760, "Computação Gráfica - Trabalhos Práticos (C++ / raylib)");
    SetWindowMinSize(1024, 640);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    initUiFonts();

    // Registra a grade inicial e todos os módulos práticos das aulas em ordem.
    modules_.push_back(std::make_unique<HomeGridModule>());
    modules_.push_back(std::make_unique<Aula03CoresModule>());
    modules_.push_back(std::make_unique<Aula04PixelsModule>());
    modules_.push_back(std::make_unique<Aula05RetasPixelsModule>());
    modules_.push_back(std::make_unique<Aula06BresenhamRetaModule>());
    modules_.push_back(std::make_unique<Aula07CircPixelsModule>());
    modules_.push_back(std::make_unique<Aula08BresenhamCircModule>());
    modules_.push_back(std::make_unique<Aula12ProjecoesModule>());
    modules_.push_back(std::make_unique<Aula15ClippingModule>());

    selectModule(0);
}

App::~App() {
    modules_.clear();
    unloadUiFonts();
    CloseWindow();
}

void App::selectModule(int index) {
    if (index < 0 || index >= static_cast<int>(modules_.size())) return;
    clearTextInputFocus();
    activeIndex_ = index;
    modules_[static_cast<size_t>(activeIndex_)]->onEnter();
}

void App::update() {
    // Atalhos de teclado (0..8 e ESC) só atuam quando nenhuma caixa de texto está focada.
    if (!isAnyTextInputFocused()) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ZERO)) {
            selectModule(0);
        }
        for (int k = 1; k <= 8; ++k) {
            if (IsKeyPressed(KEY_ZERO + k) && k < static_cast<int>(modules_.size())) {
                selectModule(k);
            }
        }
    }

    const float sidebarW = 265.0f;
    const float headerH = 66.0f;
    Rectangle contentArea = {
        sidebarW,
        headerH,
        static_cast<float>(GetScreenWidth()) - sidebarW,
        static_cast<float>(GetScreenHeight()) - headerH
    };

    modules_[static_cast<size_t>(activeIndex_)]->update(contentArea);
}

void App::draw() {
    BeginDrawing();
    ClearBackground(theme::PanelBg);

    const float w = static_cast<float>(GetScreenWidth());
    const float h = static_cast<float>(GetScreenHeight());
    const float sidebarW = 265.0f;
    const float headerH = 66.0f;

    // Barra lateral esquerda com o menu de seleção entre a grade inicial e as aulas.
    DrawRectangle(0, 0, static_cast<int>(sidebarW), static_cast<int>(h), theme::SidebarBg);
    drawUiText("COMPUTAÇÃO GRÁFICA", 18.0f, 18.0f, 17.0f, WHITE);
    drawUiText("Menu de Práticas (C++ / raylib)", 18.0f, 40.0f, 13.0f, theme::SidebarMuted);
    DrawLineEx({14.0f, 66.0f}, {sidebarW - 14.0f, 66.0f}, 1.0f, {51, 65, 85, 255});

    Vector2 mouse = GetMousePosition();
    float btnY = 80.0f;
    for (int i = 0; i < static_cast<int>(modules_.size()); ++i) {
        Rectangle itemRect = {12.0f, btnY, sidebarW - 24.0f, 42.0f};
        bool active = (i == activeIndex_);
        bool hovered = CheckCollisionPointRec(mouse, itemRect);

        Color bg = active ? theme::SidebarActive : (hovered ? theme::SidebarHover : BLANK);
        if (bg.a > 0) {
            DrawRectangleRounded(itemRect, 0.18f, 6, bg);
        }

        Color fg = active ? WHITE : theme::SidebarText;
        drawUiText(modules_[static_cast<size_t>(i)]->menuLabel(), itemRect.x + 12.0f, itemRect.y + 13.0f, 14.0f, fg);

        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectModule(i);
        }

        btnY += (i == 0) ? 54.0f : 46.0f;
        if (i == 0) {
            DrawLineEx({14.0f, btnY - 8.0f}, {sidebarW - 14.0f, btnY - 8.0f}, 1.0f, {51, 65, 85, 255});
            drawUiText("AULAS PRÁTICAS", 18.0f, btnY, 12.0f, theme::SidebarMuted);
            btnY += 20.0f;
        }
    }

    drawUiText("Atalhos: 0 = Grade Inicial | 1-8 = Aulas", 16.0f, h - 28.0f, 12.0f, theme::SidebarMuted);

    // Cabeçalho superior exibindo título e subtítulo do módulo selecionado.
    Rectangle headerRect = {sidebarW, 0.0f, w - sidebarW, headerH};
    DrawRectangleRec(headerRect, WHITE);
    DrawLineEx({sidebarW, headerH}, {w, headerH}, 1.0f, theme::PanelBorder);

    const auto& current = modules_[static_cast<size_t>(activeIndex_)];
    drawUiText(current->title(), sidebarW + 20.0f, 12.0f, 20.0f, theme::TextDark);
    drawUiText(current->subtitle(), sidebarW + 20.0f, 38.0f, 13.5f, theme::TextMuted);

    if (activeIndex_ != 0) {
        if (uiButton({w - 154.0f, 16.0f, 136.0f, 34.0f}, "Voltar à Grade")) {
            selectModule(0);
        }
    }

    // Área útil destinada ao conteúdo do módulo ativo.
    Rectangle contentArea = {sidebarW, headerH, w - sidebarW, h - headerH};
    current->draw(contentArea);

    EndDrawing();
}

void App::tick() {
    update();
    draw();
}

void App::run() {
#ifdef __EMSCRIPTEN__
    // No navegador (WebAssembly), o loop principal deve ceder controle ao event loop via Emscripten.
    emscripten_set_main_loop_arg([](void* arg) {
        static_cast<App*>(arg)->tick();
    }, this, 0, 1);
#else
    while (!WindowShouldClose()) {
        tick();
    }
#endif
}

} // namespace cg
