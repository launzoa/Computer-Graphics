#include "aula12_projecoes.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

namespace cg {

// Cria matriz identidade 4x4.
Mat4d Mat4d::identity() {
    Mat4d r{};
    for (int i = 0; i < 4; ++i) r.m[i][i] = 1.0;
    return r;
}

Mat4d Mat4d::translation(double tx, double ty, double tz) {
    Mat4d r = identity();
    r.m[0][3] = tx;
    r.m[1][3] = ty;
    r.m[2][3] = tz;
    return r;
}

Mat4d Mat4d::scaleLocal(double sx, double sy, double sz) {
    Mat4d r = identity();
    r.m[0][0] = sx;
    r.m[1][1] = sy;
    r.m[2][2] = sz;
    return r;
}

// Escala global em coordenadas homogêneas: ajusta w = 1/s para escalar X, Y e Z uniformemente após divisão por w.
Mat4d Mat4d::scaleGlobal(double s) {
    Mat4d r = identity();
    if (std::fabs(s) > 1e-9) {
        r.m[3][3] = 1.0 / s;
    } else {
        r.m[0][0] = r.m[1][1] = r.m[2][2] = 0.0;
    }
    return r;
}

Mat4d Mat4d::rotationX(double degrees) {
    double rad = degrees * (3.14159265358979323846 / 180.0);
    double c = std::cos(rad);
    double s = std::sin(rad);
    Mat4d r = identity();
    r.m[1][1] = c;  r.m[1][2] = -s;
    r.m[2][1] = s;  r.m[2][2] = c;
    return r;
}

Mat4d Mat4d::rotationY(double degrees) {
    double rad = degrees * (3.14159265358979323846 / 180.0);
    double c = std::cos(rad);
    double s = std::sin(rad);
    Mat4d r = identity();
    r.m[0][0] = c;  r.m[0][2] = s;
    r.m[2][0] = -s; r.m[2][2] = c;
    return r;
}

Mat4d Mat4d::rotationZ(double degrees) {
    double rad = degrees * (3.14159265358979323846 / 180.0);
    double c = std::cos(rad);
    double s = std::sin(rad);
    Mat4d r = identity();
    r.m[0][0] = c;  r.m[0][1] = -s;
    r.m[1][0] = s;  r.m[1][1] = c;
    return r;
}

Mat4d multiplyMat4(const Mat4d& a, const Mat4d& b) {
    Mat4d r{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            double sum = 0.0;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[i][k] * b.m[k][j];
            }
            r.m[i][j] = sum;
        }
    }
    return r;
}

// Aplica a matriz homogênea 4x4 ao vetor v e normaliza por w (divisão perspectiva/homogênea).
Vec4d transformPoint(const Mat4d& mat, const Vec4d& v) {
    double nx = mat.m[0][0] * v.x + mat.m[0][1] * v.y + mat.m[0][2] * v.z + mat.m[0][3] * v.w;
    double ny = mat.m[1][0] * v.x + mat.m[1][1] * v.y + mat.m[1][2] * v.z + mat.m[1][3] * v.w;
    double nz = mat.m[2][0] * v.x + mat.m[2][1] * v.y + mat.m[2][2] * v.z + mat.m[2][3] * v.w;
    double nw = mat.m[3][0] * v.x + mat.m[3][1] * v.y + mat.m[3][2] * v.z + mat.m[3][3] * v.w;

    if (std::fabs(nw) > 1e-9 && std::fabs(nw - 1.0) > 1e-9) {
        nx /= nw;
        ny /= nw;
        nz /= nw;
        nw = 1.0;
    }
    return {nx, ny, nz, nw};
}

