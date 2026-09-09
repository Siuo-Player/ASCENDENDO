#pragma once

#include "Core/Config.h"

#include <cstddef>

namespace core {

// Fixed-width vertical level contract for 1.0.
// A level is an ordered stack of N logical screens. Width never changes.
class LevelLayout {
public:
    static constexpr float SCREEN_WIDTH = config::LOGICAL_WIDTH;
    static constexpr float SCREEN_HEIGHT = config::LOGICAL_HEIGHT;

    explicit constexpr LevelLayout(std::size_t screenCount = 1)
        : m_screenCount(screenCount == 0 ? 1 : screenCount) {}

    constexpr std::size_t screenCount() const { return m_screenCount; }
    constexpr float width() const { return SCREEN_WIDTH; }
    constexpr float height() const {
        return SCREEN_HEIGHT * static_cast<float>(m_screenCount);
    }

    constexpr bool containsX(float x) const {
        return x >= 0.0f && x <= SCREEN_WIDTH;
    }

    constexpr bool validScreen(std::size_t screenIndex) const {
        return screenIndex < m_screenCount;
    }

    constexpr float screenBottomY(std::size_t screenIndex) const {
        return SCREEN_HEIGHT * static_cast<float>(screenIndex);
    }

    constexpr float screenTopY(std::size_t screenIndex) const {
        return screenBottomY(screenIndex) + SCREEN_HEIGHT;
    }

    constexpr std::size_t screenIndexForY(float worldY) const {
        if (worldY < 0.0f) return 0;
        const std::size_t index = static_cast<std::size_t>(worldY / SCREEN_HEIGHT);
        return index >= m_screenCount ? m_screenCount - 1 : index;
    }

private:
    std::size_t m_screenCount;
};

} // namespace core
