// =============================================================================
// Tests/Unit/test_renderer_facade_editor_snapshot.cpp
// =============================================================================
#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/RendererFacade.h"

#include <type_traits>

TEST_SUITE("RendererFacade presentation boundary") {
    TEST_CASE("facade exposes editor snapshot data rather than EditorSession") {
        static_assert(std::is_default_constructible_v<gfx::RendererFacade>);
        static_assert(!std::is_copy_constructible_v<gfx::RendererFacade>);

        using AttachSignature = void (gfx::RendererFacade::*)(
            const logic::EditorRenderSnapshot*);
        static_assert(std::is_same_v<
            decltype(static_cast<AttachSignature>(
                &gfx::RendererFacade::attachEditorSnapshot)),
            AttachSignature>);

        gfx::RendererFacade facade;
        logic::EditorRenderSnapshot snapshot;
        facade.attachEditorSnapshot(&snapshot);
        CHECK(!facade.isInitialized());
    }
}
