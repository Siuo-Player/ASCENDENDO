// =============================================================================
//  Game/Logic/ReplayManager.h
//
//  @version 3.3
//  @history
//    v3.3 — criado (Save States, Frame Recording, Continuous Rewind, Playback)
// =============================================================================

#pragma once
#include "Logic/Player.h"
#include "Logic/Physics.h"
#include "Logic/InputManager.h"
#include <vector>
#include <unordered_map>

namespace logic {

struct GameState {
    Player player;
    float  worldAccumulator = 0.0f;
};

struct FrameInput {
    bool left              = false;
    bool right             = false;
    bool jumpHeld          = false;
    bool jumpPressed       = false;
    bool jumpReleased      = false;
};

class ReplayManager {
public:
    ReplayManager()  = default;
    ~ReplayManager() = default;

    // ── Save States (Slots Estáticos) ────────────────────────────────────────
    void saveState(size_t slot, const Player& player, const PhysicsWorld& world);
    bool loadState(size_t slot, Player& player, PhysicsWorld& world) const;

    // ── Gravação Contínua (Frame-by-Frame) ───────────────────────────────────
    void recordFrame(const Player& player, const PhysicsWorld& world, const InputManager& input);

    // ── Rewind Contínuo ──────────────────────────────────────────────────────
    bool rewind(Player& player, PhysicsWorld& world);

    // ── Playback / Reprodução ────────────────────────────────────────────────
    void startPlayback();
    void preparePlaybackFrame(InputManager& input);

    // ── Utilitários ──────────────────────────────────────────────────────────
    void clearReplay();
    size_t getReplayLength() const { return m_inputHistory.size(); }
    size_t getCurrentPlaybackFrame() const { return m_playbackIndex; }
    bool isPlaybackComplete() const { return m_playbackIndex >= m_inputHistory.size(); }

private:
    std::unordered_map<size_t, GameState> m_slots;
    std::vector<GameState>                m_stateHistory;
    std::vector<FrameInput>               m_inputHistory;
    size_t                                m_playbackIndex = 0;
};

} // namespace logic