std::vector<Vec4d> createDefaultHouseVertices() {
    // Coordenadas exatas da casa 3D definidas no slide 2 da Aula 12a.
    return {
        {0.0,   0.0,   0.0,   1.0}, // 0: frente inferior esquerdo
        {100.0, 0.0,   0.0,   1.0}, // 1: frente inferior direito
        {100.0, 100.0, 0.0,   1.0}, // 2: frente superior direito
        {50.0,  150.0, 0.0,   1.0}, // 3: frente topo do telhado
        {0.0,   100.0, 0.0,   1.0}, // 4: frente superior esquerdo
        {0.0,   0.0,   100.0, 1.0}, // 5: fundo inferior esquerdo
        {100.0, 0.0,   100.0, 1.0}, // 6: fundo inferior direito
        {100.0, 100.0, 100.0, 1.0}, // 7: fundo superior direito
        {50.0,  150.0, 100.0, 1.0}, // 8: fundo topo do telhado
        {0.0,   100.0, 100.0, 1.0}  // 9: fundo superior esquerdo
    };
}

std::vector<HouseEdge> createDefaultHouseEdges() {
    return {
        // Face frontal (z = 0)
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0},
        // Face traseira (z = 100)
        {5, 6}, {6, 7}, {7, 8}, {8, 9}, {9, 5},
        // Arestas de profundidade ligando frente e fundo
        {0, 5}, {1, 6}, {2, 7}, {3, 8}, {4, 9}
    };
}

Vec4d computeCentroid(const std::vector<Vec4d>& vertices) {
    if (vertices.empty()) return {0.0, 0.0, 0.0, 1.0};
    double sx = 0.0, sy = 0.0, sz = 0.0;
    for (const auto& v : vertices) {
        sx += v.x;
        sy += v.y;
        sz += v.z;
    }
    double inv = 1.0 / static_cast<double>(vertices.size());
    return {sx * inv, sy * inv, sz * inv, 1.0};
}

void Aula12ProjecoesModule::onEnter() {
    if (!initialized_) {
        auto setupNum = [](UiTextInput& in, const char* def) {
            in.numericOnly = true;
            in.allowFloat = true;
            in.allowSign = true;
            in.maxChars = 10;
            in.text = def;
        };

        setupNum(inScaleX_, "2");
        setupNum(inScaleY_, "3");
        setupNum(inScaleZ_, "1");
        setupNum(inScaleGlobal_, "2");
        setupNum(inTransX_, "0");
        setupNum(inTransY_, "-30");
        setupNum(inTransZ_, "10");

        inRotAxis_.numericOnly = false;
        inRotAxis_.maxChars = 1;
        inRotAxis_.text = "Y";
        setupNum(inRotDeg_, "5");

        const char* shearDefaults[16] = {
            "1", "-0.3", "0.9", "0",
            "1.3", "1", "-1.2", "0",
            "2", "2.1", "1", "0",
            "0", "0", "0", "1"
        };
        for (int i = 0; i < 16; ++i) {
            setupNum(inShear_[i], shearDefaults[i]);
        }

        edges_ = createDefaultHouseEdges();
        resetObject();
        initialized_ = true;
    }
}

void Aula12ProjecoesModule::resetObject() {
    vertices_ = createDefaultHouseVertices();
    viewOffset_ = {0.0f, 0.0f};
    viewZoom_ = 1.6f;
}

Mat4d Aula12ProjecoesModule::buildRotationMatrix(char axis, double degrees) const {
    char up = static_cast<char>(std::toupper(static_cast<unsigned char>(axis)));
    if (up == 'X') return Mat4d::rotationX(degrees);
    if (up == 'Z') return Mat4d::rotationZ(degrees);
    return Mat4d::rotationY(degrees);
}

