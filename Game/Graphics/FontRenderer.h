#pragma once
// =============================================================================
//  Game/Graphics/FontRenderer.h
//
//  @version 7.6
//  Rasteriza um TTF (via stb_truetype, baking offline em RAM) para um atlas
//  R8_UNORM na GPU, e desenha texto real (anti-aliased) como uma sequencia
//  de quads texturados atraves de TextPipeline.
//
//  Fluxo:
//    FontRenderer font;
//    font.init(&ctx, textPipeline.descriptorSetLayout(), "Game/Assets/Fonts/UIFont.ttf");
//    // por frame, dentro do render pass, apos vkCmdBindPipeline(textPipeline):
//    font.bind(cmd, textPipeline.layout());
//    font.drawText(cmd, textPipeline.layout(), "ASCENDENDO", x, baseline_y, 5.0f, r,g,b,a);
//
//  scale: 1.0 == altura de baking (ver BAKE_PIXEL_HEIGHT). Para texto maior/
//  menor, ajustar scale (o atlas e reamostrado, ligeira perda de nitidez em
//  escalas muito grandes — aceitavel para titulos/menus; nota de melhoria
//  futura: baking multi-resolucao).
// =============================================================================
#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>
#include <vector>

namespace gfx {

class VulkanContext;

class FontRenderer {
public:
    FontRenderer()  = default;
    ~FontRenderer() { cleanup(); }

    FontRenderer(const FontRenderer&)            = delete;
    FontRenderer& operator=(const FontRenderer&) = delete;

    // ttfPath: caminho para o ficheiro .ttf (relativo ao working directory
    // do executavel, tal como os shaders .spv). descriptorSetLayout: obtido
    // de TextPipeline::descriptorSetLayout() — deve ser criado ANTES.
    bool init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
              const std::string& ttfPath = "Game/Assets/Fonts/UIFont.ttf",
              float bakePixelHeight = 48.0f);
    void cleanup();

    // Vincula o descriptor set do atlas. Chamar uma vez, depois de
    // vkCmdBindPipeline(textPipeline.handle()) e antes de qualquer drawText.
    void bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

    // Desenha texto a partir de (x, baseline_y) no espaco logico (Y-cima).
    // scale=1.0 -> altura aproximada = bakePixelHeight unidades logicas.
    void drawText(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                  const char* text, float x, float baseline_y, float scale,
                  float r, float g, float b, float a) const;

    // Centrado horizontalmente em cx.
    void drawTextCentered(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                          const char* text, float cx, float baseline_y, float scale,
                          float r, float g, float b, float a) const;

    // Largura do texto renderizado (unidades logicas, ao dado scale).
    float textWidth(const char* text, float scale) const;

    bool isInitialized() const { return m_initialized; }

private:
    struct BakedChar { // espelha stbtt_bakedchar (evita expor stb_truetype.h no header publico)
        uint16_t x0, y0, x1, y1; // regiao no atlas (pixeis)
        float xoff, yoff, xadvance;
    };

    bool loadAndBakeFont(const std::string& ttfPath, float pixelHeight,
                         std::vector<uint8_t>& outAtlasPixels);
    bool createAtlasImage(const std::vector<uint8_t>& pixels);
    bool createDescriptorSet(VkDescriptorSetLayout layout);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const;

    VulkanContext* m_ctx = nullptr;

    static constexpr int ATLAS_W = 512;
    static constexpr int ATLAS_H = 512;
    static constexpr int FIRST_CHAR = 32;   // ' '
    static constexpr int NUM_CHARS  = 95;   // ate '~' (126)

    BakedChar m_chars[NUM_CHARS];
    float     m_bakePixelHeight = 48.0f;

    VkImage        m_image        = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory  = VK_NULL_HANDLE;
    VkImageView    m_imageView    = VK_NULL_HANDLE;
    VkSampler      m_sampler      = VK_NULL_HANDLE;
    VkDescriptorPool m_descPool   = VK_NULL_HANDLE;
    VkDescriptorSet  m_descSet    = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx
