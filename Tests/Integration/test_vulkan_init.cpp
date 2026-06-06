// =============================================================================
//  Tests/Integration/test_vulkan_init.cpp
//
//  Fase 2 — Testes de inicializacao headless do Vulkan.
//  Nenhuma janela, nenhuma swapchain: apenas a GPU a responder a pedidos base.
//
//  Estes testes validam:
//    1. O Vulkan SDK esta corretamente ligado ao sistema de build.
//    2. O driver da GPU suporta Vulkan >= 1.3.
//    3. Pelo menos uma GPU compativel com Vulkan esta presente no sistema.
//    4. VkInstance pode ser criada e destruida sem memory leaks.
// =============================================================================

#include "doctest/doctest.h"
#include <vulkan/vulkan.h>
#include <vector>

// -----------------------------------------------------------------------------
//  Helper: cria uma VkInstance minima (sem layers, sem extensions)
//  Usada por multiplos testes para evitar repeticao.
// -----------------------------------------------------------------------------
static VkInstance createTestInstance() {
    VkApplicationInfo app{};
    app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName   = "ASCENDENDO Tests";
    app.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app.pEngineName        = "ASCENDENDO Engine";
    app.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
    app.apiVersion         = VK_API_VERSION_1_3;

    VkInstanceCreateInfo ci{};
    ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo        = &app;
    ci.enabledExtensionCount   = 0;
    ci.enabledLayerCount       = 0;

    VkInstance instance = VK_NULL_HANDLE;
    vkCreateInstance(&ci, nullptr, &instance);
    return instance;
}

// =============================================================================
TEST_SUITE("Vulkan / Inicializacao") {

    // ── Teste 1 ───────────────────────────────────────────────────────────────
    TEST_CASE("VkInstance: criacao e destruicao sem erros") {
        VkApplicationInfo app{};
        app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName   = "ASCENDENDO Tests";
        app.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        app.pEngineName        = "ASCENDENDO Engine";
        app.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
        app.apiVersion         = VK_API_VERSION_1_3;

        VkInstanceCreateInfo ci{};
        ci.sType              = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pApplicationInfo   = &app;

        VkInstance instance   = VK_NULL_HANDLE;
        VkResult   result     = vkCreateInstance(&ci, nullptr, &instance);

        // VK_SUCCESS == 0; qualquer valor negativo e um erro fatal
        REQUIRE(result == VK_SUCCESS);
        REQUIRE(instance != VK_NULL_HANDLE);

        // Destruicao limpa — obrigatoria para evitar memory leaks
        vkDestroyInstance(instance, nullptr);
    }

    // ── Teste 2 ───────────────────────────────────────────────────────────────
    TEST_CASE("Vulkan: driver do sistema suporta versao >= 1.3") {
        uint32_t apiVersion = 0;
        VkResult result     = vkEnumerateInstanceVersion(&apiVersion);

        REQUIRE(result == VK_SUCCESS);

        uint32_t major = VK_VERSION_MAJOR(apiVersion);
        uint32_t minor = VK_VERSION_MINOR(apiVersion);
        uint32_t patch = VK_VERSION_PATCH(apiVersion);

        // O motor vai usar features de Vulkan 1.3 (dynamic rendering, etc.)
        CHECK(major == 1);
        CHECK(minor >= 3);

        MESSAGE("Versao Vulkan do driver: ", major, ".", minor, ".", patch);
    }

    // ── Teste 3 ───────────────────────────────────────────────────────────────
    TEST_CASE("GPU: pelo menos uma placa grafica Vulkan detectada") {
        VkInstance instance = createTestInstance();
        REQUIRE(instance != VK_NULL_HANDLE);

        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);

        // Sem GPU Vulkan, o motor nao pode correr
        REQUIRE(count >= 1);
        MESSAGE("GPUs Vulkan encontradas: ", count);

        // Listar todas as GPUs encontradas (util para debug em CI)
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());

        for (const auto& dev : devices) {
            VkPhysicalDeviceProperties props{};
            vkGetPhysicalDeviceProperties(dev, &props);

            const char* type = "Desconhecida";
            switch (props.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   type = "Discreta";   break;
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: type = "Integrada";  break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    type = "Virtual";    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:            type = "CPU (soft)"; break;
                default: break;
            }

            MESSAGE("  -> ", props.deviceName,
                    " [", type, "] | API ",
                    VK_VERSION_MAJOR(props.apiVersion), ".",
                    VK_VERSION_MINOR(props.apiVersion));
        }

        vkDestroyInstance(instance, nullptr);
    }

}
// =============================================================================
