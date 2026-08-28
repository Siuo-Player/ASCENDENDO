// =============================================================================
//  Game/Logic/ReplayManager.h
//
//  @version 4.0
//  @history
//    v3.3 — criado (Save States, Frame Recording, Continuous Rewind, Playback)
//    v4.0 — replay passa a armazenar TickInput explicitamente
// =============================================================================

#pragma once
#include "Logic/Player.h"
#include "Logic/Physics.h"
#include "Logic/TickInput.h"
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace logic {

struct GameState {
    Player player;
    float  worldAccumulator = 0.0f;
};

class ReplayManager {
public:
    ReplayManager()  = default;
    ~ReplayManager() = default;

    // ── Save States (Slots Estáticos) ────────────────────────────────────────
    void saveState(size_t slot, const Player& player, const PhysicsWorld& world);
    bool loadState(size_t slot, Player& player, PhysicsWorld& world) const;

    // ── Gravação por tick ────────────────────────────────────────────────────
    void recordTick(const Player& player, const PhysicsWorld& world, const TickInput& input);

    // ── Rewind ───────────────────────────────────────────────────────────────
    bool rewind(Player& player, PhysicsWorld& world);

    // ── Playback / Reprodução ────────────────────────────────────────────────
    void startPlayback();
    bool preparePlaybackTick(TickInput& input);

    // ── Utilitários ──────────────────────────────────────────────────────────
    void clearReplay();
    size_t getReplayLength() const { return m_inputHistory.size(); }
    size_t getCurrentPlaybackTick() const { return m_playbackIndex; }
    bool isPlaybackComplete() const { return m_playbackIndex >= m_inputHistory.size(); }

private:
    std::unordered_map<size_t, GameState> m_slots;
    std::vector<GameState>                m_stateHistory;
    std::vector<TickInput>                m_inputHistory;
    size_t                                m_playbackIndex = 0;
};

} // namespace logic
