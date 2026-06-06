// =============================================================================
//  Game/Graphics/VulkanContext.cpp
// =============================================================================

#include "VulkanContext.h"
#include <vector>
#include <cstring>

namespace gfx {

// =============================================================================
//  Ciclo de Vida
// =============================================================================

bool VulkanContext::init(bool enableValidationLayers,
                          const std::vector<const char*>& instanceExtensions) {
    if (m_initialized) return true;

    if (!createInstance(enableValidationLayers, instanceExtensions)) return false;
    if (!selectPhysicalDevice())                                      return false;
    if (!createLogicalDevice(enableValidationLayers))                 return false;

    m_initialized = true;
    return true;
}

void VulkanContext::shutdown() {
    if (!m_initialized) return;

    destroySurface();  // surface ANTES do device e instance

    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device        = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }

    m_physicalDevice = VK_NULL_HANDLE;
    m_families       = {};
    m_deviceProps    = {};
    m_initialized    = false;
}

// =============================================================================
//  Surface (Fase 2.3)
// =============================================================================

bool VulkanContext::createSurface(VkSurfaceKHR surface) {
    if (surface == VK_NULL_HANDLE) return false;
    destroySurface();   // substituir se ja existia uma
    m_surface = surface;
    return true;
}

void VulkanContext::destroySurface() {
    if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}

// =============================================================================
//  Criacao da Instance
// =============================================================================

bool VulkanContext::createInstance(bool enableValidation,
                                    const std::vector<const char*>& extensions) {
    std::vector<const char*> layers;
    if (enableValidation) {
        if (!checkValidationLayerSupport()) return false;
        layers.push_back(VALIDATION_LAYER);
    }

    VkApplicationInfo app{};
    app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName   = "ASCENDENDO";
    app.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app.pEngineName        = "ASCENDENDO Engine";
    app.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
    app.apiVersion         = TARGET_API_VERSION;

    VkInstanceCreateInfo ci{};
    ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo        = &app;
    ci.enabledLayerCount       = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames     = layers.empty() ? nullptr : layers.data();
    ci.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

    return vkCreateInstance(&ci, nullptr, &m_instance) == VK_SUCCESS;
}

// =============================================================================
//  Selecao da GPU
// =============================================================================

bool VulkanContext::selectPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(m_instance, &count, nullptr);
    if (count == 0) return false;

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(m_instance, &count, devices.data());

    VkPhysicalDevice fallback = VK_NULL_HANDLE;

    for (const auto& dev : devices) {
        if (!isDeviceSuitable(dev)) continue;

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(dev, &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            m_physicalDevice = dev;
            break;
        }
        if (fallback == VK_NULL_HANDLE) fallback = dev;
    }

    if (m_physicalDevice == VK_NULL_HANDLE) m_physicalDevice = fallback;
    if (m_physicalDevice == VK_NULL_HANDLE) return false;

    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProps);
    m_families = findQueueFamilies(m_physicalDevice);
    return true;
}

// =============================================================================
//  Criacao do Device Logico
// =============================================================================

bool VulkanContext::createLogicalDevice(bool enableValidation) {
    float priority = 1.0f;

    VkDeviceQueueCreateInfo queueCI{};
    queueCI.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCI.queueFamilyIndex = m_families.graphics;
    queueCI.queueCount       = 1;
    queueCI.pQueuePriorities = &priority;

    VkPhysicalDeviceFeatures features{};

    std::vector<const char*> layers;
    if (enableValidation) layers.push_back(VALIDATION_LAYER);

    // Fase 2.4: O Swapchain é obrigatório para o motor conseguir desenhar na janela.
    // Requeremos esta extensão diretamente ao criar o Logical Device.
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo ci{};
    ci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount    = 1;
    ci.pQueueCreateInfos       = &queueCI;
    ci.pEnabledFeatures        = &features;
    ci.enabledLayerCount       = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames     = layers.empty() ? nullptr : layers.data();
    ci.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    ci.ppEnabledExtensionNames = deviceExtensions.empty() ? nullptr : deviceExtensions.data();

    if (vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device) != VK_SUCCESS) {
        return false;
    }

    vkGetDeviceQueue(m_device, m_families.graphics, 0, &m_graphicsQueue);
    return true;
}

// =============================================================================
//  Auxiliares
// =============================================================================

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice dev) const {
    QueueFamilyIndices indices{};
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, nullptr);
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, families.data());

    for (uint32_t i = 0; i < count; ++i) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics = i;
            break;
        }
    }
    return indices;
}

bool VulkanContext::isDeviceSuitable(VkPhysicalDevice dev) const {
    if (!findQueueFamilies(dev).isComplete()) return false;

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(dev, &props);
    if (VK_API_VERSION_MINOR(props.apiVersion) < 3) return false;

    return true;
}

bool VulkanContext::checkValidationLayerSupport() const {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> available(count);
    vkEnumerateInstanceLayerProperties(&count, available.data());

    for (const auto& layer : available) {
        if (std::strcmp(layer.layerName, VALIDATION_LAYER) == 0) return true;
    }
    return false;
}

} // namespace gfx