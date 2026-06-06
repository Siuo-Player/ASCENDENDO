#pragma once
// =============================================================================
//  Game/Graphics/VulkanContext.h
//
//  Primeira abstracao da camada grafica: encapsula os objetos Vulkan "core"
//  necessarios antes de qualquer renderizacao.
//
//  Fase 2 (headless): Instance + PhysicalDevice + Device + fila grafica.
//  Fase 3 (com janela): adicionar Surface + Swapchain numa classe separada.
//
//  Utilizacao:
//    gfx::VulkanContext ctx;
//    if (!ctx.init()) { /* erro */ }
//    // ... usar ctx.device(), ctx.graphicsQueue() ...
//    ctx.shutdown();   // ou deixar o destrutor tratar
// =============================================================================

#include <vulkan/vulkan.h>
#include <cstdint>

namespace gfx {

// -----------------------------------------------------------------------------
//  Indices das familias de filas disponiveis na GPU selecionada.
//  UINT32_MAX == "nao encontrado".
// -----------------------------------------------------------------------------
struct QueueFamilyIndices {
    uint32_t graphics = UINT32_MAX;

    bool isComplete() const { return graphics != UINT32_MAX; }
};

// -----------------------------------------------------------------------------
//  VulkanContext — ciclo de vida: construir → init() → usar → shutdown()
// -----------------------------------------------------------------------------
class VulkanContext {
public:
    VulkanContext()  = default;
    ~VulkanContext() { shutdown(); }

    // Nao copiavel, nao movivel (os handles Vulkan sao recursos unicos)
    VulkanContext(const VulkanContext&)            = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&)                 = delete;
    VulkanContext& operator=(VulkanContext&&)      = delete;

    // ── Ciclo de Vida ─────────────────────────────────────────────────────────

    // Inicializa Instance + escolhe GPU + cria Device.
    // enableValidationLayers: ligar so em modo debug (requer Vulkan SDK instalado).
    // Retorna false em qualquer falha (sem excecoes).
    bool init(bool enableValidationLayers = false);

    // Destroi todos os recursos Vulkan. Idempotente: seguro chamar multiplas vezes.
    void shutdown();

    // ── Acessores (so validos apos init() == true) ────────────────────────────
    VkInstance         instance()        const { return m_instance;        }
    VkPhysicalDevice   physicalDevice()  const { return m_physicalDevice;  }
    VkDevice           device()          const { return m_device;          }
    VkQueue            graphicsQueue()   const { return m_graphicsQueue;   }
    uint32_t           graphicsFamily()  const { return m_families.graphics; }

    const VkPhysicalDeviceProperties& deviceProperties() const { return m_deviceProps; }

    bool isInitialized() const { return m_initialized; }

private:
    // Etapas de inicializacao (cada uma retorna false em falha)
    bool createInstance(bool enableValidation);
    bool selectPhysicalDevice();
    bool createLogicalDevice(bool enableValidation);

    // Auxiliares
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev) const;
    bool isDeviceSuitable(VkPhysicalDevice dev)                const;
    bool checkValidationLayerSupport()                         const;

    // ── Estado interno ────────────────────────────────────────────────────────
    VkInstance               m_instance       = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_device         = VK_NULL_HANDLE;
    VkQueue                  m_graphicsQueue  = VK_NULL_HANDLE;
    QueueFamilyIndices       m_families       = {};
    VkPhysicalDeviceProperties m_deviceProps  = {};
    bool                     m_initialized    = false;

    static constexpr const char* VALIDATION_LAYER  = "VK_LAYER_KHRONOS_validation";
    static constexpr uint32_t    TARGET_API_VERSION = VK_API_VERSION_1_3;
};

} // namespace gfx
