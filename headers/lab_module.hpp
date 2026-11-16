#ifndef LAB_MODULE_HPP
#define LAB_MODULE_HPP

#include "raylib.h"

namespace cg {

/**
 * @brief Interface base abstrata para todos os módulos práticos exibidos no menu.
 *
 * Cada aula prática herda desta classe para padronizar identificação no menu,
 * inicialização ao entrar na tela, processamento de entrada e renderização.
 */
class LabModule {
public:
    virtual ~LabModule() = default;

    /** @brief Rótulo curto exibido no botão da barra lateral de navegação. */
    virtual const char* menuLabel() const = 0;

    /** @brief Título principal exibido no cabeçalho superior da janela. */
    virtual const char* title() const = 0;

    /** @brief Subtítulo explicativo com resumo da prática e instruções de uso. */
    virtual const char* subtitle() const = 0;

    /** @brief Hook executado sempre que o usuário seleciona este módulo. */
    virtual void onEnter() {}

    /**
     * @brief Atualiza o estado do módulo (interações de mouse e teclado).
     * @param area Retângulo útil disponível para o módulo (excluindo barra lateral e cabeçalho).
     */
    virtual void update(Rectangle area) = 0;

    /**
     * @brief Renderiza os elementos visuais e controles do módulo na tela.
     * @param area Retângulo útil disponível para o módulo.
     */
    virtual void draw(Rectangle area) = 0;
};

} // namespace cg

#endif // LAB_MODULE_HPP
