#include "aula04_pixels.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace cg {

namespace {
const Color kBrushColors[] = {
    {220, 38, 38, 255},   // Vermelho
    {22, 163, 74, 255},   // Verde
    {37, 99, 235, 255},   // Azul
    {234, 179, 8, 255},   // Amarelo
    {15, 23, 42, 255},    // Preto
    {255, 255, 255, 255}  // Branco
};
const char* kBrushNames[] = {"Vermelho", "Verde", "Azul", "Amarelo", "Preto", "Branco"};
} // namespace

// Lê diretamente a cor RGBA do pixel (x, y) a partir do buffer da imagem.
Color getImagePixel(const Image& img, int x, int y) {
    if (!img.data || x < 0 || y < 0 || x >= img.width || y >= img.height) {
        return BLANK;
    }
    // Acesso O(1) direto ao array de bytes quando a imagem está no formato RGBA8 (4 bytes por pixel).
    if (img.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        const unsigned char* bytes = static_cast<const unsigned char*>(img.data);
        size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(img.width) + static_cast<size_t>(x)) * 4;
        return {bytes[idx], bytes[idx + 1], bytes[idx + 2], bytes[idx + 3]};
    }
    return GetImageColor(img, x, y);
}

// Escreve diretamente a cor RGBA no pixel (x, y) do buffer da imagem.
void setImagePixel(Image& img, int x, int y, Color color) {
    if (!img.data || x < 0 || y < 0 || x >= img.width || y >= img.height) {
        return;
    }
    // Escrita direta nos 4 bytes (R, G, B, A) do pixel correspondente na memória.
    if (img.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        unsigned char* bytes = static_cast<unsigned char*>(img.data);
        size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(img.width) + static_cast<size_t>(x)) * 4;
        bytes[idx]     = color.r;
        bytes[idx + 1] = color.g;
        bytes[idx + 2] = color.b;
        bytes[idx + 3] = color.a;
        return;
    }
    ImageDrawPixel(&img, x, y, color);
}

Image createSampleColorImage(int width, int height, int variant) {
    Image img = GenImageColor(width, height, RAYWHITE);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    if (variant == 0) {
        // Imagem de exemplo 1: gradiente RGB com formas geométricas coloridas
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float u = static_cast<float>(x) / static_cast<float>(std::max(1, width - 1));
                float v = static_cast<float>(y) / static_cast<float>(std::max(1, height - 1));
                unsigned char r = static_cast<unsigned char>(255.0f * (1.0f - u * 0.6f));
                unsigned char g = static_cast<unsigned char>(255.0f * v);
                unsigned char b = static_cast<unsigned char>(255.0f * u);
                setImagePixel(img, x, y, {r, g, b, 255});
            }
        }
        ImageDrawCircle(&img, width / 3, height / 2, std::min(width, height) / 4, RED);
        ImageDrawCircle(&img, (2 * width) / 3, height / 2, std::min(width, height) / 4, GREEN);
        ImageDrawCircle(&img, width / 2, height / 3, std::min(width, height) / 4, BLUE);
        ImageDrawRectangle(&img, width / 2 - 45, (2 * height) / 3 - 25, 90, 50, YELLOW);
        // Desenha moldura preta de 3 px compatível com todas as versões da raylib (5.x e 6.x).
        ImageDrawRectangle(&img, 12, 12, width - 24, 3, BLACK);
        ImageDrawRectangle(&img, 12, height - 15, width - 24, 3, BLACK);
        ImageDrawRectangle(&img, 12, 12, 3, height - 24, BLACK);
        ImageDrawRectangle(&img, width - 15, 12, 3, height - 24, BLACK);
    } else {
        // Imagem de exemplo 2: barras de cores e anéis concêntricos
        Color bars[] = {WHITE, YELLOW, SKYBLUE, GREEN, MAGENTA, RED, BLUE, BLACK};
        int barW = std::max(1, width / 8);
        for (int i = 0; i < 8; ++i) {
            int bw = (i == 7) ? (width - i * barW) : barW;
            ImageDrawRectangle(&img, i * barW, 0, bw, height / 2, bars[i]);
        }
        for (int y = height / 2; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float dx = static_cast<float>(x - width / 2);
                float dy = static_cast<float>(y - (3 * height) / 4);
                float dist = std::sqrt(dx * dx + dy * dy);
                unsigned char r = static_cast<unsigned char>(127.5f + 127.5f * std::sin(dist * 0.12f));
                unsigned char g = static_cast<unsigned char>(127.5f + 127.5f * std::cos(dist * 0.08f));
                unsigned char b = static_cast<unsigned char>((x * 255) / std::max(1, width - 1));
                setImagePixel(img, x, y, {r, g, b, 255});
            }
        }
    }

    return img;
}

