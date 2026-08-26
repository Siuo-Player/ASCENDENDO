#include "doctest/doctest.h"
#include "Graphics/VulkanContext.h"

TEST_SUITE("VulkanContext") {
    TEST_CASE("Queue families remain independently representable") {
        gfx::QueueFamilyIndices indices{};
        indices.graphics = 2;
        indices.present = 5;

        CHECK(indices.hasGraphics());
        CHECK(indices.hasPresent());
        CHECK(indices.isComplete());
        CHECK(indices.isCompleteForPresentation());
        CHECK(indices.graphics != indices.present);
    }

    TEST_CASE("Queue families can share one family when supported") {
        gfx::QueueFamilyIndices indices{};
        indices.graphics = 3;
        indices.present = 3;

        CHECK(indices.isCompleteForPresentation());
    }
}
