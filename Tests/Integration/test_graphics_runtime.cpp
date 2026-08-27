#include "doctest/doctest.h"
#include "Graphics/GraphicsRuntime.h"

#ifdef GLFW_AVAILABLE
#include <GLFW/glfw3.h>

#include <filesystem>
#include <utility>

namespace {

class CurrentPathGuard {
public:
    explicit CurrentPathGuard(std::filesystem::path original)
        : original_(std::move(original)) {}

    ~CurrentPathGuard() {
        std::error_code ec;
        std::filesystem::current_path(original_, ec);
    }

    CurrentPathGuard(const CurrentPathGuard&) = delete;
    CurrentPathGuard& operator=(const CurrentPathGuard&) = delete;

private:
    std::filesystem::path original_;
};

} // namespace

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

    TEST_CASE("failed initialization rolls back and permits retry") {
        const auto originalPath = std::filesystem::current_path();
        CurrentPathGuard restorePath(originalPath);

        const auto tempRoot = std::filesystem::temp_directory_path() /
                              "ascendendo-graphics-runtime-missing-shaders";
        std::error_code ec;
        std::filesystem::remove_all(tempRoot, ec);
        std::filesystem::create_directories(tempRoot, ec);
        REQUIRE_FALSE(ec);

        std::filesystem::current_path(tempRoot, ec);
        REQUIRE_FALSE(ec);

        {
            gfx::GraphicsRuntime runtime;
            CHECK_FALSE(runtime.init(640, 360, "ASCENDENDO rollback test"));
            CHECK_FALSE(runtime.isInitialized());
            CHECK_FALSE(runtime.window().isCreated());
            CHECK_FALSE(runtime.context().isInitialized());
            CHECK_FALSE(runtime.swapchain().isInitialized());
            CHECK_FALSE(runtime.renderPass().isInitialized());
            CHECK_FALSE(runtime.pipeline().isInitialized());
            CHECK_FALSE(runtime.renderer().isInitialized());

            // Restoring the real project working directory supplies the normal
            // shader files. A second init proves the failed attempt left a
            // neutral state rather than partially-owned graphics resources.
            std::filesystem::current_path(originalPath, ec);
            REQUIRE_FALSE(ec);
            CHECK(runtime.init(640, 360, "ASCENDENDO retry test"));
            CHECK(runtime.isInitialized());
        }

        std::filesystem::remove_all(tempRoot, ec);
    }
}
#endif
