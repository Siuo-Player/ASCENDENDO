#pragma once
// =============================================================================
// Game/Graphics/RenderSnapshot.h
//
// Dados imutáveis necessários para apresentação de um frame.
// Não contém Player, Level, GameState ou outros tipos de domínio.
// =============================================================================

#include "Graphics/PlatformCompositor.h"

#include <cstdint>
#include <string>
#include <vector>

namespace gfx {

struct RenderRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct RenderPlayerSnapshot {
    RenderRect bounds{};
    float facingDirection = 1.0f;
    float chargeRatio = 0.0f; // [0.0f, 1.0f] jump charge power ratio
};

struct RenderGhostSnapshot {
    bool visible = false;
    RenderRect bounds{};
    float facingDirection = 1.0f;
    float alpha = 0.55f;
};

struct RenderFlagSnapshot {
    bool visible = false;
    RenderRect bounds{};
};

struct RenderHudSnapshot {
    float elapsedTime = 0.0f;
    float personalBestTime = 0.0f; // 0.0f se ainda nao completado
    float altitudeMeters = 0.0f;
    bool ghostAvailable = false;
    bool ghostVisible = false;
    float ghostDeltaMeters = 0.0f; // positivo = jogador a frente; negativo = atras
    bool isReplayWatch = false;
    float replayTotalTime = 0.0f;
    char campaignName[64]{};
};

struct MenuCampaignItem {
    std::string id;
    std::string name;
    std::size_t levelCount = 0;
    float bestTimeSeconds = 0.0f;
    uint32_t completions = 0;
    bool hasReplay = false;
};

struct MenuRenderSnapshot {
    int selection = 0;
    std::size_t activeCampaignIndex = 0;
    bool ghostEnabled = true;
    bool activeHasReplay = false;
    std::string accountName;
    std::vector<MenuCampaignItem> campaigns;
};

struct RenderSnapshot {
    RenderPlayerSnapshot player{};
    RenderGhostSnapshot ghost{};
    std::vector<RenderRect> platforms;

    // Presentation-only semantic cells generated from modular platform regions.
    // The legacy platform rectangles remain available so a non-modular level is
    // never silently rewritten merely to satisfy the compositor boundary.
    bool semanticPlatformsValid = false;
    std::vector<compositor::RegionCell> semanticPlatformCells;

    RenderFlagSnapshot flag{};
    RenderHudSnapshot hud{};
};

} // namespace gfx
