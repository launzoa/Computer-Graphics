#ifndef AULA04_PIXELS_HPP
#define AULA04_PIXELS_HPP

#include "lab_module.hpp"
#include "ui_widgets.hpp"
#include <string>

namespace cg {

/**
 * @brief Lê diretamente a cor RGBA do pixel (x, y) no buffer da imagem.
 * @param img Imagem de origem (preferencialmente em formato RGBA8 não comprimido).
 * @param x Coordenada horizontal do pixel.
 * @param y Coordenada vertical do pixel.
 * @return Cor RGBA do pixel, ou BLANK caso fora dos limites.
 */
Color getImagePixel(const Image& img, int x, int y);

/**
 * @brief Grava diretamente a cor RGBA no pixel (x, y) do buffer da imagem.
 * @param img Imagem de destino a ser modificada.
 * @param x Coordenada horizontal do pixel.
 * @param y Coordenada vertical do pixel.
 * @param color Nova cor RGBA.
 */
void setImagePixel(Image& img, int x, int y, Color color);

/**
 * @brief Gera uma imagem colorida sintética para testes (variante 0 ou 1).
 * @param width Largura da imagem em pixels.
 * @param height Altura da imagem em pixels.
 * @param variant Índice do padrão visual (0 = gradiente + formas, 1 = barras + anéis).
 * @return Nova Image alocada em RAM (deve ser liberada com UnloadImage).
 */
Image createSampleColorImage(int width, int height, int variant);

/**
 * @brief Converte uma imagem colorida para tons de cinza.
 * @param src Imagem original.
 * @param luminanceWeighted Se true, usa I = 0.299R + 0.587G + 0.114B; se false, usa (R+G+B)/3.
 * @return Nova Image em tons de cinza.
 */
Image makeGrayscaleImage(const Image& src, bool luminanceWeighted = true);

/**
 * @brief Gera a versão invertida (negativo / complemento CMY: 255 - RGB) da imagem.
 * @param src Imagem original.
 * @return Nova Image com canais invertidos.
 */
Image makeInvertedImage(const Image& src);

/**
 * @brief Isola um canal individual (0=R, 1=G, 2=B) da imagem.
 * @param src Imagem original.
 * @param channelIndex Índice do canal (0=Vermelho, 1=Verde, 2=Azul).
 * @param tinted Se true, mantém na cor pura do canal; se false, exibe intensidade em PB.
 * @return Nova Image contendo apenas o canal selecionado.
 */
Image makeChannelImage(const Image& src, int channelIndex, bool tinted = true);

/**
 * @brief Abstração de cursor de desenho MoveTo / LineTo (estilo Canvas do Lazarus).
 *
 * Mantém a posição atual da caneta e utiliza o comando pronto ImageDrawLine
 * da raylib para traçar segmentos de reta.
 */
class PenCanvas {
public:
    explicit PenCanvas(Image* target = nullptr);

    /** @brief Define a imagem de destino onde as linhas serão rasterizadas. */
    void setTarget(Image* target) { target_ = target; }
    /** @brief Posiciona a caneta virtual em (x, y) sem desenhar. */
    void moveTo(int x, int y);
    /** @brief Traça uma reta da posição atual até (x, y) e atualiza a posição da caneta. */
    void lineTo(int x, int y, Color color);

    int penX() const { return penX_; }
    int penY() const { return penY_; }

private:
    Image* target_ = nullptr;
    int penX_ = 0;
    int penY_ = 0;
};

/**
 * @brief Módulo prático da Aula 04: carregamento, filtros, canais, acesso a pixels e MoveTo/LineTo.
 */
class Aula04PixelsModule : public LabModule {
public:
    Aula04PixelsModule() = default;
    ~Aula04PixelsModule() override;

    const char* menuLabel() const override { return "Aula 04: Prática com Pixels"; }
    const char* title() const override { return "Aula 04 - Acesso aos Pixels, Canais e MoveTo/LineTo"; }
    const char* subtitle() const override {
        return "Carregar imagem (ou arrastar arquivo), tons de cinza, inversão, canais R/G/B, acesso a pixels e MoveTo/LineTo.";
    }

    void onEnter() override;
    void update(Rectangle area) override;
    void draw(Rectangle area) override;

private:
    enum class ViewMode {
        EditableOriginal = 0,
        Grayscale,
        Inverted,
        ChannelR,
        ChannelG,
        ChannelB,
        SplitChannels
    };

    enum class ToolMode {
        InspectAndPaint = 0,
        MoveLineTo
    };

    void loadSample(int variant);
    bool loadFromFile(const std::string& path);
    void rebuildDerivedTextures();
    void releaseResources();
    Rectangle computeCanvasArea(Rectangle area) const;
    Rectangle computeImageRect(Rectangle canvasArea) const;

    Image baseImage_{};
    Texture2D texEditable_{};
    Texture2D texGray_{};
    Texture2D texInv_{};
    Texture2D texR_{};
    Texture2D texG_{};
    Texture2D texB_{};
    bool hasImage_ = false;
    bool initialized_ = false;

    ViewMode viewMode_ = ViewMode::EditableOriginal;
    ToolMode toolMode_ = ToolMode::InspectAndPaint;
    bool weightedGray_ = true;
    bool tintedChannels_ = true;
    int brushColorIdx_ = 0;
    int sampleVariant_ = 0;

    PenCanvas pen_;
    UiTextInput pathInput_;
    std::string statusMsg_;
};

} // namespace cg

#endif // AULA04_PIXELS_HPP
