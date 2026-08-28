// =============================================================================
// Tests/Unit/test_presentation_config.cpp
// =============================================================================

#include "doctest/doctest.h"
#include "Graphics/PresentationConfig.h"

static_assert(presentation::COLOR_PLATFORM_R == 0.35f);
static_assert(presentation::COLOR_PLAYER_R == 0.90f);
static_assert(presentation::CLEAR_MENU_B == 0.06f);
static_assert(presentation::EDITOR_GRID_VISUAL_SPACING == 16.0f);

TEST_SUITE("PresentationConfig") {
    TEST_CASE("exposes presentation-only constants") {
        CHECK(presentation::EDITOR_GRID_MAJOR_SPACING == doctest::Approx(64.0f));
        CHECK(presentation::CLEAR_EDITOR_R == doctest::Approx(0.10f));
    }
}
