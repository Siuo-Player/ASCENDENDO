#pragma once
// =============================================================================
//  Game/Logic/LevelEditor.h
//
//  Fase 9.4 tranche 1: documento editável em memória.
//  Não conhece Vulkan nem UI.
// =============================================================================

#include "Logic/Physics.h"
#include <cstddef>
#include <optional>
#include <vector>

namespace logic {

enum class EditorToolMode {
    STAMP,
    DRAG,
};

enum class EditorSizePreset {
    SMALL,
    MEDIUM,
    LARGE,
};

struct EditorPlatform {
    AABB bounds;
};

class LevelEditorDocument {
public:
    // `finalCampaignLevel` controla a única regra que depende da campanha:
    // uma FLAG de fim só pode ser criada no último nível.
    LevelEditorDocument(bool finalCampaignLevel, const AABB& initialGround);

    bool isFinalCampaignLevel() const { return m_finalCampaignLevel; }

    // ── Grid ─────────────────────────────────────────────────────────────────
    static float snap(float value);
    static Vec2 snap(const Vec2& point);
    static AABB snap(const AABB& rect);

    // ── Plataformas ──────────────────────────────────────────────────────────
    bool addPlatform(const AABB& requested, std::size_t* createdIndex = nullptr);
    bool movePlatform(std::size_t index, const Vec2& requestedMin);
    bool removePlatform(std::size_t index);

    const std::vector<EditorPlatform>& platforms() const { return m_platforms; }
    std::size_t platformCount() const { return m_platforms.size(); }

    // ── Spawn ────────────────────────────────────────────────────────────────
    // Apenas X é editável; Y é sempre o topo do chão inicial.
    bool setSpawnX(float requestedX);
    Vec2 spawnPosition() const { return m_spawnPosition; }

    float spawnMinX() const { return m_spawnMinX; }
    float spawnMaxX() const { return m_spawnMaxX; }

    // ── FLAG ─────────────────────────────────────────────────────────────────
    bool setFlag(const AABB& requested);
    void removeFlag() { m_flag.reset(); }
    bool hasFlag() const { return m_flag.has_value(); }
    const AABB* flag() const { return m_flag.has_value() ? &*m_flag : nullptr; }

    // ── Presets ──────────────────────────────────────────────────────────────
    static Vec2 presetSize(EditorSizePreset preset);

private:
    bool insideLogicalBounds(const AABB& rect) const;
    bool validPlatform(const AABB& rect) const;
    bool validFlag(const AABB& rect) const;

    bool m_finalCampaignLevel = false;
    AABB m_initialGround{};
    std::vector<EditorPlatform> m_platforms;
    Vec2 m_spawnPosition{};
    float m_spawnMinX = 0.0f;
    float m_spawnMaxX = 0.0f;
    std::optional<AABB> m_flag;
};

} // namespace logic
