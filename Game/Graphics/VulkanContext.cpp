// =============================================================================
//  Game/Graphics/VulkanContext.cpp
// =============================================================================

#include "Graphics/VulkanContext.h"
#include <vector>
#include <cstring>

namespace gfx {

// =============================================================================
//  Ciclo de Vida
// =============================================================================

bool VulkanContext::init(bool enableValidationLayers) {
    if (m_initialized) return true;  // idempotente

    if (!createInstance(enableValidationLayers))     return false;
    if (!selectPhysicalDevice())                     return false;
    if (!createLogicalDevice(enableValidationLayers)) return false;

    m_initialized = true;
    return true;
}

void VulkanContext::shutdown() {
    if (!m_initialized) return;

    // Ordem de destruicao: inversa da criacao
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);       // esperar que a GPU termine
        vkDestroyDevice(m_device, nullptr);
        m_device       = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE; // a fila e destruida com o device
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
//  Criacao da Instance
// =============================================================================

bool VulkanContext::createInstance(bool enableValidation) {
    // Verificar layers antes de tentar criar a instance
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
    ci.sType                 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo      = &app;
    ci.enabledLayerCount     = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames   = layers.empty() ? nullptr : layers.data();
    ci.enabledExtensionCount = 0;  // Fase 3: adicionar VK_KHR_surface aqui

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

    // Estrategia: preferir GPU discreta, aceitar integrada como fallback
    VkPhysicalDevice fallback = VK_NULL_HANDLE;

    for (const auto& dev : devices) {
        if (!isDeviceSuitable(dev)) continue;

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(dev, &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            m_physicalDevice = dev;
            break;  // Melhor opcao encontrada
        }
        if (fallback == VK_NULL_HANDLE) {
            fallback = dev;  // Guardar primeira opcao adequada como backup
        }
    }

    if (m_physicalDevice == VK_NULL_HANDLE) m_physicalDevice = fallback;
    if (m_physicalDevice == VK_NULL_HANDLE) return false;

    // Guardar propriedades e indices de filas da GPU escolhida
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
    // Fase 3: ativar features aqui (geometryShader, samplerAnisotropy, etc.)

    std::vector<const char*> layers;
    if (enableValidation) layers.push_back(VALIDATION_LAYER);

    VkDeviceCreateInfo ci{};
    ci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount    = 1;
    ci.pQueueCreateInfos       = &queueCI;
    ci.pEnabledFeatures        = &features;
    ci.enabledLayerCount       = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames     = layers.empty() ? nullptr : layers.data();
    ci.enabledExtensionCount   = 0;  // Fase 3: adicionar VK_KHR_swapchain aqui

    if (vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device) != VK_SUCCESS) {
        return false;
    }

    // Obter handle da fila grafica (indice 0 dentro da familia)
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
    // Criterio 1: tem fila grafica
    if (!findQueueFamilies(dev).isComplete()) return false;

    // Criterio 2: suporta Vulkan >= 1.3
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
