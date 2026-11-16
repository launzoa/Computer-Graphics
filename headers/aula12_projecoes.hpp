#ifndef AULA12_PROJECOES_HPP
#define AULA12_PROJECOES_HPP

#include "lab_module.hpp"
#include "ui_widgets.hpp"
#include <array>
#include <vector>

namespace cg {

/** @brief Vetor 3D em coordenadas homogêneas (x, y, z, w). */
struct Vec4d {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double w = 1.0;
};

/** @brief Matriz 4x4 para transformações geométricas 3D em coordenadas homogêneas. */
struct Mat4d {
    double m[4][4]{};

    /** @brief Retorna a matriz identidade 4x4. */
    static Mat4d identity();
    /** @brief Cria matriz de translação pelos deslocamentos (tx, ty, tz). */
    static Mat4d translation(double tx, double ty, double tz);
    /** @brief Cria matriz de escala local nos eixos X, Y e Z. */
    static Mat4d scaleLocal(double sx, double sy, double sz);
    /** @brief Cria matriz de escala global homogênea via fator 1/s na coordenada w. */
    static Mat4d scaleGlobal(double s);
    /** @brief Cria matriz de rotação em torno do eixo X (ângulo em graus). */
    static Mat4d rotationX(double degrees);
    /** @brief Cria matriz de rotação em torno do eixo Y (ângulo em graus). */
    static Mat4d rotationY(double degrees);
    /** @brief Cria matriz de rotação em torno do eixo Z (ângulo em graus). */
    static Mat4d rotationZ(double degrees);
};

/** @brief Multiplica duas matrizes homogêneas 4x4 (a * b). */
Mat4d multiplyMat4(const Mat4d& a, const Mat4d& b);

/** @brief Aplica a matriz 4x4 ao vetor coluna v e realiza a divisão homogênea por w. */
Vec4d transformPoint(const Mat4d& mat, const Vec4d& v);

/** @brief Aresta conectando dois índices de vértices da malha 3D. */
struct HouseEdge {
    int a;
    int b;
};

/** @brief Retorna os 10 vértices iniciais da casa 3D especificada na Aula 12a. */
std::vector<Vec4d> createDefaultHouseVertices();
/** @brief Retorna as 15 arestas da casa 3D (frente, fundo e profundidade). */
std::vector<HouseEdge> createDefaultHouseEdges();
/** @brief Calcula o centro geométrico (centroide) médio do conjunto de vértices. */
Vec4d computeCentroid(const std::vector<Vec4d>& vertices);

/**
 * @brief Módulo prático da Aula 12a: transformações 3D homogêneas e projeção paralela no plano z = 0.
 */
class Aula12ProjecoesModule : public LabModule {
public:
    const char* menuLabel() const override { return "Aula 12A: Projeções 3D -> 2D"; }
    const char* title() const override { return "Aula 12a - Transformações Homogêneas 3D e Projeção Paralela (z = 0)"; }
    const char* subtitle() const override {
        return "Escala (Local/Global), Translação, Rotação (Origem/Centro), Shearing (Matriz 4x4) e projeção paralela (x, y).";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    enum class OpMode {
        ScaleLocal = 0,
        ScaleGlobal,
        Translation,
        RotationOrigin,
        RotationCenter,
        Shearing
    };

    void resetObject();
    void executeSelectedOp();
    Mat4d buildRotationMatrix(char axis, double degrees) const;

    std::vector<Vec4d> vertices_;
    std::vector<HouseEdge> edges_;

    OpMode opMode_ = OpMode::ScaleLocal;

    // Campos de entrada com os valores padrão exibidos no slide 4 da aula.
    UiTextInput inScaleX_, inScaleY_, inScaleZ_;
    UiTextInput inScaleGlobal_;
    UiTextInput inTransX_, inTransY_, inTransZ_;
    UiTextInput inRotAxis_, inRotDeg_;
    std::array<UiTextInput, 16> inShear_;

    // Controle de zoom e deslocamento da área de visualização mantendo projeção paralela (x, y).
    float viewZoom_ = 1.6f;
    Vector2 viewOffset_{0.0f, 0.0f};
    bool initialized_ = false;
};

} // namespace cg

#endif // AULA12_PROJECOES_HPP