// Converte a imagem para tons de cinza usando luminância (0.299R + 0.587G + 0.114B) ou média aritmética.
Image makeGrayscaleImage(const Image& src, bool luminanceWeighted) {
    Image out = ImageCopy(src);
    ImageFormat(&out, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    unsigned char* bytes = static_cast<unsigned char*>(out.data);
    int total = out.width * out.height;
    // Atualiza todos os pixels aplicando a equação de luminância ou média simples.
    for (int i = 0; i < total; ++i) {
        size_t idx = static_cast<size_t>(i) * 4;
        unsigned char r = bytes[idx];
        unsigned char g = bytes[idx + 1];
        unsigned char b = bytes[idx + 2];
        unsigned char lum = luminanceWeighted
            ? static_cast<unsigned char>(std::clamp(std::round(0.299 * r + 0.587 * g + 0.114 * b), 0.0, 255.0))
            : static_cast<unsigned char>((static_cast<int>(r) + g + b) / 3);
        bytes[idx] = bytes[idx + 1] = bytes[idx + 2] = lum;
    }
    return out;
}

// Inverte as cores da imagem (complemento CMY: 255 - R, 255 - G, 255 - B).
Image makeInvertedImage(const Image& src) {
    Image out = ImageCopy(src);
    ImageFormat(&out, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    unsigned char* bytes = static_cast<unsigned char*>(out.data);
    int total = out.width * out.height;
    for (int i = 0; i < total; ++i) {
        size_t idx = static_cast<size_t>(i) * 4;
        bytes[idx]     = static_cast<unsigned char>(255 - bytes[idx]);
        bytes[idx + 1] = static_cast<unsigned char>(255 - bytes[idx + 1]);
        bytes[idx + 2] = static_cast<unsigned char>(255 - bytes[idx + 2]);
    }
    return out;
}

// Isola um canal de cor (0=R, 1=G, 2=B), colorido na sua cor pura ou em escala de cinza.
Image makeChannelImage(const Image& src, int channelIndex, bool tinted) {
    Image out = ImageCopy(src);
    ImageFormat(&out, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    unsigned char* bytes = static_cast<unsigned char*>(out.data);
    int total = out.width * out.height;
    int ch = std::clamp(channelIndex, 0, 2);
    for (int i = 0; i < total; ++i) {
        size_t idx = static_cast<size_t>(i) * 4;
        unsigned char val = bytes[idx + static_cast<size_t>(ch)];
        if (tinted) {
            bytes[idx]     = (ch == 0) ? val : 0;
            bytes[idx + 1] = (ch == 1) ? val : 0;
            bytes[idx + 2] = (ch == 2) ? val : 0;
        } else {
            bytes[idx] = bytes[idx + 1] = bytes[idx + 2] = val;
        }
    }
    return out;
}

PenCanvas::PenCanvas(Image* target) : target_(target) {}

void PenCanvas::moveTo(int x, int y) {
    penX_ = x;
    penY_ = y;
}

void PenCanvas::lineTo(int x, int y, Color color) {
    if (target_ && target_->data) {
        ImageDrawLine(target_, penX_, penY_, x, y, color);
    }
    penX_ = x;
    penY_ = y;
}

Aula04PixelsModule::~Aula04PixelsModule() {
    releaseResources();
}

void Aula04PixelsModule::releaseResources() {
    if (!hasImage_) return;
    UnloadImage(baseImage_);
    UnloadTexture(texEditable_);
    UnloadTexture(texGray_);
    UnloadTexture(texInv_);
    UnloadTexture(texR_);
    UnloadTexture(texG_);
    UnloadTexture(texB_);
    hasImage_ = false;
}

void Aula04PixelsModule::onEnter() {
    if (!initialized_) {
        pathInput_.maxChars = 256;
        pathInput_.text = "";
        loadSample(0);
        initialized_ = true;
    }
}

void Aula04PixelsModule::loadSample(int variant) {
    releaseResources();
    sampleVariant_ = variant;
    baseImage_ = createSampleColorImage(420, 320, variant);
    pen_.setTarget(&baseImage_);
    pen_.moveTo(40, 40);

    texEditable_ = LoadTextureFromImage(baseImage_);
    texGray_     = LoadTextureFromImage(baseImage_);
    texInv_      = LoadTextureFromImage(baseImage_);
    texR_        = LoadTextureFromImage(baseImage_);
    texG_        = LoadTextureFromImage(baseImage_);
    texB_        = LoadTextureFromImage(baseImage_);
    hasImage_ = true;
    rebuildDerivedTextures();
    statusMsg_ = (variant == 0) ? "Imagem exemplo 1 carregada (420x320)." : "Imagem exemplo 2 carregada (420x320).";
}

bool Aula04PixelsModule::loadFromFile(const std::string& path) {
    if (path.empty() || !FileExists(path.c_str())) {
        statusMsg_ = "Arquivo não encontrado: " + path;
        return false;
    }
    Image loaded = LoadImage(path.c_str());
    if (!loaded.data) {
        statusMsg_ = "Falha ao decodificar imagem.";
        return false;
    }
    ImageFormat(&loaded, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    releaseResources();
    baseImage_ = loaded;
    pen_.setTarget(&baseImage_);
    pen_.moveTo(0, 0);

    texEditable_ = LoadTextureFromImage(baseImage_);
    texGray_     = LoadTextureFromImage(baseImage_);
    texInv_      = LoadTextureFromImage(baseImage_);
    texR_        = LoadTextureFromImage(baseImage_);
    texG_        = LoadTextureFromImage(baseImage_);
    texB_        = LoadTextureFromImage(baseImage_);
    hasImage_ = true;
    rebuildDerivedTextures();

    char buf[96];
    std::snprintf(buf, sizeof(buf), "Imagem carregada (%dx%d).", baseImage_.width, baseImage_.height);
    statusMsg_ = buf;
    return true;
}

void Aula04PixelsModule::rebuildDerivedTextures() {
    if (!hasImage_) return;
    UpdateTexture(texEditable_, baseImage_.data);

    Image gray = makeGrayscaleImage(baseImage_, weightedGray_);
    UpdateTexture(texGray_, gray.data);
    UnloadImage(gray);

    Image inv = makeInvertedImage(baseImage_);
    UpdateTexture(texInv_, inv.data);
    UnloadImage(inv);

    Image chR = makeChannelImage(baseImage_, 0, tintedChannels_);
    UpdateTexture(texR_, chR.data);
    UnloadImage(chR);

    Image chG = makeChannelImage(baseImage_, 1, tintedChannels_);
    UpdateTexture(texG_, chG.data);
    UnloadImage(chG);

    Image chB = makeChannelImage(baseImage_, 2, tintedChannels_);
    UpdateTexture(texB_, chB.data);
    UnloadImage(chB);
}

Rectangle Aula04PixelsModule::computeCanvasArea(Rectangle area) const {
    float pad = 16.0f;
    float ctrlW = 310.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    return {canvasCard.x + 8.0f, canvasCard.y + 40.0f, canvasCard.width - 16.0f, canvasCard.height - 96.0f};
}

Rectangle Aula04PixelsModule::computeImageRect(Rectangle canvasArea) const {
    if (!hasImage_ || baseImage_.width <= 0 || baseImage_.height <= 0) {
        return canvasArea;
    }
    float pad = 16.0f;
    float availW = std::max(10.0f, canvasArea.width - 2 * pad);
    float availH = std::max(10.0f, canvasArea.height - 2 * pad);
    float scale = std::min(availW / static_cast<float>(baseImage_.width),
                           availH / static_cast<float>(baseImage_.height));
    float drawW = static_cast<float>(baseImage_.width) * scale;
    float drawH = static_cast<float>(baseImage_.height) * scale;
    return {
        canvasArea.x + (canvasArea.width - drawW) * 0.5f,
        canvasArea.y + (canvasArea.height - drawH) * 0.5f,
        drawW,
        drawH
    };
}

void Aula04PixelsModule::update(Rectangle area) {
    if (!hasImage_) return;

    if (IsFileDropped()) {
        FilePathList dropped = LoadDroppedFiles();
        if (dropped.count > 0) {
            pathInput_.text = dropped.paths[0];
            loadFromFile(pathInput_.text);
        }
        UnloadDroppedFiles(dropped);
    }

    Rectangle canvasArea = computeCanvasArea(area);
    if (viewMode_ == ViewMode::SplitChannels) return;

    Rectangle imgRect = computeImageRect(canvasArea);
    Vector2 mouse = GetMousePosition();
    if (!CheckCollisionPointRec(mouse, imgRect)) return;

    int ix = static_cast<int>(((mouse.x - imgRect.x) / imgRect.width) * static_cast<float>(baseImage_.width));
    int iy = static_cast<int>(((mouse.y - imgRect.y) / imgRect.height) * static_cast<float>(baseImage_.height));
    ix = std::clamp(ix, 0, baseImage_.width - 1);
    iy = std::clamp(iy, 0, baseImage_.height - 1);

    Color brush = kBrushColors[brushColorIdx_];

    if (toolMode_ == ToolMode::InspectAndPaint) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            // Pinta diretamente o pixel exato sob o cursor no buffer da imagem.
            setImagePixel(baseImage_, ix, iy, brush);
            rebuildDerivedTextures();
        }
    } else if (toolMode_ == ToolMode::MoveLineTo) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            pen_.moveTo(ix, iy);
            char buf[64];
            std::snprintf(buf, sizeof(buf), "MoveTo(%d, %d)", ix, iy);
            statusMsg_ = buf;
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int ox = pen_.penX();
            int oy = pen_.penY();
            pen_.lineTo(ix, iy, brush);
            rebuildDerivedTextures();
            char buf[96];
            std::snprintf(buf, sizeof(buf), "LineTo(%d, %d) a partir de (%d, %d)", ix, iy, ox, oy);
            statusMsg_ = buf;
        }
    }
}

