#pragma once
// =============================================================================
//  Game/Graphics/VulkanContext.h
//
//  Abstracao da camada grafica Vulkan: Instance + PhysicalDevice + Device.
//
//  Fase 2.2 (headless): init() sem extensoes — nao requer janela.
//  Fase 2.3 (com janela): passar extensoes da Window (VK_KHR_surface, etc.)
//    e chamar createSurface() apos init().
//
//  Exemplo com janela:
//    gfx::Window win;
//    win.create(1280, 720, "ASCENDENDO");
//
//    std::vector<const char*> exts;
//    win.appendRequiredExtensions(exts);
//
//    gfx::VulkanContext ctx;
//    ctx.init(false, exts);
//    ctx.createSurface(win.createVulkanSurface(ctx.instance()));
// =============================================================================

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

namespace gfx {

struct QueueFamilyIndices {
    uint32_t graphics = UINT32_MAX;
    bool isComplete() const { return graphics != UINT32_MAX; }
};

class VulkanContext {
public:
    VulkanContext()  = default;
    ~VulkanContext() { shutdown(); }

    VulkanContext(const VulkanContext&)            = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&)                 = delete;
    VulkanContext& operator=(VulkanContext&&)      = delete;

    // ── Ciclo de Vida ─────────────────────────────────────────────────────────

    // Fase 2.2: init() headless (sem extensoes).
    // Fase 2.3: passar extensoes para suporte a surface (VK_KHR_surface, etc.).
    bool init(bool enableValidationLayers = false,
              const std::vector<const char*>& instanceExtensions = {});

    // Destroi todos os recursos Vulkan. Idempotente.
    void shutdown();

    // ── Surface (Fase 2.3) ────────────────────────────────────────────────────

    // Associa uma surface ja criada (tipicamente por Window::createVulkanSurface).
    // Deve ser chamado APOS init() e ANTES de criar a swapchain.
    bool createSurface(VkSurfaceKHR surface);

    void destroySurface();

    // ── Acessores ─────────────────────────────────────────────────────────────
    VkInstance         instance()        const { return m_instance;        }
    VkPhysicalDevice   physicalDevice()  const { return m_physicalDevice;  }
    VkDevice           device()          const { return m_device;          }
    VkQueue            graphicsQueue()   const { return m_graphicsQueue;   }
    uint32_t           graphicsFamily()  const { return m_families.graphics; }
    VkSurfaceKHR       surface()         const { return m_surface;         }

    const VkPhysicalDeviceProperties& deviceProperties() const { return m_deviceProps; }

    bool isInitialized() const { return m_initialized; }

private:
    bool createInstance(bool enableValidation,
                        const std::vector<const char*>& extensions);
    bool selectPhysicalDevice();
    bool createLogicalDevice(bool enableValidation);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev) const;
    bool               isDeviceSuitable(VkPhysicalDevice dev)  const;
    bool               checkValidationLayerSupport()            const;

    VkInstance               m_instance       = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_device         = VK_NULL_HANDLE;
    VkQueue                  m_graphicsQueue  = VK_NULL_HANDLE;
    VkSurfaceKHR             m_surface        = VK_NULL_HANDLE;
    QueueFamilyIndices       m_families       = {};
    VkPhysicalDeviceProperties m_deviceProps  = {};
    bool                     m_initialized    = false;

    static constexpr const char* VALIDATION_LAYER   = "VK_LAYER_KHRONOS_validation";
    static constexpr uint32_t    TARGET_API_VERSION  = VK_API_VERSION_1_3;
};

} // namespace gfx
