// =============================================================================
//  Tests/Integration/test_vulkan_init.cpp
//
//  Fase 2 — Testes de inicializacao headless do Vulkan.
// =============================================================================

#include "doctest/doctest.h"
#include <vulkan/vulkan.h>
#include <cstring>
#include <vector>

static VkInstance createTestInstance() {
    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "ASCENDENDO Tests";
    app.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app.pEngineName = "ASCENDENDO Engine";
    app.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app;

    VkInstance instance = VK_NULL_HANDLE;
    if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return instance;
}

TEST_SUITE("Vulkan / Inicializacao") {

    TEST_CASE("VkInstance: criacao e destruicao sem erros") {
        VkApplicationInfo app{};
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName = "ASCENDENDO Tests";
        app.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        app.pEngineName = "ASCENDENDO Engine";
        app.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        app.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pApplicationInfo = &app;

        VkInstance instance = VK_NULL_HANDLE;
        const VkResult result = vkCreateInstance(&ci, nullptr, &instance);

        REQUIRE(result == VK_SUCCESS);
        REQUIRE(instance != VK_NULL_HANDLE);
        vkDestroyInstance(instance, nullptr);
    }

    TEST_CASE("Vulkan: driver do sistema suporta versao >= 1.3") {
        uint32_t apiVersion = 0;
        const VkResult result = vkEnumerateInstanceVersion(&apiVersion);
        REQUIRE(result == VK_SUCCESS);

        const uint32_t major = VK_VERSION_MAJOR(apiVersion);
        const uint32_t minor = VK_VERSION_MINOR(apiVersion);
        const uint32_t patch = VK_VERSION_PATCH(apiVersion);

        CHECK(major == 1);
        CHECK(minor >= 3);
        MESSAGE("Versao Vulkan do driver: ", major, ".", minor, ".", patch);
    }

    TEST_CASE("GPU: pelo menos uma placa grafica Vulkan detectada") {
        VkInstance instance = createTestInstance();
        REQUIRE(instance != VK_NULL_HANDLE);

        uint32_t count = 0;
        REQUIRE(vkEnumeratePhysicalDevices(instance, &count, nullptr) == VK_SUCCESS);
        REQUIRE(count >= 1);

        std::vector<VkPhysicalDevice> devices(count);
        REQUIRE(vkEnumeratePhysicalDevices(instance, &count, devices.data()) == VK_SUCCESS);

        for (const auto& dev : devices) {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(dev, &props);
            MESSAGE("  -> ", props.deviceName,
                    " | API ", VK_VERSION_MAJOR(props.apiVersion), ".",
                    VK_VERSION_MINOR(props.apiVersion));
        }

        vkDestroyInstance(instance, nullptr);
    }

    TEST_CASE("GPU: capability matrix minima para o runtime") {
        VkInstance instance = createTestInstance();
        REQUIRE(instance != VK_NULL_HANDLE);

        uint32_t count = 0;
        REQUIRE(vkEnumeratePhysicalDevices(instance, &count, nullptr) == VK_SUCCESS);
        REQUIRE(count >= 1);
        std::vector<VkPhysicalDevice> devices(count);
        REQUIRE(vkEnumeratePhysicalDevices(instance, &count, devices.data()) == VK_SUCCESS);

        bool runtimeCandidateFound = false;
        for (VkPhysicalDevice device : devices) {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(device, &props);

            const bool apiSupported =
                VK_VERSION_MAJOR(props.apiVersion) == 1 &&
                VK_VERSION_MINOR(props.apiVersion) >= 3;
            if (!apiSupported) {
                MESSAGE("Skipping Vulkan device below API 1.3: ", props.deviceName);
                continue;
            }

            uint32_t extensionCount = 0;
            REQUIRE(vkEnumerateDeviceExtensionProperties(device, nullptr,
                                                          &extensionCount, nullptr) == VK_SUCCESS);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            REQUIRE(vkEnumerateDeviceExtensionProperties(device, nullptr,
                                                          &extensionCount, extensions.data()) == VK_SUCCESS);

            bool hasSwapchainExtension = false;
            for (const auto& extension : extensions) {
                if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                    hasSwapchainExtension = true;
                    break;
                }
            }

            uint32_t queueCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
            REQUIRE(queueCount > 0);
            std::vector<VkQueueFamilyProperties> queues(queueCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());

            bool hasGraphicsQueue = false;
            for (const auto& queue : queues) {
                if (queue.queueCount > 0 && (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                    hasGraphicsQueue = true;
                    break;
                }
            }

            const bool runtimeCandidate = hasSwapchainExtension && hasGraphicsQueue;
            if (!runtimeCandidate) {
                MESSAGE("Skipping Vulkan device unsuitable for runtime: ", props.deviceName);
                continue;
            }

            runtimeCandidateFound = true;
            break;
        }

        CHECK(runtimeCandidateFound);
        vkDestroyInstance(instance, nullptr);
    }
}
// =============================================================================
