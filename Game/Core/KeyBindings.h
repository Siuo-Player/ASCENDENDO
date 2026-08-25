#pragma once
// =============================================================================
//  Game/Core/KeyBindings.h
// =============================================================================
#include "Core/GameAction.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace logic { class InputManager; }

namespace core {

class KeyBindings {
public:
    KeyBindings();
    ~KeyBindings();

    const std::vector<int>& keysFor(GameAction action) const;
    void rebind(GameAction action, int newKey);
    void resetToDefault(GameAction action);
    void resetAllToDefaults();

    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

    // The currently active binding set is selected by the live settings object.
    // This is a transitional bridge until input state is passed explicitly to
    // gameplay; it keeps gameplay, menus and editor on the same configured map.
    static const KeyBindings* active();

private:
    std::unordered_map<GameAction, std::vector<int>> m_bindings;
    static const KeyBindings* s_active;

    static std::unordered_map<GameAction, std::vector<int>> defaultBindings();
};

bool isActionHeld(const KeyBindings& bindings, const logic::InputManager& input, GameAction action);
bool isActionJustPressed(const KeyBindings& bindings, const logic::InputManager& input, GameAction action);
bool isActionJustReleased(const KeyBindings& bindings, const logic::InputManager& input, GameAction action);

std::string keyToString(int key);
bool stringToKey(const std::string& s, int& outKey);

} // namespace core
