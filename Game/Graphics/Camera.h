#pragma once
#include "Logic/Physics.h"

namespace gfx {

class Camera {
public:
    logic::Vec2 position = {0.0f, 0.0f};
    logic::Vec2 worldToNDC(const logic::Vec2& worldPos) const;
};

} // namespace gfx