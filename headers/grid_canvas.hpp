#ifndef GRID_CANVAS_HPP
#define GRID_CANVAS_HPP

#include "raylib.h"
#include <vector>

namespace cg {

/** @brief Coordenada discreta (inteira) de um pixel/célula na grade de desenho. */
struct GridPoint {
    int x = 0; ///< Índice da coluna (eixo X da grade).
    int y = 0; ///< Índice da linha (eixo Y da grade).
};

/** @brief Pixel colorido armazenado na grade (posição discreta + cor RGBA). */
struct GridPixel {
    int x = 0;
    int y = 0;
    Color color = BLACK;
};

/**
 * @brief Componente reutilizável de grade para desenho e visualização de rasterização.
 *
 * Permite exibir pixels ampliados (cellSize > 1) para inspeção didática de
 * algoritmos de retas/circunferências ou operar em escala real 1:1.
 */
class GridCanvas {
public:
    /** @brief Constrói uma grade com tamanho inicial de célula em pixels. */
    explicit GridCanvas(int cellSize = 12);

    /** @brief Define o tamanho em pixels de cada célula da grade (limitado entre 1 e 40). */
    void setCellSize(int size);
    /** @brief Retorna o tamanho atual da célula em pixels. */
    int cellSize() const { return cellSize_; }

    /** @brief Calcula quantas colunas inteiras cabem na área informada. */
    int cols(Rectangle area) const;
    /** @brief Calcula quantas linhas inteiras cabem na área informada. */
    int rows(Rectangle area) const;

    /** @brief Verifica se uma posição da tela está contida na área da grade. */
    bool containsScreenPoint(Vector2 screenPos, Rectangle area) const;
    /** @brief Converte coordenadas de tela (mouse) para coordenadas discretas da grade. */
    GridPoint screenToGrid(Vector2 screenPos, Rectangle area) const;
    /** @brief Retorna o centro em pixels de tela correspondente à célula (gx, gy). */
    Vector2 gridToScreenCenter(int gx, int gy, Rectangle area) const;
    /** @brief Retorna o retângulo em pixels de tela da célula (gx, gy). */
    Rectangle gridToScreenRect(int gx, int gy, Rectangle area) const;

    /** @brief Verifica se (gx, gy) está dentro dos limites visíveis da grade. */
    bool isInside(int gx, int gy, Rectangle area) const;

    /** @brief Renderiza o fundo e as linhas menores/maiores da grade. */
    void drawBackgroundAndGrid(Rectangle area, bool showGrid = true, int majorStep = 5) const;
    /** @brief Pinta um pixel discreto na posição (gx, gy) com a cor especificada. */
    void drawPixel(Rectangle area, int gx, int gy, Color color) const;
    /** @brief Renderiza uma coleção de pixels sobre a grade com recorte de área (scissor). */
    void drawPixels(Rectangle area, const std::vector<GridPixel>& pixels) const;
    /** @brief Destaca a célula sob o cursor do mouse e exibe suas coordenadas (x, y). */
    void drawHoverIndicator(Rectangle area, bool showLabel = true) const;

private:
    int cellSize_ = 12;
};

} // namespace cg

#endif // GRID_CANVAS_HPP
