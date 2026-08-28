// =============================================================================
// Tests/Unit/test_vulkan_image_upload.cpp
// =============================================================================
#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanImageUpload.h"

#include <cstdint>
#include <type_traits>
#include <vector>

TEST_SUITE("VulkanImageUpload") {
    TEST_CASE("resource is a plain handle aggregate") {
        static_assert(std::is_default_constructible_v<gfx::VulkanImageResource>);
        static_assert(std::is_copy_constructible_v<gfx::VulkanImageResource>);

        const gfx::VulkanImageResource resource{};
        CHECK(resource.image == VK_NULL_HANDLE);
        CHECK(resource.memory == VK_NULL_HANDLE);
        CHECK(resource.view == VK_NULL_HANDLE);
        CHECK(resource.sampler == VK_NULL_HANDLE);
    }

    TEST_CASE("rejects invalid upload preconditions") {
        gfx::VulkanImageResource resource{
            reinterpret_cast<VkImage>(static_cast<uintptr_t>(1)),
            reinterpret_cast<VkDeviceMemory>(static_cast<uintptr_t>(2)),
            reinterpret_cast<VkImageView>(static_cast<uintptr_t>(3)),
            reinterpret_cast<VkSampler>(static_cast<uintptr_t>(4))};

        const std::vector<uint8_t> pixels = {255};

        CHECK_FALSE(gfx::uploadVulkanImage2D(
            nullptr, pixels, 1, 1, VK_FORMAT_R8_UNORM, VK_FILTER_LINEAR, resource));
        CHECK(resource.image == VK_NULL_HANDLE);
        CHECK(resource.memory == VK_NULL_HANDLE);
        CHECK(resource.view == VK_NULL_HANDLE);
        CHECK(resource.sampler == VK_NULL_HANDLE);

        CHECK_FALSE(gfx::uploadVulkanImage2D(
            nullptr, pixels, 0, 1, VK_FORMAT_R8_UNORM, VK_FILTER_LINEAR, resource));
        CHECK_FALSE(gfx::uploadVulkanImage2D(
            nullptr, pixels, 1, 0, VK_FORMAT_R8_UNORM, VK_FILTER_LINEAR, resource));
        CHECK_FALSE(gfx::uploadVulkanImage2D(
            nullptr, {}, 1, 1, VK_FORMAT_R8_UNORM, VK_FILTER_LINEAR, resource));
    }
}