void Aula04PixelsModule::draw(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 310.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    Rectangle ctrlCard   = {canvasCard.x + canvasCard.width + pad, area.y + pad, ctrlW, area.height - 2 * pad};

    drawPanelCard(canvasCard, "Área da Imagem (Acesso aos Pixels)");
    drawPanelCard(ctrlCard, "Operações da Aula 04");

    Rectangle canvasArea = computeCanvasArea(area);
    DrawRectangleRec(canvasArea, theme::CanvasBg);
    DrawRectangleLinesEx(canvasArea, 1.0f, theme::GridMinor);

    if (hasImage_) {
        if (viewMode_ == ViewMode::SplitChannels) {
            float hw = (canvasArea.width - 18.0f) * 0.5f;
            float hh = (canvasArea.height - 18.0f) * 0.5f;
            struct Quad { const char* name; Texture2D* tex; float x; float y; };
            Quad quads[] = {
                {"Original", &texEditable_, canvasArea.x + 6.0f, canvasArea.y + 6.0f},
                {"Canal R",  &texR_,        canvasArea.x + 12.0f + hw, canvasArea.y + 6.0f},
                {"Canal G",  &texG_,        canvasArea.x + 6.0f, canvasArea.y + 12.0f + hh},
                {"Canal B",  &texB_,        canvasArea.x + 12.0f + hw, canvasArea.y + 12.0f + hh}
            };
            for (const auto& q : quads) {
                Rectangle box = {q.x, q.y, hw, hh - 18.0f};
                Rectangle fit = computeImageRect({box.x, box.y + 18.0f, box.width, box.height});
                drawUiText(q.name, box.x + 4.0f, box.y, 14.0f, theme::TextDark);
                DrawTexturePro(*q.tex, {0, 0, (float)q.tex->width, (float)q.tex->height}, fit, {0, 0}, 0.0f, WHITE);
                DrawRectangleLinesEx(fit, 1.0f, theme::PanelBorder);
            }
        } else {
            Texture2D* tex = &texEditable_;
            switch (viewMode_) {
                case ViewMode::EditableOriginal: tex = &texEditable_; break;
                case ViewMode::Grayscale:        tex = &texGray_; break;
                case ViewMode::Inverted:         tex = &texInv_; break;
                case ViewMode::ChannelR:         tex = &texR_; break;
                case ViewMode::ChannelG:         tex = &texG_; break;
                case ViewMode::ChannelB:         tex = &texB_; break;
                default: break;
            }
            Rectangle imgRect = computeImageRect(canvasArea);
            DrawTexturePro(*tex, {0, 0, (float)tex->width, (float)tex->height}, imgRect, {0, 0}, 0.0f, WHITE);
            DrawRectangleLinesEx(imgRect, 1.5f, theme::GridAxis);

            if (toolMode_ == ToolMode::MoveLineTo) {
                float px = imgRect.x + ((float)pen_.penX() + 0.5f) / (float)baseImage_.width * imgRect.width;
                float py = imgRect.y + ((float)pen_.penY() + 0.5f) / (float)baseImage_.height * imgRect.height;
                DrawCircleLinesV({px, py}, 6.0f, theme::Danger);
                DrawCircleV({px, py}, 2.5f, theme::Danger);
            }
        }
    }

    // Barra inferior de inspeção de coordenadas (x, y) e cor RGB sob o cursor.
    Rectangle infoBar = {canvasCard.x + 10.0f, canvasCard.y + canvasCard.height - 48.0f, canvasCard.width - 20.0f, 38.0f};
    DrawRectangleRounded(infoBar, 0.15f, 4, theme::PanelBg);
    DrawRectangleRoundedLinesEx(infoBar, 0.15f, 4, 1.0f, theme::PanelBorder);

    bool hoveredPixel = false;
    if (hasImage_ && viewMode_ != ViewMode::SplitChannels) {
        Rectangle imgRect = computeImageRect(canvasArea);
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, imgRect)) {
            int ix = std::clamp(static_cast<int>(((mouse.x - imgRect.x) / imgRect.width) * baseImage_.width), 0, baseImage_.width - 1);
            int iy = std::clamp(static_cast<int>(((mouse.y - imgRect.y) / imgRect.height) * baseImage_.height), 0, baseImage_.height - 1);
            Color px = getImagePixel(baseImage_, ix, iy);
            DrawRectangle(static_cast<int>(infoBar.x + 10), static_cast<int>(infoBar.y + 7), 24, 24, px);
            DrawRectangleLines(static_cast<int>(infoBar.x + 10), static_cast<int>(infoBar.y + 7), 24, 24, theme::TextDark);

            char buf[128];
            std::snprintf(buf, sizeof(buf), "Cursor: (%d, %d)   RGB: (%d, %d, %d)   #%02X%02X%02X",
                          ix, iy, px.r, px.g, px.b, px.r, px.g, px.b);
            drawUiText(buf, infoBar.x + 44.0f, infoBar.y + 11.0f, 15.0f, theme::TextDark);
            hoveredPixel = true;
        }
    }
    if (!hoveredPixel) {
        drawUiText(statusMsg_.c_str(), infoBar.x + 12.0f, infoBar.y + 11.0f, 14.0f, theme::TextMuted);
    }

    // Painel lateral de controles e ferramentas da Aula 04.
    float cx = ctrlCard.x + 14.0f;
    float cw = ctrlCard.width - 28.0f;
    float cy = ctrlCard.y + 44.0f;

    drawUiText("1) Carregar Imagem (ou arraste na janela):", cx, cy, 14.0f, theme::TextDark);
    cy += 22.0f;
    uiTextInput({cx, cy, cw - 86.0f, 30.0f}, pathInput_);
    if (uiButton({cx + cw - 80.0f, cy, 80.0f, 30.0f}, "Abrir", false, true)) {
        loadFromFile(pathInput_.text);
    }
    cy += 36.0f;
    if (uiButton({cx, cy, (cw - 8.0f) * 0.5f, 28.0f}, "Exemplo 1", sampleVariant_ == 0)) {
        loadSample(0);
    }
    if (uiButton({cx + (cw + 8.0f) * 0.5f, cy, (cw - 8.0f) * 0.5f, 28.0f}, "Exemplo 2", sampleVariant_ == 1)) {
        loadSample(1);
    }
    cy += 38.0f;

    drawUiText("2, 3, 4) Visualização / Filtros:", cx, cy, 14.0f, theme::TextDark);
    cy += 22.0f;
    float bw = (cw - 6.0f) * 0.5f;
    if (uiButton({cx, cy, bw, 28.0f}, "Original", viewMode_ == ViewMode::EditableOriginal)) viewMode_ = ViewMode::EditableOriginal;
    if (uiButton({cx + bw + 6.0f, cy, bw, 28.0f}, "Tons de Cinza", viewMode_ == ViewMode::Grayscale)) viewMode_ = ViewMode::Grayscale;
    cy += 32.0f;
    if (uiButton({cx, cy, bw, 28.0f}, "Invertida", viewMode_ == ViewMode::Inverted)) viewMode_ = ViewMode::Inverted;
    if (uiButton({cx + bw + 6.0f, cy, bw, 28.0f}, "Canais 2x2", viewMode_ == ViewMode::SplitChannels)) viewMode_ = ViewMode::SplitChannels;
    cy += 32.0f;
    float bw3 = (cw - 12.0f) / 3.0f;
    if (uiButton({cx, cy, bw3, 28.0f}, "Canal R", viewMode_ == ViewMode::ChannelR)) viewMode_ = ViewMode::ChannelR;
    if (uiButton({cx + bw3 + 6.0f, cy, bw3, 28.0f}, "Canal G", viewMode_ == ViewMode::ChannelG)) viewMode_ = ViewMode::ChannelG;
    if (uiButton({cx + 2 * (bw3 + 6.0f), cy, bw3, 28.0f}, "Canal B", viewMode_ == ViewMode::ChannelB)) viewMode_ = ViewMode::ChannelB;
    cy += 36.0f;

    if (uiCheckBox({cx, cy, 18.0f, 18.0f}, "Cinza ponderado (0.299R+0.587G+0.114B)", &weightedGray_)) {
        rebuildDerivedTextures();
    }
    cy += 26.0f;
    if (uiCheckBox({cx, cy, 18.0f, 18.0f}, "Canais coloridos (desmarcar = PB)", &tintedChannels_)) {
        rebuildDerivedTextures();
    }
    cy += 34.0f;

    drawUiText("5, 6) Ferramenta de Edição:", cx, cy, 14.0f, theme::TextDark);
    cy += 24.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "Pintar Pixels (clique/arraste)", toolMode_ == ToolMode::InspectAndPaint)) {
        toolMode_ = ToolMode::InspectAndPaint;
    }
    cy += 26.0f;
    if (uiRadioButton({cx + 10.0f, cy + 8.0f}, 7.0f, "MoveTo (Dir) / LineTo (Esq)", toolMode_ == ToolMode::MoveLineTo)) {
        toolMode_ = ToolMode::MoveLineTo;
    }
    cy += 30.0f;

    drawUiText("Cor do Pincel / Linha:", cx, cy, 14.0f, theme::TextDark);
    cy += 22.0f;
    for (int i = 0; i < 6; ++i) {
        Rectangle sw = {cx + i * 38.0f, cy, 30.0f, 26.0f};
        DrawRectangleRec(sw, kBrushColors[i]);
        DrawRectangleLinesEx(sw, i == brushColorIdx_ ? 2.5f : 1.0f, i == brushColorIdx_ ? theme::Accent : theme::GridAxis);
        if (CheckCollisionPointRec(GetMousePosition(), sw) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            brushColorIdx_ = i;
        }
    }
    cy += 34.0f;
    drawUiText(TextFormat("Cor selecionada: %s", kBrushNames[brushColorIdx_]), cx, cy, 13.0f, theme::TextMuted);
    cy += 26.0f;

    if (uiButton({cx, cy, cw, 32.0f}, "Demo MoveTo/LineTo (H, V, Diag)", false, true)) {
        if (hasImage_) {
            int w = baseImage_.width;
            int h = baseImage_.height;
            // Horizontal
            pen_.moveTo(20, 30);
            pen_.lineTo(w - 20, 30, RED);
            // Vertical
            pen_.moveTo(w - 30, 30);
            pen_.lineTo(w - 30, h - 30, BLUE);
            // Diagonal 1
            pen_.moveTo(20, 30);
            pen_.lineTo(w - 30, h - 30, GREEN);
            // Diagonal 2
            pen_.moveTo(w - 30, 30);
            pen_.lineTo(20, h - 30, BLACK);
            rebuildDerivedTextures();
            viewMode_ = ViewMode::EditableOriginal;
            statusMsg_ = "Retas horizontal, vertical e diagonais desenhadas via MoveTo/LineTo!";
        }
    }
}

} // namespace cg
