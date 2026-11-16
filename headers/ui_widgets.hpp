#ifndef UI_WIDGETS_HPP
#define UI_WIDGETS_HPP

#include "raylib.h"
#include <string>

namespace cg {

// Paleta de cores padronizada da interface gráfica (barra lateral, grade e cartões).
namespace theme {
inline constexpr Color SidebarBg       = {24, 32, 46, 255};
inline constexpr Color SidebarHover    = {41, 53, 75, 255};
inline constexpr Color SidebarActive   = {37, 99, 235, 255};
inline constexpr Color SidebarText     = {236, 242, 248, 255};
inline constexpr Color SidebarMuted    = {148, 163, 184, 255};

inline constexpr Color PanelBg         = {241, 245, 249, 255};
inline constexpr Color PanelBorder     = {203, 213, 225, 255};
inline constexpr Color CanvasBg        = {252, 253, 255, 255};
inline constexpr Color GridMinor       = {226, 232, 240, 255};
inline constexpr Color GridMajor       = {186, 198, 214, 255};
inline constexpr Color GridAxis        = {100, 116, 139, 255};

inline constexpr Color TextDark        = {15, 23, 42, 255};
inline constexpr Color TextMuted       = {71, 85, 105, 255};
inline constexpr Color Accent          = {37, 99, 235, 255};
inline constexpr Color AccentHover     = {29, 78, 216, 255};
inline constexpr Color Success         = {22, 163, 74, 255};
inline constexpr Color Danger          = {220, 38, 38, 255};
inline constexpr Color Warning         = {217, 119, 6, 255};
} // namespace theme

/**
 * @brief Estado e utilitários de conversão para caixas de entrada editáveis.
 */
struct UiTextInput {
    std::string text;          ///< Conteúdo textual atual digitado no campo.
    bool focused = false;      ///< Indica se o campo está ativo recebendo eventos do teclado.
    bool allowFloat = true;    ///< Permite ponto decimal ('.') em campos numéricos.
    bool allowSign = true;     ///< Permite sinal ('-' ou '+') no início do número.
    bool numericOnly = false;  ///< Restringe a digitação apenas a caracteres numéricos.
    int maxChars = 32;         ///< Quantidade máxima de caracteres aceitos.

    /** @brief Converte o texto atual para double, retornando fallback em caso inválido. */
    double asDouble(double fallback = 0.0) const;
    /** @brief Converte o texto atual para int, retornando fallback em caso inválido. */
    int asInt(int fallback = 0) const;
    /** @brief Formata e atribui um valor decimal ao texto do campo. */
    void setDouble(double value, int decimals = 2);
    /** @brief Formata e atribui um valor inteiro ao texto do campo. */
    void setInt(int value);
};

/** @brief Carrega a fonte TTF do sistema com glifos Latin-1 (acentos em português). */
void initUiFonts();
/** @brief Libera a fonte carregada na GPU e limpa o foco de entrada. */
void unloadUiFonts();
/** @brief Retorna a referência da fonte ativa da interface. */
const Font& getUiFont();
/** @brief Indica se existe algum campo UiTextInput atualmente focado. */
bool isAnyTextInputFocused();
/** @brief Remove o foco de qualquer campo UiTextInput ativo. */
void clearTextInputFocus();

/** @brief Desenha texto usando a fonte padrão da interface. */
void drawUiText(const char* text, float x, float y, float fontSize, Color color);
/** @brief Mede a largura em pixels de uma string na fonte da interface. */
float measureUiText(const char* text, float fontSize);

/**
 * @brief Botão clicável com suporte a estado selecionado e destaque primário.
 * @return true no frame em que o botão for clicado.
 */
bool uiButton(Rectangle rect, const char* label, bool selected = false, bool accent = false);

/**
 * @brief Botão de seleção exclusiva (radio button).
 * @return true no frame em que for clicado.
 */
bool uiRadioButton(Vector2 center, float radius, const char* label, bool selected);

/**
 * @brief Caixa de marcação booleana (checkbox).
 * @return true no frame em que o valor de *checked for alternado.
 */
bool uiCheckBox(Rectangle box, const char* label, bool* checked);

/**
 * @brief Controle deslizante para valores inteiros dentro de [minVal, maxVal].
 * @return true caso o valor inteiro tenha sido alterado pelo usuário.
 */
bool uiSliderInt(Rectangle rect, const char* label, int* value, int minVal, int maxVal);

/**
 * @brief Controle deslizante para valores em ponto flutuante dentro de [minVal, maxVal].
 * @return true caso o valor decimal tenha sido alterado pelo usuário.
 */
bool uiSliderFloat(Rectangle rect, const char* label, float* value, float minVal, float maxVal, const char* fmt = "%.2f");

/**
 * @brief Campo de texto editável com cursor piscante e validação opcional.
 * @return true sempre que o texto do campo for modificado pelo teclado.
 */
bool uiTextInput(Rectangle rect, UiTextInput& input, const char* label = nullptr);

/** @brief Desenha um cartão/painel branco com borda e cabeçalho opcional. */
void drawPanelCard(Rectangle rect, const char* title = nullptr);

} // namespace cg

#endif // UI_WIDGETS_HPP
