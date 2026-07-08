#pragma once
// =============================================================================
//  Game/Graphics/SpriteRenderer.h
//
//  @version 8.2
//  Carrega um PNG (via stb_image) e envia-o para a GPU como textura RGBA
//  amostravel com filtro NEAREST (preserva as arestas nitidas do pixel
//  art — LINEAR, usado no FontRenderer, borraria os pixeis do sprite).
//
//  Suporta UM sprite estatico por instancia (sem folhas de sprite/atlas
//  multi-frame por agora — a .pixil carregada tem 1 frame/1 layer). Se no
//  futuro houver animacao por frames, esta classe pode evoluir para um
//  atlas com varios quadros, tal como o FontRenderer faz com glyphs.
//
//  Fluxo:
//    SpriteRenderer sprite;
//    sprite.init(&ctx, spritePipeline.descriptorSetLayout(), "Game/Assets/Sprites/personagem.png");
//    // dentro do render pass, apos vkCmdBindPipeline(spritePipeline):
//    sprite.bind(cmd, spritePipeline.layout());
//    sprite.draw(cmd, spritePipeline.layout(), x, y, w, h, flipX, r,g,b,a, camPos);
// =============================================================================
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace gfx {

class VulkanContext;

class SpriteRenderer {
public:
    SpriteRenderer()  = default;
    ~SpriteRenderer() { cleanup(); }

    SpriteRenderer(const SpriteRenderer&)            = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    // pngPath: caminho para o ficheiro .png (ja convertido de .pixil).
    bool init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
              const std::string& pngPath = "Game/Assets/Sprites/personagem.png");
    void cleanup();

    // Vincula o descriptor set do sprite. Chamar uma vez, apos
    // vkCmdBindPipeline(spritePipeline.handle()).
    void bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

    // Desenha o sprite num rectangulo (x,y)=canto inferior-esquerdo, (w,h).
    // flipX: espelha horizontalmente (jogador virado para a esquerda).
    // camPosX/Y: posicao da camara (0,0 para elementos de UI sem camera).
    void draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
              float x, float y, float w, float h, bool flipX,
              float r, float g, float b, float a,
              float camPosX, float camPosY) const;

    int width()  const { return m_width; }
    int height() const { return m_height; }
    bool isInitialized() const { return m_initialized; }

private:
    bool loadPNG(const std::string& path, std::vector<unsigned char>& outPixels);
    bool createTextureImage(const std::vector<unsigned char>& pixels);
    bool createDescriptorSet(VkDescriptorSetLayout layout);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const;

    VulkanContext* m_ctx = nullptr;
    int m_width  = 0;
    int m_height = 0;

    VkImage          m_image       = VK_NULL_HANDLE;
    VkDeviceMemory   m_imageMemory = VK_NULL_HANDLE;
    VkImageView      m_imageView   = VK_NULL_HANDLE;
    VkSampler        m_sampler     = VK_NULL_HANDLE;
    VkDescriptorPool m_descPool    = VK_NULL_HANDLE;
    VkDescriptorSet  m_descSet     = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx
