#pragma once
// =============================================================================
//  Game/Graphics/VulkanContext.h
//
//  Abstracao da camada grafica Vulkan: Instance + PhysicalDevice + Device.
// =============================================================================

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

namespace gfx {

struct QueueFamilyIndices {
    uint32_t graphics = UINT32_MAX;
    uint32_t present = UINT32_MAX;

    bool hasGraphics() const { return graphics != UINT32_MAX; }
    bool hasPresent() const { return present != UINT32_MAX; }
    bool isComplete() const { return hasGraphics(); }
    bool isCompleteForPresentation() const { return hasGraphics() && hasPresent(); }
};

class VulkanContext {
public:
    VulkanContext()  = default;
    ~VulkanContext() { shutdown(); }

    VulkanContext(const VulkanContext&)            = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&)                 = delete;
    VulkanContext& operator=(VulkanContext&&)      = delete;

    bool init(bool enableValidationLayers = false,
              const std::vector<const char*>& instanceExtensions = {});
    void shutdown();

    bool createSurface(VkSurfaceKHR surface);
    void destroySurface();

    VkInstance         instance()        const { return m_instance;        }
    VkPhysicalDevice   physicalDevice()  const { return m_physicalDevice;  }
    VkDevice           device()          const { return m_device;          }
    VkQueue            graphicsQueue()   const { return m_graphicsQueue;   }
    VkQueue            presentQueue()    const { return m_presentQueue;    }
    uint32_t           graphicsFamily()  const { return m_families.graphics; }
    uint32_t           presentFamily()   const { return m_families.present; }
    VkSurfaceKHR       surface()         const { return m_surface;         }

    const VkPhysicalDeviceProperties& deviceProperties() const { return m_deviceProps; }
    bool isInitialized() const { return m_initialized; }

private:
    bool createInstance(bool enableValidation,
                        const std::vector<const char*>& extensions);
    bool selectPhysicalDevice();
    bool createLogicalDevice(bool enableValidation);
    bool reconfigureForSurface();
    bool supportsRequiredDeviceExtensions(VkPhysicalDevice dev) const;

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice dev,
                                         VkSurfaceKHR surface = VK_NULL_HANDLE) const;
    bool               isDeviceSuitable(VkPhysicalDevice dev) const;
    bool               isDeviceSuitableForSurface(VkPhysicalDevice dev,
                                                   VkSurfaceKHR surface,
                                                   QueueFamilyIndices& indices) const;
    bool               checkValidationLayerSupport()            const;

    VkInstance                  m_instance       = VK_NULL_HANDLE;
    VkPhysicalDevice            m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                    m_device         = VK_NULL_HANDLE;
    VkQueue                     m_graphicsQueue  = VK_NULL_HANDLE;
    VkQueue                     m_presentQueue   = VK_NULL_HANDLE;
    VkSurfaceKHR                m_surface        = VK_NULL_HANDLE;
    QueueFamilyIndices          m_families       = {};
    VkPhysicalDeviceProperties  m_deviceProps    = {};
    bool                        m_initialized    = false;

    static constexpr const char* VALIDATION_LAYER  = "VK_LAYER_KHRONOS_validation";
    static constexpr uint32_t    TARGET_API_VERSION = VK_API_VERSION_1_3;
};

} // namespace gfx