void Aula12ProjecoesModule::executeSelectedOp() {
    Mat4d op = Mat4d::identity();

    // Constrói a matriz homogênea 4x4 correspondente à operação selecionada pelo usuário.
    switch (opMode_) {
        case OpMode::ScaleLocal: {
            op = Mat4d::scaleLocal(inScaleX_.asDouble(1.0), inScaleY_.asDouble(1.0), inScaleZ_.asDouble(1.0));
            break;
        }
        case OpMode::ScaleGlobal: {
            op = Mat4d::scaleGlobal(inScaleGlobal_.asDouble(1.0));
            break;
        }
        case OpMode::Translation: {
            op = Mat4d::translation(inTransX_.asDouble(0.0), inTransY_.asDouble(0.0), inTransZ_.asDouble(0.0));
            break;
        }
        case OpMode::RotationOrigin: {
            char axis = inRotAxis_.text.empty() ? 'Y' : inRotAxis_.text[0];
            op = buildRotationMatrix(axis, inRotDeg_.asDouble(0.0));
            break;
        }
        case OpMode::RotationCenter: {
            // Rotação em torno do centro do objeto: T(centro) * R(eixo, ângulo) * T(-centro).
            char axis = inRotAxis_.text.empty() ? 'Y' : inRotAxis_.text[0];
            Vec4d c = computeCentroid(vertices_);
            Mat4d toOrigin = Mat4d::translation(-c.x, -c.y, -c.z);
            Mat4d rot = buildRotationMatrix(axis, inRotDeg_.asDouble(0.0));
            Mat4d back = Mat4d::translation(c.x, c.y, c.z);
            op = multiplyMat4(back, multiplyMat4(rot, toOrigin));
            break;
        }
        case OpMode::Shearing: {
            // Lê diretamente os 16 coeficientes da matriz 4x4 editável na interface.
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    op.m[r][c] = inShear_[r * 4 + c].asDouble((r == c) ? 1.0 : 0.0);
                }
            }
            break;
        }
    }

    // Aplica a matriz resultante a todos os vértices do objeto 3D.
    for (auto& v : vertices_) {
        v = transformPoint(op, v);
    }
}

void Aula12ProjecoesModule::update(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 390.0f;
    Rectangle viewRect = {area.x + pad + 8.0f, area.y + pad + 40.0f,
                          area.width - ctrlW - 3 * pad - 16.0f, area.height - 2 * pad - 52.0f};

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, viewRect)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            viewZoom_ = std::clamp(viewZoom_ + wheel * 0.15f, 0.25f, 6.0f);
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            viewOffset_.x += delta.x;
            viewOffset_.y += delta.y;
        }
    }
}

