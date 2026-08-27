// =============================================================================
//  Game/Logic/ReplayManager.cpp
//
//  @version 4.0
//  @history
//    v3.3 — criado
//    v4.0 — gravação/reprodução passa a usar TickInput por tick
// =============================================================================

#include "Logic/ReplayManager.h"

namespace logic {

void ReplayManager::saveState(size_t slot, const Player& player, const PhysicsWorld& world) {
    m_slots[slot] = GameState{ player, world.accumulator() };
}

bool ReplayManager::loadState(size_t slot, Player& player, PhysicsWorld& world) const {
    auto it = m_slots.find(slot);
    if (it == m_slots.end()) return false;

    player = it->second.player;
    world.setAccumulator(it->second.worldAccumulator);
    return true;
}

void ReplayManager::recordTick(const Player& player, const PhysicsWorld& world, const TickInput& input) {
    // Store the pre-tick state so rewind returns to the exact state preceding
    // the corresponding semantic command.
    m_stateHistory.push_back(GameState{ player, world.accumulator() });
    m_inputHistory.push_back(input);
}

bool ReplayManager::rewind(Player& player, PhysicsWorld& world) {
    if (m_stateHistory.empty() || m_inputHistory.empty()) return false;

    const GameState& prevState = m_stateHistory.back();
    player = prevState.player;
    world.setAccumulator(prevState.worldAccumulator);

    m_stateHistory.pop_back();
    m_inputHistory.pop_back();

    return true;
}

void ReplayManager::startPlayback() {
    m_playbackIndex = 0;
}

bool ReplayManager::preparePlaybackTick(TickInput& input) {
    if (isPlaybackComplete()) return false;

    input = m_inputHistory[m_playbackIndex];
    ++m_playbackIndex;
    return true;
}

void ReplayManager::clearReplay() {
    m_stateHistory.clear();
    m_inputHistory.clear();
    m_playbackIndex = 0;
}

} // namespace logic
