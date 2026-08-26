// =============================================================================
//  Game/Graphics/VulkanContext.cpp
// =============================================================================

#include "VulkanContext.h"
#include <vector>
#include <cstring>
#include <set>

namespace gfx {

bool VulkanContext::init(bool enableValidationLayers,
                          const std::vector<const char*>& instanceExtensions) {
    if (m_initialized) return true;

    if (!createInstance(enableValidationLayers, instanceExtensions)) return false;
    if (!selectPhysicalDevice()) {
        shutdown();
        return false;
    }
    if (!createLogicalDevice(enableValidationLayers)) {
        shutdown();
        return false;
    }

    m_initialized = true;
    return true;
}

void VulkanContext::shutdown() {
    destroySurface();

    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device        = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE;
        m_presentQueue  = VK_NULL_HANDLE;
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

bool VulkanContext::createSurface(VkSurfaceKHR surface) {
    if (surface == VK_NULL_HANDLE) return false;
    destroySurface();
    m_surface = surface;

    if (!reconfigureForSurface()) {
        destroySurface();
        return false;
    }
    return true;
}

void VulkanContext::destroySurface() {
    if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
}

bool VulkanContext::reconfigureForSurface() {
    if (m_instance == VK_NULL_HANDLE || m_surface == VK_NULL_HANDLE) return false;

    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(m_instance, &count, nullptr) != VK_SUCCESS || count == 0) {
        return false;
    }

    std::vector<VkPhysicalDevice> devices(count);
    if (vkEnumeratePhysicalDevices(m_instance, &count, devices.data()) != VK_SUCCESS) {
        return false;
    }

    VkPhysicalDevice selected = VK_NULL_HANDLE;
    QueueFamilyIndices selectedFamilies{};
    VkPhysicalDevice fallback = VK_NULL_HANDLE;
    QueueFamilyIndices fallbackFamilies{};

    for (const auto& dev : devices) {
        QueueFamilyIndices indices{};
        if (!isDeviceSuitableForSurface(dev, m_surface, indices)) continue;

        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(dev, &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            selected = dev;
            selectedFamilies = indices;
            break;
        }
        if (fallback == VK_NULL_HANDLE) {
            fallback = dev;
            fallbackFamilies = indices;
        }
    }

    if (selected == VK_NULL_HANDLE) {
        selected = fallback;
        selectedFamilies = fallbackFamilies;
    }
    if (selected == VK_NULL_HANDLE) return false;

    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
        m_graphicsQueue = VK_NULL_HANDLE;
        m_presentQueue = VK_NULL_HANDLE;
    }

    m_physicalDevice = selected;
    m_families = selectedFamilies;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProps);

    // Recreate the logical device using the surface-aware queue selection.
    // createLogicalDevice() consumes m_families, so graphics and present are
    // both explicitly represented even when they happen to be the same family.
    return createLogicalDevice(false);
}

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

bool VulkanContext::selectPhysicalDevice() {
    uint32_t count = 0;
    if (vkEnumeratePhysicalDevices(m_instance, &count, nullptr) != VK_SUCCESS || count == 0) return false;

    std::vector<VkPhysicalDevice> devices(count);
    if (vkEnumeratePhysicalDevices(m_instance, &count, devices.data()) != VK_SUCCESS) return false;

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

bool VulkanContext::createLogicalDevice(bool enableValidation) {
    if (!m_families.hasGraphics() || !m_families.hasPresent()) {
        return false;
    }

    float priority = 1.0f;
    std::set<uint32_t> uniqueFamilies = {m_families.graphics, m_families.present};
    std::vector<VkDeviceQueueCreateInfo> queueCIs;
    queueCIs.reserve(uniqueFamilies.size());

    for (uint32_t family : uniqueFamilies) {
        VkDeviceQueueCreateInfo queueCI{};
        queueCI.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCI.queueFamilyIndex = family;
        queueCI.queueCount       = 1;
        queueCI.pQueuePriorities = &priority;
        queueCIs.push_back(queueCI);
    }

    VkPhysicalDeviceFeatures features{};

    std::vector<const char*> layers;
    if (enableValidation) layers.push_back(VALIDATION_LAYER);

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo ci{};
    ci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount    = static_cast<uint32_t>(queueCIs.size());
    ci.pQueueCreateInfos       = queueCIs.data();
    ci.pEnabledFeatures        = &features;
    ci.enabledLayerCount       = static_cast<uint32_t>(layers.size());
    ci.ppEnabledLayerNames     = layers.empty() ? nullptr : layers.data();
    ci.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    ci.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(m_physicalDevice, &ci, nullptr, &m_device) != VK_SUCCESS) {
        return false;
    }

    vkGetDeviceQueue(m_device, m_families.graphics, 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_families.present, 0, &m_presentQueue);
    return m_graphicsQueue != VK_NULL_HANDLE && m_presentQueue != VK_NULL_HANDLE;
}

bool VulkanContext::supportsRequiredDeviceExtensions(VkPhysicalDevice dev) const {
    uint32_t count = 0;
    if (vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, nullptr) != VK_SUCCESS) return false;

    std::vector<VkExtensionProperties> available(count);
    if (vkEnumerateDeviceExtensionProperties(dev, nullptr, &count, available.data()) != VK_SUCCESS) {
        return false;
    }

    for (const auto& ext : available) {
        if (std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) return true;
    }
    return false;
}

QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice dev,
                                                     VkSurfaceKHR surface) const {
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

    if (surface != VK_NULL_HANDLE) {
        for (uint32_t i = 0; i < count; ++i) {
            VkBool32 presentSupport = VK_FALSE;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &presentSupport) == VK_SUCCESS &&
                presentSupport == VK_TRUE) {
                indices.present = i;
                if (indices.graphics == i) break;
            }
        }
    }

    if (surface == VK_NULL_HANDLE && indices.hasGraphics()) {
        indices.present = indices.graphics;
    }

    return indices;
}

bool VulkanContext::isDeviceSuitable(VkPhysicalDevice dev) const {
    if (!findQueueFamilies(dev).hasGraphics()) return false;

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(dev, &props);
    if (VK_API_VERSION_MINOR(props.apiVersion) < 3) return false;

    return supportsRequiredDeviceExtensions(dev);
}

bool VulkanContext::isDeviceSuitableForSurface(VkPhysicalDevice dev,
                                                VkSurfaceKHR surface,
                                                QueueFamilyIndices& indices) const {
    indices = findQueueFamilies(dev, surface);
    if (!indices.isCompleteForPresentation()) return false;
    if (!supportsRequiredDeviceExtensions(dev)) return false;

    VkPhysicalDeviceProperties props{};
    vkGetPhysicalDeviceProperties(dev, &props);
    return VK_API_VERSION_MINOR(props.apiVersion) >= 3;
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