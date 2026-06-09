// =============================================================================
//  Game/Logic/ReplayManager.cpp
//
//  @version 3.3
//  @history
//    v3.3 — criado
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

void ReplayManager::recordFrame(const Player& player, const PhysicsWorld& world, const InputManager& input) {
    m_stateHistory.push_back(GameState{ player, world.accumulator() });

    FrameInput fi;
    fi.left              = input.isLeft();
    fi.right             = input.isRight();
    fi.jumpHeld          = input.isKeyDown(Key::SPACE);
    fi.jumpPressed       = input.isKeyJustPressed(Key::SPACE);
    fi.jumpReleased      = input.isKeyJustReleased(Key::SPACE);
    m_inputHistory.push_back(fi);
}

bool ReplayManager::rewind(Player& player, PhysicsWorld& world) {
    if (m_stateHistory.empty() || m_inputHistory.empty()) return false;

    // 1. LER o estado do topo (o frame exato para onde queremos voltar)
    const GameState& prevState = m_stateHistory.back();
    player = prevState.player;
    world.setAccumulator(prevState.worldAccumulator);

    // 2. APAGAR o estado do topo, para que o próximo rewind volte ainda mais atrás
    m_stateHistory.pop_back();
    m_inputHistory.pop_back();

    return true;
}

void ReplayManager::startPlayback() {
    m_playbackIndex = 0;
}

void ReplayManager::preparePlaybackFrame(InputManager& input) {
    if (isPlaybackComplete()) return;

    const FrameInput& fi = m_inputHistory[m_playbackIndex];
    input.injectRawState(fi.left, fi.right, fi.jumpHeld, fi.jumpPressed, fi.jumpReleased);
    
    m_playbackIndex++;
}

void ReplayManager::clearReplay() {
    m_stateHistory.clear();
    m_inputHistory.clear();
    m_playbackIndex = 0;
}

} // namespace logic