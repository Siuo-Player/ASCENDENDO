#include "doctest/doctest.h"
#include "Graphics/GraphicsRuntime.h"

#ifdef GLFW_AVAILABLE
#include <GLFW/glfw3.h>

TEST_SUITE("Graphics Runtime") {
    TEST_CASE("owns core graphics resources in dependency order") {
        {
            gfx::GraphicsRuntime runtime;
            CHECK(runtime.init(640, 360, "ASCENDENDO test"));
            CHECK(runtime.isInitialized());
            CHECK(runtime.context().isInitialized());
            CHECK(runtime.swapchain().isInitialized());
            CHECK(runtime.renderPass().isInitialized());
            CHECK(runtime.pipeline().isInitialized());
            CHECK(runtime.renderer().isInitialized());
        }
    }
}
#endif