void Aula12ProjecoesModule::draw(Rectangle area) {
    float pad = 16.0f;
    float ctrlW = 390.0f;
    Rectangle canvasCard = {area.x + pad, area.y + pad, area.width - ctrlW - 3 * pad, area.height - 2 * pad};
    Rectangle ctrlCard   = {canvasCard.x + canvasCard.width + pad, area.y + pad, ctrlW, area.height - 2 * pad};

    drawPanelCard(canvasCard, "Projeção Paralela no Plano z = 0 (exibe apenas X e Y)");
    drawPanelCard(ctrlCard, "Transformações 3D (Coordenadas Homogêneas)");

    Rectangle viewRect = {canvasCard.x + 8.0f, canvasCard.y + 40.0f, canvasCard.width - 16.0f, canvasCard.height - 52.0f};
    DrawRectangleRec(viewRect, theme::CanvasBg);

    Vector2 origin = {
        viewRect.x + viewRect.width * 0.38f + viewOffset_.x,
        viewRect.y + viewRect.height * 0.68f + viewOffset_.y
    };

    auto toScreen = [&](double wx, double wy) -> Vector2 {
        return {
            origin.x + static_cast<float>(wx) * viewZoom_,
            origin.y - static_cast<float>(wy) * viewZoom_ // Eixo +Y cartesiano apontando para cima
        };
    };

    BeginScissorMode(static_cast<int>(viewRect.x), static_cast<int>(viewRect.y),
                     static_cast<int>(viewRect.width), static_cast<int>(viewRect.height));

    // Grade de referência (a cada 50 unidades do espaço do mundo)
    float stepPx = 50.0f * viewZoom_;
    if (stepPx >= 12.0f) {
        for (float gx = origin.x; gx < viewRect.x + viewRect.width; gx += stepPx) {
            DrawLineV({gx, viewRect.y}, {gx, viewRect.y + viewRect.height}, theme::GridMinor);
        }
        for (float gx = origin.x - stepPx; gx > viewRect.x; gx -= stepPx) {
            DrawLineV({gx, viewRect.y}, {gx, viewRect.y + viewRect.height}, theme::GridMinor);
        }
        for (float gy = origin.y; gy < viewRect.y + viewRect.height; gy += stepPx) {
            DrawLineV({viewRect.x, gy}, {viewRect.x + viewRect.width, gy}, theme::GridMinor);
        }
        for (float gy = origin.y - stepPx; gy > viewRect.y; gy -= stepPx) {
            DrawLineV({viewRect.x, gy}, {viewRect.x + viewRect.width, gy}, theme::GridMinor);
        }
    }

    // Eixos cartesianos X e Y
    DrawLineEx({viewRect.x, origin.y}, {viewRect.x + viewRect.width, origin.y}, 1.5f, theme::GridAxis);
    DrawLineEx({origin.x, viewRect.y}, {origin.x, viewRect.y + viewRect.height}, 1.5f, theme::GridAxis);
    drawUiText("X", viewRect.x + viewRect.width - 18.0f, origin.y - 20.0f, 14.0f, theme::GridAxis);
    drawUiText("Y", origin.x + 8.0f, viewRect.y + 6.0f, 14.0f, theme::GridAxis);
    drawUiText("(0,0)", origin.x + 5.0f, origin.y + 4.0f, 12.0f, theme::TextMuted);

    // Desenha a casa projetada no plano z = 0 (exibe somente X e Y, ignorando Z)
    for (size_t i = 0; i < edges_.size(); ++i) {
        const auto& e = edges_[i];
        const auto& va = vertices_[static_cast<size_t>(e.a)];
        const auto& vb = vertices_[static_cast<size_t>(e.b)];
        Vector2 sa = toScreen(va.x, va.y);
        Vector2 sb = toScreen(vb.x, vb.y);

        Color col = (i < 5) ? theme::Accent : ((i < 10) ? Color{71, 85, 105, 255} : theme::Danger);
        DrawLineEx(sa, sb, 2.0f, col);
    }

    for (const auto& v : vertices_) {
        DrawCircleV(toScreen(v.x, v.y), 3.5f, theme::TextDark);
    }

    // Marcador do centro geométrico (centroide) do objeto 3D
    Vec4d cent = computeCentroid(vertices_);
    Vector2 sc = toScreen(cent.x, cent.y);
    DrawCircleLinesV(sc, 6.0f, theme::Success);
    DrawCircleV(sc, 2.5f, theme::Success);

    EndScissorMode();
    DrawRectangleLinesEx(viewRect, 1.5f, theme::PanelBorder);

    // Legenda das cores das arestas
    drawUiText("Legenda: Azul = Frente | Cinza = Fundo | Vermelho = Profundidade | Verde = Centro",
               viewRect.x + 10.0f, viewRect.y + viewRect.height - 22.0f, 12.0f, theme::TextMuted);

    // Painel lateral de controles (seguindo a interface sugerida no slide 4)
    float cx = ctrlCard.x + 14.0f;
    float cw = ctrlCard.width - 28.0f;
    float cy = ctrlCard.y + 42.0f;

    // 1. Escala
    drawUiText("Escala", cx, cy, 15.0f, theme::TextDark);
    drawUiText("X", cx + 128.0f, cy, 13.0f, theme::TextMuted);
    drawUiText("Y", cx + 194.0f, cy, 13.0f, theme::TextMuted);
    drawUiText("Z", cx + 260.0f, cy, 13.0f, theme::TextMuted);
    cy += 22.0f;
    if (uiRadioButton({cx + 10.0f, cy + 12.0f}, 7.0f, "Local", opMode_ == OpMode::ScaleLocal)) {
        opMode_ = OpMode::ScaleLocal;
    }
    uiTextInput({cx + 105.0f, cy, 58.0f, 26.0f}, inScaleX_);
    uiTextInput({cx + 171.0f, cy, 58.0f, 26.0f}, inScaleY_);
    uiTextInput({cx + 237.0f, cy, 58.0f, 26.0f}, inScaleZ_);
    cy += 32.0f;

    if (uiRadioButton({cx + 10.0f, cy + 12.0f}, 7.0f, "Global", opMode_ == OpMode::ScaleGlobal)) {
        opMode_ = OpMode::ScaleGlobal;
    }
    uiTextInput({cx + 105.0f, cy, 58.0f, 26.0f}, inScaleGlobal_);
    cy += 42.0f;

    // 2. Translação
    drawUiText("Translação", cx, cy, 15.0f, theme::TextDark);
    drawUiText("X", cx + 128.0f, cy, 13.0f, theme::TextMuted);
    drawUiText("Y", cx + 194.0f, cy, 13.0f, theme::TextMuted);
    drawUiText("Z", cx + 260.0f, cy, 13.0f, theme::TextMuted);
    cy += 22.0f;
    if (uiRadioButton({cx + 10.0f, cy + 12.0f}, 7.0f, "Aplicar", opMode_ == OpMode::Translation)) {
        opMode_ = OpMode::Translation;
    }
    uiTextInput({cx + 105.0f, cy, 58.0f, 26.0f}, inTransX_);
    uiTextInput({cx + 171.0f, cy, 58.0f, 26.0f}, inTransY_);
    uiTextInput({cx + 237.0f, cy, 58.0f, 26.0f}, inTransZ_);
    cy += 42.0f;

    // 3. Rotação
    drawUiText("Rotação", cx, cy, 15.0f, theme::TextDark);
    cy += 22.0f;
    if (uiRadioButton({cx + 10.0f, cy + 12.0f}, 7.0f, "Origem", opMode_ == OpMode::RotationOrigin)) {
        opMode_ = OpMode::RotationOrigin;
    }
    drawUiText("Eixo:", cx + 172.0f, cy + 4.0f, 14.0f, theme::TextMuted);
    uiTextInput({cx + 225.0f, cy, 70.0f, 26.0f}, inRotAxis_);
    cy += 32.0f;

    if (uiRadioButton({cx + 10.0f, cy + 12.0f}, 7.0f, "Centro do Objeto", opMode_ == OpMode::RotationCenter)) {
        opMode_ = OpMode::RotationCenter;
    }
    drawUiText("Graus:", cx + 172.0f, cy + 4.0f, 14.0f, theme::TextMuted);
    uiTextInput({cx + 225.0f, cy, 70.0f, 26.0f}, inRotDeg_);
    cy += 42.0f;

    // 4. Shearing (matriz 4x4 editável) + botão EXECUTA
    drawUiText("Shearing (Matriz 4x4)", cx, cy, 15.0f, theme::TextDark);
    cy += 22.0f;
    if (uiRadioButton({cx + 10.0f, cy + 12.0f}, 7.0f, "Usar Matriz", opMode_ == OpMode::Shearing)) {
        opMode_ = OpMode::Shearing;
    }
    cy += 30.0f;

    float cellW = 54.0f;
    float cellH = 26.0f;
    float gap = 5.0f;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            uiTextInput({cx + c * (cellW + gap), cy + r * (cellH + gap), cellW, cellH}, inShear_[r * 4 + c]);
        }
    }

    Rectangle execBtn = {cx + 4 * (cellW + gap) + 10.0f, cy, cw - (4 * (cellW + gap) + 10.0f), 4 * cellH + 3 * gap};
    if (uiButton(execBtn, "EXECUTA", false, true)) {
        executeSelectedOp();
    }
    cy += 4 * (cellH + gap) + 14.0f;

    float halfW = (cw - 10.0f) * 0.5f;
    if (uiButton({cx, cy, halfW, 30.0f}, "Resetar Objeto")) {
        resetObject();
    }
    if (uiButton({cx + halfW + 10.0f, cy, halfW, 30.0f}, "Rotação Demo (Y+15, X+10)")) {
        Mat4d r = multiplyMat4(Mat4d::rotationX(10.0), Mat4d::rotationY(15.0));
        Vec4d c = computeCentroid(vertices_);
        Mat4d xform = multiplyMat4(Mat4d::translation(c.x, c.y, c.z),
                                   multiplyMat4(r, Mat4d::translation(-c.x, -c.y, -c.z)));
        for (auto& v : vertices_) v = transformPoint(xform, v);
    }
    cy += 38.0f;

    drawUiText(TextFormat("Centro atual: (%.1f, %.1f, %.1f)  |  Zoom: %.2fx",
                          computeCentroid(vertices_).x,
                          computeCentroid(vertices_).y,
                          computeCentroid(vertices_).z,
                          viewZoom_),
               cx, cy, 12.0f, theme::TextMuted);
}

} // namespace cg
